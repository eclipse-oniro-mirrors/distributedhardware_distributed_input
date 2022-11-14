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

#ifndef OHOS_VIRTUAL_DEVICE_H
#define OHOS_VIRTUAL_DEVICE_H

#include <map>
#include <string>
#include <vector>

#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>

#include <linux/input.h>
#include "linux/uinput.h"

#include "constants_dinput.h"

namespace OHOS {
namespace DistributedHardware {
namespace DistributedInput {
class VirtualDevice {
public:
    explicit VirtualDevice(const InputDevice& event);
    virtual ~VirtualDevice();
    bool DoIoctl(int32_t fd, int32_t request, const uint32_t value);
    bool CreateKey();
    bool SetPhys(const std::string deviceName, std::string dhId);
    bool SetUp(const std::string &devId, const std::string &dhId);
    bool InjectInputEvent(const input_event &event);
    void SetNetWorkId(const std::string netWorkId);
    std::string GetNetWorkId();

    int32_t GetDeviceFd();
    uint16_t GetDeviceType();

protected:
    VirtualDevice();
    virtual const std::vector<uint32_t>& GetEventTypes() const = 0;
    virtual const std::vector<uint32_t>& GetKeys() const = 0;
    virtual const std::vector<uint32_t>& GetProperties() const = 0;
    virtual const std::vector<uint32_t>& GetAbs() const = 0;
    virtual const std::vector<uint32_t>& GetRelBits() const = 0;

    int32_t fd_ = -1;
    std::string deviceName_;
    std::string netWorkId_;
    const uint16_t busType_;
    const uint16_t vendorId_;
    const uint16_t productId_;
    const uint16_t version_;
    const uint16_t classes_;
    struct uinput_user_dev dev_ {};
    struct uinput_abs_setup absTemp_ = {};
    std::vector<uinput_abs_setup> absInit_;
    const std::string pid_ = std::to_string(getpid());

private:
    void RecordEventLog(const input_event& event);
};
} // namespace DistributedInput
} // namespace DistributedHardware
} // namespace OHOS

#endif // OHOS_VIRTUAL_DEVICE_H
