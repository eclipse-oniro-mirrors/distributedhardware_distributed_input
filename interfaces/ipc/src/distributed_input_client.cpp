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
DistributedInputClient &DistributedInputClient::GetInstance()
{
    static DistributedInputClient instance;
    return instance;
}

void DistributedInputClient::RegisterDInputCb::OnResult(
    const std::string& devId, const std::string& dhId, const int32_t& status)
{
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
        DistributedInputClient::GetInstance().inputTypes = inputTypes;
    } else if (DInputServerType::SINK_SERVER_TYPE == static_cast<DInputServerType>(status)) {
        DistributedInputClient::GetInstance().serverType = DInputServerType::SINK_SERVER_TYPE;
        DistributedInputClient::GetInstance().inputTypes = inputTypes;
    } else {
        DistributedInputClient::GetInstance().serverType = DInputServerType::NULL_SERVER_TYPE;
        DistributedInputClient::GetInstance().inputTypes = INPUT_TYPE_NULL;
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
    if (!GetDInputSourceProxy()) {
        return ERR_DH_INPUT_CLIENT_GET_SOURCE_PROXY_FAIL;
    }
    return dInputSourceProxy_->Init();
}

int32_t DistributedInputClient::InitSink()
{
    if (!GetDInputSinkProxy()) {
        return ERR_DH_INPUT_CLIENT_GET_SINK_PROXY_FAIL;
    }
    return dInputSinkProxy_->Init();
}

int32_t DistributedInputClient::ReleaseSource()
{
    if (!GetDInputSourceProxy()) {
        return ERR_DH_INPUT_CLIENT_GET_SOURCE_PROXY_FAIL;
    }

    serverType = DInputServerType::NULL_SERVER_TYPE;
    inputTypes = INPUT_TYPE_NULL;
    m_bIsAlreadyInitWhiteList = false;
    callbackRegister = nullptr;
    callbackUnregister = nullptr;
    sinkTypeCallback = nullptr;
    sourceTypeCallback = nullptr;
    addWhiteListCallback = nullptr;
    delWhiteListCallback = nullptr;
    WhiteListUtil::GetInstance().ClearWhiteList();
    return dInputSourceProxy_->Release();
}

int32_t DistributedInputClient::ReleaseSink()
{
    if (!GetDInputSinkProxy()) {
        return ERR_DH_INPUT_CLIENT_GET_SINK_PROXY_FAIL;
    }
    serverType = DInputServerType::NULL_SERVER_TYPE;
    inputTypes = INPUT_TYPE_NULL;
    m_bIsAlreadyInitWhiteList = false;
    sinkTypeCallback = nullptr;
    WhiteListUtil::GetInstance().ClearWhiteList(localDevId_);
    return dInputSinkProxy_->Release();
}

int32_t DistributedInputClient::RegisterDistributedHardware(const std::string& devId, const std::string& dhId,
    const std::string& parameters, const std::shared_ptr<RegisterCallback>& callback)
{
    DHLOGI("%s called, deviceId: %s,  dhId: %s,  parameters: %s",
        __func__, GetAnonyString(devId).c_str(), GetAnonyString(dhId).c_str(), parameters.c_str());

    if (!GetDInputSourceProxy()) {
        DHLOGE("RegisterDistributedHardware client fail");
        return ERR_DH_INPUT_CLIENT_GET_SOURCE_PROXY_FAIL;
    }

    if (devId.empty() || dhId.empty() || parameters.empty() || !IsJsonData(parameters) || callback == nullptr) {
        return ERR_DH_INPUT_CLIENT_REGISTER_FAIL;
    }

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

    callbackRegister = new(std::nothrow) RegisterDInputCb();
    return dInputSourceProxy_->RegisterDistributedHardware(devId, dhId, parameters, callbackRegister);
}

