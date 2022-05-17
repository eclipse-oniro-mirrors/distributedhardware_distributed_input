/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#ifndef INPUT_HUB_H
#define INPUT_HUB_H

#include <mutex>
#include <unordered_map>
#include <sys/epoll.h>
#include <sys/inotify.h>
#include <linux/input.h>
#include "constants_dinput.h"

namespace OHOS {
namespace DistributedHardware {
namespace DistributedInput {
class InputHub {
public:
    InputHub();
    ~InputHub();
    size_t CollectInputEvents(RawEvent* buffer, size_t bufferSize);
    size_t CollectInputHandler(InputDeviceEvent* buffer, size_t bufferSize);
    void DeviceIsExists(InputDeviceEvent* event, size_t capacity);
    std::vector<InputDevice> GetAllInputDevices();
    void SetSupportInputType(const uint32_t& inputType);
private:
    struct Device  {
        Device* next;
        int fd; // may be -1 if device is closed
        const int32_t id;
        const std::string path;
        InputDevice identifier;
        uint32_t classes;
        uint8_t keyBitmask[(KEY_MAX + 1) / 8];
        uint8_t absBitmask[(ABS_MAX + 1) / 8];
        uint8_t relBitmask[(REL_MAX + 1) / 8];

        Device(int fd, int32_t id, const std::string& path,
                const InputDevice& identifier);
        ~Device();
        void Close();
        bool enabled; // initially true
        int32_t Enable();
        int32_t Disable();
        bool HasValidFd() const;
        const bool isVirtual; // set if fd < 0 is passed to constructor
    };

    int32_t Initialize();
    int32_t Release();

    size_t GetEvents(RawEvent* buffer, size_t bufferSize);
    size_t ReadInputEvent(int32_t readSize, Device& device);
    void GetDeviceHandler();
    int32_t RefreshEpollItem();

    void ScanInputDevices(const std::string& dirname);
    int32_t OpenInputDeviceLocked(const std::string& devicePath);
    int32_t MakeInputDevice(int fd, InputDevice& identifier);
    int32_t MakeDevice(int fd, std::unique_ptr<Device> device);
    void AssignDescriptorLocked(InputDevice& identifier);
    std::string GenerateDescriptor(InputDevice& identifier) const;
    std::string StringPrintf(const char* format, ...) const;
    std::string Sha256(const std::string& in) const;

    int32_t RegisterFdForEpoll(int fd);
    int32_t RegisterDeviceForEpollLocked(const Device& device);
    void AddDeviceLocked(std::unique_ptr<Device> device);
    void CloseDeviceLocked(Device& device);
    int32_t UnregisterDeviceFromEpollLocked(const Device& device) const;
    int32_t UnregisterFdFromEpoll(int fd) const;
    int32_t ReadNotifyLocked();
    void CloseDeviceByPathLocked(const std::string& devicePath);
    void CloseAllDevicesLocked();
    void JudgeDeviceOpenOrClose(const inotify_event& event);
    Device* GetDeviceByDescriptorLocked(const std::string& descriptor);
    Device* GetDeviceByPathLocked(const std::string& devicePath);
    Device* GetDeviceByFdLocked(int fd);
    Device* GetDeviceByFd(int fd);

    bool ContainsNonZeroByte(const uint8_t* array, uint32_t startIndex, uint32_t endIndex);
    int64_t ProcessEventTimestamp(const input_event& event);
    /* this macro is used to tell if "bit" is set in "array"
    * it selects a byte from the array, and does a boolean AND
    * operation with a byte that only has the relevant bit set.
    * eg. to check for the 12th bit, we do (array[1] & 1<<4)
    */
    bool TestBit(uint32_t bit, const uint8_t* array);
    /* this macro computes the number of bytes needed to represent a bit array of the specified size */
    uint32_t SizeofBitArray(uint32_t bit);
    bool GetIsSupportInputTypes(uint32_t classes);

    int epollFd_;
    int iNotifyFd_;
    int inputWd_;

    std::vector<std::unique_ptr<Device>> openingDevices_;
    std::vector<std::unique_ptr<Device>> closingDevices_;
    std::unordered_map<int32_t, std::unique_ptr<Device>> devices_;
    bool needToScanDevices_;
    std::string deviceId_;
    int32_t nextDeviceId_;

    // Maximum number of signalled FDs to handle at a time.
    static const int EPOLL_MAX_EVENTS = 16;
    static const int SLEEP_TIME = 100000;
    // The array of pending epoll events and the index of the next event to be handled.
    struct epoll_event mPendingEventItems[EPOLL_MAX_EVENTS];
    size_t pendingEventCount_;
    size_t pendingEventIndex_;
    bool pendingINotify_;
    std::mutex operationMutex_;
    std::mutex visitMutex_;
    bool deviceChanged_;
    uint32_t input_types_;

    static const int DIR_FILE_NAME_SECOND = 2;
    static const int MOVE_POSITION_SIXTEEN = 16;
    static const int DRIVER_VERSION_MOVE = 8;
    static const uint16_t DRIVER_VERSION_MAX = 0xff;
};
} // namespace DistributedInput
} // namespace DistributedHardware
} // namespace OHOS

#endif  // INPUT_HUB_H