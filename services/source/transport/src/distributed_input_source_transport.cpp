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

#include "distributed_input_source_transport.h"

#include <algorithm>
#include <cstring>

#include "anonymous_string.h"
#include "distributed_hardware_log.h"
#include "ipc_skeleton.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"

#include "constants_dinput.h"
#include "dinput_errcode.h"
#include "dinput_hitrace.h"
#include "dinput_low_latency.h"
#include "dinput_softbus_define.h"
#include "dinput_utils_tool.h"
#include "distributed_input_inject.h"
#include "hidumper.h"
#include "session.h"
#include "softbus_bus_center.h"
#include "softbus_common.h"

#include "distributed_input_transport_base.h"

namespace OHOS {
namespace DistributedHardware {
namespace DistributedInput {

DistributedInputSourceTransport::~DistributedInputSourceTransport()
{
    DHLOGI("Dtor DistributedInputSourceTransport");
    Release();
}

DistributedInputSourceTransport &DistributedInputSourceTransport::GetInstance()
{
    static DistributedInputSourceTransport instance;
    return instance;
}

int32_t DistributedInputSourceTransport::Init()
{
    DHLOGI("Init Source Transport");

    int32_t ret = DistributedInputTransportBase::GetInstance().Init();
    if (ret != DH_SUCCESS) {
        DHLOGE("Init Source Transport failed.");
        return ERR_DH_INPUT_SERVER_SOURCE_MANAGER_INIT_FAIL;
    }

    statuslistener_ = std::make_shared<DInputTransbaseSourceListener>(this);
    DistributedInputTransportBase::GetInstance().RegisterSrcHandleSessionCallback(statuslistener_);
    RegRespFunMap();
    return DH_SUCCESS;
}

void DistributedInputSourceTransport::RegRespFunMap()
{
    memberFuncMap_[TRANS_SINK_MSG_ONPREPARE] = &DistributedInputSourceTransport::NotifyResponsePrepareRemoteInput;
    memberFuncMap_[TRANS_SINK_MSG_ONUNPREPARE] = &DistributedInputSourceTransport::NotifyResponseUnprepareRemoteInput;
    memberFuncMap_[TRANS_SINK_MSG_ONSTART] = &DistributedInputSourceTransport::NotifyResponseStartRemoteInput;
    memberFuncMap_[TRANS_SINK_MSG_ONSTOP] = &DistributedInputSourceTransport::NotifyResponseStopRemoteInput;
    memberFuncMap_[TRANS_SINK_MSG_BODY_DATA] = &DistributedInputSourceTransport::NotifyReceivedEventRemoteInput;
    memberFuncMap_[TRANS_SINK_MSG_DHID_ONSTART] = &DistributedInputSourceTransport::NotifyResponseStartRemoteInputDhid;
    memberFuncMap_[TRANS_SINK_MSG_DHID_ONSTOP] = &DistributedInputSourceTransport::NotifyResponseStopRemoteInputDhid;
    memberFuncMap_[TRANS_SINK_MSG_KEY_STATE] = &DistributedInputSourceTransport::NotifyResponseKeyState;
    memberFuncMap_[TRANS_SOURCE_TO_SOURCE_MSG_PREPARE] = &DistributedInputSourceTransport::ReceiveSrcTSrcRelayPrepare;
    memberFuncMap_[TRANS_SINK_MSG_ON_RELAY_PREPARE] =
        &DistributedInputSourceTransport::NotifyResponseRelayPrepareRemoteInput;
    memberFuncMap_[TRANS_SINK_MSG_ON_RELAY_UNPREPARE] =
        &DistributedInputSourceTransport::NotifyResponseRelayUnprepareRemoteInput;
    memberFuncMap_[TRANS_SOURCE_TO_SOURCE_MSG_UNPREPARE] =
        &DistributedInputSourceTransport::ReceiveSrcTSrcRelayUnprepare;
    memberFuncMap_[TRANS_SOURCE_TO_SOURCE_MSG_PREPARE_RESULT] =
        &DistributedInputSourceTransport::ReceiveRelayPrepareResult;
    memberFuncMap_[TRANS_SOURCE_TO_SOURCE_MSG_UNPREPARE_RESULT] =
        &DistributedInputSourceTransport::ReceiveRelayUnprepareResult;
    memberFuncMap_[TRANS_SOURCE_TO_SOURCE_MSG_START_DHID] =
        &DistributedInputSourceTransport::ReceiveSrcTSrcRelayStartDhid;
    memberFuncMap_[TRANS_SOURCE_TO_SOURCE_MSG_STOP_DHID] =
        &DistributedInputSourceTransport::ReceiveSrcTSrcRelayStopDhid;
    memberFuncMap_[TRANS_SINK_MSG_ON_RELAY_STARTDHID] =
        &DistributedInputSourceTransport::NotifyResponseRelayStartDhidRemoteInput;
    memberFuncMap_[TRANS_SINK_MSG_ON_RELAY_STOPDHID] =
        &DistributedInputSourceTransport::NotifyResponseRelayStopDhidRemoteInput;
    memberFuncMap_[TRANS_SOURCE_TO_SOURCE_MSG_START_DHID_RESULT] =
        &DistributedInputSourceTransport::ReceiveRelayStartDhidResult;
    memberFuncMap_[TRANS_SOURCE_TO_SOURCE_MSG_STOP_DHID_RESULT] =
        &DistributedInputSourceTransport::ReceiveRelayStopDhidResult;
    memberFuncMap_[TRANS_SOURCE_TO_SOURCE_MSG_START_TYPE] =
        &DistributedInputSourceTransport::ReceiveSrcTSrcRelayStartType;
    memberFuncMap_[TRANS_SOURCE_TO_SOURCE_MSG_STOP_TYPE] =
        &DistributedInputSourceTransport::ReceiveSrcTSrcRelayStopType;
    memberFuncMap_[TRANS_SINK_MSG_ON_RELAY_STARTTYPE] =
        &DistributedInputSourceTransport::NotifyResponseRelayStartTypeRemoteInput;
    memberFuncMap_[TRANS_SINK_MSG_ON_RELAY_STOPTYPE] =
        &DistributedInputSourceTransport::NotifyResponseRelayStopTypeRemoteInput;
    memberFuncMap_[TRANS_SOURCE_TO_SOURCE_MSG_START_TYPE_RESULT] =
        &DistributedInputSourceTransport::ReceiveRelayStartTypeResult;
    memberFuncMap_[TRANS_SOURCE_TO_SOURCE_MSG_STOP_TYPE_RESULT] =
        &DistributedInputSourceTransport::ReceiveRelayStopTypeResult;
}

void DistributedInputSourceTransport::Release()
{
    DHLOGI("Release Source Transport");

    DistributedInputInject::GetInstance().StopInjectThread();
}

int32_t DistributedInputSourceTransport::OpenInputSoftbus(const std::string &remoteDevId)
{
    int32_t ret = DistributedInputTransportBase::GetInstance().StartSession(remoteDevId);
    if (ret != DH_SUCCESS) {
        DHLOGE("StartSession fail! remoteDevId:%s.", GetAnonyString(remoteDevId).c_str());
            return ERR_DH_INPUT_SERVER_SOURCE_TRANSPORT_OPEN_SESSION_FAIL;
    }

    return DH_SUCCESS;
}

void DistributedInputSourceTransport::CloseInputSoftbus(const std::string &remoteDevId)
{
    DistributedInputTransportBase::GetInstance().StopSession(remoteDevId);
}

void DistributedInputSourceTransport::RegisterSourceRespCallback(std::shared_ptr<DInputSourceTransCallback> callback)
{
    DHLOGI("RegisterSourceRespCallback");
    callback_ = callback;
}

/*
 * PrepareRemoteInput.
 * @param  deviceId is remote device
 * @return Returns 0 is success, other is fail.
 */
int32_t DistributedInputSourceTransport::PrepareRemoteInput(const std::string& deviceId)
{
    int32_t sessionId = DistributedInputTransportBase::GetInstance().GetSessionIdByDevId(deviceId);
    if (sessionId < 0) {
        DHLOGE("PrepareRemoteInput error, not find this device:%s.", GetAnonyString(deviceId).c_str());
        return ERR_DH_INPUT_SERVER_SOURCE_TRANSPORT_PREPARE_FAIL;
    }
    DHLOGI("PrepareRemoteInput sessionId:%d.", sessionId);

    nlohmann::json jsonStr;
    jsonStr[DINPUT_SOFTBUS_KEY_CMD_TYPE] = TRANS_SOURCE_MSG_PREPARE;
    jsonStr[DINPUT_SOFTBUS_KEY_DEVICE_ID] = deviceId;
    jsonStr[DINPUT_SOFTBUS_KEY_SESSION_ID] = sessionId;
    std::string smsg = jsonStr.dump();
    int32_t ret = SendMessage(sessionId, smsg);
    if (ret != DH_SUCCESS) {
        DHLOGE("PrepareRemoteInput deviceId:%s, sessionId:%d, smsg:%s, SendMsg error, ret:%d.",
            GetAnonyString(deviceId).c_str(), sessionId, SetAnonyId(smsg).c_str(), ret);
        return ERR_DH_INPUT_SERVER_SOURCE_TRANSPORT_PREPARE_FAIL;
    }
    DHLOGI("PrepareRemoteInput devId:%s, sessionId:%d, msg:%s.",
        GetAnonyString(deviceId).c_str(), sessionId, SetAnonyId(smsg).c_str());
    return DH_SUCCESS;
}

int32_t DistributedInputSourceTransport::UnprepareRemoteInput(const std::string& deviceId)
{
    int32_t sessionId = DistributedInputTransportBase::GetInstance().GetSessionIdByDevId(deviceId);
    if (sessionId < 0) {
        DHLOGE("UnprepareRemoteInput error, not find this device:%s.", GetAnonyString(deviceId).c_str());
        return ERR_DH_INPUT_SERVER_SOURCE_TRANSPORT_UNPREPARE_FAIL;
    }
    DHLOGI("UnprepareRemoteInput sessionId:%d.", sessionId);

    nlohmann::json jsonStr;
    jsonStr[DINPUT_SOFTBUS_KEY_CMD_TYPE] = TRANS_SOURCE_MSG_UNPREPARE;
    jsonStr[DINPUT_SOFTBUS_KEY_DEVICE_ID] = deviceId;
    jsonStr[DINPUT_SOFTBUS_KEY_SESSION_ID] = sessionId;
    std::string smsg = jsonStr.dump();
    int32_t ret = SendMessage(sessionId, smsg);
    if (ret != DH_SUCCESS) {
        DHLOGE("UnprepareRemoteInput deviceId:%s, sessionId:%d, smsg:%s, SendMsg error, ret:%d.",
            GetAnonyString(deviceId).c_str(), sessionId, SetAnonyId(smsg).c_str(), ret);
        return ERR_DH_INPUT_SERVER_SOURCE_TRANSPORT_UNPREPARE_FAIL;
    }
    DHLOGI("UnprepareRemoteInput deviceId:%s, sessionId:%d, smsg:%s.",
        GetAnonyString(deviceId).c_str(), sessionId, SetAnonyId(smsg).c_str());
    return DH_SUCCESS;
}

int32_t DistributedInputSourceTransport::PrepareRemoteInput(int32_t srcTsrcSeId, const std::string &peerDevId)
{
    int32_t sinkSessionId = DistributedInputTransportBase::GetInstance().GetSessionIdByDevId(peerDevId);
    if (sinkSessionId < 0) {
        DHLOGE("PrepareRemoteInput error, not find this device:%s.", GetAnonyString(peerDevId).c_str());
        return ERR_DH_INPUT_SERVER_SOURCE_TRANSPORT_PREPARE_FAIL;
    }

    DHLOGI("PrepareRemoteInput srcTsrcSeId:%d, sinkSessionId:%d.", srcTsrcSeId, sinkSessionId);

    nlohmann::json jsonStr;
    jsonStr[DINPUT_SOFTBUS_KEY_CMD_TYPE] = TRANS_SOURCE_MSG_PREPARE_FOR_REL;
    jsonStr[DINPUT_SOFTBUS_KEY_DEVICE_ID] = peerDevId;
    jsonStr[DINPUT_SOFTBUS_KEY_SESSION_ID] = srcTsrcSeId;
    std::string smsg = jsonStr.dump();
    int32_t ret = SendMessage(sinkSessionId, smsg);
    if (ret != DH_SUCCESS) {
        DHLOGE("PrepareRemoteInput peerDevId:%s, smsg:%s, SendMsg error, ret:%d.",
            GetAnonyString(peerDevId).c_str(), SetAnonyId(smsg).c_str(), ret);
        return ERR_DH_INPUT_SERVER_SOURCE_TRANSPORT_PREPARE_FAIL;
    }
    DHLOGI("PrepareRemoteInput send success, devId:%s, msg:%s.",
        GetAnonyString(peerDevId).c_str(), SetAnonyId(smsg).c_str());
    return DH_SUCCESS;
}
int32_t DistributedInputSourceTransport::UnprepareRemoteInput(int32_t srcTsrcSeId, const std::string &peerDevId)
{
    int32_t sinkSessionId = DistributedInputTransportBase::GetInstance().GetSessionIdByDevId(peerDevId);
    if (sinkSessionId < 0) {
        DHLOGE("UnprepareRemoteInput error, not find this device:%s.", GetAnonyString(peerDevId).c_str());
        return ERR_DH_INPUT_SERVER_SOURCE_TRANSPORT_UNPREPARE_FAIL;
    }
    DHLOGI("UnprepareRemoteInput srcTsrcSeId:%d, sinkSessionId:%d.", srcTsrcSeId, sinkSessionId);

    nlohmann::json jsonStr;
    jsonStr[DINPUT_SOFTBUS_KEY_CMD_TYPE] = TRANS_SOURCE_MSG_UNPREPARE_FOR_REL;
    jsonStr[DINPUT_SOFTBUS_KEY_DEVICE_ID] = peerDevId;
    jsonStr[DINPUT_SOFTBUS_KEY_SESSION_ID] = srcTsrcSeId;
    std::string smsg = jsonStr.dump();
    int32_t ret = SendMessage(sinkSessionId, smsg);
    if (ret != DH_SUCCESS) {
        DHLOGE("UnprepareRemoteInput peerDevId:%s, smsg:%s, SendMsg error, ret:%d.",
            GetAnonyString(peerDevId).c_str(), SetAnonyId(smsg).c_str(), ret);
        return ERR_DH_INPUT_SERVER_SOURCE_TRANSPORT_UNPREPARE_FAIL;
    }
    DHLOGI("UnprepareRemoteInput send success, devId:%s, msg:%s.",
        GetAnonyString(peerDevId).c_str(), SetAnonyId(smsg).c_str());
    return DH_SUCCESS;
}

int32_t DistributedInputSourceTransport::StartRemoteInputDhids(int32_t srcTsrcSeId, const std::string &deviceId,
    const std::string &dhids)
{
    int32_t sinkSessionId = DistributedInputTransportBase::GetInstance().GetSessionIdByDevId(deviceId);
    if (sinkSessionId < 0) {
        DHLOGE("StartRemoteInputDhids error, not find this device:%s.", GetAnonyString(deviceId).c_str());
        return ERR_DH_INPUT_SERVER_SOURCE_TRANSPORT_START_FAIL;
    }
    DHLOGI("StartRemoteInputDhids srcTsrcSeId:%d, sinkSessionId:%d.", srcTsrcSeId, sinkSessionId);

    nlohmann::json jsonStr;
    jsonStr[DINPUT_SOFTBUS_KEY_CMD_TYPE] = TRANS_SOURCE_MSG_START_DHID_FOR_REL;
    jsonStr[DINPUT_SOFTBUS_KEY_DEVICE_ID] = deviceId;
    jsonStr[DINPUT_SOFTBUS_KEY_SESSION_ID] = srcTsrcSeId;
    jsonStr[DINPUT_SOFTBUS_KEY_VECTOR_DHID] = dhids;
    std::string smsg = jsonStr.dump();
    int32_t ret = SendMessage(sinkSessionId, smsg);
    if (ret != DH_SUCCESS) {
        DHLOGE("StartRemoteInputDhids deviceId:%s, smsg:%s, SendMsg error, ret:%d.",
            GetAnonyString(deviceId).c_str(), SetAnonyId(smsg).c_str(), ret);
        return ERR_DH_INPUT_SERVER_SOURCE_TRANSPORT_START_FAIL;
    }
    DHLOGI("StartRemoteInputDhids send success, devId:%s, msg:%s.", GetAnonyString(deviceId).c_str(),
        SetAnonyId(smsg).c_str());
    DistributedInputInject::GetInstance().StartInjectThread();
    DHLOGI("StartInjectThread successed");
    return DH_SUCCESS;
}

int32_t DistributedInputSourceTransport::StopRemoteInputDhids(int32_t srcTsrcSeId, const std::string &deviceId,
    const std::string &dhids)
{
    int32_t sinkSessionId = DistributedInputTransportBase::GetInstance().GetSessionIdByDevId(deviceId);
    if (sinkSessionId < 0) {
        DHLOGE("StopRemoteInputDhids error, not find this device:%s.", GetAnonyString(deviceId).c_str());
        return ERR_DH_INPUT_SERVER_SOURCE_TRANSPORT_STOP_FAIL;
    }
    DHLOGI("StopRemoteInputDhids srcTsrcSeId:%d, sinkSessionId:%d.", srcTsrcSeId, sinkSessionId);

    nlohmann::json jsonStr;
    jsonStr[DINPUT_SOFTBUS_KEY_CMD_TYPE] = TRANS_SOURCE_MSG_STOP_DHID_FOR_REL;
    jsonStr[DINPUT_SOFTBUS_KEY_DEVICE_ID] = deviceId;
    jsonStr[DINPUT_SOFTBUS_KEY_SESSION_ID] = srcTsrcSeId;
    jsonStr[DINPUT_SOFTBUS_KEY_VECTOR_DHID] = dhids;
    std::string smsg = jsonStr.dump();
    int32_t ret = SendMessage(sinkSessionId, smsg);
    if (ret != DH_SUCCESS) {
        DHLOGE("StopRemoteInputDhids deviceId:%s, smsg:%s, SendMsg error, ret:%d.",
            GetAnonyString(deviceId).c_str(), SetAnonyId(smsg).c_str(), ret);
        return ERR_DH_INPUT_SERVER_SOURCE_TRANSPORT_STOP_FAIL;
    }
    DHLOGI("StopRemoteInputDhids send success, devId:%s, msg:%s.", GetAnonyString(deviceId).c_str(),
        SetAnonyId(smsg).c_str());
    return DH_SUCCESS;
}

int32_t DistributedInputSourceTransport::StartRemoteInputType(int32_t srcTsrcSeId, const std::string &deviceId,
    const uint32_t& inputTypes)
{
    int32_t sinkSessionId = DistributedInputTransportBase::GetInstance().GetSessionIdByDevId(deviceId);
    if (sinkSessionId < 0) {
        DHLOGE("StartRemoteInputType error, not find this device:%s.", GetAnonyString(deviceId).c_str());
        return ERR_DH_INPUT_SERVER_SOURCE_TRANSPORT_START_FAIL;
    }
    DHLOGI("StartRemoteInputType srcTsrcSeId:%d, sinkSessionId:%d.", srcTsrcSeId, sinkSessionId);

    nlohmann::json jsonStr;
    jsonStr[DINPUT_SOFTBUS_KEY_CMD_TYPE] = TRANS_SOURCE_MSG_START_TYPE_FOR_REL;
    jsonStr[DINPUT_SOFTBUS_KEY_DEVICE_ID] = deviceId;
    jsonStr[DINPUT_SOFTBUS_KEY_SESSION_ID] = srcTsrcSeId;
    jsonStr[DINPUT_SOFTBUS_KEY_INPUT_TYPE] = inputTypes;
    std::string smsg = jsonStr.dump();
    int32_t ret = SendMessage(sinkSessionId, smsg);
    if (ret != DH_SUCCESS) {
        DHLOGE("StartRemoteInputType deviceId:%s, smsg:%s, SendMsg error, ret:%d.",
            GetAnonyString(deviceId).c_str(), SetAnonyId(smsg).c_str(), ret);
        return ERR_DH_INPUT_SERVER_SOURCE_TRANSPORT_START_FAIL;
    }
    DHLOGI("StartRemoteInputType send success, devId:%s, msg:%s.", GetAnonyString(deviceId).c_str(),
        SetAnonyId(smsg).c_str());
    DistributedInputInject::GetInstance().StartInjectThread();
    DHLOGI("StartInjectThread successed");
    return DH_SUCCESS;
}

int32_t DistributedInputSourceTransport::StopRemoteInputType(int32_t srcTsrcSeId, const std::string &deviceId,
    const uint32_t& inputTypes)
{
    int32_t sinkSessionId = DistributedInputTransportBase::GetInstance().GetSessionIdByDevId(deviceId);
    if (sinkSessionId < 0) {
        DHLOGE("StopRemoteInputType error, not find this device:%s.", GetAnonyString(deviceId).c_str());
        return ERR_DH_INPUT_SERVER_SOURCE_TRANSPORT_STOP_FAIL;
    }
    DHLOGI("StopRemoteInputType srcTsrcSeId:%d, sinkSessionId:%d.", srcTsrcSeId, sinkSessionId);

    nlohmann::json jsonStr;
    jsonStr[DINPUT_SOFTBUS_KEY_CMD_TYPE] = TRANS_SOURCE_MSG_STOP_TYPE_FOR_REL;
    jsonStr[DINPUT_SOFTBUS_KEY_DEVICE_ID] = deviceId;
    jsonStr[DINPUT_SOFTBUS_KEY_SESSION_ID] = srcTsrcSeId;
    jsonStr[DINPUT_SOFTBUS_KEY_INPUT_TYPE] = inputTypes;
    std::string smsg = jsonStr.dump();
    int32_t ret = SendMessage(sinkSessionId, smsg);
    if (ret != DH_SUCCESS) {
        DHLOGE("StopRemoteInputType deviceId:%s, smsg:%s, SendMsg error, ret:%d.",
            GetAnonyString(deviceId).c_str(), SetAnonyId(smsg).c_str(), ret);
        return ERR_DH_INPUT_SERVER_SOURCE_TRANSPORT_STOP_FAIL;
    }
    DHLOGI("StopRemoteInputType send success, devId:%s, msg:%s.", GetAnonyString(deviceId).c_str(),
        SetAnonyId(smsg).c_str());
    return DH_SUCCESS;
}

int32_t DistributedInputSourceTransport::SendRelayPrepareRequest(const std::string &srcId, const std::string &sinkId)
{
    int32_t sessionId = DistributedInputTransportBase::GetInstance().GetSessionIdByDevId(srcId);
    if (sessionId < 0) {
        DHLOGE("SendRelayPrepareRequest error, not find this device:%s.", GetAnonyString(srcId).c_str());
        return ERR_DH_INPUT_SERVER_SOURCE_TRANSPORT_PREPARE_FAIL;
    }
    DHLOGI("SendRelayPrepareRequest sessionId:%d.", sessionId);

    nlohmann::json jsonStr;
    jsonStr[DINPUT_SOFTBUS_KEY_CMD_TYPE] = TRANS_SOURCE_TO_SOURCE_MSG_PREPARE;
    jsonStr[DINPUT_SOFTBUS_KEY_DEVICE_ID] = sinkId;
    std::string smsg = jsonStr.dump();
    int32_t ret = SendMessage(sessionId, smsg);
    if (ret != DH_SUCCESS) {
        DHLOGE("SendRelayPrepareRequest srcId:%s, sessionId:%d, smsg:%s, SendMsg error, ret:%d.",
            GetAnonyString(srcId).c_str(), sessionId, SetAnonyId(smsg).c_str(), ret);
        return ERR_DH_INPUT_SERVER_SOURCE_TRANSPORT_PREPARE_FAIL;
    }
    DHLOGI("SendRelayPrepareRequest srcId:%s, sessionId:%d, smsg:%s.",
        GetAnonyString(srcId).c_str(), sessionId, SetAnonyId(smsg).c_str());
    return DH_SUCCESS;
}

int32_t DistributedInputSourceTransport::SendRelayUnprepareRequest(const std::string &srcId, const std::string &sinkId)
{
    int32_t sessionId = DistributedInputTransportBase::GetInstance().GetSessionIdByDevId(srcId);
    if (sessionId < 0) {
        DHLOGE("SendRelayUnprepareRequest error, not find this device:%s.", GetAnonyString(srcId).c_str());
        return ERR_DH_INPUT_SERVER_SOURCE_TRANSPORT_PREPARE_FAIL;
    }
    DHLOGI("SendRelayUnprepareRequest sessionId:%d.", sessionId);

    nlohmann::json jsonStr;
    jsonStr[DINPUT_SOFTBUS_KEY_CMD_TYPE] = TRANS_SOURCE_TO_SOURCE_MSG_UNPREPARE;
    jsonStr[DINPUT_SOFTBUS_KEY_DEVICE_ID] = sinkId;
    std::string smsg = jsonStr.dump();
    int32_t ret = SendMessage(sessionId, smsg);
    if (ret != DH_SUCCESS) {
        DHLOGE("SendRelayUnprepareRequest srcId:%s, sessionId:%d, smsg:%s, SendMsg error, ret:%d.",
            GetAnonyString(srcId).c_str(), sessionId, SetAnonyId(smsg).c_str(), ret);
        return ERR_DH_INPUT_SERVER_SOURCE_TRANSPORT_PREPARE_FAIL;
    }
    DHLOGI("SendRelayUnprepareRequest srcId:%s, sessionId:%d, smsg:%s.",
        GetAnonyString(srcId).c_str(), sessionId, SetAnonyId(smsg).c_str());
    return DH_SUCCESS;
}

int32_t DistributedInputSourceTransport::NotifyOriginPrepareResult(int32_t srcTsrcSeId, const std::string &srcId,
    const std::string &sinkId, int32_t status)
{
    std::unique_lock<std::mutex> sessionLock(operationMutex_);
    nlohmann::json jsonStr;
    jsonStr[DINPUT_SOFTBUS_KEY_CMD_TYPE] = TRANS_SOURCE_TO_SOURCE_MSG_PREPARE_RESULT;
    jsonStr[DINPUT_SOFTBUS_KEY_SRC_DEV_ID] = srcId;
    jsonStr[DINPUT_SOFTBUS_KEY_SINK_DEV_ID] = sinkId;
    jsonStr[DINPUT_SOFTBUS_KEY_RESP_VALUE] = status;

    std::string smsg = jsonStr.dump();
    int32_t ret = SendMessage(srcTsrcSeId, smsg);
    if (ret != DH_SUCCESS) {
        DHLOGE("NotifyOriginPrepareResult srcTsrcSeId:%s, smsg:%s, SendMsg error, ret:%d.",
            GetAnonyInt32(srcTsrcSeId).c_str(), SetAnonyId(smsg).c_str(), ret);
        return ERR_DH_INPUT_SERVER_SOURCE_TRANSPORT_PREPARE_FAIL;
    }
    DHLOGI("NotifyOriginPrepareResult srcTsrcSeId:%d, smsg:%s.", srcTsrcSeId, SetAnonyId(smsg).c_str());
    return DH_SUCCESS;
}

int32_t DistributedInputSourceTransport::NotifyOriginUnprepareResult(int32_t srcTsrcSeId, const std::string &srcId,
    const std::string &sinkId, int32_t status)
{
    std::unique_lock<std::mutex> sessionLock(operationMutex_);
    nlohmann::json jsonStr;
    jsonStr[DINPUT_SOFTBUS_KEY_CMD_TYPE] = TRANS_SOURCE_TO_SOURCE_MSG_UNPREPARE_RESULT;
    jsonStr[DINPUT_SOFTBUS_KEY_SRC_DEV_ID] = srcId;
    jsonStr[DINPUT_SOFTBUS_KEY_SINK_DEV_ID] = sinkId;
    jsonStr[DINPUT_SOFTBUS_KEY_RESP_VALUE] = status;

    std::string smsg = jsonStr.dump();
    int32_t ret = SendMessage(srcTsrcSeId, smsg);
    if (ret != DH_SUCCESS) {
        DHLOGE("NotifyOriginUnprepareResult srcTsrcSeId:%s, smsg:%s, SendMsg error, ret:%d.",
            GetAnonyInt32(srcTsrcSeId).c_str(), SetAnonyId(smsg).c_str(), ret);
        return ERR_DH_INPUT_SERVER_SOURCE_TRANSPORT_UNPREPARE_FAIL;
    }
    DHLOGI("NotifyOriginUnprepareResult srcTsrcSeId:%d, smsg:%s.", srcTsrcSeId, SetAnonyId(smsg).c_str());
    return DH_SUCCESS;
}

int32_t DistributedInputSourceTransport::NotifyOriginStartDhidResult(int32_t srcTsrcSeId, const std::string &srcId,
    const std::string &sinkId, int32_t status, const std::string &dhids)
{
    std::unique_lock<std::mutex> sessionLock(operationMutex_);
    nlohmann::json jsonStr;
    jsonStr[DINPUT_SOFTBUS_KEY_CMD_TYPE] = TRANS_SOURCE_TO_SOURCE_MSG_START_DHID_RESULT;
    jsonStr[DINPUT_SOFTBUS_KEY_SRC_DEV_ID] = srcId;
    jsonStr[DINPUT_SOFTBUS_KEY_SINK_DEV_ID] = sinkId;
    jsonStr[DINPUT_SOFTBUS_KEY_RESP_VALUE] = status;
    jsonStr[DINPUT_SOFTBUS_KEY_VECTOR_DHID] = dhids;

    std::string smsg = jsonStr.dump();
    int32_t ret = SendMessage(srcTsrcSeId, smsg);
    if (ret != DH_SUCCESS) {
        DHLOGE("NotifyOriginStartDhidResult srcTsrcSeId:%s, smsg:%s, SendMsg error, ret:%d.",
            GetAnonyInt32(srcTsrcSeId).c_str(), SetAnonyId(smsg).c_str(), ret);
        return ERR_DH_INPUT_SERVER_SOURCE_TRANSPORT_START_FAIL;
    }
    DHLOGI("NotifyOriginStartDhidResult srcTsrcSeId:%d, smsg:%s.", srcTsrcSeId, SetAnonyId(smsg).c_str());
    return DH_SUCCESS;
}

int32_t DistributedInputSourceTransport::NotifyOriginStopDhidResult(int32_t srcTsrcSeId, const std::string &srcId,
    const std::string &sinkId, int32_t status, const std::string &dhids)
{
    std::unique_lock<std::mutex> sessionLock(operationMutex_);
    nlohmann::json jsonStr;
    jsonStr[DINPUT_SOFTBUS_KEY_CMD_TYPE] = TRANS_SOURCE_TO_SOURCE_MSG_STOP_DHID_RESULT;
    jsonStr[DINPUT_SOFTBUS_KEY_SRC_DEV_ID] = srcId;
    jsonStr[DINPUT_SOFTBUS_KEY_SINK_DEV_ID] = sinkId;
    jsonStr[DINPUT_SOFTBUS_KEY_RESP_VALUE] = status;
    jsonStr[DINPUT_SOFTBUS_KEY_VECTOR_DHID] = dhids;

    std::string smsg = jsonStr.dump();
    int32_t ret = SendMessage(srcTsrcSeId, smsg);
    if (ret != DH_SUCCESS) {
        DHLOGE("NotifyOriginStopDhidResult srcTsrcSeId:%s, smsg:%s, SendMsg error, ret:%d.",
            GetAnonyInt32(srcTsrcSeId).c_str(), SetAnonyId(smsg).c_str(), ret);
        return ERR_DH_INPUT_SERVER_SOURCE_TRANSPORT_STOP_FAIL;
    }
    DHLOGI("NotifyOriginStopDhidResult srcTsrcSeId:%d, smsg:%s.", srcTsrcSeId, SetAnonyId(smsg).c_str());
    return DH_SUCCESS;
}

int32_t DistributedInputSourceTransport::NotifyOriginStartTypeResult(int32_t srcTsrcSeId, const std::string &srcId,
    const std::string &sinkId, int32_t status, uint32_t inputTypes)
{
    std::unique_lock<std::mutex> sessionLock(operationMutex_);
    nlohmann::json jsonStr;
    jsonStr[DINPUT_SOFTBUS_KEY_CMD_TYPE] = TRANS_SOURCE_TO_SOURCE_MSG_START_TYPE_RESULT;
    jsonStr[DINPUT_SOFTBUS_KEY_SRC_DEV_ID] = srcId;
    jsonStr[DINPUT_SOFTBUS_KEY_SINK_DEV_ID] = sinkId;
    jsonStr[DINPUT_SOFTBUS_KEY_RESP_VALUE] = status;
    jsonStr[DINPUT_SOFTBUS_KEY_INPUT_TYPE] = inputTypes;

    std::string smsg = jsonStr.dump();
    int32_t ret = SendMessage(srcTsrcSeId, smsg);
    if (ret != DH_SUCCESS) {
        DHLOGE("NotifyOriginStartTypeResult srcTsrcSeId:%s, smsg:%s, SendMsg error, ret:%d.",
            GetAnonyInt32(srcTsrcSeId).c_str(), SetAnonyId(smsg).c_str(), ret);
        return ERR_DH_INPUT_SERVER_SOURCE_TRANSPORT_START_FAIL;
    }
    DHLOGI("NotifyOriginStartTypeResult srcTsrcSeId:%d, smsg:%s.", srcTsrcSeId, SetAnonyId(smsg).c_str());
    return DH_SUCCESS;
}

int32_t DistributedInputSourceTransport::NotifyOriginStopTypeResult(int32_t srcTsrcSeId, const std::string &srcId,
    const std::string &sinkId, int32_t status, uint32_t inputTypes)
{
    std::unique_lock<std::mutex> sessionLock(operationMutex_);
    nlohmann::json jsonStr;
    jsonStr[DINPUT_SOFTBUS_KEY_CMD_TYPE] = TRANS_SOURCE_TO_SOURCE_MSG_STOP_TYPE_RESULT;
    jsonStr[DINPUT_SOFTBUS_KEY_SRC_DEV_ID] = srcId;
    jsonStr[DINPUT_SOFTBUS_KEY_SINK_DEV_ID] = sinkId;
    jsonStr[DINPUT_SOFTBUS_KEY_RESP_VALUE] = status;
    jsonStr[DINPUT_SOFTBUS_KEY_INPUT_TYPE] = inputTypes;

    std::string smsg = jsonStr.dump();
    int32_t ret = SendMessage(srcTsrcSeId, smsg);
    if (ret != DH_SUCCESS) {
        DHLOGE("NotifyOriginStopTypeResult srcTsrcSeId:%s, smsg:%s, SendMsg error, ret:%d.",
            GetAnonyInt32(srcTsrcSeId).c_str(), SetAnonyId(smsg).c_str(), ret);
        return ERR_DH_INPUT_SERVER_SOURCE_TRANSPORT_STOP_FAIL;
    }
    DHLOGI("NotifyOriginStopTypeResult srcTsrcSeId:%d, smsg:%s.", srcTsrcSeId, SetAnonyId(smsg).c_str());
    return DH_SUCCESS;
}


int32_t DistributedInputSourceTransport::StartRemoteInput(const std::string& deviceId, const uint32_t& inputTypes)
{
    int32_t sessionId = DistributedInputTransportBase::GetInstance().GetSessionIdByDevId(deviceId);
    if (sessionId < 0) {
        DHLOGE("StartRemoteInput error, not find this device:%s.", GetAnonyString(deviceId).c_str());
        return ERR_DH_INPUT_SERVER_SOURCE_TRANSPORT_START_FAIL;
    }
    DHLOGI("StartRemoteInput sessionId:%d.", sessionId);

    nlohmann::json jsonStr;
    jsonStr[DINPUT_SOFTBUS_KEY_CMD_TYPE] = TRANS_SOURCE_MSG_START_TYPE;
    jsonStr[DINPUT_SOFTBUS_KEY_DEVICE_ID] = deviceId;
    jsonStr[DINPUT_SOFTBUS_KEY_SESSION_ID] = sessionId;
    jsonStr[DINPUT_SOFTBUS_KEY_INPUT_TYPE] = inputTypes;
    std::string smsg = jsonStr.dump();
    int32_t ret = SendMessage(sessionId, smsg);
    if (ret != DH_SUCCESS) {
        DHLOGE("StartRemoteInput deviceId:%s, sessionId:%d, smsg:%s, SendMsg error, ret:%d.",
            GetAnonyString(deviceId).c_str(), sessionId, SetAnonyId(smsg).c_str(), ret);
        return ERR_DH_INPUT_SERVER_SOURCE_TRANSPORT_START_FAIL;
    }
    DHLOGI("StartRemoteInput deviceId:%s, sessionId:%d, smsg:%s.",
        GetAnonyString(deviceId).c_str(), sessionId, SetAnonyId(smsg).c_str());

    DistributedInputInject::GetInstance().StartInjectThread();
    DHLOGI("StartInjectThread successed");

    return DH_SUCCESS;
}

int32_t DistributedInputSourceTransport::StopRemoteInput(
    const std::string& deviceId, const uint32_t& inputTypes)
{
    int32_t sessionId = DistributedInputTransportBase::GetInstance().GetSessionIdByDevId(deviceId);
    if (sessionId < 0) {
        DHLOGE("StopRemoteInput error, not find this device:%s.", GetAnonyString(deviceId).c_str());
        return ERR_DH_INPUT_SERVER_SOURCE_TRANSPORT_STOP_FAIL;
    }
    DHLOGI("StopRemoteInput sessionId:%d.", sessionId);

    nlohmann::json jsonStr;
    jsonStr[DINPUT_SOFTBUS_KEY_CMD_TYPE] = TRANS_SOURCE_MSG_STOP_TYPE;
    jsonStr[DINPUT_SOFTBUS_KEY_DEVICE_ID] = deviceId;
    jsonStr[DINPUT_SOFTBUS_KEY_SESSION_ID] = sessionId;
    jsonStr[DINPUT_SOFTBUS_KEY_INPUT_TYPE] = inputTypes;
    std::string smsg = jsonStr.dump();
    int32_t ret = SendMessage(sessionId, smsg);
    if (ret != DH_SUCCESS) {
        DHLOGE("StopRemoteInput deviceId:%s, sessionId:%d, smsg:%s, SendMsg error, ret:%d.",
            GetAnonyString(deviceId).c_str(), sessionId, SetAnonyId(smsg).c_str(), ret);
        return ERR_DH_INPUT_SERVER_SOURCE_TRANSPORT_STOP_FAIL;
    }
    DHLOGI("StopRemoteInput deviceId:%s, sessionId:%d, smsg:%s.",
        GetAnonyString(deviceId).c_str(), sessionId, SetAnonyId(smsg).c_str());
    return DH_SUCCESS;
}

int32_t DistributedInputSourceTransport::StartRemoteInput(const std::string &deviceId,
    const std::vector<std::string> &dhids)
{
    int32_t sessionId = DistributedInputTransportBase::GetInstance().GetSessionIdByDevId(deviceId);
    if (sessionId < 0) {
        DHLOGE("StartRemoteInput error, not find this device:%s.", GetAnonyString(deviceId).c_str());
        return ERR_DH_INPUT_SERVER_SOURCE_TRANSPORT_START_FAIL;
    }
    DHLOGI("StartRemoteInput sessionId:%d.", sessionId);

    nlohmann::json jsonStr;
    jsonStr[DINPUT_SOFTBUS_KEY_CMD_TYPE] = TRANS_SOURCE_MSG_START_DHID;
    jsonStr[DINPUT_SOFTBUS_KEY_DEVICE_ID] = deviceId;
    jsonStr[DINPUT_SOFTBUS_KEY_SESSION_ID] = sessionId;
    jsonStr[DINPUT_SOFTBUS_KEY_VECTOR_DHID] = JointDhIds(dhids);
    std::string smsg = jsonStr.dump();
    int32_t ret = SendMessage(sessionId, smsg);
    if (ret != DH_SUCCESS) {
        DHLOGE("StartRemoteInput deviceId:%s, sessionId: %d, smsg:%s, SendMsg error, ret:%d.",
            GetAnonyString(deviceId).c_str(), sessionId, SetAnonyId(smsg).c_str(), ret);
        return ERR_DH_INPUT_SERVER_SOURCE_TRANSPORT_START_FAIL;
    }
    DHLOGI("StartRemoteInput deviceId:%s, sessionId: %d, smsg:%s.", GetAnonyString(deviceId).c_str(),
        sessionId, SetAnonyId(smsg).c_str());

    DistributedInputInject::GetInstance().StartInjectThread();
    DHLOGI("StartInjectThread successed");

    return DH_SUCCESS;
}

int32_t DistributedInputSourceTransport::StopRemoteInput(const std::string &deviceId,
    const std::vector<std::string> &dhids)
{
    int32_t sessionId = DistributedInputTransportBase::GetInstance().GetSessionIdByDevId(deviceId);
    if (sessionId < 0) {
        DHLOGE("StopRemoteInput error, not find this device:%s.", GetAnonyString(deviceId).c_str());
        return ERR_DH_INPUT_SERVER_SOURCE_TRANSPORT_STOP_FAIL;
    }
    DHLOGI("StopRemoteInput sessionId:%d.", sessionId);

    nlohmann::json jsonStr;
    jsonStr[DINPUT_SOFTBUS_KEY_CMD_TYPE] = TRANS_SOURCE_MSG_STOP_DHID;
    jsonStr[DINPUT_SOFTBUS_KEY_DEVICE_ID] = deviceId;
    jsonStr[DINPUT_SOFTBUS_KEY_SESSION_ID] = sessionId;
    jsonStr[DINPUT_SOFTBUS_KEY_VECTOR_DHID] = JointDhIds(dhids);
    std::string smsg = jsonStr.dump();
    int32_t ret = SendMessage(sessionId, smsg);
    if (ret != DH_SUCCESS) {
        DHLOGE("StopRemoteInput deviceId:%s, sessionId: %d, smsg:%s, SendMsg error, ret:%d.",
            GetAnonyString(deviceId).c_str(), sessionId, SetAnonyId(smsg).c_str(), ret);
        return ERR_DH_INPUT_SERVER_SOURCE_TRANSPORT_STOP_FAIL;
    }
    DHLOGI("StopRemoteInput deviceId:%s, sessionId: %d, smsg:%s.", GetAnonyString(deviceId).c_str(),
        sessionId, SetAnonyId(smsg).c_str());
    return DH_SUCCESS;
}

int32_t DistributedInputSourceTransport::SendRelayStartDhidRequest(const std::string &srcId, const std::string &sinkId,
    const std::vector<std::string> &dhids)
{
    int32_t sessionId = DistributedInputTransportBase::GetInstance().GetSessionIdByDevId(srcId);
    if (sessionId < 0) {
        DHLOGE("SendRelayStartDhidRequest error, not find this device:%s.", GetAnonyString(srcId).c_str());
        return ERR_DH_INPUT_SERVER_SOURCE_TRANSPORT_START_FAIL;
    }
    DHLOGI("SendRelayStartDhidRequest sessionId:%d.", sessionId);

    nlohmann::json jsonStr;
    jsonStr[DINPUT_SOFTBUS_KEY_CMD_TYPE] = TRANS_SOURCE_TO_SOURCE_MSG_START_DHID;
    jsonStr[DINPUT_SOFTBUS_KEY_DEVICE_ID] = sinkId;
    jsonStr[DINPUT_SOFTBUS_KEY_VECTOR_DHID] = JointDhIds(dhids);
    std::string smsg = jsonStr.dump();
    int32_t ret = SendMessage(sessionId, smsg);
    if (ret != DH_SUCCESS) {
        DHLOGE("SendRelayStartDhidRequest srcId:%s, sessionId:%d, smsg:%s, SendMsg error, ret:%d.",
            GetAnonyString(srcId).c_str(), sessionId, SetAnonyId(smsg).c_str(), ret);
        return ERR_DH_INPUT_SERVER_SOURCE_TRANSPORT_START_FAIL;
    }
    DHLOGI("SendRelayStartDhidRequest srcId:%s, sessionId:%d, smsg:%s.",
        GetAnonyString(srcId).c_str(), sessionId, SetAnonyId(smsg).c_str());
    return DH_SUCCESS;
}

int32_t DistributedInputSourceTransport::SendRelayStopDhidRequest(const std::string &srcId, const std::string &sinkId,
    const std::vector<std::string> &dhids)
{
    int32_t sessionId = DistributedInputTransportBase::GetInstance().GetSessionIdByDevId(srcId);
    if (sessionId < 0) {
        DHLOGE("SendRelayStopDhidRequest error, not find this device:%s.", GetAnonyString(srcId).c_str());
        return ERR_DH_INPUT_SERVER_SOURCE_TRANSPORT_STOP_FAIL;
    }
    DHLOGI("SendRelayStopDhidRequest sessionId:%d.", sessionId);

    nlohmann::json jsonStr;
    jsonStr[DINPUT_SOFTBUS_KEY_CMD_TYPE] = TRANS_SOURCE_TO_SOURCE_MSG_STOP_DHID;
    jsonStr[DINPUT_SOFTBUS_KEY_DEVICE_ID] = sinkId;
    jsonStr[DINPUT_SOFTBUS_KEY_VECTOR_DHID] = JointDhIds(dhids);
    std::string smsg = jsonStr.dump();
    int32_t ret = SendMessage(sessionId, smsg);
    if (ret != DH_SUCCESS) {
        DHLOGE("SendRelayStopDhidRequest srcId:%s, sessionId:%d, smsg:%s, SendMsg error, ret:%d.",
            GetAnonyString(srcId).c_str(), sessionId, SetAnonyId(smsg).c_str(), ret);
        return ERR_DH_INPUT_SERVER_SOURCE_TRANSPORT_STOP_FAIL;
    }
    DHLOGI("SendRelayStopDhidRequest srcId:%s, sessionId:%d, smsg:%s.",
        GetAnonyString(srcId).c_str(), sessionId, SetAnonyId(smsg).c_str());
    return DH_SUCCESS;
}

int32_t DistributedInputSourceTransport::SendRelayStartTypeRequest(const std::string &srcId, const std::string &sinkId,
    const uint32_t& inputTypes)
{
    int32_t sessionId = DistributedInputTransportBase::GetInstance().GetSessionIdByDevId(srcId);
    if (sessionId < 0) {
        DHLOGE("SendRelayStartTypeRequest error, not find this device:%s.", GetAnonyString(srcId).c_str());
        return ERR_DH_INPUT_SERVER_SOURCE_TRANSPORT_START_FAIL;
    }
    DHLOGI("SendRelayStartTypeRequest sessionId:%d.", sessionId);

    nlohmann::json jsonStr;
    jsonStr[DINPUT_SOFTBUS_KEY_CMD_TYPE] = TRANS_SOURCE_TO_SOURCE_MSG_START_TYPE;
    jsonStr[DINPUT_SOFTBUS_KEY_DEVICE_ID] = sinkId;
    jsonStr[DINPUT_SOFTBUS_KEY_INPUT_TYPE] = inputTypes;
    std::string smsg = jsonStr.dump();
    int32_t ret = SendMessage(sessionId, smsg);
    if (ret != DH_SUCCESS) {
        DHLOGE("SendRelayStartTypeRequest srcId:%s, sessionId:%d, smsg:%s, SendMsg error, ret:%d.",
            GetAnonyString(srcId).c_str(), sessionId, SetAnonyId(smsg).c_str(), ret);
        return ERR_DH_INPUT_SERVER_SOURCE_TRANSPORT_START_FAIL;
    }
    DHLOGI("SendRelayStartTypeRequest srcId:%s, sessionId:%d, smsg:%s.",
        GetAnonyString(srcId).c_str(), sessionId, SetAnonyId(smsg).c_str());
    return DH_SUCCESS;
}

int32_t DistributedInputSourceTransport::SendRelayStopTypeRequest(const std::string &srcId, const std::string &sinkId,
    const uint32_t& inputTypes)
{
    int32_t sessionId = DistributedInputTransportBase::GetInstance().GetSessionIdByDevId(srcId);
    if (sessionId < 0) {
        DHLOGE("SendRelayStopTypeRequest error, not find this device:%s.", GetAnonyString(srcId).c_str());
        return ERR_DH_INPUT_SERVER_SOURCE_TRANSPORT_STOP_FAIL;
    }
    DHLOGI("SendRelayStopTypeRequest sessionId:%d.", sessionId);

    nlohmann::json jsonStr;
    jsonStr[DINPUT_SOFTBUS_KEY_CMD_TYPE] = TRANS_SOURCE_TO_SOURCE_MSG_STOP_TYPE;
    jsonStr[DINPUT_SOFTBUS_KEY_DEVICE_ID] = sinkId;
    jsonStr[DINPUT_SOFTBUS_KEY_INPUT_TYPE] = inputTypes;
    std::string smsg = jsonStr.dump();
    int32_t ret = SendMessage(sessionId, smsg);
    if (ret != DH_SUCCESS) {
        DHLOGE("SendRelayStopTypeRequest srcId:%s, sessionId:%d, smsg:%s, SendMsg error, ret:%d.",
            GetAnonyString(srcId).c_str(), sessionId, SetAnonyId(smsg).c_str(), ret);
        return ERR_DH_INPUT_SERVER_SOURCE_TRANSPORT_STOP_FAIL;
    }
    DHLOGI("SendRelayStopTypeRequest srcId:%s, sessionId:%d, smsg:%s.",
        GetAnonyString(srcId).c_str(), sessionId, SetAnonyId(smsg).c_str());
    return DH_SUCCESS;
}

std::string DistributedInputSourceTransport::JointDhIds(const std::vector<std::string> &dhids)
{
    if (dhids.size() <= 0) {
        return "";
    }
    auto dotFold = [](std::string a, std::string b) {return std::move(a) + '.' + std::move(b);};
    return std::accumulate(std::next(dhids.begin()), dhids.end(), dhids[0], dotFold);
}

int32_t DistributedInputSourceTransport::SendMessage(int32_t sessionId, std::string &message)
{
    return DistributedInputTransportBase::GetInstance().SendMsg(sessionId, message);
}

int32_t DistributedInputSourceTransport::GetCurrentSessionId()
{
    return sessionId_;
}

DistributedInputSourceTransport::DInputTransbaseSourceListener::DInputTransbaseSourceListener(
    DistributedInputSourceTransport *transport)
{
    sourceTransportObj_ = transport;
    DHLOGI("DInputTransbaseSourceListener init.");
}

DistributedInputSourceTransport::DInputTransbaseSourceListener::~DInputTransbaseSourceListener()
{
    sourceTransportObj_ = nullptr;
    DHLOGI("DInputTransbaseSourceListener destory.");
}

void DistributedInputSourceTransport::NotifyResponsePrepareRemoteInput(int32_t sessionId,
    const nlohmann::json &recMsg)
{
    DHLOGI("OnBytesReceived cmdType is TRANS_SINK_MSG_ONPREPARE.");
    if (!recMsg[DINPUT_SOFTBUS_KEY_RESP_VALUE].is_boolean()) {
        DHLOGE("OnBytesReceived cmdType is TRANS_SINK_MSG_ONPREPARE, data type is error.");
        return;
    }
    std::string deviceId = DistributedInputTransportBase::GetInstance().GetDevIdBySessionId(sessionId);
    if (deviceId.empty()) {
        DHLOGE("OnBytesReceived cmdType is TRANS_SINK_MSG_ONPREPARE, deviceId is error.");
        return;
    }
    callback_->onResponsePrepareRemoteInput(deviceId, recMsg[DINPUT_SOFTBUS_KEY_RESP_VALUE]);
}

void DistributedInputSourceTransport::NotifyResponseUnprepareRemoteInput(int32_t sessionId,
    const nlohmann::json &recMsg)
{
    DHLOGI("OnBytesReceived cmdType is TRANS_SINK_MSG_ONUNPREPARE.");
    if (!recMsg[DINPUT_SOFTBUS_KEY_RESP_VALUE].is_boolean()) {
        DHLOGE("OnBytesReceived cmdType is TRANS_SINK_MSG_ONUNPREPARE data type is error.");
        return;
    }
    std::string deviceId = DistributedInputTransportBase::GetInstance().GetDevIdBySessionId(sessionId);
    if (deviceId.empty()) {
        DHLOGE("OnBytesReceived cmdType is TRANS_SINK_MSG_ONUNPREPARE, deviceId is error.");
        return;
    }
    callback_->onResponseUnprepareRemoteInput(deviceId, recMsg[DINPUT_SOFTBUS_KEY_RESP_VALUE]);
    CloseInputSoftbus(deviceId);
}

void DistributedInputSourceTransport::NotifyResponseStartRemoteInput(int32_t sessionId, const nlohmann::json &recMsg)
{
    DHLOGI("OnBytesReceived cmdType is TRANS_SINK_MSG_ONSTART.");
    if (!recMsg[DINPUT_SOFTBUS_KEY_RESP_VALUE].is_boolean() ||
        !recMsg[DINPUT_SOFTBUS_KEY_WHITE_LIST].is_string()) {
        DHLOGE("OnBytesReceived cmdType is TRANS_SINK_MSG_ONSTART, data type is error.");
        return;
    }
    std::string deviceId = DistributedInputTransportBase::GetInstance().GetDevIdBySessionId(sessionId);
    if (deviceId.empty()) {
        DHLOGE("OnBytesReceived cmdType is TRANS_SINK_MSG_ONSTART, deviceId is error.");
        return;
    }
    callback_->onResponseStartRemoteInput(
        deviceId, recMsg[DINPUT_SOFTBUS_KEY_INPUT_TYPE], recMsg[DINPUT_SOFTBUS_KEY_RESP_VALUE],
        recMsg[DINPUT_SOFTBUS_KEY_WHITE_LIST]);
}

void DistributedInputSourceTransport::NotifyResponseStopRemoteInput(int32_t sessionId, const nlohmann::json &recMsg)
{
    DHLOGI("OnBytesReceived cmdType is TRANS_SINK_MSG_ONSTOP.");
    if (!recMsg[DINPUT_SOFTBUS_KEY_RESP_VALUE].is_boolean()) {
        DHLOGE("OnBytesReceived cmdType TRANS_SINK_MSG_ONSTOP data type is error.");
        return;
    }
    std::string deviceId = DistributedInputTransportBase::GetInstance().GetDevIdBySessionId(sessionId);
    if (deviceId.empty()) {
        DHLOGE("OnBytesReceived cmdType TRANS_SINK_MSG_ONSTOP, deviceId is error.");
        return;
    }
    callback_->onResponseStopRemoteInput(
        deviceId, recMsg[DINPUT_SOFTBUS_KEY_INPUT_TYPE], recMsg[DINPUT_SOFTBUS_KEY_RESP_VALUE]);
}

void DistributedInputSourceTransport::NotifyResponseStartRemoteInputDhid(int32_t sessionId,
    const nlohmann::json &recMsg)
{
    DHLOGI("OnBytesReceived cmdType is TRANS_SINK_MSG_DHID_ONSTART.");
    if (!recMsg[DINPUT_SOFTBUS_KEY_RESP_VALUE].is_boolean() ||
        !recMsg[DINPUT_SOFTBUS_KEY_WHITE_LIST].is_string()) {
        DHLOGE("OnBytesReceived cmdType is TRANS_SINK_MSG_DHID_ONSTART, data type is error.");
        return;
    }
    std::string deviceId = DistributedInputTransportBase::GetInstance().GetDevIdBySessionId(sessionId);
    if (deviceId.empty()) {
        DHLOGE("OnBytesReceived cmdType is TRANS_SINK_MSG_DHID_ONSTART, deviceId is error.");
        return;
    }
    callback_->onResponseStartRemoteInputDhid(
        deviceId, recMsg[DINPUT_SOFTBUS_KEY_VECTOR_DHID], recMsg[DINPUT_SOFTBUS_KEY_RESP_VALUE],
        recMsg[DINPUT_SOFTBUS_KEY_WHITE_LIST]);
}

void DistributedInputSourceTransport::NotifyResponseStopRemoteInputDhid(int32_t sessionId, const nlohmann::json &recMsg)
{
    DHLOGI("OnBytesReceived cmdType is TRANS_SINK_MSG_DHID_ONSTOP.");
    if (!recMsg[DINPUT_SOFTBUS_KEY_RESP_VALUE].is_boolean()) {
        DHLOGE("OnBytesReceived cmdType is TRANS_SINK_MSG_DHID_ONSTOP, data type is error.");
        return;
    }
    std::string deviceId = DistributedInputTransportBase::GetInstance().GetDevIdBySessionId(sessionId);
    if (deviceId.empty()) {
        DHLOGE("OnBytesReceived cmdType is TRANS_SINK_MSG_DHID_ONSTOP, deviceId is error.");
        return;
    }
    callback_->onResponseStopRemoteInputDhid(
        deviceId, recMsg[DINPUT_SOFTBUS_KEY_VECTOR_DHID], recMsg[DINPUT_SOFTBUS_KEY_RESP_VALUE]);
}

void DistributedInputSourceTransport::NotifyResponseKeyState(int32_t sessionId, const nlohmann::json &recMsg)
{
    DHLOGI("OnBytesReceived cmdType is TRANS_SINK_MSG_KEY_STATE.");
    std::string deviceId = DistributedInputTransportBase::GetInstance().GetDevIdBySessionId(sessionId);
    if (deviceId.empty()) {
        DHLOGE("OnBytesReceived cmdType is TRANS_SINK_MSG_KEY_STATE, deviceId is error.");
        return;
    }
    callback_->onResponseKeyState(deviceId, recMsg[DINPUT_SOFTBUS_KEY_KEYSTATE_DHID],
        recMsg[DINPUT_SOFTBUS_KEY_KEYSTATE_TYPE], recMsg[DINPUT_SOFTBUS_KEY_KEYSTATE_CODE],
        recMsg[DINPUT_SOFTBUS_KEY_KEYSTATE_VALUE]);
}

void DistributedInputSourceTransport::NotifyReceivedEventRemoteInput(int32_t sessionId, const nlohmann::json &recMsg)
{
    DHLOGI("OnBytesReceived cmdType is TRANS_SINK_MSG_BODY_DATA.");
    if (!recMsg[DINPUT_SOFTBUS_KEY_INPUT_DATA].is_string()) {
        DHLOGE("OnBytesReceived cmdType is TRANS_SINK_MSG_BODY_DATA, data type is error.");
        return;
    }

    std::string deviceId = DistributedInputTransportBase::GetInstance().GetDevIdBySessionId(sessionId);
    if (deviceId.empty()) {
        DHLOGE("OnBytesReceived cmdType is TRANS_SINK_MSG_BODY_DATA, deviceId is error.");
        return;
    }
    std::string inputDataStr = recMsg[DINPUT_SOFTBUS_KEY_INPUT_DATA];
    callback_->onReceivedEventRemoteInput(deviceId, inputDataStr);
}

void DistributedInputSourceTransport::ReceiveSrcTSrcRelayPrepare(int32_t sessionId, const nlohmann::json &recMsg)
{
    DHLOGI("OnBytesReceived cmdType is TRANS_SOURCE_TO_SOURCE_MSG_PREPARE.");
    if (!recMsg[DINPUT_SOFTBUS_KEY_DEVICE_ID].is_string()) {
        DHLOGE("OnBytesReceived cmdType is TRANS_SOURCE_TO_SOURCE_MSG_PREPARE, data type is error.");
        return;
    }

    std::string peerDevId = recMsg[DINPUT_SOFTBUS_KEY_DEVICE_ID];

    int32_t ret = OpenInputSoftbus(peerDevId);
    if (ret != DH_SUCCESS) {
        callback_->onResponseRelayPrepareRemoteInput(sessionId, peerDevId, false);
        return;
    }

    ret = PrepareRemoteInput(sessionId, peerDevId);
    if (ret != DH_SUCCESS) {
        callback_->onResponseRelayPrepareRemoteInput(sessionId, peerDevId, false);
        return;
    }
}

void DistributedInputSourceTransport::ReceiveSrcTSrcRelayUnprepare(int32_t sessionId, const nlohmann::json &recMsg)
{
    DHLOGI("OnBytesReceived cmdType is TRANS_SOURCE_TO_SOURCE_MSG_UNPREPARE.");
    if (!recMsg[DINPUT_SOFTBUS_KEY_DEVICE_ID].is_string()) {
        DHLOGE("OnBytesReceived cmdType is TRANS_SOURCE_TO_SOURCE_MSG_UNPREPARE, data type is error.");
        return;
    }

    std::string peerDevId = recMsg[DINPUT_SOFTBUS_KEY_DEVICE_ID];
    int32_t ret = UnprepareRemoteInput(sessionId, peerDevId);
    if (ret != DH_SUCCESS) {
        DHLOGE("Can not send message by softbus, unprepare fail.");
        callback_->onResponseRelayUnprepareRemoteInput(sessionId, peerDevId, false);
        return;
    }
}

void DistributedInputSourceTransport::NotifyResponseRelayPrepareRemoteInput(int32_t sessionId,
    const nlohmann::json &recMsg)
{
    DHLOGI("OnBytesReceived cmdType is TRANS_SINK_MSG_ON_RELAY_PREPARE.");
    if (!recMsg[DINPUT_SOFTBUS_KEY_RESP_VALUE].is_boolean() ||
        !recMsg[DINPUT_SOFTBUS_KEY_SESSION_ID].is_number()) {
        DHLOGE("OnBytesReceived cmdType is TRANS_SINK_MSG_ON_RELAY_PREPARE, data type is error.");
        return;
    }
    std::string sinkDevId = DistributedInputTransportBase::GetInstance().GetDevIdBySessionId(sessionId);
    if (sinkDevId.empty()) {
        DHLOGE("OnBytesReceived cmdType is TRANS_SINK_MSG_ON_RELAY_PREPARE, sinkDevId is error.");
        return;
    }
    callback_->onResponseRelayPrepareRemoteInput(recMsg[DINPUT_SOFTBUS_KEY_SESSION_ID], sinkDevId,
        recMsg[DINPUT_SOFTBUS_KEY_RESP_VALUE]);
}

void DistributedInputSourceTransport::NotifyResponseRelayUnprepareRemoteInput(int32_t sessionId,
    const nlohmann::json &recMsg)
{
    DHLOGI("OnBytesReceived cmdType is TRANS_SINK_MSG_ON_RELAY_UNPREPARE.");
    if (!recMsg[DINPUT_SOFTBUS_KEY_RESP_VALUE].is_boolean() ||
        !recMsg[DINPUT_SOFTBUS_KEY_SESSION_ID].is_number()) {
        DHLOGE("OnBytesReceived cmdType is TRANS_SINK_MSG_ON_RELAY_UNPREPARE, data type is error.");
        return;
    }
    std::string sinkDevId = DistributedInputTransportBase::GetInstance().GetDevIdBySessionId(sessionId);
    if (sinkDevId.empty()) {
        DHLOGE("OnBytesReceived cmdType is TRANS_SINK_MSG_ON_RELAY_UNPREPARE, sinkDevId is error.");
        return;
    }
    callback_->onResponseRelayUnprepareRemoteInput(recMsg[DINPUT_SOFTBUS_KEY_SESSION_ID], sinkDevId,
        recMsg[DINPUT_SOFTBUS_KEY_RESP_VALUE]);

    int32_t toSrcSessionId = recMsg[DINPUT_SOFTBUS_KEY_SESSION_ID];
    if (toSrcSessionId != sessionId) {
        DHLOGE("Close to sink session.");
        CloseInputSoftbus(sinkDevId);
    }
}

void DistributedInputSourceTransport::ReceiveRelayPrepareResult(int32_t sessionId, const nlohmann::json &recMsg)
{
    DHLOGI("OnBytesReceived cmdType is TRANS_SOURCE_TO_SOURCE_MSG_PREPARE_RESULT.");
    if (!recMsg[DINPUT_SOFTBUS_KEY_SRC_DEV_ID].is_string() ||
        !recMsg[DINPUT_SOFTBUS_KEY_SINK_DEV_ID].is_string() ||
        !recMsg[DINPUT_SOFTBUS_KEY_RESP_VALUE].is_number()) {
        DHLOGE("OnBytesReceived cmdType is TRANS_SOURCE_TO_SOURCE_MSG_PREPARE_RESULT, data type error.");
        return;
    }

    std::string srcId = recMsg[DINPUT_SOFTBUS_KEY_SRC_DEV_ID];
    std::string sinkId = recMsg[DINPUT_SOFTBUS_KEY_SINK_DEV_ID];
    int32_t status = recMsg[DINPUT_SOFTBUS_KEY_RESP_VALUE];
    callback_->onReceiveRelayPrepareResult(status, srcId, sinkId);
}

void DistributedInputSourceTransport::ReceiveRelayUnprepareResult(int32_t sessionId, const nlohmann::json &recMsg)
{
    DHLOGI("OnBytesReceived cmdType is TRANS_SOURCE_TO_SOURCE_MSG_UNPREPARE_RESULT.");
    if (!recMsg[DINPUT_SOFTBUS_KEY_SRC_DEV_ID].is_string() ||
        !recMsg[DINPUT_SOFTBUS_KEY_SINK_DEV_ID].is_string() ||
        !recMsg[DINPUT_SOFTBUS_KEY_RESP_VALUE].is_number()) {
        DHLOGE("OnBytesReceived cmdType is TRANS_SOURCE_TO_SOURCE_MSG_UNPREPARE_RESULT, data type error.");
        return;
    }

    std::string srcId = recMsg[DINPUT_SOFTBUS_KEY_SRC_DEV_ID];
    std::string sinkId = recMsg[DINPUT_SOFTBUS_KEY_SINK_DEV_ID];
    int32_t status = recMsg[DINPUT_SOFTBUS_KEY_RESP_VALUE];
    callback_->onReceiveRelayUnprepareResult(status, srcId, sinkId);
    CloseInputSoftbus(srcId);
}

void DistributedInputSourceTransport::ReceiveSrcTSrcRelayStartDhid(int32_t sessionId, const nlohmann::json &recMsg)
{
    DHLOGI("OnBytesReceived cmdType is TRANS_SOURCE_TO_SOURCE_MSG_START_DHID.");
    if (!recMsg[DINPUT_SOFTBUS_KEY_DEVICE_ID].is_string() ||
        !recMsg[DINPUT_SOFTBUS_KEY_VECTOR_DHID].is_string()) {
        DHLOGE("OnBytesReceived cmdType is TRANS_SOURCE_TO_SOURCE_MSG_START_DHID, data type is error.");
        return;
    }
    std::string deviceId = recMsg[DINPUT_SOFTBUS_KEY_DEVICE_ID];
    std::string dhids =  recMsg[DINPUT_SOFTBUS_KEY_VECTOR_DHID];
    int32_t ret = StartRemoteInputDhids(sessionId, deviceId, dhids);
    if (ret != DH_SUCCESS) {
        DHLOGE("Can not send message by softbus, start fail.");
        std::string localNetworkId = GetLocalNetworkId();
        if (localNetworkId.empty()) {
            DHLOGE("Could not get local device id.");
            return;
        }
        NotifyOriginStartDhidResult(sessionId, localNetworkId, deviceId,
            ERR_DH_INPUT_SERVER_SOURCE_MANAGER_START_MSG_IS_BAD, dhids);
    }
}

void DistributedInputSourceTransport::ReceiveSrcTSrcRelayStopDhid(int32_t sessionId, const nlohmann::json &recMsg)
{
    DHLOGI("OnBytesReceived cmdType is TRANS_SOURCE_TO_SOURCE_MSG_STOP_DHID.");
    if (!recMsg[DINPUT_SOFTBUS_KEY_DEVICE_ID].is_string() ||
        !recMsg[DINPUT_SOFTBUS_KEY_VECTOR_DHID].is_string()) {
        DHLOGE("OnBytesReceived cmdType is TRANS_SOURCE_TO_SOURCE_MSG_STOP_DHID, data type is error.");
        return;
    }
    std::string deviceId = recMsg[DINPUT_SOFTBUS_KEY_DEVICE_ID];
    std::string dhids =  recMsg[DINPUT_SOFTBUS_KEY_VECTOR_DHID];
    int32_t ret = StopRemoteInputDhids(sessionId, deviceId, dhids);
    if (ret != DH_SUCCESS) {
        DHLOGE("Can not send message by softbus, start fail.");
        std::string localNetworkId = GetLocalNetworkId();
        if (localNetworkId.empty()) {
            DHLOGE("Could not get local device id.");
            return;
        }
        NotifyOriginStopDhidResult(sessionId, localNetworkId, deviceId,
            ERR_DH_INPUT_SERVER_SOURCE_MANAGER_STOP_MSG_IS_BAD, dhids);
    }
}

void DistributedInputSourceTransport::NotifyResponseRelayStartDhidRemoteInput(int32_t sessionId,
    const nlohmann::json &recMsg)
{
    DHLOGI("OnBytesReceived cmdType is TRANS_SINK_MSG_ON_RELAY_STARTDHID.");
    if (!recMsg[DINPUT_SOFTBUS_KEY_RESP_VALUE].is_boolean() ||
        !recMsg[DINPUT_SOFTBUS_KEY_SESSION_ID].is_number() ||
        !recMsg[DINPUT_SOFTBUS_KEY_VECTOR_DHID].is_string() ||
        !recMsg[DINPUT_SOFTBUS_KEY_WHITE_LIST].is_string()) {
        DHLOGE("OnBytesReceived cmdType is TRANS_SINK_MSG_ON_RELAY_STARTDHID, data type is error.");
        return;
    }
    std::string sinkDevId = DistributedInputTransportBase::GetInstance().GetDevIdBySessionId(sessionId);
    if (sinkDevId.empty()) {
        DHLOGE("OnBytesReceived cmdType is TRANS_SINK_MSG_ON_RELAY_STARTDHID, sinkDevId is error.");
        return;
    }
    std::string localNetworkId = GetLocalNetworkId();
    if (localNetworkId.empty()) {
        DHLOGE("Could not get local device id.");
        return;
    }
    int32_t srcTsrcSeId = recMsg[DINPUT_SOFTBUS_KEY_SESSION_ID];
    bool result = recMsg[DINPUT_SOFTBUS_KEY_RESP_VALUE];
    std::string dhids = recMsg[DINPUT_SOFTBUS_KEY_VECTOR_DHID];
    NotifyOriginStartDhidResult(srcTsrcSeId, localNetworkId, sinkDevId,
        result ? DH_SUCCESS : ERR_DH_INPUT_SERVER_SOURCE_MANAGER_START_MSG_IS_BAD, dhids);
    callback_->onResponseRelayStartDhidRemoteInput(sinkDevId, recMsg[DINPUT_SOFTBUS_KEY_WHITE_LIST]);
}

void DistributedInputSourceTransport::NotifyResponseRelayStopDhidRemoteInput(int32_t sessionId,
    const nlohmann::json &recMsg)
{
    DHLOGI("OnBytesReceived cmdType is TRANS_SINK_MSG_ON_RELAY_STOPDHID.");
    if (!recMsg[DINPUT_SOFTBUS_KEY_RESP_VALUE].is_boolean() ||
        !recMsg[DINPUT_SOFTBUS_KEY_SESSION_ID].is_number() ||
        !recMsg[DINPUT_SOFTBUS_KEY_VECTOR_DHID].is_string()) {
        DHLOGE("OnBytesReceived cmdType is TRANS_SINK_MSG_ON_RELAY_STOPDHID, data type is error.");
        return;
    }
    std::string sinkDevId = DistributedInputTransportBase::GetInstance().GetDevIdBySessionId(sessionId);
    if (sinkDevId.empty()) {
        DHLOGE("OnBytesReceived cmdType is TRANS_SINK_MSG_ON_RELAY_STOPDHID, sinkDevId is error.");
        return;
    }
    std::string localNetworkId = GetLocalNetworkId();
    if (localNetworkId.empty()) {
        DHLOGE("Could not get local device id.");
        return;
    }
    int32_t srcTsrcSeId = recMsg[DINPUT_SOFTBUS_KEY_SESSION_ID];
    bool result = recMsg[DINPUT_SOFTBUS_KEY_RESP_VALUE];
    std::string dhids = recMsg[DINPUT_SOFTBUS_KEY_VECTOR_DHID];
    NotifyOriginStopDhidResult(srcTsrcSeId, localNetworkId, sinkDevId,
        result ? DH_SUCCESS : ERR_DH_INPUT_SERVER_SOURCE_MANAGER_STOP_MSG_IS_BAD, dhids);
}

void DistributedInputSourceTransport::ReceiveRelayStartDhidResult(int32_t sessionId, const nlohmann::json &recMsg)
{
    DHLOGI("OnBytesReceived cmdType is TRANS_SOURCE_TO_SOURCE_MSG_START_DHID_RESULT.");
    if (!recMsg[DINPUT_SOFTBUS_KEY_SRC_DEV_ID].is_string() ||
        !recMsg[DINPUT_SOFTBUS_KEY_SINK_DEV_ID].is_string() ||
        !recMsg[DINPUT_SOFTBUS_KEY_RESP_VALUE].is_number() ||
        !recMsg[DINPUT_SOFTBUS_KEY_VECTOR_DHID].is_string()) {
        DHLOGE("OnBytesReceived cmdType is TRANS_SOURCE_TO_SOURCE_MSG_START_DHID_RESULT, data type error.");
        return;
    }

    std::string srcId = recMsg[DINPUT_SOFTBUS_KEY_SRC_DEV_ID];
    std::string sinkId = recMsg[DINPUT_SOFTBUS_KEY_SINK_DEV_ID];
    int32_t status = recMsg[DINPUT_SOFTBUS_KEY_RESP_VALUE];
    std::string dhids = recMsg[DINPUT_SOFTBUS_KEY_VECTOR_DHID];
    callback_->onReceiveRelayStartDhidResult(status, srcId, sinkId, dhids);
}

void DistributedInputSourceTransport::ReceiveRelayStopDhidResult(int32_t sessionId, const nlohmann::json &recMsg)
{
    DHLOGI("OnBytesReceived cmdType is TRANS_SOURCE_TO_SOURCE_MSG_STOP_DHID_RESULT.");
    if (!recMsg[DINPUT_SOFTBUS_KEY_SRC_DEV_ID].is_string() ||
        !recMsg[DINPUT_SOFTBUS_KEY_SINK_DEV_ID].is_string() ||
        !recMsg[DINPUT_SOFTBUS_KEY_RESP_VALUE].is_number() ||
        !recMsg[DINPUT_SOFTBUS_KEY_VECTOR_DHID].is_string()) {
        DHLOGE("OnBytesReceived cmdType is TRANS_SOURCE_TO_SOURCE_MSG_STOP_DHID_RESULT, data type error.");
        return;
    }

    std::string srcId = recMsg[DINPUT_SOFTBUS_KEY_SRC_DEV_ID];
    std::string sinkId = recMsg[DINPUT_SOFTBUS_KEY_SINK_DEV_ID];
    int32_t status = recMsg[DINPUT_SOFTBUS_KEY_RESP_VALUE];
    std::string dhids = recMsg[DINPUT_SOFTBUS_KEY_VECTOR_DHID];
    callback_->onReceiveRelayStopDhidResult(status, srcId, sinkId, dhids);
}

void DistributedInputSourceTransport::ReceiveSrcTSrcRelayStartType(int32_t sessionId, const nlohmann::json &recMsg)
{
    DHLOGI("OnBytesReceived cmdType is TRANS_SOURCE_TO_SOURCE_MSG_START_TYPE.");
    if (!recMsg[DINPUT_SOFTBUS_KEY_DEVICE_ID].is_string() ||
        !recMsg[DINPUT_SOFTBUS_KEY_INPUT_TYPE].is_number()) {
        DHLOGE("OnBytesReceived cmdType is TRANS_SOURCE_TO_SOURCE_MSG_START_TYPE, data type is error.");
        return;
    }
    std::string deviceId = recMsg[DINPUT_SOFTBUS_KEY_DEVICE_ID];
    int32_t inputTypes =  recMsg[DINPUT_SOFTBUS_KEY_INPUT_TYPE];
    int32_t ret = StartRemoteInputType(sessionId, deviceId, inputTypes);
    if (ret != DH_SUCCESS) {
        DHLOGE("Can not send message by softbus, start fail.");
        std::string localNetworkId = GetLocalNetworkId();
        if (localNetworkId.empty()) {
            DHLOGE("Could not get local device id.");
            return;
        }
        NotifyOriginStartTypeResult(sessionId, localNetworkId, deviceId,
            ERR_DH_INPUT_SERVER_SOURCE_MANAGER_START_MSG_IS_BAD, inputTypes);
    }
}

void DistributedInputSourceTransport::ReceiveSrcTSrcRelayStopType(int32_t sessionId, const nlohmann::json &recMsg)
{
    DHLOGI("OnBytesReceived cmdType is TRANS_SOURCE_TO_SOURCE_MSG_STOP_TYPE.");
    if (!recMsg[DINPUT_SOFTBUS_KEY_DEVICE_ID].is_string() ||
        !recMsg[DINPUT_SOFTBUS_KEY_INPUT_TYPE].is_number()) {
        DHLOGE("OnBytesReceived cmdType is TRANS_SOURCE_TO_SOURCE_MSG_STOP_TYPE, data type is error.");
        return;
    }
    std::string deviceId = recMsg[DINPUT_SOFTBUS_KEY_DEVICE_ID];
    int32_t inputTypes =  recMsg[DINPUT_SOFTBUS_KEY_INPUT_TYPE];
    int32_t ret = StopRemoteInputType(sessionId, deviceId, inputTypes);
    if (ret != DH_SUCCESS) {
        DHLOGE("Can not send message by softbus, start fail.");
        std::string localNetworkId = GetLocalNetworkId();
        if (localNetworkId.empty()) {
            DHLOGE("Could not get local device id.");
            return;
        }
        NotifyOriginStopTypeResult(sessionId, localNetworkId, deviceId,
            ERR_DH_INPUT_SERVER_SOURCE_MANAGER_STOP_MSG_IS_BAD, inputTypes);
    }
}

void DistributedInputSourceTransport::NotifyResponseRelayStartTypeRemoteInput(int32_t sessionId,
    const nlohmann::json &recMsg)
{
    DHLOGI("OnBytesReceived cmdType is TRANS_SINK_MSG_ON_RELAY_STARTTYPE.");
    if (!recMsg[DINPUT_SOFTBUS_KEY_RESP_VALUE].is_boolean() ||
        !recMsg[DINPUT_SOFTBUS_KEY_SESSION_ID].is_number() ||
        !recMsg[DINPUT_SOFTBUS_KEY_INPUT_TYPE].is_number() ||
        !recMsg[DINPUT_SOFTBUS_KEY_WHITE_LIST].is_string()) {
        DHLOGE("OnBytesReceived cmdType is TRANS_SINK_MSG_ON_RELAY_STARTTYPE, data type is error.");
        return;
    }
    std::string sinkDevId = DistributedInputTransportBase::GetInstance().GetDevIdBySessionId(sessionId);
    if (sinkDevId.empty()) {
        DHLOGE("OnBytesReceived cmdType is TRANS_SINK_MSG_ON_RELAY_STARTTYPE, sinkDevId is error.");
        return;
    }
    std::string localNetworkId = GetLocalNetworkId();
    if (localNetworkId.empty()) {
        DHLOGE("Could not get local device id.");
        return;
    }
    int32_t srcTsrcSeId = recMsg[DINPUT_SOFTBUS_KEY_SESSION_ID];
    bool result = recMsg[DINPUT_SOFTBUS_KEY_RESP_VALUE];
    uint32_t inputTypes = recMsg[DINPUT_SOFTBUS_KEY_INPUT_TYPE];
    NotifyOriginStartTypeResult(srcTsrcSeId, localNetworkId, sinkDevId,
        result ? DH_SUCCESS : ERR_DH_INPUT_SERVER_SOURCE_MANAGER_START_MSG_IS_BAD, inputTypes);
    callback_->onResponseRelayStartTypeRemoteInput(sinkDevId, recMsg[DINPUT_SOFTBUS_KEY_WHITE_LIST]);
}

void DistributedInputSourceTransport::NotifyResponseRelayStopTypeRemoteInput(int32_t sessionId,
    const nlohmann::json &recMsg)
{
    DHLOGI("OnBytesReceived cmdType is TRANS_SINK_MSG_ON_RELAY_STOPTYPE.");
    if (!recMsg[DINPUT_SOFTBUS_KEY_RESP_VALUE].is_boolean() ||
        !recMsg[DINPUT_SOFTBUS_KEY_SESSION_ID].is_number() ||
        !recMsg[DINPUT_SOFTBUS_KEY_INPUT_TYPE].is_number()) {
        DHLOGE("OnBytesReceived cmdType is TRANS_SINK_MSG_ON_RELAY_STOPTYPE, data type is error.");
        return;
    }
    std::string sinkDevId = DistributedInputTransportBase::GetInstance().GetDevIdBySessionId(sessionId);
    if (sinkDevId.empty()) {
        DHLOGE("OnBytesReceived cmdType is TRANS_SINK_MSG_ON_RELAY_STOPTYPE, sinkDevId is error.");
        return;
    }
    std::string localNetworkId = GetLocalNetworkId();
    if (localNetworkId.empty()) {
        DHLOGE("Could not get local device id.");
        return;
    }
    int32_t srcTsrcSeId = recMsg[DINPUT_SOFTBUS_KEY_SESSION_ID];
    bool result = recMsg[DINPUT_SOFTBUS_KEY_RESP_VALUE];
    uint32_t inputTypes = recMsg[DINPUT_SOFTBUS_KEY_INPUT_TYPE];
    NotifyOriginStopTypeResult(srcTsrcSeId, localNetworkId, sinkDevId,
        result ? DH_SUCCESS : ERR_DH_INPUT_SERVER_SOURCE_MANAGER_STOP_MSG_IS_BAD, inputTypes);
}

void DistributedInputSourceTransport::ReceiveRelayStartTypeResult(int32_t sessionId, const nlohmann::json &recMsg)
{
    DHLOGI("OnBytesReceived cmdType is TRANS_SOURCE_TO_SOURCE_MSG_START_TYPE_RESULT.");
    if (!recMsg[DINPUT_SOFTBUS_KEY_SRC_DEV_ID].is_string() ||
        !recMsg[DINPUT_SOFTBUS_KEY_SINK_DEV_ID].is_string() ||
        !recMsg[DINPUT_SOFTBUS_KEY_RESP_VALUE].is_number() ||
        !recMsg[DINPUT_SOFTBUS_KEY_INPUT_TYPE].is_number()) {
        DHLOGE("OnBytesReceived cmdType is TRANS_SOURCE_TO_SOURCE_MSG_START_TYPE_RESULT, data type error.");
        return;
    }

    std::string srcId = recMsg[DINPUT_SOFTBUS_KEY_SRC_DEV_ID];
    std::string sinkId = recMsg[DINPUT_SOFTBUS_KEY_SINK_DEV_ID];
    int32_t status = recMsg[DINPUT_SOFTBUS_KEY_RESP_VALUE];
    uint32_t inputTypes = recMsg[DINPUT_SOFTBUS_KEY_INPUT_TYPE];
    callback_->onReceiveRelayStartTypeResult(status, srcId, sinkId, inputTypes);
}

void DistributedInputSourceTransport::ReceiveRelayStopTypeResult(int32_t sessionId, const nlohmann::json &recMsg)
{
    DHLOGI("OnBytesReceived cmdType is TRANS_SOURCE_TO_SOURCE_MSG_STOP_TYPE_RESULT.");
    if (!recMsg[DINPUT_SOFTBUS_KEY_SRC_DEV_ID].is_string() ||
        !recMsg[DINPUT_SOFTBUS_KEY_SINK_DEV_ID].is_string() ||
        !recMsg[DINPUT_SOFTBUS_KEY_RESP_VALUE].is_number() ||
        !recMsg[DINPUT_SOFTBUS_KEY_INPUT_TYPE].is_number()) {
        DHLOGE("OnBytesReceived cmdType is TRANS_SOURCE_TO_SOURCE_MSG_STOP_TYPE_RESULT, data type error.");
        return;
    }

    std::string srcId = recMsg[DINPUT_SOFTBUS_KEY_SRC_DEV_ID];
    std::string sinkId = recMsg[DINPUT_SOFTBUS_KEY_SINK_DEV_ID];
    int32_t status = recMsg[DINPUT_SOFTBUS_KEY_RESP_VALUE];
    uint32_t inputTypes = recMsg[DINPUT_SOFTBUS_KEY_INPUT_TYPE];
    callback_->onReceiveRelayStopTypeResult(status, srcId, sinkId, inputTypes);
}

void DistributedInputSourceTransport::DInputTransbaseSourceListener::HandleSessionData(int32_t sessionId,
    const std::string& message)
{
    DistributedInputSourceTransport::GetInstance().HandleData(sessionId, message);
}

void DistributedInputSourceTransport::HandleData(int32_t sessionId, const std::string& message)
{
    if (callback_ == nullptr) {
        DHLOGE("OnBytesReceived the callback_ is null, the message:%s abort.", SetAnonyId(message).c_str());
        return;
    }

    nlohmann::json recMsg = nlohmann::json::parse(message, nullptr, false);
    uint32_t cmdType = recMsg[DINPUT_SOFTBUS_KEY_CMD_TYPE];
    auto iter = memberFuncMap_.find(cmdType);
    if (iter == memberFuncMap_.end()) {
        DHLOGE("OnBytesReceived cmdType %d is undefined.", cmdType);
        return;
    }
    SourceTransportFunc &func = iter->second;
    (this->*func)(sessionId, recMsg);
}

} // namespace DistributedInput
} // namespace DistributedHardware
} // namespace OHOS
