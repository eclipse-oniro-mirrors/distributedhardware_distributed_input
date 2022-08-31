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

#include "distributed_input_source_manager.h"

#include <cinttypes>
#include <dlfcn.h>
#include <fstream>

#include "anonymous_string.h"
#include "distributed_hardware_log.h"
#include "dinput_softbus_define.h"
#include "if_system_ability_manager.h"
#include "iservice_registry.h"
#include "nlohmann/json.hpp"
#include "system_ability_definition.h"
#include "string_ex.h"
#include "softbus_bus_center.h"

#include "constants_dinput.h"
#include "dinput_errcode.h"
#include "dinput_hitrace.h"
#include "dinput_sa_process_state.h"
#include "dinput_utils_tool.h"
#include "distributed_hardware_fwk_kit.h"
#include "distributed_input_client.h"
#include "distributed_input_inject.h"
#include "distributed_input_source_proxy.h"
#include "distributed_input_source_transport.h"
#include "hisysevent_util.h"
#include "hidumper.h"
#include "ipublisher_listener.h"
#include "idinput_dbg_itf.h"
#include "white_list_util.h"

#ifdef __LP64__
const std::string LIB_LOAD_PATH = "/system/lib64/libdinput_dbg_itf.z.so";
#else
const std::string LIB_LOAD_PATH = "/system/lib/libdinput_dbg_itf.z.so";
#endif

using GetDInputDBGItfFunc = OHOS::DistributedHardware::DistributedInput::IDInputDBGItf* (*)();

