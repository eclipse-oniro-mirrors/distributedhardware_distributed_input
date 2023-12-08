/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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

#include "distributed_input_node_manager.h"

#include <cinttypes>
#include <cstring>

#include <pthread.h>

#include "softbus_bus_center.h"

#include "dinput_context.h"
#include "dinput_errcode.h"
#include "dinput_log.h"
#include "dinput_softbus_define.h"
#include "dinput_utils_tool.h"

namespace OHOS {
namespace DistributedHardware {
namespace DistributedInput {
DistributedInputNodeManager::DistributedInputNodeManager() : isInjectThreadCreated_(false),
    isInjectThreadRunning_(false), inputHub_(std::make_unique<InputHub>()), virtualTouchScreenFd_(UN_INIT_FD_VALUE)
{
    DHLOGI("DistributedInputNodeManager ctor");
    std::shared_ptr<AppExecFwk::EventRunner> runner = AppExecFwk::EventRunner::Create(true);
    callBackHandler_ = std::make_shared<DistributedInputNodeManager::DInputNodeManagerEventHandler>(runner, this);
}

DistributedInputNodeManager::~DistributedInputNodeManager()
{
    DHLOGI("DistributedInputNodeManager dtor");
    isInjectThreadCreated_.store(false);
    isInjectThreadRunning_.store(false);
    if (eventInjectThread_.joinable()) {
        eventInjectThread_.join();
    }
    {
        std::lock_guard<std::mutex> lock(virtualDeviceMapMutex_);
        virtualDeviceMap_.clear();
    }
    DHLOGI("destructor end");
}

int32_t DistributedInputNodeManager::OpenDevicesNode(const std::string &devId, const std::string &dhId,
    const std::string &parameters)
{
    if (devId.size() > DEV_ID_LENGTH_MAX || devId.empty() || dhId.size() > DH_ID_LENGTH_MAX || dhId.empty() ||
        parameters.size() > STRING_MAX_SIZE || parameters.empty()) {
        DHLOGE("Params is invalid!");
        return ERR_DH_INPUT_SERVER_SOURCE_OPEN_DEVICE_NODE_FAIL;
    }
    InputDevice event;
    ParseInputDeviceJson(parameters, event);
    if (CreateHandle(event, devId, dhId) < 0) {
        DHLOGE("Can not create virtual node!");
        return ERR_DH_INPUT_SERVER_SOURCE_OPEN_DEVICE_NODE_FAIL;
    }
    return DH_SUCCESS;
}

void DistributedInputNodeManager::ParseInputDeviceJson(const std::string &str, InputDevice &pBuf)
{
    nlohmann::json inputDeviceJson = nlohmann::json::parse(str, nullptr, false);
    if (inputDeviceJson.is_discarded()) {
        DHLOGE("recMsg parse failed!");
        return;
    }
    ParseInputDevice(inputDeviceJson, pBuf);
}

void DistributedInputNodeManager::ParseInputDevice(const nlohmann::json &inputDeviceJson, InputDevice &pBuf)
{
    ParseInputDeviceBasicInfo(inputDeviceJson, pBuf);
    ParseInputDeviceEvents(inputDeviceJson, pBuf);
}

void DistributedInputNodeManager::ParseInputDeviceBasicInfo(const nlohmann::json &inputDeviceJson, InputDevice &pBuf)
{
    if (IsString(inputDeviceJson, DEVICE_NAME)) {
        pBuf.name = inputDeviceJson[DEVICE_NAME].get<std::string>();
    }
    if (IsString(inputDeviceJson, PHYSICAL_PATH)) {
        pBuf.physicalPath = inputDeviceJson[PHYSICAL_PATH].get<std::string>();
    }
    if (IsString(inputDeviceJson, UNIQUE_ID)) {
        pBuf.uniqueId = inputDeviceJson[UNIQUE_ID].get<std::string>();
    }
    if (IsUInt16(inputDeviceJson, BUS)) {
        pBuf.bus = inputDeviceJson[BUS].get<uint16_t>();
    }
    if (IsUInt16(inputDeviceJson, VENDOR)) {
        pBuf.vendor = inputDeviceJson[VENDOR].get<uint16_t>();
    }
    if (IsUInt16(inputDeviceJson, PRODUCT)) {
        pBuf.product = inputDeviceJson[PRODUCT].get<uint16_t>();
    }
    if (IsUInt16(inputDeviceJson, VERSION)) {
        pBuf.version = inputDeviceJson[VERSION].get<uint16_t>();
    }
    if (IsString(inputDeviceJson, DESCRIPTOR)) {
        pBuf.descriptor = inputDeviceJson[DESCRIPTOR].get<std::string>();
    }
    if (IsUInt32(inputDeviceJson, CLASSES)) {
        pBuf.classes = inputDeviceJson[CLASSES].get<uint32_t>();
    }
}

void DistributedInputNodeManager::ParseInputDeviceEvents(const nlohmann::json &inputDeviceJson, InputDevice &pBuf)
{
    if (IsArray(inputDeviceJson, EVENT_TYPES)) {
        pBuf.eventTypes = inputDeviceJson[EVENT_TYPES].get<std::vector<uint32_t>>();
    }
    if (IsArray(inputDeviceJson, EVENT_KEYS)) {
        pBuf.eventKeys = inputDeviceJson[EVENT_KEYS].get<std::vector<uint32_t>>();
    }
    if (IsArray(inputDeviceJson, ABS_TYPES)) {
        pBuf.absTypes = inputDeviceJson[ABS_TYPES].get<std::vector<uint32_t>>();
    }
    if (IsArray(inputDeviceJson, ABS_INFOS)) {
        pBuf.absInfos = inputDeviceJson[ABS_INFOS].get<std::map<uint32_t, std::vector<int32_t>>>();
    }
    if (IsArray(inputDeviceJson, REL_TYPES)) {
        pBuf.relTypes = inputDeviceJson[REL_TYPES].get<std::vector<uint32_t>>();
    }
    if (IsArray(inputDeviceJson, PROPERTIES)) {
        pBuf.properties = inputDeviceJson[PROPERTIES].get<std::vector<uint32_t>>();
    }
    if (IsArray(inputDeviceJson, MISCELLANEOUS)) {
        pBuf.miscellaneous = inputDeviceJson[MISCELLANEOUS].get<std::vector<uint32_t>>();
    }
    if (IsArray(inputDeviceJson, LEDS)) {
        pBuf.leds = inputDeviceJson[LEDS].get<std::vector<uint32_t>>();
    }
    if (IsArray(inputDeviceJson, REPEATS)) {
        pBuf.repeats = inputDeviceJson[REPEATS].get<std::vector<uint32_t>>();
    }
    if (IsArray(inputDeviceJson, SWITCHS)) {
        pBuf.switchs = inputDeviceJson[SWITCHS].get<std::vector<uint32_t>>();
    }
}

void DistributedInputNodeManager::ScanSinkInputDevices(const std::string &devId, const std::string &dhId)
{
    DHLOGI("ScanSinkInputDevices enter, deviceId: %s, dhId: %s.",
        GetAnonyString(devId).c_str(), GetAnonyString(dhId).c_str());
    std::vector<std::string> vecInputDevPath;
    ScanInputDevicesPath(DEVICE_PATH, vecInputDevPath);
    for (auto &tempPath: vecInputDevPath) {
        if (MatchAndSavePhysicalPath(tempPath, devId, dhId)) {
            DHLOGI("Set physical path success");
            break;
        }
    }
}

void DistributedInputNodeManager::DInputNodeManagerEventHandler::ProcessEvent(
    const AppExecFwk::InnerEvent::Pointer &event)
{
    DHLOGI("ProcessEvent enter.");
    auto iter = eventFuncMap_.find(event->GetInnerEventId());
    if (iter == eventFuncMap_.end()) {
        DHLOGE("Event Id %d is undefined.", event->GetInnerEventId());
        return;
    }
    nodeMgrFunc &func = iter->second;
    (this->*func)(event);
}

DistributedInputNodeManager::DInputNodeManagerEventHandler::DInputNodeManagerEventHandler(
    const std::shared_ptr<AppExecFwk::EventRunner> &runner, DistributedInputNodeManager *manager)
    : AppExecFwk::EventHandler(runner)
{
    eventFuncMap_[DINPUT_NODE_MANAGER_SCAN_ALL_NODE] = &DInputNodeManagerEventHandler::ScanAllNode;

    nodeManagerObj_ = manager;
}

DistributedInputNodeManager::DInputNodeManagerEventHandler::~DInputNodeManagerEventHandler()
{
    eventFuncMap_.clear();
    nodeManagerObj_ = nullptr;
}

void DistributedInputNodeManager::DInputNodeManagerEventHandler::ScanAllNode(
    const AppExecFwk::InnerEvent::Pointer &event)
{
    DHLOGI("ScanAllNode enter.");
    std::shared_ptr<nlohmann::json> dataMsg = event->GetSharedObject<nlohmann::json>();
    auto it = dataMsg->begin();
    nlohmann::json innerMsg = *(it);
    std::string devId = innerMsg[INPUT_NODE_DEVID];
    std::string devicedhId = innerMsg[INPUT_NODE_DHID];
    nodeManagerObj_->ScanSinkInputDevices(devId, devicedhId);
}

void DistributedInputNodeManager::NotifyNodeMgrScanVirNode(const std::string &devId, const std::string &dhId)
{
    DHLOGI("NotifyNodeMgrScanVirNode enter.");
    std::shared_ptr<nlohmann::json> jsonArrayMsg = std::make_shared<nlohmann::json>();
    nlohmann::json tmpJson;
    tmpJson[INPUT_NODE_DEVID] = devId;
    tmpJson[INPUT_NODE_DHID] = dhId;
    jsonArrayMsg->push_back(tmpJson);
    AppExecFwk::InnerEvent::Pointer msgEvent = AppExecFwk::InnerEvent::Get(
        DINPUT_NODE_MANAGER_SCAN_ALL_NODE, jsonArrayMsg, 0);
    callBackHandler_->SendEvent(msgEvent, 0, AppExecFwk::EventQueue::Priority::IMMEDIATE);
}

bool DistributedInputNodeManager::IsVirtualDev(int fd)
{
    char buffer[INPUT_EVENT_BUFFER_SIZE] = {0};
    std::string deviceName;
    if (ioctl(fd, EVIOCGNAME(sizeof(buffer) - 1), &buffer) < 1) {
        DHLOGE("Could not get device name for %s.", ConvertErrNo().c_str());
        return false;
    }
    buffer[sizeof(buffer) - 1] = '\0';
    deviceName = buffer;

    DHLOGD("IsVirtualDev deviceName: %s", buffer);
    if (deviceName.find(VIRTUAL_DEVICE_NAME) == std::string::npos) {
        DHLOGD("This is not a virtual device, fd %d, deviceName: %s.", fd, deviceName.c_str());
        return false;
    }
    return true;
}

bool DistributedInputNodeManager::GetDevDhUniqueIdByFd(int fd, DhUniqueID &dhUnqueId, std::string &physicalPath)
{
    char buffer[INPUT_EVENT_BUFFER_SIZE] = {0};
    if (ioctl(fd, EVIOCGPHYS(sizeof(buffer) - 1), &buffer) < 1) {
        DHLOGE("Could not get device physicalPath for %s.", ConvertErrNo().c_str());
        return false;
    }
    buffer[sizeof(buffer) - 1] = '\0';
    physicalPath = buffer;

    DHLOGD("GetDevDhUniqueIdByFd physicalPath %s.", physicalPath.c_str());
    std::vector<std::string> phyPathVec;
    SplitStringToVector(physicalPath, VIR_NODE_SPLIT_CHAR, phyPathVec);
    if (phyPathVec.size() != VIR_NODE_PHY_LEN) {
        DHLOGE("The physical path is invalid");
        return false;
    }
    std::string devId = phyPathVec[VIR_NODE_PHY_DEVID_IDX];
    std::string dhId = phyPathVec[VIR_NODE_PHY_DHID_IDX];
    if (devId.empty() || dhId.empty()) {
        DHLOGE("Get dev deviceid and dhid failed.");
        return false;
    }
    dhUnqueId.first = devId;
    dhUnqueId.second = dhId;
    DHLOGD("Device deviceid: %s, dhId %s.", GetAnonyString(devId).c_str(), GetAnonyString(dhId).c_str());
    return true;
}

void DistributedInputNodeManager::SetPathForVirDev(const DhUniqueID &dhUniqueId, const std::string &devicePath)
{
    std::lock_guard<std::mutex> lock(virtualDeviceMapMutex_);
    auto iter = virtualDeviceMap_.begin();
    while (iter != virtualDeviceMap_.end()) {
        DHLOGD("Check Virtual device, deviceId %s, dhid %s.", iter->first.first.c_str(), iter->first.second.c_str());
        if (iter->first == dhUniqueId) {
            DHLOGD("Found the virtual device, set path :%s", devicePath.c_str());
            iter->second->SetPath(devicePath);
            break;
        }
        iter++;
    }
}

bool DistributedInputNodeManager::MatchAndSavePhysicalPath(const std::string &devicePath,
    const std::string &devId, const std::string &dhId)
{
    DHLOGI("Opening input device path: %s", devicePath.c_str());
    DhUniqueID curDhUniqueId;
    std::string physicalPath;
    int fd = OpenInputDeviceFdByPath(devicePath);
    if (fd == UN_INIT_FD_VALUE) {
        DHLOGE("The fd open failed, devicePath %s.", devicePath.c_str());
        return false;
    }
    if (!IsVirtualDev(fd)) {
        DHLOGE("The dev not virtual, devicePath %s.", devicePath.c_str());
        CloseFd(fd);
        return false;
    }
    if (!GetDevDhUniqueIdByFd(fd, curDhUniqueId, physicalPath)) {
        DHLOGE("Get unique id failed, device path %s.", devicePath.c_str());
        CloseFd(fd);
        return false;
    }

    DHLOGD("This opening node deviceId: %s, dhId: %s, to match node deviceId: %s, dhId: %s",
        GetAnonyString(curDhUniqueId.first).c_str(), GetAnonyString(curDhUniqueId.second).c_str(),
        GetAnonyString(devId).c_str(), GetAnonyString(dhId).c_str());

    if (curDhUniqueId.first != devId || curDhUniqueId.second != dhId) {
        DHLOGW("It is not the target vir node, skip it.");
        CloseFd(fd);
        return false;
    }

    SetPathForVirDev(curDhUniqueId, devicePath);
    CloseFd(fd);
    return true;
}

void DistributedInputNodeManager::GetVirtualKeyboardPaths(const std::vector<DhUniqueID> &dhUniqueIds,
    std::vector<std::string> &virKeyboardPaths)
{
    std::lock_guard<std::mutex> lock(virtualDeviceMapMutex_);
    for (const auto &dhUniqueId : dhUniqueIds) {
        auto iter = virtualDeviceMap_.begin();
        while (iter != virtualDeviceMap_.end()) {
            if (iter->second == nullptr) {
                DHLOGE("device is nullptr");
                continue;
            }
            if ((iter->first == dhUniqueId) &&
                ((iter->second->GetClasses() & INPUT_DEVICE_CLASS_KEYBOARD) != 0)) {
                DHLOGI("Found vir keyboard path %s, deviceId %s, dhid %s", iter->second->GetPath().c_str(),
                    GetAnonyString(dhUniqueId.first).c_str(), GetAnonyString(dhUniqueId.second).c_str());
                virKeyboardPaths.push_back(iter->second->GetPath());
            }
            iter++;
        }
    }
}

int32_t DistributedInputNodeManager::CreateHandle(const InputDevice &inputDevice, const std::string &devId,
    const std::string &dhId)
{
    std::unique_lock<std::mutex> my_lock(operationMutex_);
    std::call_once(callOnceFlag_, [this]() { inputHub_->ScanInputDevices(DEVICE_PATH); });
    std::unique_ptr<VirtualDevice> virtualDevice = std::make_unique<VirtualDevice>(inputDevice);

    virtualDevice->SetNetWorkId(devId);

    if (!virtualDevice->SetUp(inputDevice, devId, dhId)) {
        DHLOGE("could not create new virtual device\n");
        return ERR_DH_INPUT_SERVER_SOURCE_CREATE_HANDLE_FAIL;
    }
    AddDeviceLocked(devId, inputDevice.descriptor, std::move(virtualDevice));
    return DH_SUCCESS;
}

int32_t DistributedInputNodeManager::CreateVirtualTouchScreenNode(const std::string &devId, const std::string &dhId,
    const uint64_t srcWinId, const uint32_t sourcePhyWidth, const uint32_t sourcePhyHeight)
{
    std::unique_lock<std::mutex> my_lock(operationMutex_);
    std::unique_ptr<VirtualDevice> device;
    LocalAbsInfo info = DInputContext::GetInstance().GetLocalTouchScreenInfo().localAbsInfo;
    DHLOGI("CreateVirtualTouchScreenNode start, dhId: %s, sourcePhyWidth: %d, sourcePhyHeight: %d",
        GetAnonyString(dhId).c_str(), sourcePhyWidth, sourcePhyHeight);
    device = std::make_unique<VirtualDevice>(info.deviceInfo);
    if (!device->SetUp(info.deviceInfo, devId, dhId)) {
        DHLOGE("Virtual touch Screen setUp fail, devId: %s, dhId: %s", GetAnonyString(devId).c_str(),
            GetAnonyString(dhId).c_str());
        return ERR_DH_INPUT_SERVER_SOURCE_CREATE_HANDLE_FAIL;
    }
    virtualTouchScreenFd_ = device->GetDeviceFd();
    AddDeviceLocked(devId, dhId, std::move(device));
    DHLOGI("CreateVirtualTouchScreenNode end, dhId: %s", GetAnonyString(dhId).c_str());
    return DH_SUCCESS;
}

int32_t DistributedInputNodeManager::RemoveVirtualTouchScreenNode(const std::string &devId, const std::string &dhId)
{
    return CloseDeviceLocked(devId, dhId);
}

int32_t DistributedInputNodeManager::GetVirtualTouchScreenFd()
{
    return virtualTouchScreenFd_;
}

void DistributedInputNodeManager::AddDeviceLocked(const std::string &networkId, const std::string &dhId,
    std::unique_ptr<VirtualDevice> device)
{
    DHLOGI("AddDeviceLocked deviceId=%s, dhId=%s",
        GetAnonyString(networkId).c_str(), GetAnonyString(dhId).c_str());
    std::lock_guard<std::mutex> lock(virtualDeviceMapMutex_);
    auto [dev_it, inserted] = virtualDeviceMap_.insert_or_assign(
        {networkId, dhId}, std::move(device));
    if (!inserted) {
        DHLOGI("Device exists, deviceId=%s, dhId=%s, replaced. \n",
            GetAnonyString(networkId).c_str(), GetAnonyString(dhId).c_str());
    }
}

int32_t DistributedInputNodeManager::CloseDeviceLocked(const std::string &devId, const std::string &dhId)
{
    DHLOGI("CloseDeviceLocked called, deviceId=%s, dhId=%s",
        GetAnonyString(devId).c_str(), GetAnonyString(dhId).c_str());
    std::lock_guard<std::mutex> lock(virtualDeviceMapMutex_);
    DhUniqueID dhUniqueId = {devId, dhId};
    std::map<DhUniqueID, std::unique_ptr<VirtualDevice>>::iterator iter = virtualDeviceMap_.find(dhUniqueId);
    if (iter != virtualDeviceMap_.end()) {
        DHLOGI("CloseDeviceLocked called success, deviceId=%s, dhId=%s",
            GetAnonyString(devId).c_str(), GetAnonyString(dhId).c_str());
        virtualDeviceMap_.erase(iter);
        return DH_SUCCESS;
    }
    DHLOGE("CloseDeviceLocked called failure, deviceId=%s, dhId=%s",
        GetAnonyString(devId).c_str(), GetAnonyString(dhId).c_str());
    return ERR_DH_INPUT_SERVER_SOURCE_CLOSE_DEVICE_FAIL;
}

int32_t DistributedInputNodeManager::GetDevice(const std::string &devId, const std::string &dhId,
    VirtualDevice *&device)
{
    std::lock_guard<std::mutex> lock(virtualDeviceMapMutex_);
    auto iter = virtualDeviceMap_.find({devId, dhId});
    if (iter != virtualDeviceMap_.end()) {
        device = iter->second.get();
        return DH_SUCCESS;
    }
    return ERR_DH_INPUT_SERVER_SOURCE_GET_DEVICE_FAIL;
}

void DistributedInputNodeManager::StartInjectThread()
{
    if (isInjectThreadCreated_.load()) {
        DHLOGI("InjectThread has been created.");
        return;
    }
    DHLOGI("InjectThread does not created");
    isInjectThreadCreated_.store(true);
    isInjectThreadRunning_.store(true);
    eventInjectThread_ = std::thread(&DistributedInputNodeManager::InjectEvent, this);
}

void DistributedInputNodeManager::StopInjectThread()
{
    if (!isInjectThreadCreated_.load()) {
        DHLOGI("InjectThread does not created, and not need to stop.");
    }
    DHLOGI("InjectThread has been created, and soon will be stopped.");
    isInjectThreadRunning_.store(false);
    isInjectThreadCreated_.store(false);
    conditionVariable_.notify_all();
    if (eventInjectThread_.joinable()) {
        eventInjectThread_.join();
    }
}

void DistributedInputNodeManager::ReportEvent(const std::string &devId, const std::vector<RawEvent> &events)
{
    std::lock_guard<std::mutex> lockGuard(injectThreadMutex_);
    injectQueue_.push({devId, events});
    conditionVariable_.notify_all();
}

void DistributedInputNodeManager::InjectEvent()
{
    int32_t ret = pthread_setname_np(pthread_self(), EVENT_INJECT_THREAD_NAME);
    if (ret != 0) {
        DHLOGE("InjectEvent setname failed.");
    }
    DHLOGD("start");
    while (isInjectThreadRunning_.load()) {
        EventBatch events;
        {
            std::unique_lock<std::mutex> waitEventLock(injectThreadMutex_);
            conditionVariable_.wait(waitEventLock,
                [this]() { return !isInjectThreadRunning_.load() || !injectQueue_.empty(); });
            if (injectQueue_.empty()) {
                continue;
            }
            events = injectQueue_.front();
            injectQueue_.pop();
        }

        DHLOGD("process event, inject queue size: %zu", injectQueue_.size());
        ProcessInjectEvent(events);
    }
    DHLOGD("end");
}

void DistributedInputNodeManager::RegisterInjectEventCb(sptr<ISessionStateCallback> callback)
{
    DHLOGI("RegisterInjectEventCb");
    SessionStateCallback_ = callback;
}

void DistributedInputNodeManager::UnregisterInjectEventCb()
{
    DHLOGI("UnregisterInjectEventCb");
    SessionStateCallback_ = nullptr;
}

void DistributedInputNodeManager::RunInjectEventCallback(const std::string &dhId, const uint32_t injectEvent)
{
    DHLOGI("RunInjectEventCallback start.");
    if (SessionStateCallback_ == nullptr) {
        DHLOGE("RunSessionStateCallback SessionStateCallback_ is null.");
        return;
    }
    SessionStateCallback_->OnResult(dhId, DINPUT_INJECT_EVENT_FAIL);
}

bool DistributedInputNodeManager::IsTouchPad(const std::string &deviceName)
{
    DHLOGD("device name is %s.", deviceName.c_str());
    transform(deviceName.begin(), deviceName.end(), deviceName.begin(), ::tolower);
    if (deviceName.find(DH_TOUCH_PAD) == std::string::npos) {
        return false;
    }
    return true;
}

void DistributedInputNodeManager::AddBtnMouseDownState(int32_t fd)
{
    std::lock_guard<std::mutex> mapLock(downBtnMouseFdsMtx_);
    downTouchPadBtnMouseFds_.insert(fd);
}

void DistributedInputNodeManager::RemoveBtnMouseDownState(int32_t fd)
{
    std::lock_guard<std::mutex> mapLock(downBtnMouseFdsMtx_);
    downTouchPadBtnMouseFds_.erase(fd);
}

void DistributedInputNodeManager::ClearCachedState(int32_t fd)
{
    std::lock_guard<std::mutex> lock(absPosMtx_);
    absPositionsMap_.erase(fd);

    std::lock_guard<std::mutex> mapLock(downBtnMouseFdsMtx_);
    downTouchPadBtnMouseFds_.erase(fd);
}

void DistributedInputNodeManager::RecordEvents(const RawEvent &event, VirtualDevice* device)
{
    bool isTouchEvent = false;
    if (((device->GetClasses() & INPUT_DEVICE_CLASS_TOUCH_MT) || (device->GetClasses() & INPUT_DEVICE_CLASS_TOUCH)) &&
        IsTouchPad(device->GetDeviceName())) {
        isTouchEvent = true;
    }

    if (!isTouchEvent) {
        return;
    }

    if (event.type == EV_ABS && (event.code == ABS_MT_POSITION_X || event.code == ABS_X)) {
        RefreshABSPosition(device->GetDeviceFd(), event.value, -1);
    }

    if (event.type == EV_ABS && (event.code == ABS_MT_POSITION_Y || event.code == ABS_Y)) {
        RefreshABSPosition(device->GetDeviceFd(), -1, event.value);
    }

    // Deal btn mouse state
    if (event.type == EV_KEY && event.code == BTN_MOUSE && event.value == KEY_DOWN_STATE) {
        AddBtnMouseDownState(device->GetDeviceFd());
        RecordChangeEventLog(event);
    }

    if (event.type == EV_KEY && event.code == BTN_MOUSE && event.value == KEY_UP_STATE) {
        RemoveBtnMouseDownState(device->GetDeviceFd());
        RecordChangeEventLog(event);
    }
}

void DistributedInputNodeManager::RecordChangeEventLog(const RawEvent &event)
{
    std::string eventType = "";
    switch (event.type) {
        case EV_KEY:
            eventType = "EV_KEY";
            break;
        case EV_REL:
            eventType = "EV_REL";
            break;
        case EV_ABS:
            eventType = "EV_ABS";
            break;
        case EV_SYN:
            eventType = "EV_SYN";
            break;
        default:
            eventType = "other type " + std::to_string(event.type);
            break;
    }
    DHLOGI("6.E2E-Test Sink collect change event, EventType: %s, Code: %d, Value: %d, Path: %s, descriptor: %s,"
        "When:%" PRId64 "", eventType.c_str(), event.code, event.value, event.path.c_str(),
        GetAnonyString(event.descriptor).c_str(), event.when);
}

void DistributedInputNodeManager::RefreshABSPosition(int32_t fd, int32_t absX, int32_t absY)
{
    std::lock_guard<std::mutex> lock(absPosMtx_);
    if (absX != -1) {
        absPositionsMap_[fd].first = absX;
    }

    if (absY != -1) {
        absPositionsMap_[fd].second = absY;
    }
}

void DistributedInputNodeManager::ResetTouchPadBtnMouseState(const std::string &deviceId,
    const std::vector<std::string> &dhIds)
{
    VirtualDevice* device = nullptr;
    bool isTouchPadDevice = false;
    for (auto const &dhId : dhIds) {
        if (GetDevice(deviceId, dhId, device) < 0 || device == nullptr) {
            DHLOGE("could not find the device");
            continue;
        }

        if (((device->GetClasses() & INPUT_DEVICE_CLASS_TOUCH_MT) ||
            (device->GetClasses() & INPUT_DEVICE_CLASS_TOUCH)) &&
            IsTouchPad(device->GetDeviceName())) {
            isTouchPadDevice = true;
        }

        if (!isTouchPadDevice) {
            continue;
        }

        int32_t fd = device->GetDeviceFd();

        DHLOGI("Find the touchpad stopped, try reset it's state, deviceId: %s, dhId: %s, fd: %d",
            deviceId.c_str(), dhId.c_str(), fd);
        int32_t dx = -1;
        int32_t dy = -1;
        {
            std::lock_guard<std::mutex> lock(downBtnMouseFdsMtx_);
            if (downTouchPadBtnMouseFds_.count(fd) == 0) {
                continue;
            } else {
                downTouchPadBtnMouseFds_.erase(fd);
                DHLOGI("Find this touchpad fd should reset, fd: %d", fd);
            }
        }
        
        {
            std::lock_guard<std::mutex> lock(absPosMtx_);
            if (absPositionsMap_.count(fd) == 0) {
                DHLOGE("Find touch pad need reset, but CAN NOT find dx,dy");
                dx = 0;
                dy = 0;
            } else {
                dx = absPositionsMap_[fd].first;
                dy = absPositionsMap_[fd].second;
                absPositionsMap_.erase(fd);
            }
        }
        SimulateTouchPadUpState(deviceId, dhId, fd, dx, dy);
    }
}

void DistributedInputNodeManager::SimulateTouchPadUpState(const std::string &deviceId, const std::string &dhId,
    int32_t fd, int32_t dx, int32_t dy)
{
    DHLOGI("Sinmulate touch pad UP state events, deviceId: %s, dhId: %s, fd: %d, dx: %d, dy: %d",
        deviceId.c_str(), dhId.c_str(), fd, dx, dy);
    int32_t simTrackingId = 0xffffffff;
    input_event touchTrackingIdEv1 = { .type = EV_ABS, .code = ABS_MT_TRACKING_ID, .value = simTrackingId };
    input_event btnToolFingerDownEv = { .type = EV_KEY, .code = BTN_TOOL_FINGER, .value = KEY_DOWN_STATE };
    input_event btnToolDoubleTapUpEv = { .type = EV_KEY, .code = BTN_TOOL_DOUBLETAP, .value = KEY_UP_STATE };
    input_event mscEv1 = { .type = EV_MSC, .code = MSC_TIMESTAMP, .value = 0x0 };
    input_event sycReportEv1 = { .type = EV_SYN, .code = SYN_REPORT, .value = 0x0 };

    input_event absMtPosX1 = { .type = EV_ABS, .code = ABS_MT_POSITION_X, .value = dx };
    input_event absMtPosY1 = { .type = EV_ABS, .code = ABS_MT_POSITION_Y, .value = dy };
    input_event absPosX1 = { .type = EV_ABS, .code = ABS_X, .value = dx };
    input_event absPosY1 = { .type = EV_ABS, .code = ABS_Y, .value = dy };
    input_event mscEv2 = { .type = EV_MSC, .code = MSC_TIMESTAMP, .value = 0x0 };
    input_event sycReportEv2 = { .type = EV_SYN, .code = SYN_REPORT, .value = 0x0 };

    input_event absMtPosX2 = { .type = EV_ABS, .code = ABS_MT_POSITION_X, .value = dx };
    input_event absMtPosY2 = { .type = EV_ABS, .code = ABS_MT_POSITION_Y, .value = dy };
    input_event btnMouseUpEv = { .type = EV_KEY, .code = BTN_MOUSE, .value = KEY_UP_STATE };
    input_event absPosX2 = { .type = EV_ABS, .code = ABS_X, .value = dx };
    input_event absPosY2 = { .type = EV_ABS, .code = ABS_Y, .value = dy };
    input_event mscEv3 = { .type = EV_MSC, .code = MSC_TIMESTAMP, .value = 0x0 };
    input_event sycReportEv3 = { .type = EV_SYN, .code = SYN_REPORT, .value = 0x0 };

    input_event touchTrackingIdEv2 = { .type = EV_ABS, .code = ABS_MT_TRACKING_ID, .value = simTrackingId };
    input_event btnTouchUpEv = { .type = EV_KEY, .code = BTN_TOUCH, .value = KEY_UP_STATE };
    input_event btnToolFingerUpEv = { .type = EV_KEY, .code = BTN_TOOL_FINGER, .value = KEY_UP_STATE };
    input_event mscEv4 = { .type = EV_MSC, .code = MSC_TIMESTAMP, .value = 0x0 };
    input_event sycReportEv4 = { .type = EV_SYN, .code = SYN_REPORT, .value = 0x0 };

    std::vector<input_event> simEvents = {
        touchTrackingIdEv1, btnToolFingerDownEv, btnToolDoubleTapUpEv, mscEv1, sycReportEv1,
        absMtPosX1, absMtPosY1, absPosX1, absPosY1, mscEv2, sycReportEv2, 
        absMtPosX2, absMtPosY2, btnMouseUpEv, absPosX2, absPosY2, mscEv3, sycReportEv3,
        touchTrackingIdEv2, btnTouchUpEv, btnToolFingerUpEv, mscEv4, sycReportEv4 };
    for (auto &event : simEvents) {
        WriteEventToDevice(fd, event);
    }
}

void DistributedInputNodeManager::ProcessInjectEvent(const EventBatch &events)
{
    std::string deviceId = events.first;
    for (const auto &rawEvent : events.second) {
        std::string dhId = rawEvent.descriptor;
        struct input_event event = {
            .type = rawEvent.type,
            .code = rawEvent.code,
            .value = rawEvent.value
        };
        DHLOGI("InjectEvent deviceId: %s, dhId: %s, eventType: %d, eventCode: %d, eventValue: %d, when: " PRId64"",
            GetAnonyString(deviceId).c_str(), GetAnonyString(dhId).c_str(),
            event.type, event.code, event.value, rawEvent.when);
        VirtualDevice* device = nullptr;
        if (GetDevice(deviceId, dhId, device) < 0) {
            DHLOGE("could not find the device");
            return;
        }
        if (device != nullptr) {
            RecordEvents(rawEvent, device);
            device->InjectInputEvent(event);
        }
    }
}
} // namespace DistributedInput
} // namespace DistributedHardware
} // namespace OHOS
