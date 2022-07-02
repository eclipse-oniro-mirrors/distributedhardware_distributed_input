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

#include "distributed_input_client.h"

#include "anonymous_string.h"
#include "distributed_hardware_log.h"
#include "iservice_registry.h"
#include "nlohmann/json.hpp"
#include "system_ability_definition.h"

#include "constants_dinput.h"
#include "dinput_errcode.h"
#include "softbus_bus_center.h"
#include "white_list_util.h"

namespace OHOS {
namespace DistributedHardware {
namespace DistributedInput {
std::shared_ptr<DistributedInputClient> DistributedInputClient::instance(new DistributedInputClient());

DistributedInputClient::DistributedInputClient()
{
    DinputSAManager::GetInstance().Init();
}

DistributedInputClient &DistributedInputClient::GetInstance()
{
    return *instance.get();
}

void DistributedInputClient::RegisterDInputCb::OnResult(
    const std::string& devId, const std::string& dhId, const int32_t& status)
{
    std::lock_guard<std::mutex> lock(DistributedInputClient::GetInstance().operationMutex_);
    for (std::vector<DHardWareFwkRegistInfo>::iterator iter =
        DistributedInputClient::GetInstance().dHardWareFwkRstInfos.begin();
        iter != DistributedInputClient::GetInstance().dHardWareFwkRstInfos.end();
        iter++) {
        if (iter->devId == devId && iter->dhId == dhId) {
            iter->callback->OnRegisterResult(devId, dhId, status, "");
            DistributedInputClient::GetInstance().dHardWareFwkRstInfos.erase(iter);
            return;
        }
    }
}

void DistributedInputClient::UnregisterDInputCb::OnResult(
    const std::string& devId, const std::string& dhId, const int32_t& status)
{
    std::lock_guard<std::mutex> lock(DistributedInputClient::GetInstance().operationMutex_);
    for (std::vector<DHardWareFwkUnRegistInfo>::iterator iter =
        DistributedInputClient::GetInstance().dHardWareFwkUnRstInfos.begin();
        iter != DistributedInputClient::GetInstance().dHardWareFwkUnRstInfos.end();
        iter++) {
        if (iter->devId == devId && iter->dhId == dhId) {
            iter->callback->OnUnregisterResult(devId, dhId, status, "");
            DistributedInputClient::GetInstance().dHardWareFwkUnRstInfos.erase(iter);
            return;
        }
    }
}

void DistributedInputClient::StartDInputServerCb::OnResult(const int32_t& status, const uint32_t& inputTypes)
{
    if (DInputServerType::SOURCE_SERVER_TYPE == static_cast<DInputServerType>(status)) {
        DistributedInputClient::GetInstance().serverType = DInputServerType::SOURCE_SERVER_TYPE;
        DistributedInputClient::GetInstance().inputTypes_ = static_cast<DInputDeviceType>(inputTypes);
    } else if (DInputServerType::SINK_SERVER_TYPE == static_cast<DInputServerType>(status)) {
        DistributedInputClient::GetInstance().serverType = DInputServerType::SINK_SERVER_TYPE;
        DistributedInputClient::GetInstance().inputTypes_ = static_cast<DInputDeviceType>(inputTypes);
    } else {
        DistributedInputClient::GetInstance().serverType = DInputServerType::NULL_SERVER_TYPE;
        DistributedInputClient::GetInstance().inputTypes_ = DInputDeviceType::NONE;
    }
}

void DistributedInputClient::AddWhiteListInfosCb::OnResult(const std::string &deviceId, const std::string &strJson)
{
    if (!strJson.empty()) {
        DistributedInputClient::GetInstance().AddWhiteListInfos(deviceId, strJson);
    }
}

void DistributedInputClient::DelWhiteListInfosCb::OnResult(const std::string& deviceId)
{
    DistributedInputClient::GetInstance().DelWhiteListInfos(deviceId);
}

int32_t DistributedInputClient::InitSource()
{
    if (!DinputSAManager::GetInstance().GetDInputSourceProxy()) {
        return ERR_DH_INPUT_CLIENT_GET_SOURCE_PROXY_FAIL;
    }
    return DinputSAManager::GetInstance().dInputSourceProxy_->Init();
}

int32_t DistributedInputClient::InitSink()
{
    if (!DinputSAManager::GetInstance().GetDInputSinkProxy()) {
        return ERR_DH_INPUT_CLIENT_GET_SINK_PROXY_FAIL;
    }
    return DinputSAManager::GetInstance().dInputSinkProxy_->Init();
}

int32_t DistributedInputClient::ReleaseSource()
{
    if (!DinputSAManager::GetInstance().GetDInputSourceProxy()) {
        return ERR_DH_INPUT_CLIENT_GET_SOURCE_PROXY_FAIL;
    }

    serverType = DInputServerType::NULL_SERVER_TYPE;
    inputTypes_ = DInputDeviceType::NONE;
    m_bIsAlreadyInitWhiteList = false;
    sinkTypeCallback = nullptr;
    sourceTypeCallback = nullptr;
    addWhiteListCallback = nullptr;
    delWhiteListCallback = nullptr;
    WhiteListUtil::GetInstance().ClearWhiteList();
    return DinputSAManager::GetInstance().dInputSourceProxy_->Release();
}

int32_t DistributedInputClient::ReleaseSink()
{
    if (!DinputSAManager::GetInstance().GetDInputSinkProxy()) {
        return ERR_DH_INPUT_CLIENT_GET_SINK_PROXY_FAIL;
    }
    serverType = DInputServerType::NULL_SERVER_TYPE;
    inputTypes_ = DInputDeviceType::NONE;
    m_bIsAlreadyInitWhiteList = false;
    sinkTypeCallback = nullptr;
    WhiteListUtil::GetInstance().ClearWhiteList(localDevId_);
    return DinputSAManager::GetInstance().dInputSinkProxy_->Release();
}

int32_t DistributedInputClient::RegisterDistributedHardware(const std::string& devId, const std::string& dhId,
    const std::string& parameters, const std::shared_ptr<RegisterCallback>& callback)
{
    DHLOGI("%s called, deviceId: %s,  dhId: %s,  parameters: %s",
        __func__, GetAnonyString(devId).c_str(), GetAnonyString(dhId).c_str(), parameters.c_str());

    if (!DinputSAManager::GetInstance().GetDInputSourceProxy()) {
        DHLOGE("RegisterDistributedHardware client fail");
        return ERR_DH_INPUT_CLIENT_GET_SOURCE_PROXY_FAIL;
    }

    if (devId.empty() || dhId.empty() || parameters.empty() || !IsJsonData(parameters) || callback == nullptr) {
        return ERR_DH_INPUT_CLIENT_REGISTER_FAIL;
    }

    {
        std::lock_guard<std::mutex> lock(DistributedInputClient::GetInstance().operationMutex_);
        for (auto iter : dHardWareFwkRstInfos) {
            if (iter.devId == devId && iter.dhId == dhId) {
                return ERR_DH_INPUT_CLIENT_REGISTER_FAIL;
            }
        }

        DHardWareFwkRegistInfo info;
        info.devId = devId;
        info.dhId = dhId;
        info.callback = callback;
        dHardWareFwkRstInfos.push_back(info);
    }

    return DinputSAManager::GetInstance().dInputSourceProxy_->RegisterDistributedHardware(devId, dhId, parameters,
        new(std::nothrow) RegisterDInputCb());
}

int32_t DistributedInputClient::UnregisterDistributedHardware(const std::string& devId, const std::string& dhId,
    const std::shared_ptr<UnregisterCallback>& callback)
{
    DHLOGI("%s called, deviceId: %s,  dhId: %s",
        __func__, GetAnonyString(devId).c_str(), GetAnonyString(dhId).c_str());

    if (!DinputSAManager::GetInstance().GetDInputSourceProxy()) {
        DHLOGE("UnregisterDistributedHardware client fail");
        return ERR_DH_INPUT_CLIENT_GET_SOURCE_PROXY_FAIL;
    }

    if (devId.empty() || dhId.empty() || callback == nullptr) {
        return ERR_DH_INPUT_CLIENT_UNREGISTER_FAIL;
    }

    {
        std::lock_guard<std::mutex> lock(DistributedInputClient::GetInstance().operationMutex_);
        for (auto iter : dHardWareFwkUnRstInfos) {
            if (iter.devId == devId && iter.dhId == dhId) {
                return ERR_DH_INPUT_CLIENT_UNREGISTER_FAIL;
            }
        }

        DHardWareFwkUnRegistInfo info;
        info.devId = devId;
        info.dhId = dhId;
        info.callback = callback;
        dHardWareFwkUnRstInfos.push_back(info);
    }

    return DinputSAManager::GetInstance().dInputSourceProxy_->UnregisterDistributedHardware(devId, dhId,
        new(std::nothrow) UnregisterDInputCb());
}

int32_t DistributedInputClient::PrepareRemoteInput(
    const std::string& deviceId, sptr<IPrepareDInputCallback> callback)
{
    DHLOGI("%s called, deviceId: %s", __func__, GetAnonyString(deviceId).c_str());

    if (!DinputSAManager::GetInstance().GetDInputSourceProxy()) {
        DHLOGE("PrepareRemoteInput client fail");
        return ERR_DH_INPUT_CLIENT_GET_SOURCE_PROXY_FAIL;
    }

    if (deviceId.empty() || callback == nullptr) {
        return ERR_DH_INPUT_CLIENT_PREPARE_FAIL;
    }

    addWhiteListCallback = new(std::nothrow) AddWhiteListInfosCb();
    return DinputSAManager::GetInstance().dInputSourceProxy_->PrepareRemoteInput(deviceId, callback,
        addWhiteListCallback);
}

int32_t DistributedInputClient::UnprepareRemoteInput(
    const std::string& deviceId, sptr<IUnprepareDInputCallback> callback)
{
    DHLOGI("%s called, deviceId: %s", __func__, GetAnonyString(deviceId).c_str());

    if (!DinputSAManager::GetInstance().GetDInputSourceProxy()) {
        DHLOGE("PrepareRemoteInput client fail");
        return ERR_DH_INPUT_CLIENT_GET_SOURCE_PROXY_FAIL;
    }

    if (deviceId.empty() || callback == nullptr) {
        return ERR_DH_INPUT_CLIENT_UNPREPARE_FAIL;
    }

    delWhiteListCallback = new(std::nothrow) DelWhiteListInfosCb();
    return DinputSAManager::GetInstance().dInputSourceProxy_->UnprepareRemoteInput(deviceId, callback,
        delWhiteListCallback);
}

int32_t DistributedInputClient::StartRemoteInput(
    const std::string& deviceId, const uint32_t& inputTypes, sptr<IStartDInputCallback> callback)
{
    DHLOGI("%s called, deviceId: %s, inputTypes: %d", __func__, GetAnonyString(deviceId).c_str(), inputTypes);

    if (!DinputSAManager::GetInstance().GetDInputSourceProxy()) {
        DHLOGE("StartRemoteInput client fail");
        return ERR_DH_INPUT_CLIENT_GET_SOURCE_PROXY_FAIL;
    }

    if (deviceId.empty() || callback == nullptr ||
        inputTypes > static_cast<uint32_t>(DInputDeviceType::ALL) ||
        inputTypes == static_cast<uint32_t>(DInputDeviceType::NONE) ||
        !(inputTypes & static_cast<uint32_t>(DInputDeviceType::ALL))) {
        return ERR_DH_INPUT_CLIENT_START_FAIL;
    }

    return DinputSAManager::GetInstance().dInputSourceProxy_->StartRemoteInput(deviceId, inputTypes, callback);
}

int32_t DistributedInputClient::StopRemoteInput(
    const std::string& deviceId, const uint32_t& inputTypes, sptr<IStopDInputCallback> callback)
{
    DHLOGI("%s called, deviceId: %s, inputTypes: %d", __func__, GetAnonyString(deviceId).c_str(), inputTypes);

    if (!DinputSAManager::GetInstance().GetDInputSourceProxy()) {
        DHLOGE("StopRemoteInput client fail");
        return ERR_DH_INPUT_CLIENT_GET_SOURCE_PROXY_FAIL;
    }

    if (deviceId.empty() || callback == nullptr ||
        inputTypes > static_cast<uint32_t>(DInputDeviceType::ALL) ||
        inputTypes == static_cast<uint32_t>(DInputDeviceType::NONE) ||
        !(inputTypes & static_cast<uint32_t>(DInputDeviceType::ALL))) {
        return ERR_DH_INPUT_CLIENT_STOP_FAIL;
    }

    return DinputSAManager::GetInstance().dInputSourceProxy_->StopRemoteInput(deviceId, inputTypes, callback);
}

bool DistributedInputClient::IsNeedFilterOut(const std::string& deviceId, const BusinessEvent& event)
{
    DHLOGI("%s called, deviceId: %s", __func__, GetAnonyString(deviceId).c_str());
    if (serverType == DInputServerType::NULL_SERVER_TYPE) {
        DHLOGE("No sa start using.");
        return true;
    }

    if (serverType == DInputServerType::SINK_SERVER_TYPE) {
        if (m_bIsAlreadyInitWhiteList) {
            return WhiteListUtil::GetInstance().IsNeedFilterOut(localDevId_, event);
        }

        if (WhiteListUtil::GetInstance().Init(localDevId_) != DH_SUCCESS) {
            return false;
        }
        m_bIsAlreadyInitWhiteList = true;

        return WhiteListUtil::GetInstance().IsNeedFilterOut(localDevId_, event);
    }

    return !WhiteListUtil::GetInstance().IsNeedFilterOut(deviceId, event);
}

DInputServerType DistributedInputClient::IsStartDistributedInput(const uint32_t& inputType)
{
    DHLOGI("%s called, inputType: %d, inputTypes: %d, ", __func__, inputType, static_cast<uint32_t>(inputTypes_));
    int32_t retSource = 0;
    int32_t retSink = 0;

    if (sourceTypeCallback == nullptr && DinputSAManager::GetInstance().GetDInputSourceProxy()) {
        sourceTypeCallback = new(std::nothrow) StartDInputServerCb();
        retSource = DinputSAManager::GetInstance().dInputSourceProxy_->IsStartDistributedInput(inputType,
            sourceTypeCallback);
    }

    if (sinkTypeCallback == nullptr && DinputSAManager::GetInstance().GetDInputSinkProxy()) {
        sinkTypeCallback = new(std::nothrow) StartDInputServerCb();
        retSink = DinputSAManager::GetInstance().dInputSinkProxy_->IsStartDistributedInput(inputType, sinkTypeCallback);
    }

    if (static_cast<DInputServerType>(retSource) != DInputServerType::NULL_SERVER_TYPE) {
        serverType = DInputServerType::SOURCE_SERVER_TYPE;
    } else if (static_cast<DInputServerType>(retSink) != DInputServerType::NULL_SERVER_TYPE) {
        serverType = DInputServerType::SINK_SERVER_TYPE;
    }

    if (inputType & static_cast<uint32_t>(inputTypes_)) {
        return serverType;
    } else {
        return DInputServerType::NULL_SERVER_TYPE;
    }
}

bool DistributedInputClient::IsJsonData(std::string strData) const
{
    if (strData[0] != '{') {
        return false;
    }

    int num = 1;
    for (size_t i = 1; i < strData.length(); ++i) {
        if (strData[i] == '{') {
            ++num;
        } else if (strData[i] == '}') {
            --num;
        }
        if (num == 0) {
            return true;
        }
    }

    return false;
}

void DistributedInputClient::AddWhiteListInfos(
    const std::string &deviceId, const std::string &strJson) const
{
    nlohmann::json inputData = nlohmann::json::parse(strJson);
    size_t jsonSize = inputData.size();
    DHLOGI("AddWhiteListInfosCb OnResult json size:%d.\n", jsonSize);
    TYPE_WHITE_LIST_VEC vecWhiteList = inputData;
    WhiteListUtil::GetInstance().SyncWhiteList(deviceId, vecWhiteList);
}

void DistributedInputClient::DelWhiteListInfos(
    const std::string &deviceId) const
{
    WhiteListUtil::GetInstance().ClearWhiteList(deviceId);
}
} // namespace DistributedInput
} // namespace DistributedHardware
} // namespace OHOS
