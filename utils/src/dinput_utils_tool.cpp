/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "dinput_utils_tool.h"

#include "anonymous_string.h"
#include <sys/time.h>
#include "nlohmann/json.hpp"

#include "dinput_softbus_define.h"
#include "softbus_bus_center.h"

namespace OHOS {
namespace DistributedHardware {
namespace DistributedInput {
namespace {
    constexpr int32_t US_ONE_SECOND = 1000 * 1000;
    const char *const DESCRIPTOR = "descriptor";
}

DevInfo GetLocalDeviceInfo()
{
    DevInfo devInfo{"", "", 0};
    auto info = std::make_unique<NodeBasicInfo>();
    auto ret = GetLocalNodeDeviceInfo(DINPUT_PKG_NAME.c_str(), info.get());
    if (ret != 0) {
        DHLOGE("GetLocalNodeDeviceInfo failed, errCode = %d", ret);
        return devInfo;
    }

    devInfo.networkId = info->networkId;
    devInfo.deviceName = info->deviceName;
    devInfo.deviceType = info->deviceTypeId;

    return devInfo;
}

uint64_t GetCurrentTime()
{
    struct timeval tv;
    gettimeofday(&tv, nullptr);
    return tv.tv_sec * US_ONE_SECOND + tv.tv_usec;
}

std::string SetAnonyId(const std::string &message)
{
    nlohmann::json jsonObj = nlohmann::json::parse(message, nullptr, false);
    if (jsonObj.is_discarded()) {
        DHLOGE("jsonObj parse failed!");
        return "";
    }

    nlohmann::json jsonStr = nlohmann::json::parse(message);
    if (jsonStr.contains(DINPUT_SOFTBUS_KEY_DEVICE_ID)) {
        jsonStr[DINPUT_SOFTBUS_KEY_DEVICE_ID] = GetAnonyString(jsonStr[DINPUT_SOFTBUS_KEY_DEVICE_ID]);
    }
    if (jsonStr.contains(DINPUT_SOFTBUS_KEY_SESSION_ID)) {
        jsonStr[DINPUT_SOFTBUS_KEY_SESSION_ID] = GetAnonyInt32(jsonStr[DINPUT_SOFTBUS_KEY_SESSION_ID]);
    }
    if (jsonStr.contains(DESCRIPTOR)) {
        jsonStr[DESCRIPTOR] = GetAnonyString(jsonStr[DESCRIPTOR]);
    }
    return jsonStr.dump();
}
} // namespace DistributedInput
} // namespace DistributedHardware
} // namespace OHOS