int32_t DistributedInputClient::UnregisterDistributedHardware(const std::string& devId, const std::string& dhId,
    const std::shared_ptr<UnregisterCallback>& callback)
{
    DHLOGI("%s called, deviceId: %s,  dhId: %s",
        __func__, GetAnonyString(devId).c_str(), GetAnonyString(dhId).c_str());

    if (!GetDInputSourceProxy()) {
        DHLOGE("UnregisterDistributedHardware client fail");
        return ERR_DH_INPUT_CLIENT_GET_SOURCE_PROXY_FAIL;
    }

    if (devId.empty() || dhId.empty() || callback == nullptr) {
        return ERR_DH_INPUT_CLIENT_UNREGISTER_FAIL;
    }

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

    callbackUnregister = new(std::nothrow) UnregisterDInputCb();
    return dInputSourceProxy_->UnregisterDistributedHardware(devId, dhId, callbackUnregister);
}

int32_t DistributedInputClient::PrepareRemoteInput(
    const std::string& deviceId, sptr<IPrepareDInputCallback> callback)
{
    DHLOGI("%s called, deviceId: %s", __func__, GetAnonyString(deviceId).c_str());

    if (!GetDInputSourceProxy()) {
        DHLOGE("PrepareRemoteInput client fail");
        return ERR_DH_INPUT_CLIENT_GET_SOURCE_PROXY_FAIL;
    }

    if (deviceId.empty() || callback == nullptr) {
        return ERR_DH_INPUT_CLIENT_PREPARE_FAIL;
    }

    addWhiteListCallback = new(std::nothrow) AddWhiteListInfosCb();
    return dInputSourceProxy_->PrepareRemoteInput(deviceId, callback, addWhiteListCallback);
}

int32_t DistributedInputClient::UnprepareRemoteInput(
    const std::string& deviceId, sptr<IUnprepareDInputCallback> callback)
{
    DHLOGI("%s called, deviceId: %s", __func__, GetAnonyString(deviceId).c_str());

    if (!GetDInputSourceProxy()) {
        DHLOGE("PrepareRemoteInput client fail");
        return ERR_DH_INPUT_CLIENT_GET_SOURCE_PROXY_FAIL;
    }

    if (deviceId.empty() || callback == nullptr) {
        return ERR_DH_INPUT_CLIENT_UNPREPARE_FAIL;
    }

    delWhiteListCallback = new(std::nothrow) DelWhiteListInfosCb();
    return dInputSourceProxy_->UnprepareRemoteInput(deviceId, callback, delWhiteListCallback);
}

int32_t DistributedInputClient::StartRemoteInput(
    const std::string& deviceId, const uint32_t& inputTypes, sptr<IStartDInputCallback> callback)
{
    DHLOGI("%s called, deviceId: %s, inputTypes: %d", __func__, GetAnonyString(deviceId).c_str(), inputTypes);

    if (!GetDInputSourceProxy()) {
        DHLOGE("StartRemoteInput client fail");
        return ERR_DH_INPUT_CLIENT_GET_SOURCE_PROXY_FAIL;
    }

    if (deviceId.empty() || callback == nullptr ||
        inputTypes > INPUT_TYPE_ALL || inputTypes == INPUT_TYPE_NULL ||
        !(inputTypes & INPUT_TYPE_ALL)) {
        return ERR_DH_INPUT_CLIENT_START_FAIL;
    }

    return dInputSourceProxy_->StartRemoteInput(deviceId, inputTypes, callback);
}

