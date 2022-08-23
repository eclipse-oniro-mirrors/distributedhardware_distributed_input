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

#ifndef OHOS_DISTRIBUTED_INPUT_UTILS_TOOL_H
#define OHOS_DISTRIBUTED_INPUT_UTILS_TOOL_H

#include <cstdint>
#include <string>

#include "distributed_hardware_log.h"
#include "nlohmann/json.hpp"

namespace OHOS {
namespace DistributedHardware {
namespace DistributedInput {
struct DevInfo {
    std::string networkId;
    std::string deviceName;
    uint16_t deviceType;
};

DevInfo GetLocalDeviceInfo();
std::string GetLocalNetworkId();
std::string GetUUIDBySoftBus(const std::string &networkId);
uint64_t GetCurrentTime();
std::string SetAnonyId(const std::string &message);
/* IsString, IsUint32 and IsUint64 are used to valid json key */
bool IsString(const nlohmann::json& jsonObj, const std::string& key);
bool IsInt32(const nlohmann::json& jsonObj, const std::string& key);
bool IsUint32(const nlohmann::json& jsonObj, const std::string& key);
bool IsUint64(const nlohmann::json& jsonObj, const std::string& key);
std::string GetNodeDesc(std::string parameters);
} // namespace DistributedInput
} // namespace DistributedHardware
} // namespace OHOS

#endif // OHOS_DISTRIBUTED_INPUT_UTILS_TOOL_H