namespace OHOS {
namespace DistributedHardware {
namespace DistributedInput {
REGISTER_SYSTEM_ABILITY_BY_ID(DistributedInputSourceManager, DISTRIBUTED_HARDWARE_INPUT_SOURCE_SA_ID, true);

DistributedInputSourceManager::DistributedInputSourceManager(int32_t saId, bool runOnCreate)
    : SystemAbility(saId, runOnCreate)
{
    DHLOGI("DistributedInputSourceManager ctor!");
}

DistributedInputSourceManager::~DistributedInputSourceManager()
{
    DHLOGI("DistributedInputSourceManager dtor!");
    startDScreenListener_ = nullptr;
    stopDScreenListener_ = nullptr;
}

DistributedInputSourceManager::DInputSourceListener::DInputSourceListener(DistributedInputSourceManager *manager)
{
    sourceManagerObj_ = manager;
    DHLOGI("DInputSourceListener init.");
}

DistributedInputSourceManager::DInputSourceListener::~DInputSourceListener()
{
    sourceManagerObj_ = nullptr;
    DHLOGI("DInputSourceListener destory.");
}

void DistributedInputSourceManager::DInputSourceListener::onResponseRegisterDistributedHardware(
    const std::string deviceId, const std::string dhId, bool result)
{
    DHLOGI("onResponseRegisterDistributedHardware called, deviceId: %s, "
        "result: %s.", GetAnonyString(deviceId).c_str(), result);
    if (sourceManagerObj_ == nullptr) {
        DHLOGE("onResponseRegisterDistributedHardware sourceManagerObj_ is null.");
        return;
    }
    if (sourceManagerObj_->GetCallbackEventHandler() == nullptr) {
        sourceManagerObj_->RunRegisterCallback(deviceId, dhId,
            ERR_DH_INPUT_SERVER_SOURCE_MANAGERGET_CALLBACK_HANDLER_FAIL);
        DHLOGE("onResponseRegisterDistributedHardware GetCallbackEventHandler is null.");
        return;
    }

    std::shared_ptr<nlohmann::json> jsonArrayMsg = std::make_shared<nlohmann::json>();

    nlohmann::json tmpJson;
    tmpJson[INPUT_SOURCEMANAGER_KEY_DEVID] = deviceId;
    tmpJson[INPUT_SOURCEMANAGER_KEY_HWID] = dhId;
    tmpJson[INPUT_SOURCEMANAGER_KEY_RESULT] = result;
    jsonArrayMsg->push_back(tmpJson);
    AppExecFwk::InnerEvent::Pointer msgEvent = AppExecFwk::InnerEvent::Get(
        DINPUT_SOURCE_MANAGER_RIGISTER_MSG, jsonArrayMsg, 0);
    sourceManagerObj_->GetCallbackEventHandler()->SendEvent(msgEvent, 0, AppExecFwk::EventQueue::Priority::IMMEDIATE);
}

void DistributedInputSourceManager::DInputSourceListener::onResponsePrepareRemoteInput(const std::string deviceId,
    bool result, const std::string &object)
{
    DHLOGI("onResponsePrepareRemoteInput called, deviceId: %s, result: %d.",
        GetAnonyString(deviceId).c_str(), result);

    if (sourceManagerObj_ == nullptr) {
        DHLOGE("onResponsePrepareRemoteInput sourceManagerObj_ is null.");
        return;
    }
    if (sourceManagerObj_->GetCallbackEventHandler() == nullptr) {
        sourceManagerObj_->RunPrepareCallback(deviceId,
            ERR_DH_INPUT_SERVER_SOURCE_MANAGERGET_CALLBACK_HANDLER_FAIL, object);
        DHLOGE("onResponsePrepareRemoteInput GetCallbackEventHandler is null.");
        return;
    }
    std::shared_ptr<nlohmann::json> jsonArrayMsg = std::make_shared<nlohmann::json>();

    nlohmann::json tmpJson;
    tmpJson[INPUT_SOURCEMANAGER_KEY_DEVID] = deviceId;
    tmpJson[INPUT_SOURCEMANAGER_KEY_RESULT] = result;
    tmpJson[INPUT_SOURCEMANAGER_KEY_WHITELIST] = object;
    jsonArrayMsg->push_back(tmpJson);
    AppExecFwk::InnerEvent::Pointer msgEvent = AppExecFwk::InnerEvent::Get(
        DINPUT_SOURCE_MANAGER_PREPARE_MSG, jsonArrayMsg, 0);
    sourceManagerObj_->GetCallbackEventHandler()->SendEvent(msgEvent, 0, AppExecFwk::EventQueue::Priority::IMMEDIATE);
}

void DistributedInputSourceManager::DInputSourceListener::onResponseUnprepareRemoteInput(
    const std::string deviceId, bool result)
{
    DHLOGI("onResponseUnprepareRemoteInput called, deviceId: %s, "
        "result: %d.", GetAnonyString(deviceId).c_str(), result);

    if (sourceManagerObj_ == nullptr) {
        DHLOGE("onResponseUnprepareRemoteInput sourceManagerObj_ is null.");
        return;
    }
    if (sourceManagerObj_->GetCallbackEventHandler() == nullptr) {
        sourceManagerObj_->RunUnprepareCallback(deviceId,
            ERR_DH_INPUT_SERVER_SOURCE_MANAGERGET_CALLBACK_HANDLER_FAIL);
        DHLOGE("onResponseUnprepareRemoteInput GetCallbackEventHandler is null.");
        return;
    }
    std::shared_ptr<nlohmann::json> jsonArrayMsg = std::make_shared<nlohmann::json>();

    nlohmann::json tmpJson;
    tmpJson[INPUT_SOURCEMANAGER_KEY_DEVID] = deviceId;
    tmpJson[INPUT_SOURCEMANAGER_KEY_RESULT] = result;
    jsonArrayMsg->push_back(tmpJson);
    AppExecFwk::InnerEvent::Pointer msgEvent = AppExecFwk::InnerEvent::Get(
        DINPUT_SOURCE_MANAGER_UNPREPARE_MSG, jsonArrayMsg, 0);
    sourceManagerObj_->GetCallbackEventHandler()->SendEvent(msgEvent, 0, AppExecFwk::EventQueue::Priority::IMMEDIATE);
}

void DistributedInputSourceManager::DInputSourceListener::onResponseStartRemoteInput(
    const std::string deviceId, const uint32_t inputTypes, bool result)
{
    DHLOGI("onResponseStartRemoteInput called, deviceId: %s, inputTypes: %d, result: %d.",
        GetAnonyString(deviceId).c_str(), inputTypes, result);

    if (sourceManagerObj_ == nullptr) {
        DHLOGE("onResponseStartRemoteInput sourceManagerObj_ is null.");
        return;
    }
    if (sourceManagerObj_->GetCallbackEventHandler() == nullptr) {
        sourceManagerObj_->RunStartCallback(deviceId, inputTypes,
            ERR_DH_INPUT_SERVER_SOURCE_MANAGERGET_CALLBACK_HANDLER_FAIL);
        DHLOGE("onResponseStartRemoteInput GetCallbackEventHandler is null.");
        return;
    }
    if (result) {
        sourceManagerObj_->SetDeviceMapValue(deviceId, DINPUT_SOURCE_SWITCH_ON);
    }

    std::shared_ptr<nlohmann::json> jsonArrayMsg = std::make_shared<nlohmann::json>();
    if (jsonArrayMsg == nullptr) {
        DHLOGE("onResponseStartRemoteInput jsonArrayMsg is null.");
        return;
    }

    nlohmann::json tmpJson;
    tmpJson[INPUT_SOURCEMANAGER_KEY_DEVID] = deviceId;
    tmpJson[INPUT_SOURCEMANAGER_KEY_ITP] = inputTypes;
    tmpJson[INPUT_SOURCEMANAGER_KEY_RESULT] = result;
    jsonArrayMsg->push_back(tmpJson);
    AppExecFwk::InnerEvent::Pointer msgEvent = AppExecFwk::InnerEvent::Get(
        DINPUT_SOURCE_MANAGER_START_MSG, jsonArrayMsg, 0);
    sourceManagerObj_->GetCallbackEventHandler()->SendEvent(msgEvent, 0, AppExecFwk::EventQueue::Priority::IMMEDIATE);
}

void DistributedInputSourceManager::DInputSourceListener::onResponseStopRemoteInput(
    const std::string deviceId, const uint32_t inputTypes, bool result)
{
    DHLOGI("onResponseStopRemoteInput called, deviceId: %s, inputTypes: %d, result: %d.",
        GetAnonyString(deviceId).c_str(), inputTypes, result);

    if (sourceManagerObj_ == nullptr) {
        DHLOGE("onResponseStopRemoteInput sourceManagerObj_ is null.");
        return;
    }
    if (sourceManagerObj_->GetCallbackEventHandler() == nullptr) {
        DHLOGE("onResponseStopRemoteInput GetCallbackEventHandler is null.");
        sourceManagerObj_->RunStopCallback(deviceId, inputTypes,
            ERR_DH_INPUT_SERVER_SOURCE_MANAGERGET_CALLBACK_HANDLER_FAIL);
        return;
    }
    std::shared_ptr<nlohmann::json> jsonArrayMsg = std::make_shared<nlohmann::json>();

    nlohmann::json tmpJson;
    tmpJson[INPUT_SOURCEMANAGER_KEY_DEVID] = deviceId;
    tmpJson[INPUT_SOURCEMANAGER_KEY_ITP] = inputTypes;
    tmpJson[INPUT_SOURCEMANAGER_KEY_RESULT] = result;
    jsonArrayMsg->push_back(tmpJson);
    AppExecFwk::InnerEvent::Pointer msgEvent = AppExecFwk::InnerEvent::Get(
        DINPUT_SOURCE_MANAGER_STOP_MSG, jsonArrayMsg, 0);
    sourceManagerObj_->GetCallbackEventHandler()->SendEvent(msgEvent, 0, AppExecFwk::EventQueue::Priority::IMMEDIATE);
}

void DistributedInputSourceManager::DInputSourceListener::onResponseStartRemoteInputDhid(
    const std::string deviceId, const std::string &dhids, bool result)
{
    DHLOGI("onResponseStartRemoteInputDhid called, deviceId: %s, result: %d.",
        GetAnonyString(deviceId).c_str(), result);

    if (sourceManagerObj_ == nullptr) {
        DHLOGE("onResponseStartRemoteInputDhid sourceManagerObj_ is null.");
        return;
    }
    if (sourceManagerObj_->GetCallbackEventHandler() == nullptr) {
        DHLOGE("onResponseStartRemoteInputDhid GetCallbackEventHandler is null.");
        sourceManagerObj_->RunStartDhidCallback(deviceId, dhids,
                                                ERR_DH_INPUT_SERVER_SOURCE_MANAGERGET_CALLBACK_HANDLER_FAIL);
        return;
    }
    if (result) {
        sourceManagerObj_->SetDeviceMapValue(deviceId, DINPUT_SOURCE_SWITCH_ON);
    }

    std::shared_ptr<nlohmann::json> jsonArrayMsg = std::make_shared<nlohmann::json>();
    if (jsonArrayMsg == nullptr) {
        DHLOGE("onResponseStartRemoteInputDhid jsonArrayMsg is null.");
        return;
    }

    nlohmann::json tmpJson;
    tmpJson[INPUT_SOURCEMANAGER_KEY_DEVID] = deviceId;
    tmpJson[INPUT_SOURCEMANAGER_KEY_DHID] = dhids;
    tmpJson[INPUT_SOURCEMANAGER_KEY_RESULT] = result;
    jsonArrayMsg->push_back(tmpJson);
    AppExecFwk::InnerEvent::Pointer msgEvent =
        AppExecFwk::InnerEvent::Get(DINPUT_SOURCE_MANAGER_START_DHID_MSG, jsonArrayMsg, 0);
    sourceManagerObj_->GetCallbackEventHandler()->SendEvent(msgEvent, 0, AppExecFwk::EventQueue::Priority::IMMEDIATE);
}

void DistributedInputSourceManager::DInputSourceListener::onResponseStopRemoteInputDhid(
    const std::string deviceId, const std::string &dhids, bool result)
{
    DHLOGI("onResponseStopRemoteInputDhid called, deviceId: %s, result: %d.",
        GetAnonyString(deviceId).c_str(), result);

    if (sourceManagerObj_ == nullptr) {
        DHLOGE("onResponseStopRemoteInputDhid sourceManagerObj_ is null.");
        return;
    }
    if (sourceManagerObj_->GetCallbackEventHandler() == nullptr) {
        DHLOGE("onResponseStopRemoteInputDhid GetCallbackEventHandler is null.");
        sourceManagerObj_->RunStopDhidCallback(deviceId, dhids,
                                               ERR_DH_INPUT_SERVER_SOURCE_MANAGERGET_CALLBACK_HANDLER_FAIL);
        return;
    }
    std::shared_ptr<nlohmann::json> jsonArrayMsg = std::make_shared<nlohmann::json>();
    if (jsonArrayMsg == nullptr) {
        DHLOGE("onResponseStartRemoteInputDhid jsonArrayMsg is null.");
        return;
    }

    nlohmann::json tmpJson;
    tmpJson[INPUT_SOURCEMANAGER_KEY_DEVID] = deviceId;
    tmpJson[INPUT_SOURCEMANAGER_KEY_DHID] = dhids;
    tmpJson[INPUT_SOURCEMANAGER_KEY_RESULT] = result;
    jsonArrayMsg->push_back(tmpJson);
    AppExecFwk::InnerEvent::Pointer msgEvent =
        AppExecFwk::InnerEvent::Get(DINPUT_SOURCE_MANAGER_STOP_DHID_MSG, jsonArrayMsg, 0);
    sourceManagerObj_->GetCallbackEventHandler()->SendEvent(msgEvent, 0, AppExecFwk::EventQueue::Priority::IMMEDIATE);
}

void DistributedInputSourceManager::DInputSourceListener::onResponseKeyState(const std::string deviceId,
    const std::string &dhid, const uint32_t type, const uint32_t code, const uint32_t value)
{
    DHLOGI("onResponseMouseDown called, deviceId: %s, dhid: %s.", GetAnonyString(deviceId).c_str(),
        GetAnonyString(dhid).c_str());
    if (sourceManagerObj_ == nullptr) {
        DHLOGE("onResponseMouseDown sourceManagerObj_ is null.");
        return;
    }
    if (sourceManagerObj_->GetCallbackEventHandler() == nullptr) {
        DHLOGE("onResponseMouseDown GetCallbackEventHandler is null.");
        sourceManagerObj_->RunKeyStateCallback(deviceId, dhid, type, code, value);
        return;
    }
    std::shared_ptr<nlohmann::json> jsonArrayMsg = std::make_shared<nlohmann::json>();
    if (jsonArrayMsg == nullptr) {
        DHLOGE("onResponseMouseDown jsonArrayMsg is null.");
        return;
    }

    nlohmann::json tmpJson;
    tmpJson[INPUT_SOURCEMANAGER_KEY_DEVID] = deviceId;
    tmpJson[INPUT_SOURCEMANAGER_KEY_DHID] = dhid;
    tmpJson[INPUT_SOURCEMANAGER_KEY_TYPE] = type;
    tmpJson[INPUT_SOURCEMANAGER_KEY_CODE] = code;
    tmpJson[INPUT_SOURCEMANAGER_KEY_VALUE] = value;
    jsonArrayMsg->push_back(tmpJson);
    AppExecFwk::InnerEvent::Pointer msgEvent =
        AppExecFwk::InnerEvent::Get(DINPUT_SOURCE_MANAGER_KEY_STATE_MSG, jsonArrayMsg, 0);
    sourceManagerObj_->GetCallbackEventHandler()->SendEvent(msgEvent, 0, AppExecFwk::EventQueue::Priority::IMMEDIATE);
}

void DistributedInputSourceManager::DInputSourceListener::onReceivedEventRemoteInput(
    const std::string deviceId, const std::string &event)
{
    nlohmann::json inputData = nlohmann::json::parse(event);
    size_t jsonSize = inputData.size();
    DHLOGI("onReceivedEventRemoteInput called, deviceId: %s, json size:%d.",
        GetAnonyString(deviceId).c_str(), jsonSize);

    RawEvent mEventBuffer[jsonSize];
    int idx = 0;
    for (nlohmann::json::iterator it = inputData.begin(); it != inputData.end(); ++it) {
        nlohmann::json oneData = (*it);
        mEventBuffer[idx].when = oneData[INPUT_KEY_WHEN];
        mEventBuffer[idx].type = oneData[INPUT_KEY_TYPE];
        mEventBuffer[idx].code = oneData[INPUT_KEY_CODE];
        mEventBuffer[idx].value = oneData[INPUT_KEY_VALUE];
        mEventBuffer[idx].descriptor = oneData[INPUT_KEY_DESCRIPTOR];
        mEventBuffer[idx].path = oneData[INPUT_KEY_PATH];
        RecordEventLog(oneData[INPUT_KEY_WHEN], oneData[INPUT_KEY_TYPE], oneData[INPUT_KEY_CODE],
            oneData[INPUT_KEY_VALUE], oneData[INPUT_KEY_PATH]);
        idx++;
    }
    DistributedInputInject::GetInstance().RegisterDistributedEvent(mEventBuffer, jsonSize);
}

void DistributedInputSourceManager::OnStart()
{
    if (serviceRunningState_ == ServiceSourceRunningState::STATE_RUNNING) {
        DHLOGI("dinput Manager Service has already started.");
        return;
    }
    DHLOGI("dinput Manager Service started.");
    if (!InitAuto()) {
        DHLOGI("failed to init service.");
        return;
    }
    serviceRunningState_ = ServiceSourceRunningState::STATE_RUNNING;
    runner_->Run();

    /*
	 * Publish service maybe failed, so we need call this function at the last,
     * so it can't affect the TDD test program.
     */
    bool ret = Publish(this);
    if (!ret) {
        return;
    }

    DHLOGI("DistributedInputSourceManager start success.");
}

bool DistributedInputSourceManager::InitAuto()
{
    runner_ = AppExecFwk::EventRunner::Create(true);
    if (runner_ == nullptr) {
        return false;
    }

    handler_ = std::make_shared<DistributedInputSourceEventHandler>(runner_);

    DHLOGI("init success");

    std::shared_ptr<AppExecFwk::EventRunner> runner = AppExecFwk::EventRunner::Create(true);
    callBackHandler_ = std::make_shared<DistributedInputSourceManager::DInputSourceManagerEventHandler>(runner, this);

    return true;
}

DistributedInputSourceManager::DInputSourceManagerEventHandler::DInputSourceManagerEventHandler(
    const std::shared_ptr<AppExecFwk::EventRunner> &runner, DistributedInputSourceManager *manager)
    : AppExecFwk::EventHandler(runner)
{
    sourceManagerObj_ = manager;
}

void DistributedInputSourceManager::DInputSourceManagerEventHandler::NotifyRegisterCallback(
    const AppExecFwk::InnerEvent::Pointer &event)
{
    std::shared_ptr<nlohmann::json> dataMsg = event->GetSharedObject<nlohmann::json>();
    nlohmann::json::iterator it = dataMsg->begin();
    nlohmann::json innerMsg = *(it);
    std::string deviceId = innerMsg[INPUT_SOURCEMANAGER_KEY_DEVID];
    std::string dhId = innerMsg[INPUT_SOURCEMANAGER_KEY_HWID];
    bool result = innerMsg[INPUT_SOURCEMANAGER_KEY_RESULT];

    InputDeviceId inputDeviceId {deviceId, dhId};
    std::vector<InputDeviceId> tmpInputDevId = sourceManagerObj_->GetInputDeviceId();
    // Find out if the dh exists
    std::vector<InputDeviceId>::iterator devIt  = std::find(
        tmpInputDevId.begin(), tmpInputDevId.end(), inputDeviceId);
    if (devIt != tmpInputDevId.end()) {
        if (result == false) {
            sourceManagerObj_->RemoveInputDeviceId(deviceId, dhId);
        }
    } else {
        DHLOGW("ProcessEvent DINPUT_SOURCE_MANAGER_RIGISTER_MSG the "
            "devId[%s] dhId[%s] is bad data.", GetAnonyString(deviceId).c_str(), GetAnonyString(dhId).c_str());
    }

    sourceManagerObj_->RunRegisterCallback(deviceId, dhId,
        result ? DH_SUCCESS : ERR_DH_INPUT_SERVER_SOURCE_MANAGER_REGISTER_MSG_IS_BAD);
}

void DistributedInputSourceManager::DInputSourceManagerEventHandler::NotifyUnregisterCallback(
    const AppExecFwk::InnerEvent::Pointer &event)
{
    std::shared_ptr<nlohmann::json> dataMsg = event->GetSharedObject<nlohmann::json>();
    nlohmann::json::iterator it = dataMsg->begin();
    nlohmann::json innerMsg = *(it);
    std::string deviceId = innerMsg[INPUT_SOURCEMANAGER_KEY_DEVID];
    std::string dhId = innerMsg[INPUT_SOURCEMANAGER_KEY_HWID];
    bool result = innerMsg[INPUT_SOURCEMANAGER_KEY_RESULT];
    if (result) {
        sourceManagerObj_->SetDeviceMapValue(deviceId, DINPUT_SOURCE_SWITCH_OFF);
    }
    sourceManagerObj_->RunUnregisterCallback(deviceId, dhId,
        result ? DH_SUCCESS : ERR_DH_INPUT_SERVER_SOURCE_MANAGER_UNREGISTER_MSG_IS_BAD);
}

void DistributedInputSourceManager::DInputSourceManagerEventHandler::NotifyPrepareCallback(
    const AppExecFwk::InnerEvent::Pointer &event)
{
    std::shared_ptr<nlohmann::json> dataMsg = event->GetSharedObject<nlohmann::json>();
    nlohmann::json::iterator it = dataMsg->begin();
    nlohmann::json innerMsg = *(it);
    std::string deviceId = innerMsg[INPUT_SOURCEMANAGER_KEY_DEVID];
    bool result = innerMsg[INPUT_SOURCEMANAGER_KEY_RESULT];
    std::string object = innerMsg[INPUT_SOURCEMANAGER_KEY_WHITELIST];

    sourceManagerObj_->RunPrepareCallback(deviceId,
        result ? DH_SUCCESS : ERR_DH_INPUT_SERVER_SOURCE_MANAGER_PREPARE_MSG_IS_BAD, object);
}

void DistributedInputSourceManager::DInputSourceManagerEventHandler::NotifyUnprepareCallback(
    const AppExecFwk::InnerEvent::Pointer &event)
{
    std::shared_ptr<nlohmann::json> dataMsg = event->GetSharedObject<nlohmann::json>();
    nlohmann::json::iterator it = dataMsg->begin();
    nlohmann::json innerMsg = *(it);
    std::string deviceId = innerMsg[INPUT_SOURCEMANAGER_KEY_DEVID];
    bool result = innerMsg[INPUT_SOURCEMANAGER_KEY_RESULT];
    if (result) {
        sourceManagerObj_->SetDeviceMapValue(deviceId, DINPUT_SOURCE_SWITCH_OFF);
    }
    sourceManagerObj_->RunUnprepareCallback(deviceId,
        result ? DH_SUCCESS : ERR_DH_INPUT_SERVER_SOURCE_MANAGER_UNPREPARE_MSG_IS_BAD);
}

void DistributedInputSourceManager::DInputSourceManagerEventHandler::NotifyStartCallback(
    const AppExecFwk::InnerEvent::Pointer &event)
{
    std::shared_ptr<nlohmann::json> dataMsg = event->GetSharedObject<nlohmann::json>();
    nlohmann::json::iterator it = dataMsg->begin();
    nlohmann::json innerMsg = *(it);
    std::string deviceId = innerMsg[INPUT_SOURCEMANAGER_KEY_DEVID];
    uint32_t inputTypes = innerMsg[INPUT_SOURCEMANAGER_KEY_ITP];
    bool result = innerMsg[INPUT_SOURCEMANAGER_KEY_RESULT];
    DHLOGI("Start DInput Recv Callback ret: %s, devId: %s, inputTypes: %d",
        result ? "true" : "false", GetAnonyString(deviceId).c_str(), inputTypes);
    if (result) {
        sourceManagerObj_->SetInputTypesMap(
            deviceId, sourceManagerObj_->GetInputTypesMap(deviceId) | inputTypes);
    }
    sourceManagerObj_->SetStartTransFlag((result && (sourceManagerObj_->GetInputTypesMap(deviceId) > 0)) ?
        DInputServerType::SOURCE_SERVER_TYPE : DInputServerType::NULL_SERVER_TYPE);
    sourceManagerObj_->RunStartCallback(deviceId, inputTypes,
        result ? DH_SUCCESS : ERR_DH_INPUT_SERVER_SOURCE_MANAGER_START_MSG_IS_BAD);
}

void DistributedInputSourceManager::DInputSourceManagerEventHandler::NotifyStopCallback(
    const AppExecFwk::InnerEvent::Pointer &event)
{
    std::shared_ptr<nlohmann::json> dataMsg = event->GetSharedObject<nlohmann::json>();
    nlohmann::json::iterator it = dataMsg->begin();
    nlohmann::json innerMsg = *(it);
    std::string deviceId = innerMsg[INPUT_SOURCEMANAGER_KEY_DEVID];
    uint32_t inputTypes = innerMsg[INPUT_SOURCEMANAGER_KEY_ITP];
    bool result = innerMsg[INPUT_SOURCEMANAGER_KEY_RESULT];

    DHLOGI("Stop DInput Recv Callback ret: %B, devId: %s, inputTypes: %d",
        result, GetAnonyString(deviceId).c_str(), inputTypes);
    if (result && (sourceManagerObj_->GetInputTypesMap(deviceId) & inputTypes)) {
        sourceManagerObj_->SetInputTypesMap(
            deviceId, sourceManagerObj_->GetInputTypesMap(deviceId) -
            (sourceManagerObj_->GetInputTypesMap(deviceId) & inputTypes));
    }

    if (sourceManagerObj_->GetInputTypesMap(deviceId) == 0) {
        sourceManagerObj_->SetDeviceMapValue(deviceId, DINPUT_SOURCE_SWITCH_OFF);
    }

    // DeviceMap_ all sink device switch is off,call isstart's callback
    bool isAllDevSwitchOff = sourceManagerObj_->GetDeviceMapAllDevSwitchOff();
    if (isAllDevSwitchOff) {
        DHLOGI("All Dev Switch Off");
        sourceManagerObj_->SetStartTransFlag(DInputServerType::NULL_SERVER_TYPE);
    }
    sourceManagerObj_->RunStopCallback(deviceId, inputTypes,
        result ? DH_SUCCESS : ERR_DH_INPUT_SERVER_SOURCE_MANAGER_STOP_MSG_IS_BAD);
}

void DistributedInputSourceManager::DInputSourceManagerEventHandler::NotifyStartDhidCallback(
    const AppExecFwk::InnerEvent::Pointer &event)
{
    DHLOGI("ProcessEvent NotifyStartDhidCallback()");
    std::shared_ptr<nlohmann::json> dataMsg = event->GetSharedObject<nlohmann::json>();
    nlohmann::json::iterator it = dataMsg->begin();
    nlohmann::json innerMsg = *(it);
    std::string deviceId = innerMsg[INPUT_SOURCEMANAGER_KEY_DEVID];
    std::string dhidStr = innerMsg[INPUT_SOURCEMANAGER_KEY_DHID];
    bool result = innerMsg[INPUT_SOURCEMANAGER_KEY_RESULT];

    if (sourceManagerObj_ != nullptr) {
        sourceManagerObj_->RunStartDhidCallback(deviceId, dhidStr,
            result ? DH_SUCCESS : ERR_DH_INPUT_SERVER_SOURCE_MANAGER_START_MSG_IS_BAD);
    }
}

void DistributedInputSourceManager::DInputSourceManagerEventHandler::NotifyStopDhidCallback(
    const AppExecFwk::InnerEvent::Pointer &event)
{
    DHLOGI("ProcessEvent NotifyStopDhidCallback()");
    std::shared_ptr<nlohmann::json> dataMsg = event->GetSharedObject<nlohmann::json>();
    nlohmann::json::iterator it = dataMsg->begin();
    nlohmann::json innerMsg = *(it);
    std::string deviceId = innerMsg[INPUT_SOURCEMANAGER_KEY_DEVID];
    std::string dhidStr = innerMsg[INPUT_SOURCEMANAGER_KEY_DHID];
    bool result = innerMsg[INPUT_SOURCEMANAGER_KEY_RESULT];

    if (sourceManagerObj_ != nullptr) {
        sourceManagerObj_->RunStopDhidCallback(deviceId, dhidStr,
            result ? DH_SUCCESS : ERR_DH_INPUT_SERVER_SOURCE_MANAGER_STOP_MSG_IS_BAD);
    }
}

void DistributedInputSourceManager::DInputSourceManagerEventHandler::NotifyKeyStateCallback(
    const AppExecFwk::InnerEvent::Pointer &event)
{
    DHLOGI("ProcessEvent NotifyKeyStateCallback()");
    std::shared_ptr<nlohmann::json> dataMsg = event->GetSharedObject<nlohmann::json>();
    nlohmann::json::iterator it = dataMsg->begin();
    nlohmann::json innerMsg = *(it);
    std::string deviceId = innerMsg[INPUT_SOURCEMANAGER_KEY_DEVID];
    std::string dhid = innerMsg[INPUT_SOURCEMANAGER_KEY_DHID];
    uint32_t keyType = innerMsg[INPUT_SOURCEMANAGER_KEY_TYPE];
    uint32_t keyCode = innerMsg[INPUT_SOURCEMANAGER_KEY_CODE];
    uint32_t keyValue = innerMsg[INPUT_SOURCEMANAGER_KEY_VALUE];
    if (sourceManagerObj_ != nullptr) {
        sourceManagerObj_->RunKeyStateCallback(deviceId, dhid, keyType, keyCode, keyValue);
    }
}

void DistributedInputSourceManager::DInputSourceManagerEventHandler::NotifyStartServerCallback(
    const AppExecFwk::InnerEvent::Pointer &event)
{
    std::shared_ptr<nlohmann::json> dataMsg = event->GetSharedObject<nlohmann::json>();
    nlohmann::json::iterator it = dataMsg->begin();
    nlohmann::json innerMsg = *(it);
    int32_t serType = innerMsg[INPUT_SOURCEMANAGER_KEY_RESULT];
    DInputServerType startTransFlag = DInputServerType(serType);
    sourceManagerObj_->SetStartTransFlag(startTransFlag);
}

void DistributedInputSourceManager::DInputSourceManagerEventHandler::ProcessEvent(
    const AppExecFwk::InnerEvent::Pointer &event)
{
    switch (event->GetInnerEventId()) {
        case DINPUT_SOURCE_MANAGER_RIGISTER_MSG: {
            NotifyRegisterCallback(event);
            break;
        }
        case DINPUT_SOURCE_MANAGER_UNRIGISTER_MSG: {
            NotifyUnregisterCallback(event);
            break;
        }
        case DINPUT_SOURCE_MANAGER_PREPARE_MSG: {
            NotifyPrepareCallback(event);
            break;
        }
        case DINPUT_SOURCE_MANAGER_UNPREPARE_MSG: {
            NotifyUnprepareCallback(event);
            break;
        }
        case DINPUT_SOURCE_MANAGER_START_MSG: {
            NotifyStartCallback(event);
            break;
        }
        case DINPUT_SOURCE_MANAGER_STOP_MSG: {
            NotifyStopCallback(event);
            break;
        }
        case DINPUT_SOURCE_MANAGER_START_DHID_MSG: {
            NotifyStartDhidCallback(event);
            break;
        }
        case DINPUT_SOURCE_MANAGER_STOP_DHID_MSG: {
            NotifyStopDhidCallback(event);
            break;
        }
        case DINPUT_SOURCE_MANAGER_KEY_STATE_MSG: {
            NotifyKeyStateCallback(event);
            break;
        }
        case DINPUT_SOURCE_MANAGER_STARTSERVER_MSG: {
            NotifyStartServerCallback(event);
            break;
        }
        default:break;
    }
}

void DistributedInputSourceManager::OnStop()
{
    DHLOGI("stop service");
    runner_.reset();
    handler_.reset();
    serviceRunningState_ = ServiceSourceRunningState::STATE_NOT_START;
}

int32_t DistributedInputSourceManager::Init()
{
    DHLOGI("enter");
    isStartTrans_ = DInputServerType::NULL_SERVER_TYPE;

    // transport init session
    int32_t ret = DistributedInputSourceTransport::GetInstance().Init();
    if (ret != DH_SUCCESS) {
        return ERR_DH_INPUT_SERVER_SOURCE_MANAGER_INIT_FAIL;
    }

    statuslistener_ = std::make_shared<DInputSourceListener>(this);
    DistributedInputSourceTransport::GetInstance().RegisterSourceRespCallback(statuslistener_);

    serviceRunningState_ = ServiceSourceRunningState::STATE_RUNNING;

    std::shared_ptr<DistributedHardwareFwkKit> dhFwkKit = DInputContext::GetInstance().GetDHFwkKit();
    if (dhFwkKit == nullptr) {
        DHLOGE("dhFwkKit obtain fail!");
        return ERR_DH_INPUT_SERVER_SOURCE_MANAGER_INIT_FAIL;
    }
    startDScreenListener_ = new StartDScreenListener;
    stopDScreenListener_ = new StopDScreenListener;
    deviceOfflineListener_ = new DeviceOfflineListener(this);
    dhFwkKit->RegisterPublisherListener(DHTopic::TOPIC_START_DSCREEN, startDScreenListener_);
    dhFwkKit->RegisterPublisherListener(DHTopic::TOPIC_STOP_DSCREEN, stopDScreenListener_);
    dhFwkKit->RegisterPublisherListener(DHTopic::TOPIC_DEV_OFFLINE, deviceOfflineListener_);

    DHLOGI("Try InitDinputDBG");
    InitDinputDBG();
    return DH_SUCCESS;
}

void DistributedInputSourceManager::InitDinputDBG()
{
    char path[PATH_MAX + 1] = {0x00};
    if (LIB_LOAD_PATH.length() > PATH_MAX || realpath(LIB_LOAD_PATH.c_str(), path) == nullptr) {
        DHLOGE("File canonicalization failed");
        return;
    }

    void *pHandler = dlopen(path, RTLD_LAZY | RTLD_NODELETE);
    if (pHandler == nullptr) {
        DHLOGE("%s handler load failed, failed reason : %s", path, dlerror());
        return;
    }

    GetDInputDBGItfFunc getDinputDBGItfFunc = (GetDInputDBGItfFunc)dlsym(pHandler, GET_DBG_ITF_FUNC.c_str());
    if (getDinputDBGItfFunc == nullptr) {
        DHLOGE("get getDinputDBGItfFunc is null, failed reason : %s", dlerror());
        return;
    }

    dinputDbgItfPtr_ = getDinputDBGItfFunc();
    if (dinputDbgItfPtr_ == nullptr) {
        DHLOGE("Get DInput DBG iterface error");
        return;
    }

    DHLOGE("Init DInput DBG interface");
    dinputDbgItfPtr_->Init();
}

int32_t DistributedInputSourceManager::Release()
{
    DHLOGI("exit");

    // 1.remove input node
    for (std::vector<InputDeviceId>::iterator iter = inputDevice_.begin(); iter != inputDevice_.end(); ++iter) {
        std::string devId = iter->devId;
        std::string dhId = iter->dhId;
        DHLOGI("Release() devId[%s] dhId[%s]", GetAnonyString(devId).c_str(), GetAnonyString(dhId).c_str());
        int32_t ret = DistributedInputInject::GetInstance().UnregisterDistributedHardware(devId, dhId);
        if (ret != DH_SUCCESS) {
            DHLOGW("DistributedInputSourceManager Release called, remove node fail.");
        }
    }

    // 2.delete all device node data
    DHLOGI("Release transport instance");
    DistributedInputSourceTransport::GetInstance().Release();

    // 3.delete all device node data
    DHLOGI("inputDevice clear");
    inputDevice_.clear();
    DeviceMap_.clear();
    InputTypesMap_.clear();

    // 4. isStart callback
    std::shared_ptr<nlohmann::json> jsonArrayMsg = std::make_shared<nlohmann::json>();
    nlohmann::json tmpJson;
    tmpJson[INPUT_SOURCEMANAGER_KEY_RESULT] = static_cast<int32_t>(DInputServerType::NULL_SERVER_TYPE);
    jsonArrayMsg->push_back(tmpJson);
    AppExecFwk::InnerEvent::Pointer msgEvent = AppExecFwk::InnerEvent::Get(
        DINPUT_SOURCE_MANAGER_STARTSERVER_MSG, jsonArrayMsg, 0);

    if (callBackHandler_ == nullptr) {
        DHLOGE("Release callBackHandler_ is null.");
        return ERR_DH_INPUT_SERVER_SOURCE_MANSGER_RELEASE_FAIL;
    }
    callBackHandler_->SendEvent(msgEvent, 0, AppExecFwk::EventQueue::Priority::IMMEDIATE);

    serviceRunningState_ = ServiceSourceRunningState::STATE_NOT_START;
    std::shared_ptr<DistributedHardwareFwkKit> dhFwkKit = DInputContext::GetInstance().GetDHFwkKit();
    if (dhFwkKit != nullptr && startDScreenListener_ != nullptr) {
        DHLOGI("UnPublish StartDScreenListener");
        dhFwkKit->UnregisterPublisherListener(DHTopic::TOPIC_START_DSCREEN, startDScreenListener_);
    }
    if (dhFwkKit != nullptr && stopDScreenListener_ != nullptr) {
        DHLOGI("UnPublish StopDScreenListener");
        dhFwkKit->UnregisterPublisherListener(DHTopic::TOPIC_STOP_DSCREEN, stopDScreenListener_);
    }
    if (dhFwkKit != nullptr && deviceOfflineListener_ != nullptr) {
        DHLOGI("UnPublish DeviceOfflineListener");
        dhFwkKit->UnregisterPublisherListener(DHTopic::TOPIC_DEV_OFFLINE, deviceOfflineListener_);
    }
    DHLOGI("exit dinput source sa.");
    SetSourceProcessExit();
    return DH_SUCCESS;
}

int32_t DistributedInputSourceManager::RegisterDistributedHardware(const std::string& devId, const std::string& dhId,
    const std::string& parameters, sptr<IRegisterDInputCallback> callback)
{
    HisyseventUtil::GetInstance().SysEventWriteBehavior(DINPUT_REGISTER, devId, dhId, "dinput register call.");
    DHLOGI("RegisterDistributedHardware called, deviceId: %s,  dhId: %s,  parameters: %s",
        GetAnonyString(devId).c_str(), GetAnonyString(dhId).c_str(), SetAnonyId(parameters).c_str());
    if (callback == nullptr) {
        DHLOGE("RegisterDistributedHardware called, deviceId: %s callback is null.", GetAnonyString(devId).c_str());
        HisyseventUtil::GetInstance().SysEventWriteFault(DINPUT_REGISTER_FAIL, devId, dhId,
            ERR_DH_INPUT_SERVER_SOURCE_MANAGER_REGISTER_FAIL,
            "dinput register distributed hardware failed callback is nullptr.");
        return ERR_DH_INPUT_SERVER_SOURCE_MANAGER_REGISTER_FAIL;
    }

    std::lock_guard<std::mutex> lock(operationMutex_);

    DInputClientRegistInfo info;
    info.devId = devId;
    info.dhId = dhId;
    info.callback = callback;
    regCallbacks_.push_back(info);

    InputDeviceId inputDeviceId {devId, dhId, GetNodeDesc(parameters)};
    DHLOGI("RegisterDistributedHardware deviceId: %s, dhId: %s",
        GetAnonyString(devId).c_str(), GetAnonyString(dhId).c_str());

    // 1.Find out if the dh exists
    std::vector<InputDeviceId>::iterator it  = std::find(inputDevice_.begin(), inputDevice_.end(), inputDeviceId);
    if (it != inputDevice_.end()) {
        callback->OnResult(devId, dhId, DH_SUCCESS);
        return DH_SUCCESS;
    }

    // 2.create input node
    int32_t ret = DistributedInputInject::GetInstance().RegisterDistributedHardware(devId, dhId, parameters);
    if (ret != DH_SUCCESS) {
        HisyseventUtil::GetInstance().SysEventWriteFault(DINPUT_REGISTER_FAIL, devId, dhId,
            ERR_DH_INPUT_SERVER_SOURCE_MANAGER_REGISTER_FAIL,
            "dinput register distributed hardware failed in create input node.");
        DHLOGE("RegisterDistributedHardware called, create node fail.");

        for (auto iter = regCallbacks_.begin(); iter != regCallbacks_.end(); iter++) {
            if (iter->devId == devId && iter->dhId == dhId) {
                iter->callback->OnResult(iter->devId, iter->dhId, ERR_DH_INPUT_SERVER_SOURCE_MANAGER_REGISTER_FAIL);
                regCallbacks_.erase(iter);
                return ERR_DH_INPUT_SERVER_SOURCE_MANAGER_REGISTER_FAIL;
            }
        }
        return ERR_DH_INPUT_SERVER_SOURCE_MANAGER_REGISTER_FAIL;
    }

    // 3.save device
    DHLOGI("inputDevice push deviceId: %s, dhId: %s", GetAnonyString(inputDeviceId.devId).c_str(),
        GetAnonyString(inputDeviceId.dhId).c_str());
    inputDevice_.push_back(inputDeviceId);

    // 4.notify source distributedfwk register hardware success
    callback->OnResult(devId, dhId, DH_SUCCESS);

    // 5. notify remote side that this side is registerd remote dhid
    sptr<IDistributedSourceInput> cli = DInputSourceSACliMgr::GetInstance().GetRemoteCli(devId);
    if (cli == nullptr) {
        DHLOGE("Get Remote DInput Source Proxy return null");
        return ERR_DH_INPUT_SERVER_SOURCE_MANAGER_REGISTER_FAIL;
    }

    cli->SyncNodeInfoRemoteInput(GetLocalNetworkId(), dhId, GetNodeDesc(parameters));
    return DH_SUCCESS;
}

void DistributedInputSourceManager::handleStartServerCallback(const std::string& devId)
{
    bool isFindDevice = false;
    for (std::vector<InputDeviceId>::iterator iter = inputDevice_.begin(); iter != inputDevice_.end(); ++iter) {
        if (devId == iter->devId) {
            isFindDevice = true;
            break;
        }
    }
    if (!isFindDevice) {
        DeviceMap_[devId] = DINPUT_SOURCE_SWITCH_OFF;
        // DeviceMap_ all sink device switch is off,call isstart's callback
        bool isAllDevSwitchOff = true;
        for (auto it = DeviceMap_.begin(); it != DeviceMap_.end(); it++) {
            if (it->second == DINPUT_SOURCE_SWITCH_ON) {
                isAllDevSwitchOff = false;
                break;
            }
        }
        if (isAllDevSwitchOff) {
            std::shared_ptr<nlohmann::json> jsonArrayMsg = std::make_shared<nlohmann::json>();
            nlohmann::json tmpJson;
            tmpJson[INPUT_SOURCEMANAGER_KEY_RESULT] = static_cast<int32_t>(DInputServerType::NULL_SERVER_TYPE);
            jsonArrayMsg->push_back(tmpJson);
            AppExecFwk::InnerEvent::Pointer msgEvent = AppExecFwk::InnerEvent::Get(
                DINPUT_SOURCE_MANAGER_STARTSERVER_MSG, jsonArrayMsg, 0);

            if (callBackHandler_ == nullptr) {
                DHLOGE("handleStartServerCallback callBackHandler_ is null.");
                return;
            }
            callBackHandler_->SendEvent(msgEvent, 0, AppExecFwk::EventQueue::Priority::IMMEDIATE);
        }
    }
}

int32_t DistributedInputSourceManager::RemoveInputNode(const std::string& devId, const std::string& dhId)
{
    int32_t ret = DistributedInputInject::GetInstance().UnregisterDistributedHardware(devId, dhId);
    if (ret != DH_SUCCESS) {
        DHLOGE("RemoveInputNode called, remove node fail.");
        for (std::vector<DInputClientUnregistInfo>::iterator iter =
            unregCallbacks_.begin(); iter != unregCallbacks_.end(); iter++) {
            if (iter->devId == devId && iter->dhId == dhId) {
                iter->callback->OnResult(iter->devId, iter->dhId,
                    ERR_DH_INPUT_SERVER_SOURCE_MANAGER_REMOVE_INPUT_NODE_FAIL);
                unregCallbacks_.erase(iter);
                return ERR_DH_INPUT_SERVER_SOURCE_MANAGER_REMOVE_INPUT_NODE_FAIL;
            }
        }
        return ERR_DH_INPUT_SERVER_SOURCE_MANAGER_REMOVE_INPUT_NODE_FAIL;
    }
    return DH_SUCCESS;
}

int32_t DistributedInputSourceManager::DeleteDevice(const std::string& devId, const std::string& dhId)
{
    std::shared_ptr<nlohmann::json> jsonArrayMsg = std::make_shared<nlohmann::json>();
    nlohmann::json tmpJson;
    tmpJson[INPUT_SOURCEMANAGER_KEY_DEVID] = devId;
    tmpJson[INPUT_SOURCEMANAGER_KEY_HWID] = dhId;
    tmpJson[INPUT_SOURCEMANAGER_KEY_RESULT] = true;
    jsonArrayMsg->push_back(tmpJson);
    AppExecFwk::InnerEvent::Pointer msgEvent = AppExecFwk::InnerEvent::Get(
        DINPUT_SOURCE_MANAGER_UNRIGISTER_MSG, jsonArrayMsg, 0);

    if (callBackHandler_ == nullptr) {
        DHLOGE("UnregisterDistributedHardware callBackHandler_ is null.");
        return ERR_DH_INPUT_SERVER_SOURCE_MANAGER_DELETE_DEVICE_FAIL;
    }
    callBackHandler_->SendEvent(msgEvent, 0, AppExecFwk::EventQueue::Priority::IMMEDIATE);
    return DH_SUCCESS;
}

int32_t DistributedInputSourceManager::UnregisterDistributedHardware(const std::string& devId, const std::string& dhId,
    sptr<IUnregisterDInputCallback> callback)
{
    HisyseventUtil::GetInstance().SysEventWriteBehavior(DINPUT_UNREGISTER, devId, dhId, "dinput unregister call");
    DHLOGI("%s called, deviceId: %s,  dhId: %s", __func__, GetAnonyString(devId).c_str(), GetAnonyString(dhId).c_str());

    if (callback == nullptr) {
        DHLOGE("%s called, deviceId: %s callback is null.", __func__, GetAnonyString(devId).c_str());
        HisyseventUtil::GetInstance().SysEventWriteFault(DINPUT_UNREGISTER_FAIL, devId, dhId,
            ERR_DH_INPUT_SERVER_SOURCE_MANAGER_UNREGISTER_FAIL, "dinput unregister failed in callback is nullptr");
        return ERR_DH_INPUT_SERVER_SOURCE_MANAGER_UNREGISTER_FAIL;
    }

    std::lock_guard<std::mutex> lock(operationMutex_);

    DInputClientUnregistInfo info {devId, dhId, callback};
    unregCallbacks_.push_back(info);

    InputDeviceId inputDeviceId {devId, dhId};
    DHLOGI("Unregister deviceId: %s, dhId: %s", GetAnonyString(devId).c_str(), GetAnonyString(dhId).c_str());

    std::vector<InputDeviceId>::iterator it = inputDevice_.begin();
    for (; it != inputDevice_.end(); ++it) {
        if (devId == inputDeviceId.devId && dhId == inputDeviceId.dhId) {
            break;
        }
    }

    if (it == inputDevice_.end()) {
        DHLOGE("%s called, deviceId: %s is not exist.", __func__, GetAnonyString(devId).c_str());
        HisyseventUtil::GetInstance().SysEventWriteFault(DINPUT_UNREGISTER_FAIL, devId, dhId,
            ERR_DH_INPUT_SERVER_SOURCE_MANAGER_UNREGISTER_FAIL, "dinput unregister failed in deviceId is not exist");
        for (std::vector<DInputClientUnregistInfo>::iterator iter =
            unregCallbacks_.begin(); iter != unregCallbacks_.end(); iter++) {
            if (iter->devId == devId && iter->dhId == dhId) {
                iter->callback->OnResult(iter->devId, iter->dhId, ERR_DH_INPUT_SERVER_SOURCE_MANAGER_UNREGISTER_FAIL);
                unregCallbacks_.erase(iter);
                return ERR_DH_INPUT_SERVER_SOURCE_MANAGER_UNREGISTER_FAIL;
            }
        }
        return ERR_DH_INPUT_SERVER_SOURCE_MANAGER_UNREGISTER_FAIL;
    }

    // 1.remove input node
    if (RemoveInputNode(devId, dhId) != DH_SUCCESS) {
        callback->OnResult(devId, dhId, ERR_DH_INPUT_SERVER_SOURCE_MANAGER_UNREGISTER_FAIL);
        HisyseventUtil::GetInstance().SysEventWriteFault(DINPUT_UNREGISTER_FAIL, devId, dhId,
            ERR_DH_INPUT_SERVER_SOURCE_MANAGER_UNREGISTER_FAIL, "dinput unregister failed in remove input node");
        return ERR_DH_INPUT_SERVER_SOURCE_MANAGER_UNREGISTER_FAIL;
    }

    // 2.delete device
    inputDevice_.erase(it);
    if (DeleteDevice(devId, dhId) != DH_SUCCESS) {
        callback->OnResult(devId, dhId, ERR_DH_INPUT_SERVER_SOURCE_MANAGER_UNREGISTER_FAIL);
        HisyseventUtil::GetInstance().SysEventWriteFault(DINPUT_UNREGISTER_FAIL, devId, dhId,
            ERR_DH_INPUT_SERVER_SOURCE_MANAGER_UNREGISTER_FAIL, "dinput unregister failed in delete device");
        return ERR_DH_INPUT_SERVER_SOURCE_MANAGER_UNREGISTER_FAIL;
    }

    HiDumper::GetInstance().DeleteNodeInfo(devId, dhId);
    // 3.isstart callback
    handleStartServerCallback(devId);
    return DH_SUCCESS;
}

int32_t DistributedInputSourceManager::PrepareRemoteInput(
    const std::string &deviceId, sptr<IPrepareDInputCallback> callback)
{
    StartAsyncTrace(DINPUT_HITRACE_LABEL, DINPUT_PREPARE_START, DINPUT_PREPARE_TASK);
    HisyseventUtil::GetInstance().SysEventWriteBehavior(DINPUT_PREPARE, deviceId, "dinput prepare call");
    DHLOGI("%s called, deviceId: %s", __func__, GetAnonyString(deviceId).c_str());
    if (callback == nullptr) {
        DHLOGE("%s called, deviceId: %s callback is null.", __func__, GetAnonyString(deviceId).c_str());
        HisyseventUtil::GetInstance().SysEventWriteFault(DINPUT_OPT_FAIL, deviceId,
            ERR_DH_INPUT_SERVER_SOURCE_MANAGER_PREPARE_FAIL, "dinput prepare failed in callback is nullptr");
        FinishAsyncTrace(DINPUT_HITRACE_LABEL, DINPUT_PREPARE_START, DINPUT_PREPARE_TASK);
        return ERR_DH_INPUT_SERVER_SOURCE_MANAGER_PREPARE_FAIL;
    }

    for (auto iter : preCallbacks_) {
        if (iter.devId == deviceId) {
            callback->OnResult(deviceId, ERR_DH_INPUT_SERVER_SOURCE_MANAGER_PREPARE_FAIL);
            HisyseventUtil::GetInstance().SysEventWriteFault(DINPUT_OPT_FAIL, deviceId,
                ERR_DH_INPUT_SERVER_SOURCE_MANAGER_PREPARE_FAIL, "dinput prepare failed in already prepared");
            FinishAsyncTrace(DINPUT_HITRACE_LABEL, DINPUT_PREPARE_START, DINPUT_PREPARE_TASK);
            return ERR_DH_INPUT_SERVER_SOURCE_MANAGER_PREPARE_FAIL;
        }
    }

    int32_t ret = DistributedInputSourceTransport::GetInstance().OpenInputSoftbus(deviceId);
    if (ret != DH_SUCCESS) {
        DHLOGE("Open softbus session fail.");
        HisyseventUtil::GetInstance().SysEventWriteFault(DINPUT_OPT_FAIL, deviceId,
            ERR_DH_INPUT_SERVER_SOURCE_MANAGER_PREPARE_FAIL, "dinput prepare failed in open softbus");
        FinishAsyncTrace(DINPUT_HITRACE_LABEL, DINPUT_PREPARE_START, DINPUT_PREPARE_TASK);
        return ERR_DH_INPUT_SERVER_SOURCE_MANAGER_PREPARE_FAIL;
    }
    DInputClientPrepareInfo info(deviceId, callback);
    preCallbacks_.push_back(info);

    ret = DistributedInputSourceTransport::GetInstance().PrepareRemoteInput(deviceId);
    if (ret != DH_SUCCESS) {
        HisyseventUtil::GetInstance().SysEventWriteFault(DINPUT_OPT_FAIL, deviceId,
            ERR_DH_INPUT_SERVER_SOURCE_MANAGER_PREPARE_FAIL, "dinput prepare failed in transport prepare");
        FinishAsyncTrace(DINPUT_HITRACE_LABEL, DINPUT_PREPARE_START, DINPUT_PREPARE_TASK);
        DHLOGE("Can not send message by softbus, prepare fail.");
        for (auto iter = preCallbacks_.begin(); iter != preCallbacks_.end(); iter++) {
            if (iter->devId == deviceId) {
                iter->preCallback->OnResult(iter->devId, ERR_DH_INPUT_SERVER_SOURCE_MANAGER_PREPARE_FAIL);
                preCallbacks_.erase(iter);
                return ERR_DH_INPUT_SERVER_SOURCE_MANAGER_PREPARE_FAIL;
            }
        }
        return ERR_DH_INPUT_SERVER_SOURCE_MANAGER_PREPARE_FAIL;
    }

    return DH_SUCCESS;
}

int32_t DistributedInputSourceManager::UnprepareRemoteInput(
    const std::string &deviceId, sptr<IUnprepareDInputCallback> callback)
{
    StartAsyncTrace(DINPUT_HITRACE_LABEL, DINPUT_UNPREPARE_START, DINPUT_UNPREPARE_TASK);
    HisyseventUtil::GetInstance().SysEventWriteBehavior(DINPUT_UNPREPARE, deviceId, "dinput unprepare call");
    DHLOGI("%s called, deviceId: %s", __func__, GetAnonyString(deviceId).c_str());

    if (callback == nullptr) {
        DHLOGE("%s called, deviceId: %s callback is null.", __func__, GetAnonyString(deviceId).c_str());
        HisyseventUtil::GetInstance().SysEventWriteFault(DINPUT_OPT_FAIL, deviceId,
            ERR_DH_INPUT_SERVER_SOURCE_MANAGER_UNPREPARE_FAIL, "dinput unprepare failed in callback is nullptr");
        FinishAsyncTrace(DINPUT_HITRACE_LABEL, DINPUT_UNPREPARE_START, DINPUT_UNPREPARE_TASK);
        return ERR_DH_INPUT_SERVER_SOURCE_MANAGER_UNPREPARE_FAIL;
    }

    for (auto iter : unpreCallbacks_) {
        if (iter.devId == deviceId) {
            callback->OnResult(deviceId, ERR_DH_INPUT_SERVER_SOURCE_MANAGER_UNPREPARE_FAIL);
            HisyseventUtil::GetInstance().SysEventWriteFault(DINPUT_OPT_FAIL, deviceId,
                ERR_DH_INPUT_SERVER_SOURCE_MANAGER_UNPREPARE_FAIL, "dinput unprepare failed in already unprepared");
            FinishAsyncTrace(DINPUT_HITRACE_LABEL, DINPUT_UNPREPARE_START, DINPUT_UNPREPARE_TASK);
            return ERR_DH_INPUT_SERVER_SOURCE_MANAGER_UNPREPARE_FAIL;
        }
    }

    DInputClientUnprepareInfo info;
    info.devId = deviceId;
    info.unpreCallback = callback;
    unpreCallbacks_.push_back(info);

    int32_t ret = DistributedInputSourceTransport::GetInstance().UnprepareRemoteInput(deviceId);
    if (ret != DH_SUCCESS) {
        DHLOGE("Can not send message by softbus, unprepare fail.");
        HisyseventUtil::GetInstance().SysEventWriteFault(DINPUT_OPT_FAIL, deviceId,
            ERR_DH_INPUT_SERVER_SOURCE_MANAGER_UNPREPARE_FAIL, "dinput unprepare failed in transport unprepare");
        FinishAsyncTrace(DINPUT_HITRACE_LABEL, DINPUT_UNPREPARE_START, DINPUT_UNPREPARE_TASK);
        for (auto iter = unpreCallbacks_.begin(); iter != unpreCallbacks_.end(); iter++) {
            if (iter->devId == deviceId) {
                iter->unpreCallback->OnResult(iter->devId, ERR_DH_INPUT_SERVER_SOURCE_MANAGER_UNPREPARE_FAIL);
                unpreCallbacks_.erase(iter);
                return ERR_DH_INPUT_SERVER_SOURCE_MANAGER_UNPREPARE_FAIL;
            }
        }
        return ERR_DH_INPUT_SERVER_SOURCE_MANAGER_UNPREPARE_FAIL;
    }

    return DH_SUCCESS;
}

int32_t DistributedInputSourceManager::StartRemoteInput(
    const std::string& deviceId, const uint32_t& inputTypes, sptr<IStartDInputCallback> callback)
{
    StartAsyncTrace(DINPUT_HITRACE_LABEL, DINPUT_START_START, DINPUT_START_TASK);
    HisyseventUtil::GetInstance().SysEventWriteBehavior(DINPUT_START_USE, deviceId, "dinput start use call");
    DHLOGI("%s called, deviceId: %s, inputTypes: %d", __func__, GetAnonyString(deviceId).c_str(), inputTypes);

    if (callback == nullptr) {
        DHLOGE("%s called, deviceId: %s callback is null.", __func__, GetAnonyString(deviceId).c_str());
        HisyseventUtil::GetInstance().SysEventWriteFault(DINPUT_OPT_FAIL, deviceId,
            ERR_DH_INPUT_SERVER_SOURCE_MANAGER_START_FAIL, "dinput start use failed in callback is nullptr");
        FinishAsyncTrace(DINPUT_HITRACE_LABEL, DINPUT_START_START, DINPUT_START_TASK);
        return ERR_DH_INPUT_SERVER_SOURCE_MANAGER_START_FAIL;
    }

    for (auto iter : staCallbacks_) {
        if (iter.devId == deviceId && iter.inputTypes == inputTypes) {
            callback->OnResult(deviceId, inputTypes, ERR_DH_INPUT_SERVER_SOURCE_MANAGER_START_FAIL);
            HisyseventUtil::GetInstance().SysEventWriteFault(DINPUT_OPT_FAIL, deviceId,
                ERR_DH_INPUT_SERVER_SOURCE_MANAGER_START_FAIL, "dinput start use failed in already started");
            FinishAsyncTrace(DINPUT_HITRACE_LABEL, DINPUT_START_START, DINPUT_START_TASK);
            return ERR_DH_INPUT_SERVER_SOURCE_MANAGER_START_FAIL;
        }
    }

    DInputClientStartInfo info(deviceId, inputTypes, callback);
    staCallbacks_.push_back(info);

    DeviceMap_[deviceId] = DINPUT_SOURCE_SWITCH_OFF; // when sink device start success,set DINPUT_SOURCE_SWITCH_ON
    int32_t ret = DistributedInputSourceTransport::GetInstance().StartRemoteInput(deviceId, inputTypes);
    if (ret != DH_SUCCESS) {
        HisyseventUtil::GetInstance().SysEventWriteFault(DINPUT_OPT_FAIL, deviceId,
            ERR_DH_INPUT_SERVER_SOURCE_MANAGER_START_FAIL, "dinput start use failed in transport start");
        FinishAsyncTrace(DINPUT_HITRACE_LABEL, DINPUT_START_START, DINPUT_START_TASK);
        DHLOGE("%s called, start fail.", __func__);
        for (std::vector<DInputClientStartInfo>::iterator iter =
            staCallbacks_.begin(); iter != staCallbacks_.end(); iter++) {
            if (iter->devId == deviceId && iter->inputTypes == inputTypes) {
                iter->callback->OnResult(iter->devId, iter->inputTypes, ERR_DH_INPUT_SERVER_SOURCE_MANAGER_START_FAIL);
                staCallbacks_.erase(iter);
                return ERR_DH_INPUT_SERVER_SOURCE_MANAGER_START_FAIL;
            }
        }
        return ERR_DH_INPUT_SERVER_SOURCE_MANAGER_START_FAIL;
    }

    return DH_SUCCESS;
}

int32_t DistributedInputSourceManager::StopRemoteInput(
    const std::string& deviceId, const uint32_t& inputTypes, sptr<IStopDInputCallback> callback)
{
    StartAsyncTrace(DINPUT_HITRACE_LABEL, DINPUT_STOP_START, DINPUT_STOP_TASK);
    HisyseventUtil::GetInstance().SysEventWriteBehavior(DINPUT_STOP_USE, deviceId, "dinput stop use call");
    DHLOGI("%s called, deviceId: %s, inputTypes: %d", __func__, GetAnonyString(deviceId).c_str(), inputTypes);

    if (callback == nullptr) {
        DHLOGE("%s called, deviceId: %s callback is null.", __func__, GetAnonyString(deviceId).c_str());
        HisyseventUtil::GetInstance().SysEventWriteFault(DINPUT_OPT_FAIL, deviceId,
            ERR_DH_INPUT_SERVER_SOURCE_MANAGER_STOP_FAIL, "dinput stop use failed in callback is nullptr");
        FinishAsyncTrace(DINPUT_HITRACE_LABEL, DINPUT_STOP_START, DINPUT_STOP_TASK);
        return ERR_DH_INPUT_SERVER_SOURCE_MANAGER_STOP_FAIL;
    }

    for (auto iter : stpCallbacks_) {
        if (iter.devId == deviceId && iter.inputTypes == inputTypes) {
            callback->OnResult(deviceId, inputTypes, ERR_DH_INPUT_SERVER_SOURCE_MANAGER_STOP_FAIL);
            HisyseventUtil::GetInstance().SysEventWriteFault(DINPUT_OPT_FAIL, deviceId,
                ERR_DH_INPUT_SERVER_SOURCE_MANAGER_STOP_FAIL, "dinput stop use failed in already stoped");
            FinishAsyncTrace(DINPUT_HITRACE_LABEL, DINPUT_STOP_START, DINPUT_STOP_TASK);
            return ERR_DH_INPUT_SERVER_SOURCE_MANAGER_STOP_FAIL;
        }
    }

    DInputClientStopInfo info(deviceId, inputTypes, callback);
    stpCallbacks_.push_back(info);

    int32_t ret = DistributedInputSourceTransport::GetInstance().StopRemoteInput(deviceId, inputTypes);
    if (ret != DH_SUCCESS) {
        DHLOGE("%s called, stop fail.", __func__);
        HisyseventUtil::GetInstance().SysEventWriteFault(DINPUT_OPT_FAIL, deviceId,
            ERR_DH_INPUT_SERVER_SOURCE_MANAGER_STOP_FAIL, "dinput stop use failed in transport stop");
        FinishAsyncTrace(DINPUT_HITRACE_LABEL, DINPUT_STOP_START, DINPUT_STOP_TASK);
        for (std::vector<DInputClientStopInfo>::iterator iter =
            stpCallbacks_.begin(); iter != stpCallbacks_.end(); iter++) {
            if (iter->devId == deviceId && iter->inputTypes == inputTypes) {
                iter->callback->OnResult(iter->devId, iter->inputTypes, ERR_DH_INPUT_SERVER_SOURCE_MANAGER_STOP_FAIL);
                stpCallbacks_.erase(iter);
                return ERR_DH_INPUT_SERVER_SOURCE_MANAGER_STOP_FAIL;
            }
        }
        return ERR_DH_INPUT_SERVER_SOURCE_MANAGER_STOP_FAIL;
    }

    return DH_SUCCESS;
}

int32_t DistributedInputSourceManager::StartRemoteInput(const std::string &srcId, const std::string &sinkId,
    const uint32_t &inputTypes, sptr<IStartDInputCallback> callback)
{
    StartAsyncTrace(DINPUT_HITRACE_LABEL, DINPUT_START_START, DINPUT_START_TASK);
    HisyseventUtil::GetInstance().SysEventWriteBehavior(DINPUT_START_USE, sinkId, "dinput start use call");
    DHLOGI("StartRemoteInput called, srcId: %s, sinkId: %s, inputTypes: %d", GetAnonyString(srcId).c_str(),
        GetAnonyString(sinkId).c_str(), inputTypes);
    std::string localNetworkId = GetLocalNetworkId();
    if (localNetworkId.empty()) {
        DHLOGE("StartRemoteInput called, Could not get local device id.");
        HisyseventUtil::GetInstance().SysEventWriteFault(DINPUT_OPT_FAIL, sinkId,
            ERR_DH_INPUT_SERVER_SOURCE_MANAGER_START_FAIL, "dinput start use failed in get local networkId error");
        FinishAsyncTrace(DINPUT_HITRACE_LABEL, DINPUT_START_START, DINPUT_START_TASK);
        return ERR_DH_INPUT_SERVER_SOURCE_MANAGER_START_FAIL;
    }
    DHLOGI("StartRemoteInput called, device local networkId is %s", GetAnonyString(localNetworkId).c_str());
    if (srcId != localNetworkId) {
        DHLOGI("StartRemoteInput called, relay start to srcId: %s", GetAnonyString(srcId).c_str());
        return RelayStartRemoteInputByType(srcId, sinkId, inputTypes, callback);
    }

    for (auto iter : staCallbacks_) {
        if (iter.devId == sinkId && iter.inputTypes == inputTypes) {
            DHLOGE("StartRemoteInput called, srcId: %s, sinkId: %s repeat call.",
                GetAnonyString(srcId).c_str(), GetAnonyString(sinkId).c_str());
            HisyseventUtil::GetInstance().SysEventWriteFault(DINPUT_OPT_FAIL, sinkId,
                ERR_DH_INPUT_SERVER_SOURCE_MANAGER_START_FAIL, "dinput start use failed in already started");
            FinishAsyncTrace(DINPUT_HITRACE_LABEL, DINPUT_START_START, DINPUT_START_TASK);
            return ERR_DH_INPUT_SERVER_SOURCE_MANAGER_START_FAIL;
        }
    }

    DInputClientStartInfo info(sinkId, inputTypes, callback);
    staCallbacks_.push_back(info);

    DeviceMap_[sinkId] = DINPUT_SOURCE_SWITCH_OFF; // when sink device start success,set DINPUT_SOURCE_SWITCH_ON
    int32_t ret = DistributedInputSourceTransport::GetInstance().StartRemoteInput(sinkId, inputTypes);
    if (ret != DH_SUCCESS) {
        HisyseventUtil::GetInstance().SysEventWriteFault(DINPUT_OPT_FAIL, sinkId,
            ERR_DH_INPUT_SERVER_SOURCE_MANAGER_START_FAIL, "dinput start use failed in transport start");
        FinishAsyncTrace(DINPUT_HITRACE_LABEL, DINPUT_START_START, DINPUT_START_TASK);
        DHLOGE("StartRemoteInput called, start fail.");
        for (std::vector<DInputClientStartInfo>::iterator it = staCallbacks_.begin(); it != staCallbacks_.end(); it++) {
            if (it->devId == sinkId && it->inputTypes == inputTypes) {
                staCallbacks_.erase(it);
                return ERR_DH_INPUT_SERVER_SOURCE_MANAGER_START_FAIL;
            }
        }
        return ERR_DH_INPUT_SERVER_SOURCE_MANAGER_START_FAIL;
    }
    return DH_SUCCESS;
}

int32_t DistributedInputSourceManager::StopRemoteInput(const std::string &srcId, const std::string &sinkId,
    const uint32_t &inputTypes, sptr<IStopDInputCallback> callback)
{
    StartAsyncTrace(DINPUT_HITRACE_LABEL, DINPUT_STOP_START, DINPUT_STOP_TASK);
    HisyseventUtil::GetInstance().SysEventWriteBehavior(DINPUT_STOP_USE, sinkId, "dinput stop use call");
    DHLOGI("StopRemoteInput called, srcId: %s, sinkId: %s, inputTypes: %d", GetAnonyString(srcId).c_str(),
        GetAnonyString(sinkId).c_str(), inputTypes);
    std::string localNetworkId = GetLocalNetworkId();
    if (localNetworkId.empty()) {
        DHLOGE("StopRemoteInput called, Could not get local device id.");
        HisyseventUtil::GetInstance().SysEventWriteFault(DINPUT_OPT_FAIL, sinkId,
            ERR_DH_INPUT_SERVER_SOURCE_MANAGER_STOP_FAIL, "dinput stop use failed in get networkId");
        FinishAsyncTrace(DINPUT_HITRACE_LABEL, DINPUT_STOP_START, DINPUT_STOP_TASK);
        return ERR_DH_INPUT_SERVER_SOURCE_MANAGER_STOP_FAIL;
    }
    DHLOGI("StopRemoteInput called, device local networkId is %s", GetAnonyString(localNetworkId).c_str());
    if (srcId != localNetworkId) {
        DHLOGI("StopRemoteInput called, relay start to srcId: %s", GetAnonyString(srcId).c_str());
        return RelayStopRemoteInputByType(srcId, sinkId, inputTypes, callback);
    }

    for (auto iter : stpCallbacks_) {
        if (iter.devId == sinkId && iter.inputTypes == inputTypes) {
            DHLOGE("StopRemoteInput called, srcId: %s, sinkId: %s repeat call.",
                GetAnonyString(srcId).c_str(), GetAnonyString(sinkId).c_str());
            HisyseventUtil::GetInstance().SysEventWriteFault(DINPUT_OPT_FAIL, sinkId,
                ERR_DH_INPUT_SERVER_SOURCE_MANAGER_STOP_FAIL, "dinput stop use failed in already stoped");
            FinishAsyncTrace(DINPUT_HITRACE_LABEL, DINPUT_STOP_START, DINPUT_STOP_TASK);
            return ERR_DH_INPUT_SERVER_SOURCE_MANAGER_STOP_FAIL;
        }
    }
    DInputClientStopInfo info(sinkId, inputTypes, callback);
    stpCallbacks_.push_back(info);
    int32_t ret = DistributedInputSourceTransport::GetInstance().StopRemoteInput(sinkId, inputTypes);
    if (ret != DH_SUCCESS) {
        DHLOGE("StopRemoteInput called, stop fail.");
        HisyseventUtil::GetInstance().SysEventWriteFault(DINPUT_OPT_FAIL, sinkId,
            ERR_DH_INPUT_SERVER_SOURCE_MANAGER_STOP_FAIL, "dinput stop use failed in transport stop");
        FinishAsyncTrace(DINPUT_HITRACE_LABEL, DINPUT_STOP_START, DINPUT_STOP_TASK);
        for (std::vector<DInputClientStopInfo>::iterator it = stpCallbacks_.begin(); it != stpCallbacks_.end(); it++) {
            if (it->devId == sinkId && it->inputTypes == inputTypes) {
                stpCallbacks_.erase(it);
                return ERR_DH_INPUT_SERVER_SOURCE_MANAGER_STOP_FAIL;
            }
        }
        return ERR_DH_INPUT_SERVER_SOURCE_MANAGER_STOP_FAIL;
    }
    return DH_SUCCESS;
}

int32_t DistributedInputSourceManager::RelayStartRemoteInputByType(const std::string &srcId, const std::string &sinkId,
    const uint32_t &inputTypes, sptr<IStartDInputCallback> callback)
{
    std::lock_guard<std::mutex> lock(mutex_);
    sptr<ISystemAbilityManager> samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (!samgr) {
        DHLOGE("get sa fail.");
        return ERR_DH_INPUT_SERVER_SOURCE_MANAGER_START_FAIL;
    }

    DHLOGE("try get sa: %d from %s", DISTRIBUTED_HARDWARE_INPUT_SOURCE_SA_ID, GetAnonyString(srcId).c_str());
    sptr<IRemoteObject> remoteObject = samgr->GetSystemAbility(DISTRIBUTED_HARDWARE_INPUT_SOURCE_SA_ID, srcId);
    if (!remoteObject) {
        DHLOGE("Could not get remoteObject.");
        return ERR_DH_INPUT_SERVER_SOURCE_MANAGER_START_FAIL;
    }

    sptr<DistributedInputSourceProxy> dInputSourceProxy(new DistributedInputSourceProxy(remoteObject));
    if (!dInputSourceProxy) {
        DHLOGE("new proxy Object fail.");
        return ERR_DH_INPUT_SERVER_SOURCE_MANAGER_START_FAIL;
    }

    return dInputSourceProxy->StartRemoteInput(srcId, sinkId, inputTypes, callback);
}

int32_t DistributedInputSourceManager::RelayStopRemoteInputByType(
    const std::string &srcId, const std::string &sinkId, const uint32_t &inputTypes, sptr<IStopDInputCallback> callback)
{
    std::lock_guard<std::mutex> lock(mutex_);
    sptr<ISystemAbilityManager> samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (!samgr) {
        DHLOGE("get sa fail.");
        return ERR_DH_INPUT_SERVER_SOURCE_MANAGER_STOP_FAIL;
    }

    DHLOGE("try get sa: %d from %s", DISTRIBUTED_HARDWARE_INPUT_SOURCE_SA_ID, GetAnonyString(srcId).c_str());
    sptr<IRemoteObject> remoteObject = samgr->GetSystemAbility(DISTRIBUTED_HARDWARE_INPUT_SOURCE_SA_ID, srcId);
    if (!remoteObject) {
        DHLOGE("Could not get remoteObject.");
        return ERR_DH_INPUT_SERVER_SOURCE_MANAGER_STOP_FAIL;
    }

    sptr<DistributedInputSourceProxy> dInputSourceProxy(new DistributedInputSourceProxy(remoteObject));
    if (!dInputSourceProxy) {
        DHLOGE("new proxy Object fail.");
        return ERR_DH_INPUT_SERVER_SOURCE_MANAGER_STOP_FAIL;
    }

    return dInputSourceProxy->StopRemoteInput(srcId, sinkId, inputTypes, callback);
}

int32_t DistributedInputSourceManager::PrepareRemoteInput(const std::string &srcId, const std::string &sinkId,
    sptr<IPrepareDInputCallback> callback)
{
    if (srcId.empty() || sinkId.empty()) {
        DHLOGE("srcId: %d or sinkId: %d is empty.", srcId.empty(), sinkId.empty());
        return ERR_DH_INPUT_SERVER_SOURCE_MANAGER_PREPARE_FAIL;
    }
    if (callback == nullptr) {
        DHLOGE("srcId: %s, sinkId: %s callback is null.",
            GetAnonyString(srcId).c_str(), GetAnonyString(sinkId).c_str());
        return ERR_DH_INPUT_SERVER_SOURCE_MANAGER_PREPARE_FAIL;
    }
    DHLOGI("srcId: %s, sinkId: %s", GetAnonyString(srcId).c_str(), GetAnonyString(sinkId).c_str());
    std::string localNetworkId = GetLocalNetworkId();
    if (localNetworkId.empty()) {
        DHLOGE("Could not get local device id.");
        return ERR_DH_INPUT_SERVER_SOURCE_MANAGER_PREPARE_FAIL;
    }
    DHLOGI("device local networkId is %s", GetAnonyString(localNetworkId).c_str());
    if (srcId != localNetworkId) {
        DHLOGE("relay prepare to srcId: %s", GetAnonyString(srcId).c_str());
        return RelayPrepareRemoteInput(srcId, sinkId, callback);
    }
    // current device is source device
    for (auto iter : preCallbacks_) {
        if (iter.devId == sinkId) {
            return ERR_DH_INPUT_SERVER_SOURCE_MANAGER_PREPARE_FAIL;
        }
    }
    int32_t ret = DistributedInputSourceTransport::GetInstance().OpenInputSoftbus(sinkId);
    if (ret != DH_SUCCESS) {
        DHLOGE("Open softbus session fail ret=%d.", ret);
        return ERR_DH_INPUT_SERVER_SOURCE_MANAGER_PREPARE_FAIL;
    }
    DInputClientPrepareInfo info(sinkId, callback);
    preCallbacks_.push_back(info);

    ret = DistributedInputSourceTransport::GetInstance().PrepareRemoteInput(sinkId);
    if (ret != DH_SUCCESS) {
        DHLOGE("Can not send message by softbus, prepare fail.");
        for (auto iter = preCallbacks_.begin(); iter != preCallbacks_.end(); iter++) {
            if (iter->devId == sinkId) {
                preCallbacks_.erase(iter);
                return ERR_DH_INPUT_SERVER_SOURCE_MANAGER_PREPARE_FAIL;
            }
        }
        return ERR_DH_INPUT_SERVER_SOURCE_MANAGER_PREPARE_FAIL;
    }

    return DH_SUCCESS;
}

int32_t DistributedInputSourceManager::UnprepareRemoteInput(const std::string &srcId, const std::string &sinkId,
    sptr<IUnprepareDInputCallback> callback)
{
    if (srcId.empty() || sinkId.empty()) {
        DHLOGE("srcId: %d or sinkId: %d is empty.", srcId.empty(), sinkId.empty());
        return ERR_DH_INPUT_SERVER_SOURCE_MANAGER_UNPREPARE_FAIL;
    }
    if (callback == nullptr) {
        DHLOGE("srcId: %s, sinkId: %s callback is null.",
            GetAnonyString(srcId).c_str(), GetAnonyString(sinkId).c_str());
        return ERR_DH_INPUT_SERVER_SOURCE_MANAGER_UNPREPARE_FAIL;
    }
    DHLOGI("srcId: %s, sinkId: %s", GetAnonyString(srcId).c_str(),
        GetAnonyString(sinkId).c_str());
    std::string localNetworkId = GetLocalNetworkId();
    if (localNetworkId.empty()) {
        DHLOGE("Could not get local device id.");
        return ERR_DH_INPUT_SERVER_SOURCE_MANAGER_UNPREPARE_FAIL;
    }
    DHLOGI("device local localNetworkId is %s", GetAnonyString(localNetworkId).c_str());
    if (srcId != localNetworkId) {
        DHLOGE("relay unprepare to srcId: %s", GetAnonyString(srcId).c_str());
        return RelayUnprepareRemoteInput(srcId, sinkId, callback);
    }

    // current device is source device
    for (auto iter : unpreCallbacks_) {
        if (iter.devId == sinkId) {
            return ERR_DH_INPUT_SERVER_SOURCE_MANAGER_UNPREPARE_FAIL;
        }
    }

    DInputClientUnprepareInfo info;
    info.devId = sinkId;
    info.unpreCallback = callback;
    unpreCallbacks_.push_back(info);

    int32_t ret = DistributedInputSourceTransport::GetInstance().UnprepareRemoteInput(sinkId);
    if (ret != DH_SUCCESS) {
        DHLOGE("Can not send message by softbus, unprepare fail.");
        for (auto iter = unpreCallbacks_.begin(); iter != unpreCallbacks_.end(); iter++) {
            if (iter->devId == sinkId) {
                unpreCallbacks_.erase(iter);
                return ERR_DH_INPUT_SERVER_SOURCE_MANAGER_UNPREPARE_FAIL;
            }
        }
        return ERR_DH_INPUT_SERVER_SOURCE_MANAGER_UNPREPARE_FAIL;
    }
    return DH_SUCCESS;
}

bool DistributedInputSourceManager::IsStringDataSame(const std::vector<std::string> &oldDhIds,
    std::vector<std::string> newDhIds)
{
    if (oldDhIds.size() != newDhIds.size()) {
        DHLOGI("Size is not same, return false.");
        return false;
    }
    bool isSame = true;
    for (auto oDhid : oldDhIds) {
        std::vector<std::string>::iterator it = find(newDhIds.begin(), newDhIds.end(), oDhid);
        if (it == newDhIds.end()) {
            isSame = false;
            break;
        }
    }
    DHLOGI("IsSame: %d.", isSame);
    return isSame;
}

int32_t DistributedInputSourceManager::StartRemoteInput(const std::string &sinkId,
    const std::vector<std::string> &dhIds, sptr<IStartStopDInputsCallback> callback)
{
    StartAsyncTrace(DINPUT_HITRACE_LABEL, DINPUT_START_START, DINPUT_START_TASK);
    HisyseventUtil::GetInstance().SysEventWriteBehavior(DINPUT_START_USE, sinkId, "dinput start use call");
    DHLOGI("sinkId: %s, vector.string.size: %d", GetAnonyString(sinkId).c_str(), dhIds.size());
    std::string localNetworkId = GetLocalNetworkId();
    if (localNetworkId.empty()) {
        DHLOGE("Could not get local device id.");
        HisyseventUtil::GetInstance().SysEventWriteFault(DINPUT_OPT_FAIL, sinkId,
            ERR_DH_INPUT_SERVER_SOURCE_MANAGER_START_FAIL, "dinput start use failed in get networkId");
        FinishAsyncTrace(DINPUT_HITRACE_LABEL, DINPUT_START_START, DINPUT_START_TASK);
        return ERR_DH_INPUT_SERVER_SOURCE_MANAGER_START_FAIL;
    }
    DHLOGI("device local networkId is %s", GetAnonyString(localNetworkId).c_str());

    // current device is source device
    for (auto iter : staStringCallbacks_) {
        if (iter.sinkId == sinkId && IsStringDataSame(iter.dhIds, dhIds)) {
            HisyseventUtil::GetInstance().SysEventWriteFault(DINPUT_OPT_FAIL, sinkId,
                ERR_DH_INPUT_SERVER_SOURCE_MANAGER_START_FAIL, "dinput start use failed in already started");
            FinishAsyncTrace(DINPUT_HITRACE_LABEL, DINPUT_START_START, DINPUT_START_TASK);
            DHLOGE("sinkId: %s, repeat call.", GetAnonyString(sinkId).c_str());
            return ERR_DH_INPUT_SERVER_SOURCE_MANAGER_START_FAIL;
        }
    }

    DInputClientStartDhidInfo info;
    info.srcId = localNetworkId;
    info.sinkId = sinkId;
    info.dhIds = dhIds;
    info.callback = callback;
    staStringCallbacks_.push_back(info);

    DeviceMap_[sinkId] = DINPUT_SOURCE_SWITCH_OFF; // when sink device start success,set DINPUT_SOURCE_SWITCH_ON

    int32_t ret = DistributedInputSourceTransport::GetInstance().StartRemoteInput(sinkId, dhIds);
    if (ret != DH_SUCCESS) {
        HisyseventUtil::GetInstance().SysEventWriteFault(DINPUT_OPT_FAIL, sinkId,
            ERR_DH_INPUT_SERVER_SOURCE_MANAGER_START_FAIL, "dinput start use failed in transport start");
        FinishAsyncTrace(DINPUT_HITRACE_LABEL, DINPUT_START_START, DINPUT_START_TASK);
        DHLOGE("StartRemoteInput start fail.");
        for (std::vector<DInputClientStartDhidInfo>::iterator iter = staStringCallbacks_.begin();
            iter != staStringCallbacks_.end(); iter++) {
            if (iter->sinkId == sinkId && IsStringDataSame(iter->dhIds, dhIds)) {
                staStringCallbacks_.erase(iter);
                return ERR_DH_INPUT_SERVER_SOURCE_MANAGER_START_FAIL;
            }
        }
        return ERR_DH_INPUT_SERVER_SOURCE_MANAGER_START_FAIL;
    }
    return DH_SUCCESS;
}

int32_t DistributedInputSourceManager::StopRemoteInput(const std::string &sinkId, const std::vector<std::string> &dhIds,
    sptr<IStartStopDInputsCallback> callback)
{
    StartAsyncTrace(DINPUT_HITRACE_LABEL, DINPUT_STOP_START, DINPUT_STOP_TASK);
    HisyseventUtil::GetInstance().SysEventWriteBehavior(DINPUT_STOP_USE, sinkId, "dinput stop use call");
    DHLOGI("sinkId: %s, vector.string.size: %d", GetAnonyString(sinkId).c_str(), dhIds.size());
    std::string localNetworkId = GetLocalNetworkId();
    if (localNetworkId.empty()) {
        DHLOGE("Could not get local device id.");
        HisyseventUtil::GetInstance().SysEventWriteFault(DINPUT_OPT_FAIL, sinkId,
            ERR_DH_INPUT_SERVER_SOURCE_MANAGER_STOP_FAIL, "dinput stop use failed in get networkId");
        FinishAsyncTrace(DINPUT_HITRACE_LABEL, DINPUT_STOP_START, DINPUT_STOP_TASK);
        return ERR_DH_INPUT_SERVER_SOURCE_MANAGER_STOP_FAIL;
    }
    DHLOGI("device local networkId is %s", GetAnonyString(localNetworkId).c_str());
    for (auto iter : stpStringCallbacks_) {
        if (iter.sinkId == sinkId && IsStringDataSame(iter.dhIds, dhIds)) {
            HisyseventUtil::GetInstance().SysEventWriteFault(DINPUT_OPT_FAIL, sinkId,
                ERR_DH_INPUT_SERVER_SOURCE_MANAGER_STOP_FAIL, "dinput stop use failed in already stop");
            FinishAsyncTrace(DINPUT_HITRACE_LABEL, DINPUT_STOP_START, DINPUT_STOP_TASK);
            DHLOGE("sinkId: %s, repeat call.", GetAnonyString(sinkId).c_str());
            return ERR_DH_INPUT_SERVER_SOURCE_MANAGER_STOP_FAIL;
        }
    }

    DInputClientStopDhidInfo info;
    info.srcId = localNetworkId;
    info.sinkId = sinkId;
    info.dhIds = dhIds;
    info.callback = callback;
    stpStringCallbacks_.push_back(info);

    int32_t ret = DistributedInputSourceTransport::GetInstance().StopRemoteInput(sinkId, dhIds);
    if (ret != DH_SUCCESS) {
        HisyseventUtil::GetInstance().SysEventWriteFault(DINPUT_OPT_FAIL, sinkId,
            ERR_DH_INPUT_SERVER_SOURCE_MANAGER_STOP_FAIL, "dinput stop use failed in transport stop");
        FinishAsyncTrace(DINPUT_HITRACE_LABEL, DINPUT_STOP_START, DINPUT_STOP_TASK);
        DHLOGE("StopRemoteInput stop fail.");
        for (std::vector<DInputClientStopDhidInfo>::iterator iter = stpStringCallbacks_.begin();
            iter != stpStringCallbacks_.end(); iter++) {
            if (iter->sinkId == sinkId && IsStringDataSame(iter->dhIds, dhIds)) {
                stpStringCallbacks_.erase(iter);
                return ERR_DH_INPUT_SERVER_SOURCE_MANAGER_STOP_FAIL;
            }
        }
        return ERR_DH_INPUT_SERVER_SOURCE_MANAGER_STOP_FAIL;
    }
    return DH_SUCCESS;
}

int32_t DistributedInputSourceManager::StartRemoteInput(const std::string &srcId, const std::string &sinkId,
    const std::vector<std::string> &dhIds, sptr<IStartStopDInputsCallback> callback)
{
    StartAsyncTrace(DINPUT_HITRACE_LABEL, DINPUT_START_START, DINPUT_START_TASK);
    HisyseventUtil::GetInstance().SysEventWriteBehavior(DINPUT_START_USE, sinkId, "dinput start use call");
    DHLOGI("srcId: %s, sinkId: %s, dhids size: %d",
        GetAnonyString(srcId).c_str(), GetAnonyString(sinkId).c_str(), dhIds.size());
    std::string localNetworkId = GetLocalNetworkId();
    if (localNetworkId.empty()) {
        DHLOGE("Could not get local device id.");
        HisyseventUtil::GetInstance().SysEventWriteFault(DINPUT_OPT_FAIL, sinkId,
            ERR_DH_INPUT_SERVER_SOURCE_MANAGER_START_FAIL, "dinput start use failed in get networkId");
        FinishAsyncTrace(DINPUT_HITRACE_LABEL, DINPUT_START_START, DINPUT_START_TASK);
        return ERR_DH_INPUT_SERVER_SOURCE_MANAGER_START_FAIL;
    }
    DHLOGI("device local networkId is %s", GetAnonyString(localNetworkId).c_str());
    if (srcId != localNetworkId) {
        DHLOGI("relay start to srcId: %s", GetAnonyString(srcId).c_str());
        return RelayStartRemoteInputByDhid(srcId, sinkId, dhIds, callback);
    }

    // current device is source device
    for (auto iter : staStringCallbacks_) {
        if (iter.srcId == srcId && iter.sinkId == sinkId && IsStringDataSame(iter.dhIds, dhIds)) {
            HisyseventUtil::GetInstance().SysEventWriteFault(DINPUT_OPT_FAIL, sinkId,
                ERR_DH_INPUT_SERVER_SOURCE_MANAGER_START_FAIL, "dinput start use failed in already start");
            FinishAsyncTrace(DINPUT_HITRACE_LABEL, DINPUT_START_START, DINPUT_START_TASK);
            DHLOGE("sinkId: %s, repeat call.", GetAnonyString(sinkId).c_str());
            return ERR_DH_INPUT_SERVER_SOURCE_MANAGER_START_FAIL;
        }
    }

    DInputClientStartDhidInfo info{srcId, sinkId, dhIds, callback};
    staStringCallbacks_.push_back(info);

    DeviceMap_[sinkId] = DINPUT_SOURCE_SWITCH_OFF; // when sink device start success,set DINPUT_SOURCE_SWITCH_ON

    int32_t ret = DistributedInputSourceTransport::GetInstance().StartRemoteInput(sinkId, dhIds);
    if (ret != DH_SUCCESS) {
        HisyseventUtil::GetInstance().SysEventWriteFault(DINPUT_OPT_FAIL, sinkId,
            ERR_DH_INPUT_SERVER_SOURCE_MANAGER_START_FAIL, "dinput start use failed in transport start");
        FinishAsyncTrace(DINPUT_HITRACE_LABEL, DINPUT_START_START, DINPUT_START_TASK);
        DHLOGE("StartRemoteInput start fail.");
        for (std::vector<DInputClientStartDhidInfo>::iterator iter = staStringCallbacks_.begin();
            iter != staStringCallbacks_.end(); iter++) {
            if (iter->srcId == srcId && iter->sinkId == sinkId && IsStringDataSame(iter->dhIds, dhIds)) {
                staStringCallbacks_.erase(iter);
                return ERR_DH_INPUT_SERVER_SOURCE_MANAGER_START_FAIL;
            }
        }
        return ERR_DH_INPUT_SERVER_SOURCE_MANAGER_START_FAIL;
    }
    return DH_SUCCESS;
}

int32_t DistributedInputSourceManager::StopRemoteInput(const std::string &srcId, const std::string &sinkId,
    const std::vector<std::string> &dhIds, sptr<IStartStopDInputsCallback> callback)
{
    StartAsyncTrace(DINPUT_HITRACE_LABEL, DINPUT_STOP_START, DINPUT_STOP_TASK);
    HisyseventUtil::GetInstance().SysEventWriteBehavior(DINPUT_STOP_USE, sinkId, "dinput stop use call");
    DHLOGI("srcId: %s, sinkId: %s, vector.string.size: %d",
        GetAnonyString(srcId).c_str(), GetAnonyString(sinkId).c_str(), dhIds.size());
    std::string localNetworkId = GetLocalNetworkId();
    if (localNetworkId.empty()) {
        DHLOGE("Could not get local device id.");
        HisyseventUtil::GetInstance().SysEventWriteFault(DINPUT_OPT_FAIL, sinkId,
            ERR_DH_INPUT_SERVER_SOURCE_MANAGER_STOP_FAIL, "dinput stop use failed in get networkId");
        FinishAsyncTrace(DINPUT_HITRACE_LABEL, DINPUT_STOP_START, DINPUT_STOP_TASK);
        return ERR_DH_INPUT_SERVER_SOURCE_MANAGER_STOP_FAIL;
    }
    DHLOGI("device local networkId is %s", GetAnonyString(localNetworkId).c_str());
    if (srcId != localNetworkId) {
        DHLOGI("relay stop to srcId: %s", GetAnonyString(srcId).c_str());
        return RelayStopRemoteInputByDhid(srcId, sinkId, dhIds, callback);
    }

    for (auto iter : stpStringCallbacks_) {
        if (iter.srcId == srcId && iter.sinkId == sinkId && IsStringDataSame(iter.dhIds, dhIds)) {
            HisyseventUtil::GetInstance().SysEventWriteFault(DINPUT_OPT_FAIL, sinkId,
                ERR_DH_INPUT_SERVER_SOURCE_MANAGER_STOP_FAIL, "dinput stop use failed in already stop");
            FinishAsyncTrace(DINPUT_HITRACE_LABEL, DINPUT_STOP_START, DINPUT_STOP_TASK);
            DHLOGE("sinkId: %s, repeat call.", GetAnonyString(sinkId).c_str());
            return ERR_DH_INPUT_SERVER_SOURCE_MANAGER_STOP_FAIL;
        }
    }

    DInputClientStopDhidInfo info;
    info.srcId = srcId;
    info.sinkId = sinkId;
    info.dhIds = dhIds;
    info.callback = callback;
    stpStringCallbacks_.push_back(info);

    int32_t ret = DistributedInputSourceTransport::GetInstance().StopRemoteInput(sinkId, dhIds);
    if (ret != DH_SUCCESS) {
        HisyseventUtil::GetInstance().SysEventWriteFault(DINPUT_OPT_FAIL, sinkId,
            ERR_DH_INPUT_SERVER_SOURCE_MANAGER_STOP_FAIL, "dinput stop use failed in transport stop");
        FinishAsyncTrace(DINPUT_HITRACE_LABEL, DINPUT_STOP_START, DINPUT_STOP_TASK);
        DHLOGE("StopRemoteInput stop fail.");
        for (std::vector<DInputClientStopDhidInfo>::iterator iter = stpStringCallbacks_.begin();
            iter != stpStringCallbacks_.end(); iter++) {
            if (iter->srcId == srcId && iter->sinkId == sinkId && IsStringDataSame(iter->dhIds, dhIds)) {
                stpStringCallbacks_.erase(iter);
                return ERR_DH_INPUT_SERVER_SOURCE_MANAGER_STOP_FAIL;
            }
        }
        return ERR_DH_INPUT_SERVER_SOURCE_MANAGER_STOP_FAIL;
    }
    return DH_SUCCESS;
}

int32_t DistributedInputSourceManager::RegisterAddWhiteListCallback(sptr<IAddWhiteListInfosCallback> callback)
{
    DHLOGI("RegisterAddWhiteListCallback called.");
    if (callback == nullptr) {
        DHLOGE("RegisterAddWhiteListCallback callback is null.");
        return ERR_DH_INPUT_SERVER_SOURCE_MANAGER_REG_CALLBACK_ERR;
    }
    std::lock_guard<std::mutex> lock(valMutex_);
    addWhiteListCallback_ = callback;
    return DH_SUCCESS;
}

int32_t DistributedInputSourceManager::RegisterDelWhiteListCallback(sptr<IDelWhiteListInfosCallback> callback)
{
    DHLOGI("RegisterDelWhiteListCallback called.");
    if (callback == nullptr) {
        DHLOGE("RegisterDelWhiteListCallback callback is null.");
        return ERR_DH_INPUT_SERVER_SOURCE_MANAGER_REG_CALLBACK_ERR;
    }
    std::lock_guard<std::mutex> lock(valMutex_);
    delWhiteListCallback_ = callback;
    return DH_SUCCESS;
}

int32_t DistributedInputSourceManager::RegisterInputNodeListener(sptr<InputNodeListener> listener)
{
    DHLOGI("RegisterInputNodeListener, addr: %p", &listener);
    DistributedInputInject::GetInstance().RegisterInputNodeListener(listener);
    SendExistVirNodeInfos(listener);
    return DH_SUCCESS;
}

int32_t DistributedInputSourceManager::UnregisterInputNodeListener(sptr<InputNodeListener> listener)
{
    DHLOGI("UnregisterInputNodeListener, addr: %p", &listener);
    DistributedInputInject::GetInstance().UnregisterInputNodeListener(listener);
    return DH_SUCCESS;
}

void DistributedInputSourceManager::SendExistVirNodeInfos(sptr<InputNodeListener> listener)
{
    DHLOGI("SendExistVirNodeInfos call");
    std::lock_guard<std::mutex> lock(operationMutex_);
    DevInfo localDevInfo = GetLocalDeviceInfo();
    for (const auto &node : inputDevice_) {
        DHLOGI("Send Exist Vir Node: srcId: %s, sinkId: %s, dhId: %s", GetAnonyString(localDevInfo.networkId).c_str(),
            GetAnonyString(node.devId).c_str(), GetAnonyString(node.dhId).c_str());
        listener->OnNodeOnLine(localDevInfo.networkId, node.devId, node.dhId, node.nodeDesc);
    }
}

int32_t DistributedInputSourceManager::RegisterSimulationEventListener(sptr<ISimulationEventListener> listener)
{
    DHLOGI("RegisterSimulationEventListener called.");
    if (listener == nullptr) {
        DHLOGE("RegisterSimulationEventListener callback is null.");
        return ERR_DH_INPUT_SERVER_SOURCE_MANAGER_SIMULATION_EVENT_CALLBACK_ERR;
    }
    std::lock_guard<std::mutex> lock(valMutex_);
    this->simulationEventCallbacks_.insert(listener);
    return DH_SUCCESS;
}

int32_t DistributedInputSourceManager::UnregisterSimulationEventListener(sptr<ISimulationEventListener> listener)
{
    DHLOGI("UnregisterSimulationEventListener called.");
    if (listener == nullptr) {
        DHLOGE("UnregisterSimulationEventListener callback is null.");
        return ERR_DH_INPUT_SERVER_SOURCE_MANAGER_SIMULATION_EVENT_CALLBACK_ERR;
    }
    std::lock_guard<std::mutex> lock(valMutex_);
    this->simulationEventCallbacks_.erase(listener);
    return DH_SUCCESS;
}

int32_t DistributedInputSourceManager::SyncNodeInfoRemoteInput(const std::string &userDevId, const std::string &dhId,
    const std::string &nodeDesc)
{
    // store info
    UpdateSyncNodeInfo(userDevId, dhId, nodeDesc);
    // notify multimodal
    DistributedInputInject::GetInstance().SyncNodeOnlineInfo(userDevId, GetLocalNetworkId(), dhId, nodeDesc);
    return DH_SUCCESS;
}

int32_t DistributedInputSourceManager::RelayPrepareRemoteInput(const std::string &srcId, const std::string &sinkId,
    sptr<IPrepareDInputCallback> callback)
{
    std::lock_guard<std::mutex> lock(mutex_);
    sptr<ISystemAbilityManager> samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (!samgr) {
        DHLOGE("get sa fail.");
        return ERR_DH_INPUT_SERVER_SOURCE_MANAGER_PREPARE_FAIL;
    }

    DHLOGI("try get sa: %d from device:%s.", DISTRIBUTED_HARDWARE_INPUT_SOURCE_SA_ID,
        GetAnonyString(srcId).c_str());
    sptr<IRemoteObject> remoteObject = samgr->GetSystemAbility(DISTRIBUTED_HARDWARE_INPUT_SOURCE_SA_ID, srcId);
    if (!remoteObject) {
        DHLOGE("get remoteObject error.");
        return ERR_DH_INPUT_SERVER_SOURCE_MANAGER_PREPARE_FAIL;
    }

    sptr<DistributedInputSourceProxy> dInputSourceProxy(new DistributedInputSourceProxy(remoteObject));
    if (!dInputSourceProxy) {
        DHLOGE("dInputSourceProxy error.");
        return ERR_DH_INPUT_SERVER_SOURCE_MANAGER_PREPARE_FAIL;
    }

    return dInputSourceProxy->PrepareRemoteInput(sinkId, callback);
}

int32_t DistributedInputSourceManager::RelayUnprepareRemoteInput(const std::string &srcId, const std::string &sinkId,
    sptr<IUnprepareDInputCallback> callback)
{
    std::lock_guard<std::mutex> lock(mutex_);
    sptr<ISystemAbilityManager> samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (!samgr) {
        DHLOGE("get sa fail.");
        return ERR_DH_INPUT_SERVER_SOURCE_MANAGER_UNPREPARE_FAIL;
    }

    DHLOGI("try get sa: %d", DISTRIBUTED_HARDWARE_INPUT_SOURCE_SA_ID);
    sptr<IRemoteObject> remoteObject = samgr->GetSystemAbility(DISTRIBUTED_HARDWARE_INPUT_SOURCE_SA_ID, srcId);
    if (!remoteObject) {
        DHLOGE("get remoteObject error.");
        return ERR_DH_INPUT_SERVER_SOURCE_MANAGER_UNPREPARE_FAIL;
    }

    sptr<DistributedInputSourceProxy> dInputSourceProxy(new DistributedInputSourceProxy(remoteObject));
    if (!dInputSourceProxy) {
        DHLOGE("dInputSourceProxy error.");
        return ERR_DH_INPUT_SERVER_SOURCE_MANAGER_UNPREPARE_FAIL;
    }

    return dInputSourceProxy->UnprepareRemoteInput(sinkId, callback);
}

int32_t DistributedInputSourceManager::RelayStartRemoteInputByDhid(const std::string &srcId, const std::string &sinkId,
    const std::vector<std::string> &dhIds, sptr<IStartStopDInputsCallback> callback)
{
    std::lock_guard<std::mutex> lock(mutex_);
    sptr<ISystemAbilityManager> samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (!samgr) {
        DHLOGE("get sa fail.");
        return ERR_DH_INPUT_SERVER_SOURCE_MANAGER_START_FAIL;
    }

    DHLOGI("try get sa: %d from %s", DISTRIBUTED_HARDWARE_INPUT_SOURCE_SA_ID,
        GetAnonyString(srcId).c_str());
    sptr<IRemoteObject> remoteObject = samgr->GetSystemAbility(DISTRIBUTED_HARDWARE_INPUT_SOURCE_SA_ID, srcId);
    if (!remoteObject) {
        DHLOGE("remoteObject error.");
        return ERR_DH_INPUT_SERVER_SOURCE_MANAGER_START_FAIL;
    }

    sptr<DistributedInputSourceProxy> dInputSourceProxy(new DistributedInputSourceProxy(remoteObject));
    if (!dInputSourceProxy) {
        DHLOGE("dInputSourceProxy error.");
        return ERR_DH_INPUT_SERVER_SOURCE_MANAGER_START_FAIL;
    }

    return dInputSourceProxy->StartRemoteInput(srcId, sinkId, dhIds, callback);
}

int32_t DistributedInputSourceManager::RelayStopRemoteInputByDhid(const std::string &srcId, const std::string &sinkId,
    const std::vector<std::string> &dhIds, sptr<IStartStopDInputsCallback> callback)
{
    std::lock_guard<std::mutex> lock(mutex_);
    sptr<ISystemAbilityManager> samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (!samgr) {
        DHLOGE("get sa fail.");
        return ERR_DH_INPUT_SERVER_SOURCE_MANAGER_STOP_FAIL;
    }

    DHLOGI("try get sa: %d from %s", DISTRIBUTED_HARDWARE_INPUT_SOURCE_SA_ID,
        GetAnonyString(srcId).c_str());
    sptr<IRemoteObject> remoteObject = samgr->GetSystemAbility(DISTRIBUTED_HARDWARE_INPUT_SOURCE_SA_ID, srcId);
    if (!remoteObject) {
        DHLOGE("get remoteObject error.");
        return ERR_DH_INPUT_SERVER_SOURCE_MANAGER_STOP_FAIL;
    }

    sptr<DistributedInputSourceProxy> dInputSourceProxy(new DistributedInputSourceProxy(remoteObject));
    if (!dInputSourceProxy) {
        DHLOGE("dInputSourceProxy error.");
        return ERR_DH_INPUT_SERVER_SOURCE_MANAGER_STOP_FAIL;
    }

    return dInputSourceProxy->StopRemoteInput(srcId, sinkId, dhIds, callback);
}

void DistributedInputSourceManager::RunRegisterCallback(
    const std::string& devId, const std::string& dhId, const int32_t& status)
{
    std::lock_guard<std::mutex> lock(operationMutex_);
    for (std::vector<DInputClientRegistInfo>::iterator iter =
        regCallbacks_.begin(); iter != regCallbacks_.end(); iter++) {
        if (iter->devId == devId && iter->dhId == dhId) {
            DHLOGI("ProcessEvent DINPUT_SOURCE_MANAGER_RIGISTER_MSG");
            iter->callback->OnResult(devId, dhId, status);
            regCallbacks_.erase(iter);
            return;
        }
    }

    DHLOGE("ProcessEvent registerCallback is null.");
}

void DistributedInputSourceManager::RunUnregisterCallback(
    const std::string& devId, const std::string& dhId, const int32_t& status)
{
    std::lock_guard<std::mutex> lock(operationMutex_);
    for (std::vector<DInputClientUnregistInfo>::iterator iter =
        unregCallbacks_.begin(); iter != unregCallbacks_.end(); iter++) {
        if (iter->devId == devId && iter->dhId == dhId) {
            DHLOGI("ProcessEvent DINPUT_SOURCE_MANAGER_UNRIGISTER_MSG");
            iter->callback->OnResult(devId, dhId, status);
            unregCallbacks_.erase(iter);
            return;
        }
    }

    DHLOGE("ProcessEvent unregisterCallback is null.");
}

void DistributedInputSourceManager::RunPrepareCallback(
    const std::string& devId, const int32_t& status, const std::string& object)
{
    FinishAsyncTrace(DINPUT_HITRACE_LABEL, DINPUT_PREPARE_START, DINPUT_PREPARE_TASK);
    for (std::vector<DInputClientPrepareInfo>::iterator iter =
        preCallbacks_.begin(); iter != preCallbacks_.end(); iter++) {
        if (iter->devId == devId) {
            DHLOGI("ProcessEvent DINPUT_SOURCE_MANAGER_PREPARE_MSG");
            iter->preCallback->OnResult(devId, status);
            preCallbacks_.erase(iter);
            if (addWhiteListCallback_ == nullptr) {
                DHLOGE("ProcessEvent DINPUT_SOURCE_MANAGER_PREPARE_MSG addWhiteListCallback is null.");
                return;
            }
            addWhiteListCallback_->OnResult(devId, object);
            return;
        }
    }

    DHLOGE("ProcessEvent parepareCallback is null.");
}

void DistributedInputSourceManager::RunUnprepareCallback(const std::string &devId, const int32_t &status)
{
    FinishAsyncTrace(DINPUT_HITRACE_LABEL, DINPUT_UNPREPARE_START, DINPUT_UNPREPARE_TASK);
    for (std::vector<DInputClientUnprepareInfo>::iterator iter =
        unpreCallbacks_.begin(); iter != unpreCallbacks_.end(); iter++) {
        if (iter->devId == devId) {
            DHLOGI("ProcessEvent DINPUT_SOURCE_MANAGER_UNPREPARE_MSG");
            iter->unpreCallback->OnResult(devId, status);
            unpreCallbacks_.erase(iter);
            if (delWhiteListCallback_ == nullptr) {
                DHLOGE("ProcessEvent DINPUT_SOURCE_MANAGER_UNPREPARE_MSG delWhiteListCallback is null.");
                return;
            }
            delWhiteListCallback_->OnResult(devId);
            return;
        }
    }

    DHLOGE("ProcessEvent unparepareCallback is null.");
}

void DistributedInputSourceManager::RunStartCallback(
    const std::string& devId, const uint32_t& inputTypes, const int32_t& status)
{
    FinishAsyncTrace(DINPUT_HITRACE_LABEL, DINPUT_START_START, DINPUT_START_TASK);
    for (std::vector<DInputClientStartInfo>::iterator iter = staCallbacks_.begin();
        iter != staCallbacks_.end(); iter++) {
        if (iter->devId == devId && iter->inputTypes == inputTypes) {
            DHLOGI("ProcessEvent DINPUT_SOURCE_MANAGER_START_MSG");
            iter->callback->OnResult(devId, inputTypes, status);
            staCallbacks_.erase(iter);
            break;
        }
    }
}

void DistributedInputSourceManager::RunStopCallback(
    const std::string& devId, const uint32_t& inputTypes, const int32_t& status)
{
    FinishAsyncTrace(DINPUT_HITRACE_LABEL, DINPUT_STOP_START, DINPUT_STOP_TASK);
    for (std::vector<DInputClientStopInfo>::iterator iter = stpCallbacks_.begin();
        iter != stpCallbacks_.end(); iter++) {
        if (iter->devId == devId && iter->inputTypes == inputTypes) {
            DHLOGI("ProcessEvent DINPUT_SOURCE_MANAGER_STOP_MSG");
            iter->callback->OnResult(devId, inputTypes, status);
            stpCallbacks_.erase(iter);
            break;
        }
    }
}

void DistributedInputSourceManager::RunStartDhidCallback(const std::string &sinkId, const std::string &dhIds,
    const int32_t &status)
{
    std::vector<std::string> dhidsVec;
    StringSplitToVector(dhIds, INPUT_STRING_SPLIT_POINT, dhidsVec);
    DHLOGI("ProcessEvent DINPUT_SOURCE_MANAGER_START_DHID_MSG dhIds:%s, vec-size:%d", dhIds.c_str(), dhidsVec.size());
    std::string localNetWorkId = GetLocalNetworkId();
    if (localNetWorkId.empty()) {
        DHLOGE("Could not get local device id.");
        return;
    }

    for (std::vector<DInputClientStartDhidInfo>::iterator iter = staStringCallbacks_.begin();
        iter != staStringCallbacks_.end(); iter++) {
        if (iter->sinkId != sinkId || !IsStringDataSame(iter->dhIds, dhidsVec)) {
            continue;
        }
        iter->callback->OnResultDhids(sinkId, status);
        staStringCallbacks_.erase(iter);
        break;
    }
}

void DistributedInputSourceManager::RunStopDhidCallback(const std::string &sinkId, const std::string &dhIds,
    const int32_t &status)
{
    std::vector<std::string> dhidsVec;
    StringSplitToVector(dhIds, INPUT_STRING_SPLIT_POINT, dhidsVec);
    std::string localNetworkId = GetLocalNetworkId();
    if (localNetworkId.empty()) {
        DHLOGE("Could not get local device id.");
        return;
    }

    for (std::vector<DInputClientStopDhidInfo>::iterator iter = stpStringCallbacks_.begin();
        iter != stpStringCallbacks_.end(); iter++) {
        if (iter->sinkId != sinkId || !IsStringDataSame(iter->dhIds, dhidsVec)) {
            continue;
        }
        iter->callback->OnResultDhids(sinkId, status);
        stpStringCallbacks_.erase(iter);
        break;
    }
}

void DistributedInputSourceManager::RunKeyStateCallback(const std::string &sinkId, const std::string &dhId,
    const uint32_t type, const uint32_t code, const uint32_t value)
{
    // 1.notify multiinput
    std::lock_guard<std::mutex> lock(valMutex_);
    for (const auto &cb : simulationEventCallbacks_) {
        cb->OnSimulationEvent(type, code, value);
    }

    DHLOGI("ProcessEvent notify multimodal OnSimulationEvent success.");
    // 2.if return success, write to virtulnode
    RawEvent mEventBuffer;
    mEventBuffer.type = type;
    mEventBuffer.code = code;
    mEventBuffer.value = value;
    mEventBuffer.descriptor = dhId;
    DistributedInputInject::GetInstance().RegisterDistributedEvent(&mEventBuffer, DINPUT_SOURCE_WRITE_EVENT_SIZE);
    return;
}

void DistributedInputSourceManager::StringSplitToVector(const std::string &str, const char split,
    std::vector<std::string> &vecStr)
{
    if (str.empty()) {
        DHLOGE("StringSplitToVector param str is error.");
        return;
    }
    std::string strTmp = str + split;
    size_t pos = strTmp.find(split);
    while (pos != strTmp.npos) {
        std::string matchTmp = strTmp.substr(0, pos);
        vecStr.push_back(matchTmp);

        strTmp = strTmp.substr(pos + 1, strTmp.size());
        pos = strTmp.find(split);
    }
}

DInputServerType DistributedInputSourceManager::GetStartTransFlag()
{
    return isStartTrans_;
}

void DistributedInputSourceManager::SetStartTransFlag(const DInputServerType flag)
{
    DHLOGI("Set Source isStartTrans_ %d", (int32_t)flag);
    isStartTrans_ = flag;
}

std::vector<DistributedInputSourceManager::InputDeviceId> DistributedInputSourceManager::GetInputDeviceId()
{
    return inputDevice_;
}

void DistributedInputSourceManager::RemoveInputDeviceId(const std::string deviceId, const std::string dhId)
{
    InputDeviceId inputDeviceId {deviceId, dhId};

    std::vector<InputDeviceId>::iterator it  = std::find(inputDevice_.begin(), inputDevice_.end(), inputDeviceId);
    if (it == inputDevice_.end()) {
        return;
    }

    // delete device
    DHLOGI("inputDevice erase deviceId: %s, dhId: %s", GetAnonyString(it->devId).c_str(),
        GetAnonyString(it->dhId).c_str());
    inputDevice_.erase(it);
}

bool DistributedInputSourceManager::GetDeviceMapAllDevSwitchOff()
{
    bool isAllDevSwitchOff = true;
    for (auto it = DeviceMap_.begin(); it != DeviceMap_.end(); it++) {
        if (it->second == DINPUT_SOURCE_SWITCH_ON) {
            isAllDevSwitchOff = false;
            break;
        }
    }
    return isAllDevSwitchOff;
}

void DistributedInputSourceManager::SetDeviceMapValue(const std::string deviceId, int32_t value)
{
    DeviceMap_[deviceId] = value;
}

uint32_t DistributedInputSourceManager::GetInputTypesMap(const std::string deviceId)
{
    std::map<std::string, uint32_t>::iterator key = InputTypesMap_.find(deviceId);
    if (key != InputTypesMap_.end()) {
        return InputTypesMap_[deviceId];
    }
    return static_cast<uint32_t>(DInputDeviceType::NONE);
}

uint32_t DistributedInputSourceManager::GetAllInputTypesMap()
{
    uint32_t rInputTypes = static_cast<uint32_t>(DInputDeviceType::NONE);
    std::map<std::string, uint32_t>::iterator iter;
    for (iter = InputTypesMap_.begin(); iter != InputTypesMap_.end(); iter++) {
        rInputTypes |= iter->second;
    }
    return rInputTypes;
}

void DistributedInputSourceManager::SetInputTypesMap(const std::string deviceId, uint32_t value)
{
    if (value == static_cast<uint32_t>(DInputDeviceType::NONE)) {
        std::map<std::string, uint32_t>::iterator key = InputTypesMap_.find(deviceId);
        if (key != InputTypesMap_.end()) {
            InputTypesMap_.erase(key);
            return;
        }
    }
    InputTypesMap_[deviceId] = value;
}

std::set<BeRegNodeInfo> DistributedInputSourceManager::GetSyncNodeInfo(const std::string& devId)
{
    std::lock_guard<std::mutex> lock(syncNodeInfoMutex_);
    if (syncNodeInfoMap_.find(devId) == syncNodeInfoMap_.end()) {
        DHLOGI("syncNodeInfoMap find not the key: %s", GetAnonyString(devId).c_str());
        return {};
    }
    return syncNodeInfoMap_[devId];
}

void DistributedInputSourceManager::UpdateSyncNodeInfo(const std::string& userDevId, const std::string& dhId,
    const std::string &nodeDesc)
{
    std::lock_guard<std::mutex> lock(syncNodeInfoMutex_);
    if (syncNodeInfoMap_.find(userDevId) == syncNodeInfoMap_.end()) {
        DHLOGI("syncNodeInfoMap has not the key: %s, So create this entry", GetAnonyString(userDevId).c_str());
        std::set<BeRegNodeInfo> syncNodeInfo;
        syncNodeInfoMap_[userDevId] = syncNodeInfo;
    }
    syncNodeInfoMap_[userDevId].insert({userDevId, dhId, nodeDesc});
}

void DistributedInputSourceManager::DeleteSyncNodeInfo(const std::string& devId)
{
    std::lock_guard<std::mutex> lock(syncNodeInfoMutex_);
    syncNodeInfoMap_.erase(devId);
}

void DistributedInputSourceManager::DInputSourceListener::RecordEventLog(int64_t when, int32_t type, int32_t code,
    int32_t value, const std::string &path)
{
    std::string eventType = "";
    switch (type) {
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
    DHLOGD("3.E2E-Test Source softBus receive event, EventType: %s, Code: %d, Value: %d, Path: %s, When: %" PRId64 "",
        eventType.c_str(), code, value, path.c_str(), when);
}

DistributedInputSourceManager::StartDScreenListener::StartDScreenListener()
{
    DHLOGI("StartDScreenListener ctor!");
}

DistributedInputSourceManager::StartDScreenListener::~StartDScreenListener()
{
    DHLOGI("StartDScreenListener dtor!");
}

void DistributedInputSourceManager::StartDScreenListener::OnMessage(const DHTopic topic, const std::string& message)
{
    DHLOGI("StartDScreenListener OnMessage!");
    if (topic != DHTopic::TOPIC_START_DSCREEN) {
        DHLOGE("this topic is wrong, %d", static_cast<uint32_t>(topic));
        return;
    }
    std::string sinkDevId = "";
    SrcScreenInfo srcScreenInfo = {};
    int32_t parseRes = ParseMessage(message, sinkDevId, srcScreenInfo);
    if (parseRes != DH_SUCCESS) {
        DHLOGE("Parse message failed!");
        return;
    }

    std::string srcDevId = GetLocalNetworkId();
    std::string virtualTouchScreenDHId = DistributedInputInject::GetInstance().GenerateVirtualTouchScreenDHId(
        srcScreenInfo.sourceWinId, srcScreenInfo.sourceWinWidth, srcScreenInfo.sourceWinHeight);
    int32_t createNodeRes = DistributedInputInject::GetInstance().CreateVirtualTouchScreenNode(srcDevId,
        virtualTouchScreenDHId, srcScreenInfo.sourceWinId, srcScreenInfo.sourceWinWidth,
        srcScreenInfo.sourceWinHeight);
    if (createNodeRes != DH_SUCCESS) {
        DHLOGE("Create virtual touch screen Node failed!");
        return;
    }

    int32_t cacheRes = UpdateSrcScreenInfoCache(srcScreenInfo);
    if (cacheRes != DH_SUCCESS) {
        DHLOGE("Update SrcScreenInfo cache failed!");
        return;
    }

    int32_t rpcRes = DistributedInputClient::GetInstance().NotifyStartDScreen(sinkDevId, srcDevId,
        srcScreenInfo.sourceWinId);
    if (rpcRes != DH_SUCCESS) {
        DHLOGE("Rpc invoke failed!");
        return;
    }
}

int32_t DistributedInputSourceManager::StartDScreenListener::ParseMessage(const std::string& message,
    std::string& sinkDevId, SrcScreenInfo& srcScreenInfo)
{
    nlohmann::json jsonObj = nlohmann::json::parse(message, nullptr, false);
    if (jsonObj.is_discarded()) {
        DHLOGE("jsonObj parse failed!");
        return ERR_DH_INPUT_JSON_PARSE_FAIL;
    }
    if (!IsString(jsonObj, SINK_DEVICE_ID)) {
        DHLOGE("devId key is invalid");
        return ERR_DH_INPUT_JSON_PARSE_FAIL;
    }
    sinkDevId = jsonObj[SINK_DEVICE_ID].get<std::string>();
    if (!IsUint64(jsonObj, SOURCE_WINDOW_ID)) {
        DHLOGE("sourceWinId key is invalid");
        return ERR_DH_INPUT_JSON_PARSE_FAIL;
    }
    srcScreenInfo.sourceWinId = jsonObj[SOURCE_WINDOW_ID].get<uint64_t>();
    if (!IsUint32(jsonObj, SOURCE_WINDOW_WIDTH)) {
        DHLOGE("sourceWinWidth key is invalid");
        return ERR_DH_INPUT_JSON_PARSE_FAIL;
    }
    srcScreenInfo.sourceWinWidth = jsonObj[SOURCE_WINDOW_WIDTH].get<std::uint32_t>();
    if (!IsUint32(jsonObj, SOURCE_WINDOW_HEIGHT)) {
        DHLOGE("sourceWinHeight key is invalid");
        return ERR_DH_INPUT_JSON_PARSE_FAIL;
    }
    srcScreenInfo.sourceWinHeight = jsonObj[SOURCE_WINDOW_HEIGHT].get<std::uint32_t>();
    return DH_SUCCESS;
}

int32_t DistributedInputSourceManager::StartDScreenListener::UpdateSrcScreenInfoCache(const SrcScreenInfo& TmpInfo)
{
    std::string srcDevId = GetLocalNetworkId();
    std::string srcScreenInfoKey = DInputContext::GetInstance().GetScreenInfoKey(srcDevId, TmpInfo.sourceWinId);
    SrcScreenInfo srcScreenInfo = DInputContext::GetInstance().GetSrcScreenInfo(srcScreenInfoKey);
    srcScreenInfo.devId = srcDevId;
    srcScreenInfo.sessionId = DistributedInputSourceTransport::GetInstance().GetCurrentSessionId();
    srcScreenInfo.uuid = GetUUIDBySoftBus(srcDevId);
    srcScreenInfo.sourceWinId = TmpInfo.sourceWinId;
    srcScreenInfo.sourceWinWidth = TmpInfo.sourceWinWidth;
    srcScreenInfo.sourceWinHeight = TmpInfo.sourceWinHeight;
    srcScreenInfo.sourcePhyId = DistributedInputInject::GetInstance().GenerateVirtualTouchScreenDHId(
        srcScreenInfo.sourceWinId, srcScreenInfo.sourceWinWidth, srcScreenInfo.sourceWinHeight);
    int32_t virtualScreenFd = DistributedInputInject::GetInstance().GetVirtualTouchScreenFd();
    if (virtualScreenFd < 0) {
        DHLOGE("virtualScreenFd is invalid");
        return ERR_DH_INPUT_SERVER_SOURCE_VIRTUAL_SCREEN_NODE_IS_INVALID;
    }
    srcScreenInfo.sourcePhyFd = (uint32_t)virtualScreenFd;
    srcScreenInfo.sourcePhyWidth = TmpInfo.sourceWinWidth;
    srcScreenInfo.sourcePhyHeight = TmpInfo.sourceWinHeight;
    DHLOGI("StartDScreenListener UpdateSrcScreenInfo the data: devId: %s, sourceWinId: %d, sourceWinWidth: %d,"
        "sourceWinHeight: %d, sourcePhyId: %s, sourcePhyFd: %d, sourcePhyWidth: %d, sourcePhyHeight: %d",
        GetAnonyString(srcScreenInfo.devId).c_str(), srcScreenInfo.sourceWinId, srcScreenInfo.sourceWinWidth,
        srcScreenInfo.sourceWinHeight, GetAnonyString(srcScreenInfo.sourcePhyId).c_str(), srcScreenInfo.sourcePhyFd,
        srcScreenInfo.sourcePhyWidth, srcScreenInfo.sourcePhyHeight);
    return DInputContext::GetInstance().UpdateSrcScreenInfo(srcScreenInfoKey, srcScreenInfo);
}

DistributedInputSourceManager::StopDScreenListener::StopDScreenListener()
{
    DHLOGI("StopDScreenListener ctor!");
}

DistributedInputSourceManager::StopDScreenListener::~StopDScreenListener()
{
    DHLOGI("StopDScreenListener dtor!");
}

void DistributedInputSourceManager::StopDScreenListener::OnMessage(const DHTopic topic, const std::string& message)
{
    DHLOGI("StopDScreenListener OnMessage!");
    if (topic != DHTopic::TOPIC_STOP_DSCREEN) {
        DHLOGE("this topic is wrong, %d", static_cast<uint32_t>(topic));
        return;
    }
    std::string sinkDevId = "";
    uint64_t sourceWinId = 0;
    int32_t parseRes = ParseMessage(message, sinkDevId, sourceWinId);
    if (parseRes != DH_SUCCESS) {
        DHLOGE("Message parse failed!");
        return;
    }

    std::string sourceDevId = GetLocalNetworkId();
    std::string screenInfoKey = DInputContext::GetInstance().GetScreenInfoKey(sourceDevId, sourceWinId);
    DHLOGI("screenInfoKey: %s", GetAnonyString(screenInfoKey).c_str());
    SrcScreenInfo srcScreenInfo = DInputContext::GetInstance().GetSrcScreenInfo(screenInfoKey);

    int32_t removeNodeRes = DistributedInputInject::GetInstance().RemoveVirtualTouchScreenNode(
        srcScreenInfo.sourcePhyId);
    if (removeNodeRes != DH_SUCCESS) {
        DHLOGE("Remove virtual touch screen node failed!");
        return;
    }

    int32_t removeCacheRes = DInputContext::GetInstance().RemoveSrcScreenInfo(screenInfoKey);
    if (removeCacheRes != DH_SUCCESS) {
        DHLOGE("Remove src cache failed!");
        return;
    }

    int32_t rpcRes = DistributedInputClient::GetInstance().NotifyStopDScreen(sinkDevId, screenInfoKey);
    if (rpcRes != DH_SUCCESS) {
        DHLOGE("Rpc invoke failed!");
        return;
    }
}

int32_t DistributedInputSourceManager::StopDScreenListener::ParseMessage(const std::string& message,
    std::string& sinkDevId, uint64_t& sourceWinId)
{
    nlohmann::json jsonObj = nlohmann::json::parse(message, nullptr, false);
    if (jsonObj.is_discarded()) {
        DHLOGE("jsonObj parse failed!");
        return ERR_DH_INPUT_JSON_PARSE_FAIL;
    }
    if (!IsString(jsonObj, SINK_DEVICE_ID)) {
        DHLOGE("sourceWinId key is invalid");
        return ERR_DH_INPUT_JSON_PARSE_FAIL;
    }
    sinkDevId = jsonObj[SINK_DEVICE_ID].get<std::string>();
    if (!IsUint64(jsonObj, SOURCE_WINDOW_ID)) {
        DHLOGE("sourceWinId key is invalid");
        return ERR_DH_INPUT_JSON_PARSE_FAIL;
    }
    sourceWinId = jsonObj[SOURCE_WINDOW_ID].get<uint64_t>();
    return DH_SUCCESS;
}

DistributedInputSourceManager::DeviceOfflineListener::DeviceOfflineListener(
    DistributedInputSourceManager* srcManagerContext)
{
    DHLOGI("DeviceOfflineListener ctor!");
    sourceManagerContext_ = srcManagerContext;
}

DistributedInputSourceManager::DeviceOfflineListener::~DeviceOfflineListener()
{
    DHLOGI("DeviceOfflineListener dtor!");
}

void DistributedInputSourceManager::DeviceOfflineListener::OnMessage(const DHTopic topic, const std::string &message)
{
    DHLOGI("DeviceOfflineListener OnMessage!");
    if (topic != DHTopic::TOPIC_DEV_OFFLINE) {
        DHLOGE("this topic is wrong, %d", static_cast<uint32_t>(topic));
        return;
    }
    if (message.empty()) {
        DHLOGE("this message is empty");
        return;
    }
    DeleteNodeInfoAndNotify(message);
}

void DistributedInputSourceManager::DeviceOfflineListener::DeleteNodeInfoAndNotify(const std::string& offlineDevId)
{
    DHLOGI("DeviceOfflineListener DeleteNodeInfoAndNotify!");
    if (sourceManagerContext_ == nullptr) {
        DHLOGE("sourceManagerContext is nullptr!");
        return;
    }
    std::set<BeRegNodeInfo> nodeSet = sourceManagerContext_->GetSyncNodeInfo(offlineDevId);
    std::string localNetWorkId = GetLocalNetworkId();
    for (const auto &node : nodeSet) {
        DHLOGI("DeleteNodeInfoAndNotify device: %s, dhId: %s", GetAnonyString(offlineDevId).c_str(),
            GetAnonyString(node.dhId).c_str());
        // Notify multimodal
        DistributedInputInject::GetInstance().SyncNodeOfflineInfo(offlineDevId, localNetWorkId, node.dhId);
    }
    // Delete info
    sourceManagerContext_->DeleteSyncNodeInfo(offlineDevId);
}

int32_t DistributedInputSourceManager::Dump(int32_t fd, const std::vector<std::u16string>& args)
{
    DHLOGI("DistributedInputSourceManager Dump.");
    std::vector<std::string> argsStr;
    for (auto iter : args) {
        argsStr.emplace_back(Str16ToStr8(iter));
    }
    std::string result("");
    if (!HiDumper::GetInstance().HiDump(argsStr, result)) {
        DHLOGI("Hidump error.");
        return ERR_DH_INPUT_HIDUMP_DUMP_PROCESS_FAIL;
    }

    int ret = dprintf(fd, "%s\n", result.c_str());
    if (ret < 0) {
        DHLOGE("dprintf error.");
        return ERR_DH_INPUT_HIDUMP_DPRINTF_FAIL;
    }
    return DH_SUCCESS;
}
} // namespace DistributedInput
} // namespace DistributedHardware
} // namespace OHOS