int32_t DistributedInputClient::StopRemoteInput(
    const std::string& deviceId, const uint32_t& inputTypes, sptr<IStopDInputCallback> callback)
{
    DHLOGI("%s called, deviceId: %s, inputTypes: %d", __func__, GetAnonyString(deviceId).c_str(), inputTypes);

    if (!GetDInputSourceProxy()) {
        DHLOGE("StopRemoteInput client fail");
        return ERR_DH_INPUT_CLIENT_GET_SOURCE_PROXY_FAIL;
    }

    if (deviceId.empty() || callback == nullptr ||
        inputTypes > INPUT_TYPE_ALL || inputTypes == INPUT_TYPE_NULL || !(inputTypes & INPUT_TYPE_ALL)) {
        return ERR_DH_INPUT_CLIENT_STOP_FAIL;
    }

    return dInputSourceProxy_->StopRemoteInput(deviceId, inputTypes, callback);
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
    DHLOGI("%s called, inputType: %d, inputTypes: %d, ", __func__, inputType, inputTypes);
    int32_t retSource = 0;
    int32_t retSink = 0;

    if (sourceTypeCallback == nullptr && GetDInputSourceProxy()) {
        sourceTypeCallback = new(std::nothrow) StartDInputServerCb();
        retSource = dInputSourceProxy_->IsStartDistributedInput(inputType, sourceTypeCallback);
    }

    if (sinkTypeCallback == nullptr && GetDInputSinkProxy()) {
        sinkTypeCallback = new(std::nothrow) StartDInputServerCb();
        retSink = dInputSinkProxy_->IsStartDistributedInput(inputType, sinkTypeCallback);
    }

    if (static_cast<DInputServerType>(retSource) != DInputServerType::NULL_SERVER_TYPE) {
        serverType = DInputServerType::SOURCE_SERVER_TYPE;
    } else if (static_cast<DInputServerType>(retSink) != DInputServerType::NULL_SERVER_TYPE) {
        serverType = DInputServerType::SINK_SERVER_TYPE;
    }

    if (inputType & inputTypes) {
        return serverType;
    } else {
        return DInputServerType::NULL_SERVER_TYPE;
    }
}

bool DistributedInputClient::GetDInputSourceProxy()
{
    if (!dInputSourceProxy_) {
        std::lock_guard<std::mutex> lock(mutex_);
        sptr<ISystemAbilityManager> systemAbilityManager =
            SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();

        if (!systemAbilityManager) {
            return false;
        }
        DHLOGI("%s try get sa: %d", __func__, DISTRIBUTED_HARDWARE_INPUT_SOURCE_SA_ID);
        sptr<IRemoteObject> remoteObject = systemAbilityManager->GetSystemAbility(
            DISTRIBUTED_HARDWARE_INPUT_SOURCE_SA_ID);
        if (!remoteObject) {
            return false;
        }

        dInputSourceProxy_ = iface_cast<IDistributedSourceInput>(remoteObject);

        if ((!dInputSourceProxy_) || (!dInputSourceProxy_->AsObject())) {
            return false;
        }
    }
    return true;
}

bool DistributedInputClient::HasDInputSourceProxy()
{
    return dInputSourceProxy_ != nullptr;
}

bool DistributedInputClient::SetDInputSourceProxy(const sptr<IRemoteObject> &remoteObject)
{
    dInputSourceProxy_ = iface_cast<IDistributedSourceInput>(remoteObject);

    if ((!dInputSourceProxy_) || (!dInputSourceProxy_->AsObject())) {
        DHLOGE("Failed to get dinput source proxy.");
        return false;
    }
    return true;
}

bool DistributedInputClient::HasDInputSinkProxy()
{
    return dInputSinkProxy_ != nullptr;
}

bool DistributedInputClient::SetDInputSinkProxy(const sptr<IRemoteObject> &remoteObject)
{
    dInputSinkProxy_ = iface_cast<IDistributedSinkInput>(remoteObject);

    if ((!dInputSinkProxy_) || (!dInputSinkProxy_->AsObject())) {
        DHLOGE("Failed to get dinput sink proxy.");
        return false;
    }
    return true;
}

bool DistributedInputClient::GetDInputSinkProxy()
{
    if (!dInputSinkProxy_) {
        std::lock_guard<std::mutex> lock(mutex_);
        sptr<ISystemAbilityManager> systemAbilityManager =
            SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
        if (!systemAbilityManager) {
            return false;
        }

        sptr<IRemoteObject> remoteObject = systemAbilityManager->GetSystemAbility(
            DISTRIBUTED_HARDWARE_INPUT_SINK_SA_ID);
        if (!remoteObject) {
            return false;
        }

        dInputSinkProxy_ = iface_cast<IDistributedSinkInput>(remoteObject);

        if ((!dInputSinkProxy_) || (!dInputSinkProxy_->AsObject())) {
            return false;
        }
    }
    return true;
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
