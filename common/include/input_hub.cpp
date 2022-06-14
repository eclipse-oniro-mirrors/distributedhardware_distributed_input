/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "input_hub.h"

#include <cinttypes>
#include <cstring>
#include <filesystem>
#include <sstream>

#include <dirent.h>
#include <fcntl.h>
#include <openssl/sha.h>
#include <securec.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "anonymous_string.h"
#include "distributed_hardware_log.h"

#include "dinput_errcode.h"

namespace OHOS {
namespace DistributedHardware {
namespace DistributedInput {
namespace {
const char *DEVICE_PATH = "/dev/input";
const uint32_t SLEEP_TIME_MS = 100000;
}

InputHub::InputHub() : needToScanDevices_(true),
    nextDeviceId_(1), pendingEventCount_(0),
    pendingEventIndex_(0), pendingINotify_(false),
    deviceChanged_(false)
{
    Initialize();
}

InputHub::~InputHub()
{
    Release();
}

int32_t InputHub::Initialize()
{
    epollFd_ = epoll_create1(EPOLL_CLOEXEC);
    if (epollFd_ < 0) {
        DHLOGE("Could not create epoll instance: %s", strerror(errno));
        return ERR_DH_INPUT_HUB_EPOLL_INIT_FAIL;
    }

    iNotifyFd_ = inotify_init();
    inputWd_ = inotify_add_watch(iNotifyFd_, DEVICE_PATH, IN_DELETE | IN_CREATE);
    if (inputWd_ < 0) {
        DHLOGE(
            "Could not register INotify for %s: %s", DEVICE_PATH, strerror(errno));
        return ERR_DH_INPUT_HUB_EPOLL_INIT_FAIL;
    }

    struct epoll_event eventItem = {};
    eventItem.events = EPOLLIN;
    eventItem.data.fd = iNotifyFd_;
    int result = epoll_ctl(epollFd_, EPOLL_CTL_ADD, iNotifyFd_, &eventItem);
    if (result != 0) {
        DHLOGE("Could not add INotify to epoll instance.  errno=%d", errno);
        return ERR_DH_INPUT_HUB_EPOLL_INIT_FAIL;
    }

    return DH_SUCCESS;
}

int32_t InputHub::Release()
{
    CloseAllDevicesLocked();

    ::close(epollFd_);
    ::close(iNotifyFd_);
    return DH_SUCCESS;
}

size_t InputHub::CollectInputEvents(RawEvent* buffer, size_t bufferSize)
{
    size_t count;
    for (;;) {
        if (needToScanDevices_) {
            needToScanDevices_ = false;
            ScanInputDevices(DEVICE_PATH);
        }
        while (!openingDevices_.empty()) {
            std::unique_lock<std::mutex> deviceLock(visitMutex_);
            std::unique_ptr<Device> device = std::move(*openingDevices_.rbegin());
            openingDevices_.pop_back();
            DHLOGI("Reporting device opened: id=%s, name=%s\n",
                GetAnonyInt32(device->id).c_str(), device->path.c_str());
            auto [dev_it, inserted] = devices_.insert_or_assign(device->id, std::move(device));
            if (!inserted) {
                DHLOGI("Device id %s exists, replaced. \n", GetAnonyInt32(device->id).c_str());
            }
        }
        deviceChanged_ = false;
        count = GetEvents(buffer, bufferSize);
        // readNotify() will modify the list of devices so this must be done after
        // processing all other events to ensure that we read all remaining events
        // before closing the devices.
        if (pendingINotify_ && pendingEventIndex_ >= pendingEventCount_) {
            pendingINotify_ = false;
            ReadNotifyLocked();
            deviceChanged_ = true;
        }

        // Report added or removed devices immediately.
        if (deviceChanged_) {
            continue;
        }

        if (count > 0) {
            break;
        }

        if (RefreshEpollItem() < 0) {
            break;
        }
    }

    // All done, return the number of events we read.
    return count;
}

size_t InputHub::GetEvents(RawEvent* buffer, size_t bufferSize)
{
    RawEvent* event = buffer;
    size_t capacity = bufferSize;
    while (pendingEventIndex_ < pendingEventCount_) {
        std::unique_lock<std::mutex> my_lock(operationMutex_);
        const struct epoll_event& eventItem = mPendingEventItems[pendingEventIndex_++];
        if (eventItem.data.fd == iNotifyFd_) {
            if (eventItem.events & EPOLLIN) {
                pendingINotify_ = true;
            } else {
                DHLOGI("Received no epoll event 0x%08x.", eventItem.events);
            }
            continue;
        }
        Device* device = GetDeviceByFd(eventItem.data.fd);
        if (!device) {
            continue;
        }
        if (eventItem.events & EPOLLIN) {
            struct input_event readBuffer[bufferSize];
            int32_t readSize = read(device->fd, readBuffer, sizeof(struct input_event) * capacity);
            size_t count = ReadInputEvent(readSize, *device);
            for (size_t i = 0; i < count; i++) {
                struct input_event& iev = readBuffer[i];
                event->when = ProcessEventTimestamp(iev);
                event->type = iev.type;
                event->code = iev.code;
                event->value = iev.value;
                event->path = device->path;
                RecordEventLog(event);
                event->descriptor = device->identifier.descriptor;
                event += 1;
                capacity -= 1;
            }
            if (capacity == 0) {
                pendingEventIndex_ -= 1;
                break;
            }
        } else if (eventItem.events & EPOLLHUP) {
            DHLOGI("Removing device %s due to epoll hang-up event.",
                device->identifier.name.c_str());
            deviceChanged_ = true;
            CloseDeviceLocked(*device);
        }
    }
    return event - buffer;
}

size_t InputHub::ReadInputEvent(int32_t readSize, Device& device)
{
    size_t count = 0;
    if (readSize == 0 || (readSize < 0 && errno == ENODEV)) {
        // Device was removed before INotify noticed.
        DHLOGE("could not get event, removed? (fd: %d size: %d"
            " errno: %d)\n",
            device.fd, readSize, errno);
        deviceChanged_ = true;
        CloseDeviceLocked(device);
    } else if (readSize < 0) {
        if (errno != EAGAIN && errno != EINTR) {
            DHLOGW("could not get event (errno=%d)", errno);
        }
    } else if ((readSize % sizeof(struct input_event)) != 0) {
        DHLOGW("could not get event (wrong size: %d)", readSize);
    } else {
        count = size_t(readSize) / sizeof(struct input_event);
        return count;
    }
    return count;
}

size_t InputHub::DeviceIsExists(InputDeviceEvent* buffer, size_t bufferSize)
{
    InputDeviceEvent* event = buffer;
    size_t capacity = bufferSize;
    // Report any devices that had last been added/removed.
    for (auto it = closingDevices_.begin(); it != closingDevices_.end();) {
        std::unique_ptr<Device> device = std::move(*it);
        DHLOGI("Reporting device closed: id=%s, name=%s\n", GetAnonyInt32(device->id).c_str(), device->path.c_str());
        event->type = DeviceType::DEVICE_REMOVED;
        event->deviceInfo = device->identifier;
        event += 1;
        it = closingDevices_.erase(it);
        if (--capacity == 0) {
            break;
        }
    }
    if (needToScanDevices_) {
        needToScanDevices_ = false;
        ScanInputDevices(DEVICE_PATH);
    }
    while (!openingDevices_.empty()) {
        std::unique_lock<std::mutex> deviceLock(visitMutex_);
        std::unique_ptr<Device> device = std::move(*openingDevices_.rbegin());
        openingDevices_.pop_back();
        DHLOGI("Reporting device opened: id=%s, name=%s\n", GetAnonyInt32(device->id).c_str(), device->path.c_str());
        event->type = DeviceType::DEVICE_ADDED;
        event->deviceInfo = device->identifier;
        event += 1;

        auto [dev_it, inserted] = devices_.insert_or_assign(device->id, std::move(device));
        if (!inserted) {
            DHLOGI("Device id %s exists, replaced. \n", GetAnonyInt32(device->id).c_str());
        }
        if (--capacity == 0) {
            break;
        }
    }
    return event - buffer;
}

size_t InputHub::CollectInputHandler(InputDeviceEvent* buffer, size_t bufferSize)
{
    size_t count;
    for (;;) {
        count = DeviceIsExists(buffer, bufferSize);
        deviceChanged_ = false;
        GetDeviceHandler();

        if (pendingINotify_ && pendingEventIndex_ >= pendingEventCount_) {
            pendingINotify_ = false;
            ReadNotifyLocked();
            deviceChanged_ = true;
        }

        // Report added or removed devices immediately.
        if (deviceChanged_) {
            continue;
        }
        if (count > 0) {
            break;
        }
        if (RefreshEpollItem() < 0) {
            break;
        }
    }

    // All done, return the number of events we read.
    return count;
}

void InputHub::GetDeviceHandler()
{
    while (pendingEventIndex_ < pendingEventCount_) {
        std::unique_lock<std::mutex> my_lock(operationMutex_);
        const struct epoll_event& eventItem = mPendingEventItems[pendingEventIndex_++];
        if (eventItem.data.fd == iNotifyFd_) {
            if (eventItem.events & EPOLLIN) {
                pendingINotify_ = true;
            } else {
                DHLOGI(
                    "Received unexpected epoll event 0x%08x for INotify.", eventItem.events);
            }
            continue;
        }

        Device* device = GetDeviceByFdLocked(eventItem.data.fd);
        if (!device) {
            DHLOGE(
                "Received unexpected epoll event 0x%08x for unknown fd %d.",
                eventItem.events, eventItem.data.fd);
            continue;
        }

        if (eventItem.events & EPOLLHUP) {
            DHLOGI("Removing device %s due to epoll hang-up event.",
                device->identifier.name.c_str());
            deviceChanged_ = true;
            CloseDeviceLocked(*device);
        }
    }
}

int32_t InputHub::RefreshEpollItem()
{
    pendingEventIndex_ = 0;
    int pollResult = epoll_wait(epollFd_, mPendingEventItems, EPOLL_MAX_EVENTS, 0);
    if (pollResult == 0) {
        // Timed out.
        pendingEventCount_ = 0;
        return ERR_DH_INPUT_HUB_EPOLL_WAIT_TIMEOUT;
    }

    if (pollResult < 0) {
        // An error occurred.
        pendingEventCount_ = 0;

        // Sleep after errors to avoid locking up the system.
        // Hopefully the error is transient.
        if (errno != EINTR) {
            DHLOGE("poll failed (errno=%d)\n", errno);
            usleep(SLEEP_TIME_MS);
        }
    } else {
        // Some events occurred.
        pendingEventCount_ = size_t(pollResult);
    }
    return DH_SUCCESS;
}

std::vector<InputDevice> InputHub::GetAllInputDevices()
{
    std::unique_lock<std::mutex> deviceLock(visitMutex_);
    std::vector<InputDevice> vecDevice;
    for (const auto& [id, device] : devices_) {
        vecDevice.push_back(device->identifier);
    }
    return vecDevice;
}

void InputHub::ScanInputDevices(const std::string& dirname)
{
    DIR *dir;
    struct dirent *de;
    dir = opendir(dirname.c_str());
    if (dir == nullptr) {
        DHLOGE("error opendir dev/input :%{public}s\n", strerror(errno));
        return;
    }

    while ((de = readdir(dir))) {
        if (de->d_name[0] == '.' &&
            (de->d_name[1] == '\0' ||
            (de->d_name[1] == '.' && de->d_name[DIR_FILE_NAME_SECOND] == '\0'))) {
            continue;
        }
        std::string devName = dirname + "/" + std::string(de->d_name);
        OpenInputDeviceLocked(devName);
    }
    closedir(dir);
}

int32_t InputHub::OpenInputDeviceLocked(const std::string& devicePath)
{
    {
        std::unique_lock<std::mutex> deviceLock(visitMutex_);
        for (const auto& [deviceId, device] : devices_) {
            if (device->path == devicePath) {
                return DH_SUCCESS; // device was already registered
            }
        }
    }
    
    std::unique_lock<std::mutex> my_lock(operationMutex_);
    DHLOGI("Opening device: %s", devicePath.c_str());
    chmod(devicePath.c_str(), S_IWRITE | S_IREAD);
    int fd = open(devicePath.c_str(), O_RDWR | O_CLOEXEC | O_NONBLOCK);
    if (fd < 0) {
        DHLOGE("could not open %s, %s\n", devicePath.c_str(), strerror(errno));
        return ERR_DH_INPUT_HUB_OPEN_DEVICEPATH_FAIL;
    }

    InputDevice identifier;
    if (MakeInputDevice(fd, identifier) < 0) {
        return ERR_DH_INPUT_HUB_MAKE_INPUT_DEVICE_FAIL;
    }
    AssignDescriptorLocked(identifier);

    // Allocate device.  (The device object takes ownership of the fd at this point.)
    int32_t deviceId = nextDeviceId_++;
    std::unique_ptr<Device> device = std::make_unique<Device>(fd, deviceId, devicePath, identifier);

    DHLOGI("add device %d: %s\n", deviceId, devicePath.c_str());
    DHLOGI("  bus:        %04x\n"
           "  vendor      %04x\n"
           "  product     %04x\n"
           "  version     %04x\n",
        identifier.bus, identifier.vendor, identifier.product, identifier.version);
    DHLOGI("  name:       \"%s\"\n", identifier.name.c_str());
    DHLOGI("  location:   \"%s\"\n", identifier.location.c_str());
    DHLOGI("  unique id:  \"%s\"\n", identifier.uniqueId.c_str());
    DHLOGI("  descriptor: \"%s\"\n", identifier.descriptor.c_str());
    
    if (MakeDevice(fd, std::move(device)) < 0) {
        return ERR_DH_INPUT_HUB_MAKE_DEVICE_FAIL;
    }
    
    return DH_SUCCESS;
}

int32_t InputHub::MakeInputDevice(int fd, InputDevice& identifier)
{
    char buffer[80] = {};
    // Get device name.
    if (ioctl(fd, EVIOCGNAME(sizeof(buffer) - 1), &buffer) < 1) {
        DHLOGE(
            "Could not get device name for %s", strerror(errno));
    } else {
        buffer[sizeof(buffer) - 1] = '\0';
        identifier.name = buffer;
    }

    // If the device is already a virtual device, don't monitor it.
    if (identifier.name.find(VIRTUAL_DEVICE_NAME) != std::string::npos) {
        return ERR_DH_INPUT_HUB_MAKE_INPUT_DEVICE_FAIL;
    }
    // Get device driver version.
    int driverVersion;
    if (ioctl(fd, EVIOCGVERSION, &driverVersion)) {
        DHLOGE("could not get driver version for %s\n", strerror(errno));
        close(fd);
        return ERR_DH_INPUT_HUB_MAKE_INPUT_DEVICE_FAIL;
    }
    // Get device identifier.
    struct input_id inputId;
    if (ioctl(fd, EVIOCGID, &inputId)) {
        DHLOGE("could not get device input id for %s\n", strerror(errno));
        close(fd);
        return ERR_DH_INPUT_HUB_MAKE_INPUT_DEVICE_FAIL;
    }
    identifier.bus = inputId.bustype;
    identifier.product = inputId.product;
    identifier.vendor = inputId.vendor;
    identifier.version = inputId.version;
    // Get device physical location.
    if (ioctl(fd, EVIOCGPHYS(sizeof(buffer) - 1), &buffer) < 1) {
        DHLOGE("could not get location for %s\n", strerror(errno));
    } else {
        buffer[sizeof(buffer) - 1] = '\0';
        identifier.location = buffer;
    }
    // Get device unique id.
    if (ioctl(fd, EVIOCGUNIQ(sizeof(buffer) - 1), &buffer) < 1) {
        DHLOGE("could not get idstring for %s\n", strerror(errno));
    } else {
        buffer[sizeof(buffer) - 1] = '\0';
        identifier.uniqueId = buffer;
    }

    return DH_SUCCESS;
}

int32_t InputHub::MakeDevice(int fd, std::unique_ptr<Device> device)
{
    // Figure out the kinds of events the device reports.
    ioctl(fd, EVIOCGBIT(EV_KEY, sizeof(device->keyBitmask)), device->keyBitmask);
    ioctl(fd, EVIOCGBIT(EV_ABS, sizeof(device->absBitmask)), device->absBitmask);
    ioctl(fd, EVIOCGBIT(EV_REL, sizeof(device->relBitmask)), device->relBitmask);

    // See if this is a keyboard.
    bool haveKeyboardKeys = ContainsNonZeroByte(device->keyBitmask, 0, SizeofBitArray(BTN_MISC));
    if (haveKeyboardKeys) {
        device->classes |= INPUT_DEVICE_CLASS_KEYBOARD;
    }

    // See if this is a cursor device such as a trackball or mouse.
    if (TestBit(BTN_MOUSE, device->keyBitmask)
        && TestBit(REL_X, device->relBitmask)
        && TestBit(REL_Y, device->relBitmask)) {
        device->classes |= INPUT_DEVICE_CLASS_CURSOR;
    }

    // If the device isn't recognized as something we handle, don't monitor it.
    if (device->classes == 0) {
        DHLOGI("Dropping device: name='%s'", device->identifier.name.c_str());
        return ERR_DH_INPUT_HUB_MAKE_DEVICE_FAIL;
    }

    if (RegisterDeviceForEpollLocked(*device) != DH_SUCCESS) {
        return ERR_DH_INPUT_HUB_MAKE_DEVICE_FAIL;
    }

    DHLOGI("New device: fd=%d, name='%s', classes=0x%x,",
        fd, device->identifier.name.c_str(), device->classes);
    device->identifier.classes = device->classes;
    AddDeviceLocked(std::move(device));
    return DH_SUCCESS;
}

void InputHub::AssignDescriptorLocked(InputDevice& identifier)
{
    identifier.nonce = 0;
    std::string rawDescriptor = GenerateDescriptor(identifier);
    if (identifier.uniqueId.empty()) {
        // If it didn't have a unique id check for conflicts and enforce
        // uniqueness if necessary.
        while (GetDeviceByDescriptorLocked(identifier.descriptor) != nullptr) {
            identifier.nonce++;
            rawDescriptor = GenerateDescriptor(identifier);
        }
    }
    DHLOGI(
        "Created descriptor: raw=%s, cooked=%s", rawDescriptor.c_str(),
        identifier.descriptor.c_str());
}

std::string InputHub::StringPrintf(const char* format, ...) const
{
    static const int kSpaceLength = 1024;
    char space[kSpaceLength];

    va_list ap;
    va_start(ap, format);
    std::string result;
    int ret = vsnprintf_s(space, sizeof(space), sizeof(space) - 1, format, ap);
    if (ret >= DH_SUCCESS && ret < sizeof(space)) {
        result = space;
    } else {
        return "the buffer is overflow!";
    }
    va_end(ap);
    return result;
}

std::string InputHub::Sha256(const std::string& in) const
{
    SHA256_CTX ctx;
    SHA256_Init(&ctx);
    SHA256_Update(&ctx, reinterpret_cast<const u_char*>(in.c_str()), in.size());
    u_char digest[SHA_DIGEST_LENGTH];
    SHA256_Final(digest, &ctx);

    std::string out;
    for (size_t i = 0; i < SHA_DIGEST_LENGTH; i++) {
        out += StringPrintf("%02x", digest[i]);
    }
    return out;
}

std::string InputHub::GenerateDescriptor(InputDevice& identifier) const
{
    std::string rawDescriptor;
    rawDescriptor += StringPrintf(":%04x:%04x:", identifier.vendor,
        identifier.product);
    // add handling for USB devices to not uniqueify kbs that show up twice
    if (!identifier.uniqueId.empty()) {
        rawDescriptor += "uniqueId:";
        rawDescriptor += identifier.uniqueId;
    } else if (identifier.nonce != 0) {
        rawDescriptor += StringPrintf("nonce:%04x", identifier.nonce);
    }

    if (identifier.vendor == 0 && identifier.product == 0) {
        // If we don't know the vendor and product id, then the device is probably
        // built-in so we need to rely on other information to uniquely identify
        // the input device.  Usually we try to avoid relying on the device name or
        // location but for built-in input device, they are unlikely to ever change.
        if (!identifier.name.empty()) {
            rawDescriptor += "name:";
            rawDescriptor += identifier.name;
        } else if (!identifier.location.empty()) {
            rawDescriptor += "location:";
            rawDescriptor += identifier.location;
        }
    }
    identifier.descriptor = Sha256(rawDescriptor);
    return rawDescriptor;
}

int32_t InputHub::RegisterDeviceForEpollLocked(const Device& device)
{
    int32_t result = RegisterFdForEpoll(device.fd);
    if (result != DH_SUCCESS) {
        DHLOGE("Could not add input device fd to epoll for device %d", device.id);
        return result;
    }
    return result;
}

int32_t InputHub::RegisterFdForEpoll(int fd)
{
    struct epoll_event eventItem = {};
    eventItem.events = EPOLLIN | EPOLLWAKEUP;
    eventItem.data.fd = fd;
    if (epoll_ctl(epollFd_, EPOLL_CTL_ADD, fd, &eventItem)) {
        DHLOGE(
            "Could not add fd to epoll instance: %s", strerror(errno));
        return -errno;
    }
    return DH_SUCCESS;
}

void InputHub::AddDeviceLocked(std::unique_ptr<Device> device)
{
    std::unique_lock<std::mutex> deviceLock(visitMutex_);
    openingDevices_.push_back(std::move(device));
}

void InputHub::CloseDeviceLocked(Device& device)
{
    DHLOGI(
        "Removed device: path=%s name=%s id=%s fd=%d classes=0x%x",
        device.path.c_str(), device.identifier.name.c_str(), GetAnonyInt32(device.id).c_str(),
        device.fd, device.classes);

    UnregisterDeviceFromEpollLocked(device);
    device.Close();
    std::unique_lock<std::mutex> deviceLock(visitMutex_);
    closingDevices_.push_back(std::move(devices_[device.id]));
    devices_.erase(device.id);
}

int32_t InputHub::UnregisterDeviceFromEpollLocked(const Device& device) const
{
    if (device.HasValidFd()) {
        int32_t result = UnregisterFdFromEpoll(device.fd);
        if (result != DH_SUCCESS) {
            DHLOGE(
                "Could not remove input device fd from epoll for device %s", GetAnonyInt32(device.id).c_str());
            return result;
        }
    }
    return DH_SUCCESS;
}

int32_t InputHub::UnregisterFdFromEpoll(int fd) const
{
    if (epoll_ctl(epollFd_, EPOLL_CTL_DEL, fd, nullptr)) {
        DHLOGE(
            "Could not remove fd from epoll instance: %s", strerror(errno));
        return ERR_DH_INPUT_HUB_UNREGISTER_FD_FAIL;
    }
    return DH_SUCCESS;
}

int32_t InputHub::ReadNotifyLocked()
{
    int res;
    char event_buf[512];
    unsigned int event_size;
    int event_pos = 0;
    struct inotify_event *event;

    DHLOGI("readNotify nfd: %d\n", iNotifyFd_);
    res = read(iNotifyFd_, event_buf, sizeof(event_buf));
    if (res < (int)sizeof(*event)) {
        if (errno == EINTR)
            return DH_SUCCESS;
        DHLOGE("could not get event, %s\n", strerror(errno));
        return ERR_DH_INPUT_HUB_GET_EVENT_FAIL;
    }

    while (res >= (int) sizeof(*event)) {
        event = (struct inotify_event *)(event_buf + event_pos);
        JudgeDeviceOpenOrClose(*event);
        event_size = sizeof(*event) + event->len;
        res -= event_size;
        event_pos += event_size;
    }
    return DH_SUCCESS;
}

void InputHub::JudgeDeviceOpenOrClose(const inotify_event& event)
{
    if (event.len) {
        if (event.wd == inputWd_) {
            std::string filename = std::string(DEVICE_PATH) + "/" + event.name;
            if (event.mask & IN_CREATE) {
                OpenInputDeviceLocked(filename);
            } else {
                DHLOGI(
                    "Removing device '%s' due to inotify event\n", filename.c_str());
                CloseDeviceByPathLocked(filename);
            }
        } else {
            DHLOGI("Unexpected inotify event, wd = %i", event.wd);
        }
    }
}

void InputHub::CloseDeviceByPathLocked(const std::string& devicePath)
{
    Device* device = GetDeviceByPathLocked(devicePath);
    if (device) {
        CloseDeviceLocked(*device);
        return;
    }
    DHLOGI(
        "Remove device: %s not found, device may already have been removed.", devicePath.c_str());
}

void InputHub::CloseAllDevicesLocked()
{
    while (!devices_.empty()) {
        CloseDeviceLocked(*(devices_.begin()->second));
    }
}

InputHub::Device* InputHub::GetDeviceByDescriptorLocked(const std::string& descriptor)
{
    std::unique_lock<std::mutex> deviceLock(visitMutex_);
    for (const auto& [id, device] : devices_) {
        if (descriptor == device->identifier.descriptor) {
            return device.get();
        }
    }
    return nullptr;
}

InputHub::Device* InputHub::GetDeviceByPathLocked(const std::string& devicePath)
{
    std::unique_lock<std::mutex> deviceLock(visitMutex_);
    for (const auto& [id, device] : devices_) {
        if (device->path == devicePath) {
            return device.get();
        }
    }
    return nullptr;
}

InputHub::Device* InputHub::GetDeviceByFdLocked(int fd)
{
    std::unique_lock<std::mutex> deviceLock(visitMutex_);
    for (const auto& [id, device] : devices_) {
        if (device->fd == fd) {
            return device.get();
        }
    }
    return nullptr;
}

InputHub::Device* InputHub::GetDeviceByFd(int fd)
{
    std::unique_lock<std::mutex> deviceLock(visitMutex_);
    for (const auto& [id, device] : devices_) {
        if (device->fd == fd) {
            if (GetIsSupportInputTypes(device->classes)) {
                return device.get();
            }
        }
    }
    return nullptr;
}

bool InputHub::ContainsNonZeroByte(const uint8_t* array, uint32_t startIndex, uint32_t endIndex)
{
    const uint8_t* end = array + endIndex;
    array += startIndex;
    while (array != end) {
        if (*(array++) != 0) {
            return true;
        }
    }
    return false;
}

int64_t InputHub::ProcessEventTimestamp(const input_event& event)
{
    const int64_t inputEventTime = event.input_event_sec * 1000000000LL + event.input_event_usec * 1000LL;
    return inputEventTime;
}

bool InputHub::TestBit(uint32_t bit, const uint8_t* array)
{
    constexpr int units = 8;
    return (array)[(bit) / units] & (1 << ((bit) % units));
}

uint32_t InputHub::SizeofBitArray(uint32_t bit)
{
    constexpr int round = 7;
    constexpr int divisor = 8;
    return ((bit) + round) / divisor;
}

bool InputHub::GetIsSupportInputTypes(uint32_t classes)
{
    return classes & inputTypes_;
}
void InputHub::SetSupportInputType(const uint32_t& inputTypes)
{
    inputTypes_ = inputTypes;
    DHLOGI("SetSupportInputType: inputTypes=0x%x,", inputTypes_);
}

void InputHub::RecordEventLog(const RawEvent* event)
{
    std::string eventType = "";
    switch (event->type) {
        case EV_KEY:
            eventType = "EV_KEY";
            break;
        case EV_REL:
            eventType = "EV_REL";
            break;
        case EV_ABS:
            eventType = "EV_ABS";
            break;
        default:
            eventType = "other type";
            break;
    }
    DHLOGD("1.E2E-Test Sink collect event, EventType: %s, Code: %d, Value: %d, Path: %s, When: " PRId64"",
        eventType.c_str(), event->code, event->value, event->path.c_str(), event->when);
}

InputHub::Device::Device(int fd, int32_t id, const std::string& path,
    const InputDevice& identifier) : next(nullptr), fd(fd), id(id), path(path), identifier(identifier),
    classes(0), enabled(true), isVirtual(fd < 0) {
    memset_s(keyBitmask, sizeof(keyBitmask), 0, sizeof(keyBitmask));
    memset_s(absBitmask, sizeof(absBitmask), 0, sizeof(absBitmask));
    memset_s(relBitmask, sizeof(relBitmask), 0, sizeof(relBitmask));
}

InputHub::Device::~Device()
{
    Close();
}

void InputHub::Device::Close()
{
    if (fd >= 0) {
        ::close(fd);
        fd = -1;
    }
}

int32_t InputHub::Device::Enable()
{
    char pathCheck[PATH_MAX + 1] = {0x00};

    if (std::strlen(path.c_str()) > PATH_MAX || realpath(path.c_str(), pathCheck) == NULL) {
        DHLOGE("path check fail\n");
        return ERR_DH_INPUT_HUB_DEVICE_ENABLE_FAIL;
    }
    fd = open(path.c_str(), O_RDWR | O_CLOEXEC | O_NONBLOCK);
    if (fd < 0) {
        DHLOGE("could not open %s, %s\n", path.c_str(), strerror(errno));
        return ERR_DH_INPUT_HUB_DEVICE_ENABLE_FAIL;
    }
    enabled = true;
    return DH_SUCCESS;
}

int32_t InputHub::Device::Disable()
{
    Close();
    enabled = false;
    return DH_SUCCESS;
}

bool InputHub::Device::HasValidFd() const
{
    return !isVirtual && enabled;
}
} // namespace DistributedInput
} // namespace DistributedHardware
} // namespace OHOS
