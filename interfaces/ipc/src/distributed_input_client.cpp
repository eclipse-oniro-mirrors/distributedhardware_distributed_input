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
#include "dinput_context.h"
#include "dinput_errcode.h"
#include "dinput_utils_tool.h"
#include "distributed_input_source_proxy.h"
#include "softbus_bus_center.h"
#include "white_list_util.h"

namespace OHOS {
namespace DistributedHardware {
namespace DistributedInput {
std::shared_ptr<DistributedInputClient> DistributedInputClient::instance(new DistributedInputClient());
DistributedInputClient::DistributedInputClient() : isAddWhiteListCbReg(false), isDelWhiteListCbReg(false),
                                                   isNodeMonitorCbReg(false), isNodeMonitorCbUnreg(false),
                                                   isSimulationEventCbReg(false), isSimulationEventCbUnreg(false)
{
    DHLOGI("DistributedInputClient init start");
    std::shared_ptr<AppExecFwk::EventRunner> runner = AppExecFwk::EventRunner::Create(true);
    eventHandler_ = std::make_shared<DistributedInputClient::DInputClientEventHandler>(runner);
    DInputSAManager::GetInstance().RegisterEventHandler(eventHandler_);
    DInputSAManager::GetInstance().Init();
    DHLOGI("DistributedInputClient init end.");
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
    DHLOGI("StartDInputServerCb status: %d, inputTypes: %d, addr: %p", status, inputTypes, this);
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

DistributedInputClient::DInputClientEventHandler::DInputClientEventHandler(
    const std::shared_ptr<AppExecFwk::EventRunner> &runner)
    : AppExecFwk::EventHandler(runner)
{
}

void DistributedInputClient::DInputClientEventHandler::ProcessEvent(const AppExecFwk::InnerEvent::Pointer &event)
{
    uint32_t eventId = event->GetInnerEventId();
    DHLOGI("DInputClientEventHandler ProcessEvent start eventId:%d.", eventId);
    if (eventId == DINPUT_CLIENT_CHECK_CALLBACK_REGISTER_MSG) {
        DistributedInputClient::GetInstance().CheckRegisterCallback();
    } else {
        DHLOGE("DInputClientEventHandler ProcessEvent error, because eventId is unkonwn.");
    }
    DHLOGI("DInputClientEventHandler ProcessEvent end");
}

void DistributedInputClient::CheckRegisterCallback()
{
    DHLOGI("CheckRegisterCallback called, awl[%d], dwl[%d], rnm[%d], unm[%d], rmd[%d], umd[%d]",
           isAddWhiteListCbReg, isDelWhiteListCbReg, isNodeMonitorCbReg, isNodeMonitorCbUnreg, isSimulationEventCbReg,
           isSimulationEventCbUnreg);

    CheckWhiteListCallback();
    CheckNodeMonitorCallback();
    CheckKeyStateCallback();
}

void DistributedInputClient::CheckWhiteListCallback()
{
    if (!DInputSAManager::GetInstance().GetDInputSourceProxy()) {
        DHLOGE("CheckWhiteListCallback client get source proxy fail");
        return;
    }
    if (!isAddWhiteListCbReg) {
        if (addWhiteListCallback_ == nullptr) {
            addWhiteListCallback_ = new (std::nothrow) AddWhiteListInfosCb();
        }
        int32_t ret =
            DInputSAManager::GetInstance().dInputSourceProxy_->RegisterAddWhiteListCallback(addWhiteListCallback_);
        if (ret == DH_SUCCESS) {
            isAddWhiteListCbReg = true;
        } else {
            DHLOGE("CheckWhiteListCallback client RegisterAddWhiteListCallback fail");
        }
    }
    if (!isDelWhiteListCbReg) {
        if (delWhiteListCallback_ == nullptr) {
            delWhiteListCallback_ = new (std::nothrow) DelWhiteListInfosCb();
        }
        int32_t ret =
            DInputSAManager::GetInstance().dInputSourceProxy_->RegisterDelWhiteListCallback(delWhiteListCallback_);
        if (ret == DH_SUCCESS) {
            isDelWhiteListCbReg = true;
        } else {
            DHLOGE("CheckWhiteListCallback client RegisterDelWhiteListCallback fail");
        }
    }
}

void DistributedInputClient::CheckNodeMonitorCallback()
{
    if (!DInputSAManager::GetInstance().GetDInputSourceProxy()) {
        DHLOGE("CheckNodeMonitorCallback client get source proxy fail");
        return;
    }
    if (!isNodeMonitorCbReg && regNodeListener_ != nullptr) {
        DHLOGI("CheckNodeMonitorCallback need continue register regNodeListener_.");
        DInputSAManager::GetInstance().dInputSourceProxy_->RegisterInputNodeListener(regNodeListener_);
        isNodeMonitorCbReg = true;
    }

    if (!isNodeMonitorCbUnreg && unregNodeListener_ != nullptr) {
        DHLOGI("CheckNodeMonitorCallback need continue register unregNodeListener_.");
        DInputSAManager::GetInstance().dInputSourceProxy_->UnregisterInputNodeListener(unregNodeListener_);
        isNodeMonitorCbUnreg = true;
    }
}

void DistributedInputClient::CheckKeyStateCallback()
{
    if (!DInputSAManager::GetInstance().GetDInputSourceProxy()) {
        DHLOGE("CheckKeyStateCallback client get source proxy fail");
        return;
    }
    if (!isSimulationEventCbReg && regSimulationEventListener_ != nullptr) {
        DInputSAManager::GetInstance().dInputSourceProxy_->RegisterSimulationEventListener(regSimulationEventListener_);
        isSimulationEventCbReg = true;
    }

    if (!isSimulationEventCbUnreg && unregSimulationEventListener_ != nullptr) {
        DInputSAManager::GetInstance().dInputSourceProxy_->UnregisterSimulationEventListener(
            unregSimulationEventListener_);
        isSimulationEventCbUnreg = true;
    }
}

int32_t DistributedInputClient::InitSource()
{
    if (!DInputSAManager::GetInstance().GetDInputSourceProxy()) {
        return ERR_DH_INPUT_CLIENT_GET_SOURCE_PROXY_FAIL;
    }
    return DInputSAManager::GetInstance().dInputSourceProxy_->Init();
}

int32_t DistributedInputClient::InitSink()
{
    if (!DInputSAManager::GetInstance().GetDInputSinkProxy()) {
        return ERR_DH_INPUT_CLIENT_GET_SINK_PROXY_FAIL;
    }
    return DInputSAManager::GetInstance().dInputSinkProxy_->Init();
}

int32_t DistributedInputClient::ReleaseSource()
{
    if (!DInputSAManager::GetInstance().GetDInputSourceProxy()) {
        return ERR_DH_INPUT_CLIENT_GET_SOURCE_PROXY_FAIL;
    }

    serverType = DInputServerType::NULL_SERVER_TYPE;
    inputTypes_ = DInputDeviceType::NONE;
    sinkTypeCallback = nullptr;
    sourceTypeCallback = nullptr;
    addWhiteListCallback_ = nullptr;
    delWhiteListCallback_ = nullptr;
    regNodeListener_ = nullptr;
    unregNodeListener_ = nullptr;
    regSimulationEventListener_ = nullptr;
    unregSimulationEventListener_ = nullptr;
    WhiteListUtil::GetInstance().ClearWhiteList();

    return DInputSAManager::GetInstance().dInputSourceProxy_->Release();
}

int32_t DistributedInputClient::ReleaseSink()
{
    if (!DInputSAManager::GetInstance().GetDInputSinkProxy()) {
        return ERR_DH_INPUT_CLIENT_GET_SINK_PROXY_FAIL;
    }
    serverType = DInputServerType::NULL_SERVER_TYPE;
    inputTypes_ = DInputDeviceType::NONE;
    sinkTypeCallback = nullptr;
    WhiteListUtil::GetInstance().ClearWhiteList();
    return DInputSAManager::GetInstance().dInputSinkProxy_->Release();
}

int32_t DistributedInputClient::RegisterDistributedHardware(const std::string& devId, const std::string& dhId,
    const std::string& parameters, const std::shared_ptr<RegisterCallback>& callback)
{
    DHLOGI("RegisterDistributedHardware called, deviceId: %s,  dhId: %s,  parameters: %s",
        GetAnonyString(devId).c_str(), GetAnonyString(dhId).c_str(), SetAnonyId(parameters).c_str());

    if (!DInputSAManager::GetInstance().GetDInputSourceProxy()) {
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

    return DInputSAManager::GetInstance().dInputSourceProxy_->RegisterDistributedHardware(devId, dhId, parameters,
        new(std::nothrow) RegisterDInputCb());
}

int32_t DistributedInputClient::UnregisterDistributedHardware(const std::string& devId, const std::string& dhId,
    const std::shared_ptr<UnregisterCallback>& callback)
{
    DHLOGI("UnregisterDistributedHardware called, deviceId: %s,  dhId: %s",
        GetAnonyString(devId).c_str(), GetAnonyString(dhId).c_str());

    if (!DInputSAManager::GetInstance().GetDInputSourceProxy()) {
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

    return DInputSAManager::GetInstance().dInputSourceProxy_->UnregisterDistributedHardware(devId, dhId,
        new(std::nothrow) UnregisterDInputCb());
}

int32_t DistributedInputClient::PrepareRemoteInput(
    const std::string& deviceId, sptr<IPrepareDInputCallback> callback)
{
    DHLOGI("PrepareRemoteInput called, deviceId: %s", GetAnonyString(deviceId).c_str());

    if (!DInputSAManager::GetInstance().GetDInputSourceProxy()) {
        DHLOGE("PrepareRemoteInput client fail");
        return ERR_DH_INPUT_CLIENT_GET_SOURCE_PROXY_FAIL;
    }

    if (deviceId.empty() || callback == nullptr) {
        return ERR_DH_INPUT_CLIENT_PREPARE_FAIL;
    }

    return DInputSAManager::GetInstance().dInputSourceProxy_->PrepareRemoteInput(deviceId, callback);
}

int32_t DistributedInputClient::UnprepareRemoteInput(const std::string& deviceId,
    sptr<IUnprepareDInputCallback> callback)
{
    DHLOGI("UnprepareRemoteInput called, deviceId: %s", GetAnonyString(deviceId).c_str());

    if (!DInputSAManager::GetInstance().GetDInputSourceProxy()) {
        DHLOGE("PrepareRemoteInput client fail");
        return ERR_DH_INPUT_CLIENT_GET_SOURCE_PROXY_FAIL;
    }

    if (deviceId.empty() || callback == nullptr) {
        DHLOGE("UnprepareRemoteInput param error, client fail");
        return ERR_DH_INPUT_CLIENT_UNPREPARE_FAIL;
    }

    return DInputSAManager::GetInstance().dInputSourceProxy_->UnprepareRemoteInput(deviceId, callback);
}

int32_t DistributedInputClient::StartRemoteInput(
    const std::string& deviceId, const uint32_t& inputTypes, sptr<IStartDInputCallback> callback)
{
    DHLOGI("StartRemoteInput called, deviceId: %s, inputTypes: %d",
        GetAnonyString(deviceId).c_str(), inputTypes);
    if (!DInputSAManager::GetInstance().GetDInputSourceProxy()) {
        DHLOGE("StartRemoteInput client fail");
        return ERR_DH_INPUT_CLIENT_GET_SOURCE_PROXY_FAIL;
    }

    if (deviceId.empty() || callback == nullptr ||
        inputTypes > static_cast<uint32_t>(DInputDeviceType::ALL) ||
        inputTypes == static_cast<uint32_t>(DInputDeviceType::NONE) ||
        !(inputTypes & static_cast<uint32_t>(DInputDeviceType::ALL))) {
        DHLOGE("StartRemoteInput param error, client fail");
        return ERR_DH_INPUT_CLIENT_START_FAIL;
    }
    return DInputSAManager::GetInstance().dInputSourceProxy_->StartRemoteInput(deviceId, inputTypes, callback);
}

int32_t DistributedInputClient::StopRemoteInput(const std::string& deviceId, const uint32_t& inputTypes,
    sptr<IStopDInputCallback> callback)
{
    DHLOGI("StopRemoteInput called, deviceId: %s, inputTypes: %d",
        GetAnonyString(deviceId).c_str(), inputTypes);
    if (!DInputSAManager::GetInstance().GetDInputSourceProxy()) {
        DHLOGE("StopRemoteInput client fail");
        return ERR_DH_INPUT_CLIENT_GET_SOURCE_PROXY_FAIL;
    }

    if (deviceId.empty() || callback == nullptr ||
        inputTypes > static_cast<uint32_t>(DInputDeviceType::ALL) ||
        inputTypes == static_cast<uint32_t>(DInputDeviceType::NONE) ||
        !(inputTypes & static_cast<uint32_t>(DInputDeviceType::ALL))) {
        DHLOGE("StopRemoteInput param error, client fail");
        return ERR_DH_INPUT_CLIENT_STOP_FAIL;
    }

    return DInputSAManager::GetInstance().dInputSourceProxy_->StopRemoteInput(deviceId, inputTypes, callback);
}

int32_t DistributedInputClient::StartRemoteInput(const std::string &srcId, const std::string &sinkId,
    const uint32_t &inputTypes, sptr<IStartDInputCallback> callback)
{
    DHLOGI("StartRemoteInput relay by type called, srcId: %s, sinkId: %s, inputTypes: %d",
        GetAnonyString(srcId).c_str(), GetAnonyString(sinkId).c_str(), inputTypes);

    if (!DInputSAManager::GetInstance().GetDInputSourceProxy()) {
        DHLOGE("StartRemoteInput relay type client fail");
        return ERR_DH_INPUT_CLIENT_GET_SOURCE_PROXY_FAIL;
    }

    if (srcId.empty() || sinkId.empty() || callback == nullptr ||
        inputTypes > static_cast<uint32_t>(DInputDeviceType::ALL) ||
        inputTypes == static_cast<uint32_t>(DInputDeviceType::NONE) ||
        !(inputTypes & static_cast<uint32_t>(DInputDeviceType::ALL))) {
        DHLOGE("StartRemoteInput relay type param error, client fail");
        return ERR_DH_INPUT_CLIENT_START_FAIL;
    }

    return DInputSAManager::GetInstance().dInputSourceProxy_->StartRemoteInput(srcId, sinkId, inputTypes, callback);
}

int32_t DistributedInputClient::StopRemoteInput(const std::string &srcId, const std::string &sinkId,
    const uint32_t &inputTypes, sptr<IStopDInputCallback> callback)
{
    DHLOGI("StopRemoteInput relay by type called, srcId: %s, sinkId: %s, inputTypes: %d",
        GetAnonyString(srcId).c_str(), GetAnonyString(sinkId).c_str(), inputTypes);

    if (!DInputSAManager::GetInstance().GetDInputSourceProxy()) {
        DHLOGE("StopRemoteInput relay type client fail");
        return ERR_DH_INPUT_CLIENT_GET_SOURCE_PROXY_FAIL;
    }

    if (srcId.empty() || sinkId.empty() || callback == nullptr ||
        inputTypes > static_cast<uint32_t>(DInputDeviceType::ALL) ||
        inputTypes == static_cast<uint32_t>(DInputDeviceType::NONE) ||
        !(inputTypes & static_cast<uint32_t>(DInputDeviceType::ALL))) {
        DHLOGE("StopRemoteInput relay type param error, client fail");
        return ERR_DH_INPUT_CLIENT_STOP_FAIL;
    }

    return DInputSAManager::GetInstance().dInputSourceProxy_->StopRemoteInput(srcId, sinkId, inputTypes, callback);
}

int32_t DistributedInputClient::PrepareRemoteInput(const std::string &srcId, const std::string &sinkId,
    sptr<IPrepareDInputCallback> callback)
{
    DHLOGI("PrepareRemoteInput relay called, srcId: %s, sinkId: %s",
        GetAnonyString(srcId).c_str(), GetAnonyString(sinkId).c_str());
    if (!DInputSAManager::GetInstance().GetDInputSourceProxy()) {
        DHLOGE("PrepareRemoteInput relay proxy error, client fail");
        return ERR_DH_INPUT_CLIENT_GET_SOURCE_PROXY_FAIL;
    }
    if (srcId.empty() || sinkId.empty() || callback == nullptr) {
        DHLOGE("PrepareRemoteInput relay param error, client fail");
        return ERR_DH_INPUT_CLIENT_PREPARE_FAIL;
    }
    return DInputSAManager::GetInstance().dInputSourceProxy_->PrepareRemoteInput(srcId, sinkId, callback);
}

int32_t DistributedInputClient::UnprepareRemoteInput(const std::string &srcId, const std::string &sinkId,
    sptr<IUnprepareDInputCallback> callback)
{
    DHLOGI("UnprepareRemoteInput relay called, srcId: %s, sinkId: %s",
        GetAnonyString(srcId).c_str(), GetAnonyString(sinkId).c_str());

    if (!DInputSAManager::GetInstance().GetDInputSourceProxy()) {
        DHLOGE("UnprepareRemoteInput relay proxy error, client fail");
        return ERR_DH_INPUT_CLIENT_GET_SOURCE_PROXY_FAIL;
    }

    if (srcId.empty() || sinkId.empty() || callback == nullptr) {
        DHLOGE("UnprepareRemoteInput relay param error, client fail");
        return ERR_DH_INPUT_CLIENT_UNPREPARE_FAIL;
    }
    return DInputSAManager::GetInstance().dInputSourceProxy_->UnprepareRemoteInput(srcId, sinkId, callback);
}

int32_t DistributedInputClient::StartRemoteInput(const std::string &sinkId, const std::vector<std::string> &dhIds,
    sptr<IStartStopDInputsCallback> callback)
{
    DHLOGI("StartRemoteInput relay by dhid called, sinkId: %s", GetAnonyString(sinkId).c_str());

    if (!DInputSAManager::GetInstance().GetDInputSourceProxy()) {
        DHLOGE("StartRemoteInput dhid proxy error, client fail");
        return ERR_DH_INPUT_CLIENT_GET_SOURCE_PROXY_FAIL;
    }

    if (sinkId.empty() || callback == nullptr || (dhIds.size() <= 0)) {
        DHLOGE("StartRemoteInput dhid param error, client fail");
        return ERR_DH_INPUT_CLIENT_START_FAIL;
    }

    return DInputSAManager::GetInstance().dInputSourceProxy_->StartRemoteInput(sinkId, dhIds, callback);
}

int32_t DistributedInputClient::StopRemoteInput(const std::string &sinkId, const std::vector<std::string> &dhIds,
    sptr<IStartStopDInputsCallback> callback)
{
    DHLOGI("StopRemoteInput relay by dhid called, sinkId: %s", GetAnonyString(sinkId).c_str());

    if (!DInputSAManager::GetInstance().GetDInputSourceProxy()) {
        DHLOGE("StopRemoteInput dhid proxy error, client fail");
        return ERR_DH_INPUT_CLIENT_GET_SOURCE_PROXY_FAIL;
    }

    if (sinkId.empty() || callback == nullptr || (dhIds.size() <= 0)) {
        DHLOGE("StopRemoteInput dhid param error, client fail");
        return ERR_DH_INPUT_CLIENT_STOP_FAIL;
    }

    return DInputSAManager::GetInstance().dInputSourceProxy_->StopRemoteInput(sinkId, dhIds, callback);
}

int32_t DistributedInputClient::StartRemoteInput(const std::string &srcId, const std::string &sinkId,
    const std::vector<std::string> &dhIds, sptr<IStartStopDInputsCallback> callback)
{
    DHLOGI("StartRemoteInput relay by dhid called, srcId: %s, sinkId: %s",
        GetAnonyString(srcId).c_str(), GetAnonyString(sinkId).c_str());

    if (!DInputSAManager::GetInstance().GetDInputSourceProxy()) {
        DHLOGE("StartRemoteInput proxy error, client fail");
        return ERR_DH_INPUT_CLIENT_GET_SOURCE_PROXY_FAIL;
    }

    if (srcId.empty() || sinkId.empty() || callback == nullptr || (dhIds.size() <= 0)) {
        DHLOGE("StartRemoteInput param error, client fail");
        return ERR_DH_INPUT_CLIENT_START_FAIL;
    }

    return DInputSAManager::GetInstance().dInputSourceProxy_->StartRemoteInput(srcId, sinkId, dhIds, callback);
}

int32_t DistributedInputClient::StopRemoteInput(const std::string &srcId, const std::string &sinkId,
    const std::vector<std::string> &dhIds, sptr<IStartStopDInputsCallback> callback)
{
    DHLOGI("StopRemoteInput relay by dhid called, srcId: %s, sinkId: %s",
        GetAnonyString(srcId).c_str(), GetAnonyString(sinkId).c_str());

    if (!DInputSAManager::GetInstance().GetDInputSourceProxy()) {
        DHLOGE("StopRemoteInput proxy error, client fail");
        return ERR_DH_INPUT_CLIENT_GET_SOURCE_PROXY_FAIL;
    }

    if (srcId.empty() || sinkId.empty() || callback == nullptr || (dhIds.size() <= 0)) {
        DHLOGE("StopRemoteInput param error, client fail");
        return ERR_DH_INPUT_CLIENT_STOP_FAIL;
    }

    return DInputSAManager::GetInstance().dInputSourceProxy_->StopRemoteInput(srcId, sinkId, dhIds, callback);
}

bool DistributedInputClient::IsNeedFilterOut(const std::string& deviceId, const BusinessEvent& event)
{
    DHLOGI("IsNeedFilterOut called, deviceId: %s", GetAnonyString(deviceId).c_str());
    return WhiteListUtil::GetInstance().IsNeedFilterOut(deviceId, event);
}

bool DistributedInputClient::IsTouchEventNeedFilterOut(const TouchScreenEvent &event)
{
    auto sinkInfos = DInputContext::GetInstance().GetAllSinkScreenInfo();

    for (const auto& [id, sinkInfo] : sinkInfos) {
        auto info = sinkInfo.transformInfo;
        if ((event.absX >= info.sinkWinPhyX) && (event.absX <= (info.sinkWinPhyX + info.sinkProjPhyWidth))
            && (event.absY >= info.sinkWinPhyY)  && (event.absY <= (info.sinkWinPhyY + info.sinkProjPhyHeight))) {
            return true;
        }
    }
    return false;
}

DInputServerType DistributedInputClient::IsStartDistributedInput(const uint32_t& inputType)
{
    DHLOGI("IsStartDistributedInput called, inputType: %d, current inputTypes: %d",
        inputType, static_cast<uint32_t>(inputTypes_));
    int32_t retSource = 0;
    int32_t retSink = 0;

    if (sourceTypeCallback == nullptr && DInputSAManager::GetInstance().GetDInputSourceProxy()) {
        DHLOGI("Init sourceTypeCallback");
        sourceTypeCallback = new(std::nothrow) StartDInputServerCb();
        retSource = DInputSAManager::GetInstance().dInputSourceProxy_->IsStartDistributedInput(inputType,
            sourceTypeCallback);
    }

    if (sinkTypeCallback == nullptr && DInputSAManager::GetInstance().GetDInputSinkProxy()) {
        DHLOGI("Init sinkTypeCallback");
        sinkTypeCallback = new(std::nothrow) StartDInputServerCb();
        retSink = DInputSAManager::GetInstance().dInputSinkProxy_->IsStartDistributedInput(inputType, sinkTypeCallback);
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

int32_t DistributedInputClient::RegisterInputNodeListener(sptr<InputNodeListener> listener)
{
    DHLOGI("RegisterInputNodeListener called");
    if (listener == nullptr) {
        DHLOGE("RegisterInputNodeListener param error, client fail");
        return ERR_DH_INPUT_CLIENT_REG_NODE_CB_FAIL;
    }
    if (!DInputSAManager::GetInstance().GetDInputSourceProxy()) {
        DHLOGE("RegisterInputNodeListener proxy error, client fail");
        isNodeMonitorCbReg = false;
        regNodeListener_ = listener;
        return ERR_DH_INPUT_CLIENT_GET_SOURCE_PROXY_FAIL;
    }

    int32_t ret = DInputSAManager::GetInstance().dInputSourceProxy_->RegisterInputNodeListener(listener);
    if (ret == DH_SUCCESS) {
        isNodeMonitorCbReg = true;
    } else {
        isNodeMonitorCbReg = false;
        regNodeListener_ = listener;
        DHLOGE("RegisterInputNodeListener Failed, ret = %d", ret);
    }
    return ret;
}

int32_t DistributedInputClient::UnregisterInputNodeListener(sptr<InputNodeListener> listener)
{
    DHLOGI("UnregisterInputNodeListener called");
    if (listener == nullptr) {
        DHLOGE("UnregisterInputNodeListener param error, client fail");
        return ERR_DH_INPUT_CLIENT_UNREG_NODE_CB_FAIL;
    }
    if (!DInputSAManager::GetInstance().GetDInputSourceProxy()) {
        DHLOGE("UnregisterInputNodeListener proxy error, client fail");
        isNodeMonitorCbUnreg = false;
        unregNodeListener_ = listener;
        return ERR_DH_INPUT_CLIENT_GET_SOURCE_PROXY_FAIL;
    }

    int32_t ret = DInputSAManager::GetInstance().dInputSourceProxy_->UnregisterInputNodeListener(listener);
    if (ret == DH_SUCCESS) {
        isNodeMonitorCbUnreg = true;
    } else {
        isNodeMonitorCbUnreg = false;
        unregNodeListener_ = listener;
        DHLOGE("DInputSAManager UnregisterInputNodeListener Failed, ret = %d", ret);
    }
    return ret;
}

int32_t DistributedInputClient::RegisterSimulationEventListener(sptr<ISimulationEventListener> listener)
{
    DHLOGI("RegisterSimulationEventListener called Simulation Event Listener Register.");
    if (listener == nullptr) {
        DHLOGE("RegisterSimulationEventListener param error");
        return ERR_DH_INPUT_CLIENT_REG_UNREG_KEY_STATE_FAIL;
    }
    if (!DInputSAManager::GetInstance().GetDInputSourceProxy()) {
        DHLOGE("RegisterSimulationEventListener proxy error, client fail");
        isSimulationEventCbReg = false;
        regSimulationEventListener_ = listener;
        return ERR_DH_INPUT_CLIENT_GET_SOURCE_PROXY_FAIL;
    }

    int32_t ret = DInputSAManager::GetInstance().dInputSourceProxy_->RegisterSimulationEventListener(listener);
    if (ret == DH_SUCCESS) {
        isSimulationEventCbReg = true;
    } else {
        isSimulationEventCbReg = false;
        regSimulationEventListener_ = listener;
        DHLOGE("RegisterSimulationEventListener Failed, ret = %d", ret);
    }
    return ret;
}

int32_t DistributedInputClient::UnregisterSimulationEventListener(sptr<ISimulationEventListener> listener)
{
    DHLOGI("UnregisterSimulationEventListener called Simulation Event Listener UnRegister.");
    if (listener == nullptr) {
        DHLOGE("UnregisterSimulationEventListener param error");
        return ERR_DH_INPUT_CLIENT_REG_UNREG_KEY_STATE_FAIL;
    }
    if (!DInputSAManager::GetInstance().GetDInputSourceProxy()) {
        DHLOGE("UnregisterSimulationEventListener proxy error, client fail");
        isSimulationEventCbUnreg = false;
        unregSimulationEventListener_ = listener;
        return ERR_DH_INPUT_CLIENT_GET_SOURCE_PROXY_FAIL;
    }

    int32_t ret = DInputSAManager::GetInstance().dInputSourceProxy_->UnregisterSimulationEventListener(listener);
    if (ret == DH_SUCCESS) {
        isSimulationEventCbUnreg = true;
    } else {
        isSimulationEventCbUnreg = false;
        unregSimulationEventListener_ = listener;
        DHLOGE("UnregisterSimulationEventListener Failed, ret = %d", ret);
    }
    return ret;
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

void DistributedInputClient::AddWhiteListInfos(const std::string &deviceId, const std::string &strJson) const
{
    nlohmann::json inputData = nlohmann::json::parse(strJson);
    size_t jsonSize = inputData.size();
    DHLOGI("AddWhiteListInfosCb OnResult deviceId: %s, json str: %s, json size:%d.\n",
        GetAnonyString(deviceId).c_str(), GetAnonyString(strJson).c_str(), jsonSize);
    TYPE_WHITE_LIST_VEC vecWhiteList = inputData;
    WhiteListUtil::GetInstance().SyncWhiteList(deviceId, vecWhiteList);
}

void DistributedInputClient::DelWhiteListInfos(const std::string &deviceId) const
{
    WhiteListUtil::GetInstance().ClearWhiteList(deviceId);
}

int32_t DistributedInputClient::NotifyStartDScreen(const std::string &sinkDevId, const std::string& srcDevId,
    const uint64_t srcWinId)
{
    sptr<IDistributedSinkInput> remoteDInput = GetRemoteDInput(sinkDevId);
    if (remoteDInput == nullptr || !remoteDInput->AsObject()) {
        DHLOGE("GetRemoteDInput failed, networkId = %s", GetAnonyString(sinkDevId).c_str());
        return ERR_DH_INPUT_RPC_GET_REMOTE_DINPUT_FAIL;
    }
    std::string srcScreenInfoKey = DInputContext::GetInstance().GetScreenInfoKey(srcDevId, srcWinId);
    SrcScreenInfo srcScreenInfo = DInputContext::GetInstance().GetSrcScreenInfo(srcScreenInfoKey);
    DHLOGI("DistributedInputSinkProxy the data: devId: %s, sourceWinId: %d, sourceWinWidth: %d, sourceWinHeight: %d,"
        "sourcePhyId: %s, sourcePhyFd: %d, sourcePhyWidth: %d, sourcePhyHeight: %d",
        GetAnonyString(srcScreenInfo.devId).c_str(), srcScreenInfo.sourceWinId, srcScreenInfo.sourceWinWidth,
        srcScreenInfo.sourceWinHeight, GetAnonyString(srcScreenInfo.sourcePhyId).c_str(), srcScreenInfo.sourcePhyFd,
        srcScreenInfo.sourcePhyWidth, srcScreenInfo.sourcePhyHeight);
    auto ret = remoteDInput->NotifyStartDScreen(srcScreenInfo);
    DHLOGI("NotifyStartDScreen, retCode = %d", ret);
    if (ret != DH_SUCCESS) {
        DHLOGE("NotifyStartDScreen failed, errCode = %d", ret);
    }
    return ret;
}

int32_t DistributedInputClient::NotifyStopDScreen(const std::string &networkId, const std::string& srcScreenInfoKey)
{
    sptr<IDistributedSinkInput> remoteDInput = GetRemoteDInput(networkId);
    if (remoteDInput == nullptr || !remoteDInput->AsObject()) {
        DHLOGE("GetRemoteDInput failed, networkId = %s", GetAnonyString(networkId).c_str());
        return ERR_DH_INPUT_RPC_GET_REMOTE_DINPUT_FAIL;
    }
    auto ret = remoteDInput->NotifyStopDScreen(srcScreenInfoKey);
    DHLOGI("NotifyStopDScreen, retCode = %d", ret);
    if (ret != DH_SUCCESS) {
        DHLOGE("NotifyStopDScreen failed, errCode = %d", ret);
    }
    return ret;
}

sptr<IDistributedSinkInput> DistributedInputClient::GetRemoteDInput(const std::string &networkId) const
{
    DHLOGI("GetRemoteDInput start, networkId = %s", GetAnonyString(networkId).c_str());
    if (networkId.empty()) {
        DHLOGE("networkId is empty");
        return nullptr;
    }
    auto samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (samgr == nullptr) {
        DHLOGE("GetSystemAbilityManager failed");
        return nullptr;
    }
    auto object = samgr->CheckSystemAbility(DISTRIBUTED_HARDWARE_INPUT_SINK_SA_ID, networkId);
    if (object == nullptr) {
        DHLOGE("CheckSystemAbility failed");
        return nullptr;
    }
    return iface_cast<IDistributedSinkInput>(object);
}
} // namespace DistributedInput
} // namespace DistributedHardware
} // namespace OHOS
