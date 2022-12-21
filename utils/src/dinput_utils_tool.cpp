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

#include <cstdarg>
#include <cstdio>
#include <sys/time.h>

#include <openssl/sha.h>

#include "anonymous_string.h"
#include "nlohmann/json.hpp"
#include "softbus_bus_center.h"

#include "constants_dinput.h"
#include "dinput_errcode.h"
#include "dinput_softbus_define.h"

namespace OHOS {
namespace DistributedHardware {
namespace DistributedInput {
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

std::string GetLocalNetworkId()
{
    std::string localNetworkId = GetLocalDeviceInfo().networkId;
    if (localNetworkId.empty()) {
        DHLOGE("local networkId is empty!");
    }
    DHLOGI("GetLocalNetworkId, device local networkId is %s", GetAnonyString(localNetworkId).c_str());
    return localNetworkId;
}

std::string GetUUIDBySoftBus(const std::string &networkId)
{
    if (networkId.empty()) {
        return "";
    }
    char uuid[UUID_BUF_LEN] = {0};
    auto ret = GetNodeKeyInfo(DINPUT_PKG_NAME.c_str(), networkId.c_str(), NodeDeviceInfoKey::NODE_KEY_UUID,
        reinterpret_cast<uint8_t *>(uuid), UUID_BUF_LEN);
    return (ret == DH_SUCCESS) ? std::string(uuid) : "";
}

uint64_t GetCurrentTime()
{
    constexpr int32_t usOneSecond = 1000 * 1000;
    struct timeval tv;
    gettimeofday(&tv, nullptr);
    return tv.tv_sec * usOneSecond + tv.tv_usec;
}

std::string SetAnonyId(const std::string &message)
{
    nlohmann::json jsonObj = nlohmann::json::parse(message, nullptr, false);
    if (jsonObj.is_discarded()) {
        DHLOGE("jsonObj parse failed!");
        return "";
    }
    if (IsString(jsonObj, DINPUT_SOFTBUS_KEY_DEVICE_ID)) {
        jsonObj[DINPUT_SOFTBUS_KEY_DEVICE_ID] = GetAnonyString(jsonObj[DINPUT_SOFTBUS_KEY_DEVICE_ID]);
    }
    if (IsString(jsonObj, DESCRIPTOR)) {
        jsonObj[DESCRIPTOR] = GetAnonyString(jsonObj[DESCRIPTOR]);
    }
    if (IsString(jsonObj, DINPUT_SOFTBUS_KEY_INPUT_DATA)) {
        jsonObj[DINPUT_SOFTBUS_KEY_INPUT_DATA] = GetAnonyString(jsonObj[DINPUT_SOFTBUS_KEY_INPUT_DATA]);
    }
    if (IsString(jsonObj, DINPUT_SOFTBUS_KEY_VECTOR_DHID)) {
        std::string dhidStr = jsonObj[DINPUT_SOFTBUS_KEY_VECTOR_DHID];
        dhidStr.append(".");
        size_t pos = dhidStr.find(".");
        std::string anonyDhidStr = "";
        while (pos != dhidStr.npos) {
            anonyDhidStr += GetAnonyString(dhidStr.substr(0, pos)) + ".";
            dhidStr = dhidStr.substr(pos + 1, dhidStr.size());
            pos = dhidStr.find(".");
        }
        jsonObj[DINPUT_SOFTBUS_KEY_VECTOR_DHID] = anonyDhidStr.substr(0, anonyDhidStr.length() - 1);
    }
    if (IsString(jsonObj, DINPUT_SOFTBUS_KEY_SRC_DEV_ID)) {
        jsonObj[DINPUT_SOFTBUS_KEY_SRC_DEV_ID] = GetAnonyString(jsonObj[DINPUT_SOFTBUS_KEY_SRC_DEV_ID]);
    }
    if (IsString(jsonObj, DINPUT_SOFTBUS_KEY_SINK_DEV_ID)) {
        jsonObj[DINPUT_SOFTBUS_KEY_SINK_DEV_ID] = GetAnonyString(jsonObj[DINPUT_SOFTBUS_KEY_SINK_DEV_ID]);
    }
    return jsonObj.dump();
}

bool IsBoolean(const nlohmann::json& jsonObj, const std::string& key)
{
    bool res = jsonObj.contains(key) && jsonObj[key].is_boolean();
    if (!res) {
        DHLOGE("The key %s in jsonObj is invalid.", key.c_str());
    }
    return res;
}

bool IsString(const nlohmann::json& jsonObj, const std::string& key)
{
    bool res = jsonObj.contains(key) && jsonObj[key].is_string();
    if (!res) {
        DHLOGE("The key %s in jsonObj is invalid.", key.c_str());
    }
    return res;
}

bool IsInt32(const nlohmann::json& jsonObj, const std::string& key)
{
    bool res = jsonObj.contains(key) && jsonObj[key].is_number_integer() && INT32_MIN <= jsonObj[key] &&
        jsonObj[key] <= INT32_MAX;
    if (!res) {
        DHLOGE("The key %s in jsonObj is invalid.", key.c_str());
    }
    return res;
}

bool IsInt64(const nlohmann::json& jsonObj, const std::string& key)
{
    bool res = jsonObj.contains(key) && jsonObj[key].is_number_integer() && INT64_MIN <= jsonObj[key] &&
        jsonObj[key] <= INT64_MAX;
    if (!res) {
        DHLOGE("The key %s in jsonObj is invalid.", key.c_str());
    }
    return res;
}

bool IsUInt16(const nlohmann::json& jsonObj, const std::string& key)
{
    bool res = jsonObj.contains(key) && jsonObj[key].is_number_unsigned() && jsonObj[key] <= UINT16_MAX;
    if (!res) {
        DHLOGE("The key %s in jsonObj is invalid.", key.c_str());
    }
    return res;
}

bool IsUInt32(const nlohmann::json& jsonObj, const std::string& key)
{
    bool res = jsonObj.contains(key) && jsonObj[key].is_number_unsigned() && jsonObj[key] <= UINT32_MAX;
    if (!res) {
        DHLOGE("The key %s in jsonObj is invalid.", key.c_str());
    }
    return res;
}

bool IsUInt64(const nlohmann::json& jsonObj, const std::string& key)
{
    bool res = jsonObj.contains(key) && jsonObj[key].is_number_unsigned() && jsonObj[key] <= UINT64_MAX;
    if (!res) {
        DHLOGE("The key %s in jsonObj is invalid.", key.c_str());
    }
    return res;
}

bool IsArray(const nlohmann::json& jsonObj, const std::string& key)
{
    bool res = jsonObj.contains(key) && jsonObj[key].is_array();
    if (!res) {
        DHLOGE("The key %s in jsonObj is invalid.", key.c_str());
    }
    return res;
}

std::string GetNodeDesc(std::string parameters)
{
    nlohmann::json parObj = nlohmann::json::parse(parameters, nullptr, false);
    if (parObj.is_discarded()) {
        DHLOGE("parObj parse failed!");
        return "";
    }
    std::string nodeName = "N/A";
    std::string physicalPath = "N/A";
    int32_t classes = -1;

    if (IsString(parObj, DEVICE_NAME) && IsString(parObj, PHYSICAL_PATH) && IsInt32(parObj, CLASSES)) {
        nodeName = parObj.at(DEVICE_NAME).get<std::string>();
        physicalPath = parObj.at(PHYSICAL_PATH).get<std::string>();
        classes = parObj.at(CLASSES).get<int32_t>();
    }

    return "{ nodeName: " + nodeName + ", physicalPath: " + physicalPath + ", classes: " +
        std::to_string(classes) + " }";
}
} // namespace DistributedInput
} // namespace DistributedHardware
} // namespace OHOS