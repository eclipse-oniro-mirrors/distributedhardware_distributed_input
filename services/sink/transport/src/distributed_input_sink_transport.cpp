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

#include "distributed_input_sink_transport.h"

#include <cinttypes>

#include "linux/input.h"

#include "anonymous_string.h"
#include "distributed_hardware_log.h"
#include "securec.h"

#include "constants_dinput.h"
#include "dinput_errcode.h"
#include "dinput_low_latency.h"
#include "dinput_softbus_define.h"
#include "dinput_utils_tool.h"
#include "hidumper.h"
#include "session.h"
#include "softbus_bus_center.h"

namespace OHOS {
namespace DistributedHardware {
namespace DistributedInput {
DistributedInputSinkTransport::DistributedInputSinkTransport() : sessionDevMap_({}), mySessionName_("")
{
    std::shared_ptr<AppExecFwk::EventRunner> runner = AppExecFwk::EventRunner::Create(true);
    eventHandler_ = std::make_shared<DistributedInputSinkTransport::DInputSinkEventHandler>(runner);
    DHLOGI("DistributedInputSinkTransport eventHandler_");
}

DistributedInputSinkTransport::~DistributedInputSinkTransport()
{
    DHLOGI("~DistributedInputSinkTransport");
    sessionDevMap_.clear();
    (void)RemoveSessionServer(DINPUT_PKG_NAME.c_str(), mySessionName_.c_str());
}

DistributedInputSinkTransport::DInputSinkEventHandler::DInputSinkEventHandler(
    const std::shared_ptr<AppExecFwk::EventRunner> &runner) : AppExecFwk::EventHandler(runner)
{
}

static int32_t SessionOpened(int32_t sessionId, int32_t result)
{
    return DistributedInput::DistributedInputSinkTransport::GetInstance().OnSessionOpened(sessionId, result);
}

static void SessionClosed(int32_t sessionId)
{
    DistributedInput::DistributedInputSinkTransport::GetInstance().OnSessionClosed(sessionId);
}

static void BytesReceived(int32_t sessionId, const void *data, uint32_t dataLen)
{
    DistributedInput::DistributedInputSinkTransport::GetInstance().OnBytesReceived(sessionId, data, dataLen);
}

static void MessageReceived(int32_t sessionId, const void *data, uint32_t dataLen)
{
    (void)sessionId;
    (void)data;
    (void)dataLen;
    DHLOGI("sessionId: %d, dataLen:%d", sessionId, dataLen);
}

static void StreamReceived(int32_t sessionId, const StreamData *data, const StreamData *ext,
    const StreamFrameInfo *param)
{
    (void)sessionId;
    (void)data;
    (void)ext;
    (void)param;
    DHLOGI("sessionId: %d", sessionId);
}
DistributedInputSinkTransport &DistributedInputSinkTransport::GetInstance()
{
    static DistributedInputSinkTransport instance;
    return instance;
}

void DistributedInputSinkTransport::DInputSinkEventHandler::ProcessEvent(const AppExecFwk::InnerEvent::Pointer &event)
{
    DHLOGI("ProcessEvent");
    EHandlerMsgType eventId = static_cast<EHandlerMsgType>(event->GetInnerEventId());
    switch (eventId) {
        case EHandlerMsgType::DINPUT_SINK_EVENT_HANDLER_MSG: {
            std::shared_ptr<nlohmann::json> innerMsg = event->GetSharedObject<nlohmann::json>();
            nlohmann::json sendMsg;
            sendMsg[DINPUT_SOFTBUS_KEY_CMD_TYPE] = TRANS_SINK_MSG_BODY_DATA;
            sendMsg[DINPUT_SOFTBUS_KEY_INPUT_DATA] = innerMsg->dump();
            std::string smsg = sendMsg.dump();
            RecordEventLog(innerMsg);
            int32_t sessionId = DistributedInputSinkSwitch::GetInstance().GetSwitchOpenedSession();
            if (sessionId > 0) {
                DistributedInputSinkTransport::GetInstance().SendMessage(sessionId, smsg);
            } else {
                DHLOGE("ProcessEvent can't send input data, because no session switch on.");
            }
            break;
        }
        default:
            DHLOGE("ProcessEvent error, because eventId is unkonwn.");
            break;
    }
}

int32_t DistributedInputSinkTransport::Init()
{
    // 1.create session
    DHLOGI("Init");
    ISessionListener iSessionListener = {
        .OnSessionOpened = SessionOpened,
        .OnSessionClosed = SessionClosed,
        .OnBytesReceived = BytesReceived,
        .OnMessageReceived = MessageReceived,
        .OnStreamReceived = StreamReceived
    };
    auto localNode = std::make_unique<NodeBasicInfo>();
    int32_t retCode = GetLocalNodeDeviceInfo(DINPUT_PKG_NAME.c_str(), localNode.get());
    if (retCode != DH_SUCCESS) {
        DHLOGE("Init could not get local device id.");
        return ERR_DH_INPUT_SERVER_SINK_TRANSPORT_INIT_FAIL;
    }
    std::string networkId = localNode->networkId;
    DHLOGI("Init device networkId is %s", GetAnonyString(networkId).c_str());
    mySessionName_ = SESSION_NAME_SINK + networkId.substr(0, INTERCEPT_STRING_LENGTH);

    int32_t ret = CreateSessionServer(DINPUT_PKG_NAME.c_str(), mySessionName_.c_str(), &iSessionListener);
    if (ret != DH_SUCCESS) {
        DHLOGE("Init CreateSessionServer failed, error code %d.", ret);
        return ERR_DH_INPUT_SERVER_SINK_TRANSPORT_INIT_FAIL;
    }
    return DH_SUCCESS;
}

std::shared_ptr<DistributedInputSinkTransport::DInputSinkEventHandler> DistributedInputSinkTransport::GetEventHandler()
{
    DHLOGI("GetEventHandler");
    return eventHandler_;
}

void DistributedInputSinkTransport::RegistSinkRespCallback(std::shared_ptr<DInputSinkTransCallback> callback)
{
    DHLOGI("RegistSinkRespCallback");
    callback_ = callback;
}

int32_t DistributedInputSinkTransport::RespPrepareRemoteInput(
    const int32_t sessionId, std::string &smsg)
{
    if (sessionId > 0) {
        DHLOGI("RespPrepareRemoteInput sessionId: %d, smsg:%s.", sessionId, SetAnonyId(smsg).c_str());
        int32_t ret = SendMessage(sessionId, smsg);
        if (ret != DH_SUCCESS) {
            DHLOGE("RespPrepareRemoteInput error, SendMessage fail.");
            return ERR_DH_INPUT_SERVER_SINK_TRANSPORT_RESPPREPARE_FAIL;
        }
        return DH_SUCCESS;
    } else {
        DHLOGE("RespPrepareRemoteInput error, sessionId <= 0.");
        return ERR_DH_INPUT_SERVER_SINK_TRANSPORT_RESPPREPARE_FAIL;
    }
}

int32_t DistributedInputSinkTransport::RespUnprepareRemoteInput(
    const int32_t sessionId, std::string &smsg)
{
    if (sessionId > 0) {
        DHLOGI("RespUnprepareRemoteInput sessionId: %d, smsg:%s.", sessionId, SetAnonyId(smsg).c_str());
        int32_t ret = SendMessage(sessionId, smsg);
        if (ret != DH_SUCCESS) {
            DHLOGE("RespUnprepareRemoteInput error, SendMessage fail.");
            return ERR_DH_INPUT_SERVER_SINK_TRANSPORT_RESPUNPREPARE_FAIL;
        }
        return DH_SUCCESS;
    } else {
        DHLOGE("RespUnprepareRemoteInput error, sessionId <= 0.");
        return ERR_DH_INPUT_SERVER_SINK_TRANSPORT_RESPUNPREPARE_FAIL;
    }
}

int32_t DistributedInputSinkTransport::RespStartRemoteInput(
    const int32_t sessionId, std::string &smsg)
{
    if (sessionId > 0) {
        DHLOGI("RespStartRemoteInput sessionId: %d, smsg:%s.", sessionId, SetAnonyId(smsg).c_str());
        int32_t ret = SendMessage(sessionId, smsg);
        if (ret != DH_SUCCESS) {
            DHLOGE("RespStartRemoteInput error, SendMessage fail.");
            return ERR_DH_INPUT_SERVER_SINK_TRANSPORT_RESPSTART_FAIL;
        }
        return DH_SUCCESS;
    } else {
        DHLOGE("RespStartRemoteInput error, sessionId <= 0.");
        return ERR_DH_INPUT_SERVER_SINK_TRANSPORT_RESPSTART_FAIL;
    }
}

int32_t DistributedInputSinkTransport::RespStopRemoteInput(const int32_t sessionId, std::string &smsg)
{
    if (sessionId > 0) {
        DHLOGI("RespStopRemoteInput sessionId: %d, smsg:%s.", sessionId, SetAnonyId(smsg).c_str());
        int32_t ret = SendMessage(sessionId, smsg);
        if (ret != DH_SUCCESS) {
            DHLOGE("RespStopRemoteInput error, SendMessage fail.");
            return ERR_DH_INPUT_SERVER_SINK_TRANSPORT_RESPSTOP_FAIL;
        }
        return DH_SUCCESS;
    } else {
        DHLOGE("RespStopRemoteInput error, sessionId <= 0.");
        return ERR_DH_INPUT_SERVER_SINK_TRANSPORT_RESPSTOP_FAIL;
    }
}

int32_t DistributedInputSinkTransport::RespLatency(const int32_t sessionId, std::string &smsg)
{
    if (sessionId <= 0) {
        DHLOGE("RespLatency error, sessionId <= 0.");
        return ERR_DH_INPUT_SERVER_SINK_TRANSPORT_RESP_LATENCY_FAIL;
    }

    int32_t ret = SendMessage(sessionId, smsg);
    if (ret != DH_SUCCESS) {
        DHLOGE("RespLatency error, SendMessage fail.");
        return ERR_DH_INPUT_SERVER_SINK_TRANSPORT_RESP_LATENCY_FAIL;
    }

    DHLOGI("RespLatency sessionId: %d, smsg:%s.", sessionId, SetAnonyId(smsg).c_str());
    return DH_SUCCESS;
}

void DistributedInputSinkTransport::SendKeyStateNodeMsg(const int32_t sessionId, const std::string& dhId,
    const uint32_t btnCode)
{
    if (sessionId <= 0) {
        DHLOGE("SendKeyStateNodeMsg error, sessionId <= 0.");
        return;
    }
    DHLOGI("SendKeyStateNodeMsg sessionId: %d, btnCode: %d.", sessionId, btnCode);
    nlohmann::json jsonStr;
    jsonStr[DINPUT_SOFTBUS_KEY_CMD_TYPE] = TRANS_SINK_MSG_KEY_STATE;
    jsonStr[DINPUT_SOFTBUS_KEY_KEYSTATE_DHID] = dhId;
    jsonStr[DINPUT_SOFTBUS_KEY_KEYSTATE_TYPE] = EV_KEY;
    jsonStr[DINPUT_SOFTBUS_KEY_KEYSTATE_CODE] = btnCode;
    jsonStr[DINPUT_SOFTBUS_KEY_KEYSTATE_VALUE] = KEY_DOWN_STATE;
    std::string msg = jsonStr.dump();
    int32_t ret = SendMessage(sessionId, msg);
    if (ret != DH_SUCCESS) {
        DHLOGE("SendKeyStateNodeMsg error, SendMessage fail.");
    }
}

int32_t DistributedInputSinkTransport::SendMessage(int32_t sessionId, std::string &message)
{
    DHLOGD("start SendMessage");
    if (message.size() > MSG_MAX_SIZE) {
        DHLOGE("SendMessage error: message.size() > MSG_MAX_SIZE");
        return ERR_DH_INPUT_SERVER_SINK_TRANSPORT_SENDMESSAGE_FAIL;
    }
    uint8_t *buf = (uint8_t *)calloc((MSG_MAX_SIZE), sizeof(uint8_t));
    if (buf == nullptr) {
        DHLOGE("SendMessage: malloc memory failed");
        return ERR_DH_INPUT_SERVER_SINK_TRANSPORT_SENDMESSAGE_FAIL;
    }
    int32_t outLen = 0;
    if (memcpy_s(buf, MSG_MAX_SIZE, (const uint8_t *)message.c_str(), message.size()) != DH_SUCCESS) {
        DHLOGE("SendMessage: memcpy memory failed");
        free(buf);
        return ERR_DH_INPUT_SERVER_SINK_TRANSPORT_SENDMESSAGE_FAIL;
    }
    outLen = (int32_t)(message.size());
    int32_t ret = SendBytes(sessionId, buf, outLen);
    free(buf);
    return ret;
}

int32_t DistributedInputSinkTransport::GetSessionIdByNetId(const std::string &srcId)
{
    std::map<std::string, int32_t>::iterator it = sessionDevMap_.find(srcId);
    if (it != sessionDevMap_.end()) {
        return it->second;
    }
    DHLOGE("get session id failed, srcId = %s", GetAnonyString(srcId).c_str());
    return ERR_DH_INPUT_SERVER_SINK_TRANSPORT_GET_SESSIONID_FAIL;
}

void DistributedInputSinkTransport::GetDeviceIdBySessionId(int32_t sessionId, std::string &srcId)
{
    for (auto iter = sessionDevMap_.begin(); iter != sessionDevMap_.end(); iter++) {
        if (sessionId == iter->second) {
            srcId = iter->first;
            return;
        }
    }
    srcId = "";
}

int32_t DistributedInputSinkTransport::OnSessionOpened(int32_t sessionId, int32_t result)
{
    if (result != DH_SUCCESS) {
        DHLOGE("session open failed, sessionId: %d", sessionId);
        return DH_SUCCESS;
    }

#ifdef DINPUT_LOW_LATENCY
    DInputLowLatency::GetInstance().EnableSinkLowLatency();
#endif

    // return 1 is client
    int32_t sessionSide = GetSessionSide(sessionId);
    DHLOGI("session open succeed, sessionId: %d, sessionSide %d", sessionId, sessionSide);

    char mySessionName[SESSION_NAME_SIZE_MAX] = "";
    char peerSessionName[SESSION_NAME_SIZE_MAX] = "";
    char peerDevId[DEVICE_ID_SIZE_MAX] = "";
    int ret = GetMySessionName(sessionId, mySessionName, sizeof(mySessionName));
    if (ret != DH_SUCCESS) {
        DHLOGE("get my session name failed, session id is %d", sessionId);
    }
    // get other device session name
    ret = GetPeerSessionName(sessionId, peerSessionName, sizeof(peerSessionName));
    if (ret != DH_SUCCESS) {
        DHLOGE("get my peer session name failed, session id is %d", sessionId);
    }

    ret = GetPeerDeviceId(sessionId, peerDevId, sizeof(peerDevId));
    if (ret != DH_SUCCESS) {
        DHLOGE("get my peer device id failed, session id is %d", sessionId);
    } else {
        sessionDevMap_[peerDevId] = sessionId;
    }
    DHLOGI("mySessionName:%s, peerSessionName:%s, peerDevId:%s",
        mySessionName, peerSessionName, GetAnonyString(peerDevId).c_str());
    HiDumper::GetInstance().CreateSessionInfo(std::string(peerDevId), sessionId, mySessionName, peerSessionName,
        SessionStatus::OPENED);
    return DH_SUCCESS;
}

void DistributedInputSinkTransport::OnSessionClosed(int32_t sessionId)
{
    DHLOGI("OnSessionClosed, sessionId: %d", sessionId);

#ifdef DINPUT_LOW_LATENCY
    DInputLowLatency::GetInstance().DisableSinkLowLatency();
#endif

    char peerDevId[DEVICE_ID_SIZE_MAX] = "";
    int ret = GetPeerDeviceId(sessionId, peerDevId, sizeof(peerDevId));
    if (ret != DH_SUCCESS) {
        DHLOGI("get my peer device id failed, session id is %d", sessionId);
    }
    for (auto iter = sessionDevMap_.begin(); iter != sessionDevMap_.end(); iter++) {
        if (iter->second == sessionId) {
            sessionDevMap_.erase(iter);
            break;
        }
    }
    DistributedInputSinkSwitch::GetInstance().RemoveSession(sessionId);
    HiDumper::GetInstance().SetSessionStatus(std::string(peerDevId), SessionStatus::CLOSED);
    HiDumper::GetInstance().DeleteSessionInfo(std::string(peerDevId));
}

void DistributedInputSinkTransport::OnBytesReceived(int32_t sessionId, const void *data, uint32_t dataLen)
{
    DHLOGI("OnBytesReceived, sessionId: %d, dataLen:%d", sessionId, dataLen);
    if (sessionId < 0 || data == nullptr || dataLen <= 0 || dataLen > MSG_MAX_SIZE) {
        DHLOGE("OnBytesReceived param check failed");
        return;
    }

    uint8_t *buf = (uint8_t *)calloc(dataLen + 1, sizeof(uint8_t));
    if (buf == nullptr) {
        DHLOGE("OnBytesReceived: malloc memory failed");
        return;
    }

    if (memcpy_s(buf, dataLen + 1, (const uint8_t*)data, dataLen) != DH_SUCCESS) {
        DHLOGE("OnBytesReceived: memcpy memory failed");
        free(buf);
        return;
    }

    std::string message(buf, buf + dataLen);
    DHLOGI("OnBytesReceived message:%s.", SetAnonyId(message).c_str());
    HandleSessionData(sessionId, message);

    free(buf);
    DHLOGI("OnBytesReceived completed");
    return;
}

void DistributedInputSinkTransport::NotifyPrepareRemoteInput(int32_t sessionId, const nlohmann::json &recMsg)
{
    DHLOGI("OnBytesReceived cmdType is TRANS_SOURCE_MSG_PREPARE.");
    if (!recMsg[DINPUT_SOFTBUS_KEY_DEVICE_ID].is_string()) {
        DHLOGE("OnBytesReceived cmdType TRANS_SOURCE_MSG_PREPARE data type error.");
        return;
    }
    std::string deviceId = recMsg[DINPUT_SOFTBUS_KEY_DEVICE_ID];
    DHLOGI("OnBytesReceived cmdType is TRANS_SOURCE_MSG_PREPARE deviceId:%s.",
        GetAnonyString(deviceId).c_str());
    callback_->onPrepareRemoteInput(sessionId, deviceId);
}

void DistributedInputSinkTransport::NotifyUnprepareRemoteInput(int32_t sessionId, const nlohmann::json &recMsg)
{
    if (!recMsg[DINPUT_SOFTBUS_KEY_DEVICE_ID].is_string()) {
        DHLOGE("OnBytesReceived cmdType TRANS_SOURCE_MSG_UNPREPARE, data type is error.");
        return;
    }
    std::string deviceId = recMsg[DINPUT_SOFTBUS_KEY_DEVICE_ID];
    DHLOGI("OnBytesReceived cmdType TRANS_SOURCE_MSG_UNPREPARE deviceId:%s.",
        GetAnonyString(deviceId).c_str());
    callback_->onUnprepareRemoteInput(sessionId);
}

void DistributedInputSinkTransport::NotifyStartRemoteInput(int32_t sessionId, const nlohmann::json &recMsg)
{
    if (!recMsg[DINPUT_SOFTBUS_KEY_DEVICE_ID].is_string()) {
        DHLOGE("OnBytesReceived cmdType TRANS_SOURCE_MSG_START_TYPE, data type error.");
        return;
    }
    std::string deviceId = recMsg[DINPUT_SOFTBUS_KEY_DEVICE_ID];
    uint32_t inputTypes = recMsg[DINPUT_SOFTBUS_KEY_INPUT_TYPE];
    DHLOGI("OnBytesRecei,ved cmdType is TRANS_SOURCE_MSG_START_TYPE deviceId:%s inputTypes:%d .",
        GetAnonyString(deviceId).c_str(), inputTypes);
    callback_->onStartRemoteInput(sessionId, inputTypes);
}

void DistributedInputSinkTransport::NotifyStopRemoteInput(int32_t sessionId, const nlohmann::json &recMsg)
{
    if (!recMsg[DINPUT_SOFTBUS_KEY_DEVICE_ID].is_string()) {
        DHLOGE("OnBytesReceived cmdType TRANS_SOURCE_MSG_STOP_TYPE, data type is error.");
        return;
    }
    std::string deviceId = recMsg[DINPUT_SOFTBUS_KEY_DEVICE_ID];
    uint32_t inputTypes = recMsg[DINPUT_SOFTBUS_KEY_INPUT_TYPE];
    DHLOGI("OnBytesReceived cmdType is TRANS_SOURCE_MSG_STOP_TYPE deviceId:%s.", GetAnonyString(deviceId).c_str());
    callback_->onStopRemoteInput(sessionId, inputTypes);
}

void DistributedInputSinkTransport::NotifyLatency(int32_t sessionId, const nlohmann::json &recMsg)
{
    if (recMsg.contains(DINPUT_SOFTBUS_KEY_DEVICE_ID) != true) {
        DHLOGE("OnBytesReceived message is error, not contain deviceId.");
        return;
    }

    if (!recMsg[DINPUT_SOFTBUS_KEY_DEVICE_ID].is_string()) {
        DHLOGE("OnBytesReceived cmdType TRANS_SOURCE_MSG_LATENCY, data type is error.");
        return;
    }

    nlohmann::json jsonStr;
    jsonStr[DINPUT_SOFTBUS_KEY_CMD_TYPE] = TRANS_SINK_MSG_LATENCY;
    jsonStr[DINPUT_SOFTBUS_KEY_RESP_VALUE] = true;
    std::string smsg = jsonStr.dump();
    RespLatency(sessionId, smsg);
}

void DistributedInputSinkTransport::NotifyStartRemoteInputDhid(int32_t sessionId, const nlohmann::json &recMsg)
{
    if (!recMsg.contains(DINPUT_SOFTBUS_KEY_DEVICE_ID) ||
        !recMsg.contains(DINPUT_SOFTBUS_KEY_VECTOR_DHID)) {
        DHLOGE("OnBytesReceived cmdType TRANS_SOURCE_MSG_START_DHID, key not exist.");
        return;
    }
    if (!recMsg[DINPUT_SOFTBUS_KEY_DEVICE_ID].is_string() ||
        !recMsg[DINPUT_SOFTBUS_KEY_VECTOR_DHID].is_string()) {
        DHLOGE("OnBytesReceived cmdType TRANS_SOURCE_MSG_START_DHID, data type error.");
        return;
    }
    std::string deviceId = recMsg[DINPUT_SOFTBUS_KEY_DEVICE_ID];
    std::string strTmp = recMsg[DINPUT_SOFTBUS_KEY_VECTOR_DHID];
    DHLOGI("OnBytesReceived cmdType is TRANS_SOURCE_MSG_START_DHID deviceId:%s .",
           GetAnonyString(deviceId).c_str());
    callback_->onStartRemoteInputDhid(sessionId, strTmp);
}

void DistributedInputSinkTransport::NotifyStopRemoteInputDhid(int32_t sessionId, const nlohmann::json &recMsg)
{
    if (!recMsg.contains(DINPUT_SOFTBUS_KEY_DEVICE_ID) ||
        !recMsg.contains(DINPUT_SOFTBUS_KEY_VECTOR_DHID)) {
        DHLOGE("OnBytesReceived cmdType TRANS_SOURCE_MSG_STOP_DHID, key not exist.");
        return;
    }
    if (!recMsg[DINPUT_SOFTBUS_KEY_DEVICE_ID].is_string() ||
        !recMsg[DINPUT_SOFTBUS_KEY_VECTOR_DHID].is_string()) {
        DHLOGE("OnBytesReceived cmdType TRANS_SOURCE_MSG_STOP_DHID, data type is error.");
        return;
    }
    std::string deviceId = recMsg[DINPUT_SOFTBUS_KEY_DEVICE_ID];
    std::string strTmp = recMsg[DINPUT_SOFTBUS_KEY_VECTOR_DHID];
    DHLOGE("OnBytesReceived cmdType is TRANS_SOURCE_MSG_STOP_DHID deviceId:%s.",
           GetAnonyString(deviceId).c_str());
    callback_->onStopRemoteInputDhid(sessionId, strTmp);
}

void DistributedInputSinkTransport::NotifyRelayPrepareRemoteInput(int32_t sessionId, const nlohmann::json &recMsg)
{
    if (!recMsg.contains(DINPUT_SOFTBUS_KEY_DEVICE_ID) || !recMsg.contains(DINPUT_SOFTBUS_KEY_SESSION_ID)) {
        DHLOGE("OnBytesReceived cmdType TRANS_SOURCE_MSG_PREPARE_FOR_REL, key not exist.");
        return;
    }
    if (!recMsg[DINPUT_SOFTBUS_KEY_DEVICE_ID].is_string() || !recMsg[DINPUT_SOFTBUS_KEY_SESSION_ID].is_number()) {
        DHLOGE("OnBytesReceived cmdType TRANS_SOURCE_MSG_PREPARE_FOR_REL, data type is error.");
        return;
    }
    std::string deviceId = recMsg[DINPUT_SOFTBUS_KEY_DEVICE_ID];
    int32_t toSrcSessionId = recMsg[DINPUT_SOFTBUS_KEY_SESSION_ID];
    DHLOGI("OnBytesReceived cmdType is TRANS_SOURCE_MSG_PREPARE_FOR_REL deviceId:%s.",
        GetAnonyString(deviceId).c_str());
    callback_->onRelayPrepareRemoteInput(toSrcSessionId, sessionId, deviceId);
}

void DistributedInputSinkTransport::NotifyRelayUnprepareRemoteInput(int32_t sessionId, const nlohmann::json &recMsg)
{
    if (!recMsg.contains(DINPUT_SOFTBUS_KEY_DEVICE_ID) || !recMsg.contains(DINPUT_SOFTBUS_KEY_SESSION_ID)) {
        DHLOGE("OnBytesReceived cmdType TRANS_SOURCE_MSG_UNPREPARE_FOR_REL, key not exist.");
        return;
    }
    if (!recMsg[DINPUT_SOFTBUS_KEY_DEVICE_ID].is_string() || !recMsg[DINPUT_SOFTBUS_KEY_SESSION_ID].is_number()) {
        DHLOGE("OnBytesReceived cmdType TRANS_SOURCE_MSG_UNPREPARE_FOR_REL, data type is error.");
        return;
    }
    std::string deviceId = recMsg[DINPUT_SOFTBUS_KEY_DEVICE_ID];
    int32_t toSrcSessionId = recMsg[DINPUT_SOFTBUS_KEY_SESSION_ID];
    DHLOGI("OnBytesReceived cmdType is TRANS_SOURCE_MSG_UNPREPARE_FOR_REL deviceId:%s.",
        GetAnonyString(deviceId).c_str());
    callback_->onRelayUnprepareRemoteInput(toSrcSessionId, sessionId, deviceId);
}

void DistributedInputSinkTransport::NotifyRelayStartDhidRemoteInput(int32_t sessionId, const nlohmann::json &recMsg)
{
    if (!recMsg.contains(DINPUT_SOFTBUS_KEY_DEVICE_ID) ||
        !recMsg.contains(DINPUT_SOFTBUS_KEY_SESSION_ID) ||
        !recMsg.contains(DINPUT_SOFTBUS_KEY_VECTOR_DHID)) {
        DHLOGE("OnBytesReceived cmdType TRANS_SOURCE_MSG_START_DHID_FOR_REL, key not exist.");
        return;
    }
    if (!recMsg[DINPUT_SOFTBUS_KEY_DEVICE_ID].is_string() ||
        !recMsg[DINPUT_SOFTBUS_KEY_SESSION_ID].is_number() ||
        !recMsg[DINPUT_SOFTBUS_KEY_VECTOR_DHID].is_string()) {
        DHLOGE("OnBytesReceived cmdType TRANS_SOURCE_MSG_START_DHID_FOR_REL, data type is error.");
        return;
    }
    std::string deviceId = recMsg[DINPUT_SOFTBUS_KEY_DEVICE_ID];
    int32_t toSrcSessionId = recMsg[DINPUT_SOFTBUS_KEY_SESSION_ID];
    std::string dhids = recMsg[DINPUT_SOFTBUS_KEY_VECTOR_DHID];
    DHLOGI("OnBytesReceived cmdType is TRANS_SOURCE_MSG_START_DHID_FOR_REL deviceId:%s.",
        GetAnonyString(deviceId).c_str());
    callback_->onRelayStartDhidRemoteInput(toSrcSessionId, sessionId, deviceId, dhids);
}

void DistributedInputSinkTransport::NotifyRelayStopDhidRemoteInput(int32_t sessionId, const nlohmann::json &recMsg)
{
    if (!recMsg.contains(DINPUT_SOFTBUS_KEY_DEVICE_ID) ||
        !recMsg.contains(DINPUT_SOFTBUS_KEY_SESSION_ID) ||
        !recMsg.contains(DINPUT_SOFTBUS_KEY_VECTOR_DHID)) {
        DHLOGE("OnBytesReceived cmdType TRANS_SOURCE_MSG_STOP_DHID_FOR_REL, key not exist.");
        return;
    }
    if (!recMsg[DINPUT_SOFTBUS_KEY_DEVICE_ID].is_string() ||
        !recMsg[DINPUT_SOFTBUS_KEY_SESSION_ID].is_number() ||
        !recMsg[DINPUT_SOFTBUS_KEY_VECTOR_DHID].is_string()) {
        DHLOGE("OnBytesReceived cmdType TRANS_SOURCE_MSG_STOP_DHID_FOR_REL, data type is error.");
        return;
    }
    std::string deviceId = recMsg[DINPUT_SOFTBUS_KEY_DEVICE_ID];
    int32_t toSrcSessionId = recMsg[DINPUT_SOFTBUS_KEY_SESSION_ID];
    std::string dhids = recMsg[DINPUT_SOFTBUS_KEY_VECTOR_DHID];
    DHLOGI("OnBytesReceived cmdType is TRANS_SOURCE_MSG_STOP_DHID_FOR_REL deviceId:%s.",
        GetAnonyString(deviceId).c_str());
    callback_->onRelayStopDhidRemoteInput(toSrcSessionId, sessionId, deviceId, dhids);
}

void DistributedInputSinkTransport::NotifyRelayStartTypeRemoteInput(int32_t sessionId, const nlohmann::json &recMsg)
{
    if (!recMsg.contains(DINPUT_SOFTBUS_KEY_DEVICE_ID) ||
        !recMsg.contains(DINPUT_SOFTBUS_KEY_SESSION_ID) ||
        !recMsg.contains(DINPUT_SOFTBUS_KEY_INPUT_TYPE)) {
        DHLOGE("OnBytesReceived cmdType TRANS_SOURCE_MSG_START_TYPE_FOR_REL, key not exist.");
        return;
    }
    if (!recMsg[DINPUT_SOFTBUS_KEY_DEVICE_ID].is_string() ||
        !recMsg[DINPUT_SOFTBUS_KEY_SESSION_ID].is_number() ||
        !recMsg[DINPUT_SOFTBUS_KEY_INPUT_TYPE].is_number()) {
        DHLOGE("OnBytesReceived cmdType TRANS_SOURCE_MSG_START_TYPE_FOR_REL, data type is error.");
        return;
    }
    std::string deviceId = recMsg[DINPUT_SOFTBUS_KEY_DEVICE_ID];
    int32_t toSrcSessionId = recMsg[DINPUT_SOFTBUS_KEY_SESSION_ID];
    uint32_t inputTypes = recMsg[DINPUT_SOFTBUS_KEY_INPUT_TYPE];
    DHLOGI("OnBytesReceived cmdType is TRANS_SOURCE_MSG_START_TYPE_FOR_REL deviceId:%s.",
        GetAnonyString(deviceId).c_str());
    callback_->onRelayStartTypeRemoteInput(toSrcSessionId, sessionId, deviceId, inputTypes);
}

void DistributedInputSinkTransport::NotifyRelayStopTypeRemoteInput(int32_t sessionId, const nlohmann::json &recMsg)
{
    if (!recMsg.contains(DINPUT_SOFTBUS_KEY_DEVICE_ID) ||
        !recMsg.contains(DINPUT_SOFTBUS_KEY_SESSION_ID) ||
        !recMsg.contains(DINPUT_SOFTBUS_KEY_INPUT_TYPE)) {
        DHLOGE("OnBytesReceived cmdType TRANS_SOURCE_MSG_STOP_TYPE_FOR_REL, key not exist.");
        return;
    }
    if (!recMsg[DINPUT_SOFTBUS_KEY_DEVICE_ID].is_string() ||
        !recMsg[DINPUT_SOFTBUS_KEY_SESSION_ID].is_number() ||
        !recMsg[DINPUT_SOFTBUS_KEY_INPUT_TYPE].is_number()) {
        DHLOGE("OnBytesReceived cmdType TRANS_SOURCE_MSG_STOP_TYPE_FOR_REL, data type is error.");
        return;
    }
    std::string deviceId = recMsg[DINPUT_SOFTBUS_KEY_DEVICE_ID];
    int32_t toSrcSessionId = recMsg[DINPUT_SOFTBUS_KEY_SESSION_ID];
    uint32_t inputTypes = recMsg[DINPUT_SOFTBUS_KEY_INPUT_TYPE];
    DHLOGI("OnBytesReceived cmdType is TRANS_SOURCE_MSG_STOP_TYPE_FOR_REL deviceId:%s.",
        GetAnonyString(deviceId).c_str());
    callback_->onRelayStopTypeRemoteInput(toSrcSessionId, sessionId, deviceId, inputTypes);
}

void DistributedInputSinkTransport::HandleSessionData(int32_t sessionId, const std::string& message)
{
    if (callback_ == nullptr) {
        DHLOGE("OnBytesReceived the callback_ is null, the message:%s abort.", SetAnonyId(message).c_str());
        return;
    }

    nlohmann::json recMsg = nlohmann::json::parse(message);
    if (recMsg.is_discarded()) {
        DHLOGE("OnBytesReceived jsonStr error.");
        return;
    }

    if (recMsg.contains(DINPUT_SOFTBUS_KEY_CMD_TYPE) != true) {
        DHLOGE("OnBytesReceived message:%s is error, not contain cmdType.", SetAnonyId(message).c_str());
        return;
    }

    if (recMsg[DINPUT_SOFTBUS_KEY_CMD_TYPE].is_number() != true) {
        DHLOGE("OnBytesReceived cmdType is not number type.");
        return;
    }

    int cmdType = recMsg[DINPUT_SOFTBUS_KEY_CMD_TYPE];
    switch (cmdType) {
        case TRANS_SOURCE_MSG_PREPARE: {
            NotifyPrepareRemoteInput(sessionId, recMsg);
            break;
        }
        case TRANS_SOURCE_MSG_UNPREPARE: {
            NotifyUnprepareRemoteInput(sessionId, recMsg);
            break;
        }
        case TRANS_SOURCE_MSG_START_TYPE: {
            NotifyStartRemoteInput(sessionId, recMsg);
            break;
        }
        case TRANS_SOURCE_MSG_STOP_TYPE: {
            NotifyStopRemoteInput(sessionId, recMsg);
            break;
        }
        case TRANS_SOURCE_MSG_LATENCY: {
            NotifyLatency(sessionId, recMsg);
            break;
        }
        case TRANS_SOURCE_MSG_START_DHID: {
            NotifyStartRemoteInputDhid(sessionId, recMsg);
            break;
        }
        case TRANS_SOURCE_MSG_STOP_DHID: {
            NotifyStopRemoteInputDhid(sessionId, recMsg);
            break;
        }
        case TRANS_SOURCE_MSG_PREPARE_FOR_REL: {
            NotifyRelayPrepareRemoteInput(sessionId, recMsg);
            break;
        }
        case TRANS_SOURCE_MSG_UNPREPARE_FOR_REL: {
            NotifyRelayUnprepareRemoteInput(sessionId, recMsg);
            break;
        }
        case TRANS_SOURCE_MSG_START_DHID_FOR_REL: {
            NotifyRelayStartDhidRemoteInput(sessionId, recMsg);
            break;
        }
        case TRANS_SOURCE_MSG_STOP_DHID_FOR_REL: {
            NotifyRelayStopDhidRemoteInput(sessionId, recMsg);
            break;
        }
        case TRANS_SOURCE_MSG_START_TYPE_FOR_REL: {
            NotifyRelayStartTypeRemoteInput(sessionId, recMsg);
            break;
        }
        case TRANS_SOURCE_MSG_STOP_TYPE_FOR_REL: {
            NotifyRelayStopTypeRemoteInput(sessionId, recMsg);
            break;
        }
        default:
            DHLOGE("OnBytesReceived cmdType is undefined.");
            break;
    }
}

void DistributedInputSinkTransport::CloseAllSession()
{
    std::vector<int32_t> vecSession = DistributedInputSinkSwitch::GetInstance().GetAllSessionId();
    DHLOGI("CloseAllSession session vector size is %d", vecSession.size());
    for (size_t kIndex = 0; kIndex < vecSession.size(); ++kIndex) {
        CloseSession(vecSession[kIndex]);
        DHLOGI("CloseAllSession [%d] sessionid is %s", kIndex, GetAnonyInt32(vecSession[kIndex]).c_str());
    }

    // clear session data
    DistributedInputSinkSwitch::GetInstance().InitSwitch();
}

void DistributedInputSinkTransport::DInputSinkEventHandler::RecordEventLog(
    const std::shared_ptr<nlohmann::json> &events)
{
    for (nlohmann::json::const_iterator iter = events->cbegin(); iter != events->cend(); ++iter) {
        nlohmann::json event = *iter;
        std::string eventType = "";
        int32_t evType = event[INPUT_KEY_TYPE];
        switch (evType) {
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
        int64_t when = event[INPUT_KEY_WHEN];
        int32_t code = event[INPUT_KEY_CODE];
        int32_t value = event[INPUT_KEY_VALUE];
        std::string path = event[INPUT_KEY_PATH];
        DHLOGD("2.E2E-Test Sink softBus send, EventType: %s, Code: %d, Value: %d, Path: %s, When: %" PRId64 "",
            eventType.c_str(), code, value, path.c_str(), when);
    }
}
} // namespace DistributedInput
} // namespace DistributedHardware
} // namespace OHOS
