/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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
#include "session.h"
#include "constants_dinput.h"
#include "securec.h"
#include "softbus_bus_center.h"
#include "distributed_hardware_log.h"

namespace OHOS {
namespace DistributedHardware {
namespace DistributedInput {
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
    DHLOGI("sessionId:%d, dataLen:%d", sessionId, dataLen);
}

static void StreamReceived(int32_t sessionId, const StreamData *data, const StreamData *ext,
    const StreamFrameInfo *param)
{
    (void)sessionId;
    (void)data;
    (void)ext;
    (void)param;
    DHLOGI("sessionId:%d", sessionId);
}
DistributedInputSinkTransport &DistributedInputSinkTransport::GetInstance()
{
    static DistributedInputSinkTransport instance;
    return instance;
}

DistributedInputSinkTransport::DistributedInputSinkTransport()
{
    std::shared_ptr<AppExecFwk::EventRunner> runner = AppExecFwk::EventRunner::Create(true);
    eventHandler_ = std::make_shared<DistributedInputSinkTransport::DInputSinkEventHandler>(runner);
    mySessionName_ = "";
    DHLOGI("DistributedInputSinkTransport eventHandler_");
}

DistributedInputSinkTransport::~DistributedInputSinkTransport()
{
    DHLOGI("~DistributedInputSinkTransport");
    sessionDevMap_.clear();
    (void)RemoveSessionServer(DH_FWK_PKG_NAME.c_str(), mySessionName_.c_str());
}

DistributedInputSinkTransport::DInputSinkEventHandler::DInputSinkEventHandler(
    const std::shared_ptr<AppExecFwk::EventRunner> &runner) : AppExecFwk::EventHandler(runner)
{
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
    if (localNode == nullptr) {
        DHLOGE("Init unique_ptr localNode nullptr.");
        return FAILURE;
    }
    int32_t retCode = GetLocalNodeDeviceInfo(DH_FWK_PKG_NAME.c_str(), localNode.get());
    if (retCode != SUCCESS) {
        DHLOGE("Init could not get local device id.");
        return FAILURE;
    }
    std::string networkId = localNode->networkId;
    DHLOGI("Init device networkId is %s", networkId.c_str());
    mySessionName_ = SESSION_NAME_SINK + networkId.substr(0, INTERCEPT_STRING_LENGTH);

    int32_t ret = CreateSessionServer(DH_FWK_PKG_NAME.c_str(), mySessionName_.c_str(), &iSessionListener);
    if (ret != SUCCESS) {
        DHLOGE("Init CreateSessionServer failed, error code %d.", ret);
        return FAILURE;
    }
    return SUCCESS;
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
        DHLOGI("RespPrepareRemoteInput session:%d, smsg:%s.", sessionId, smsg.c_str());
        int32_t ret = SendMessage(sessionId, smsg);
        if (ret != SUCCESS) {
            DHLOGE("RespPrepareRemoteInput error, SendMessage fail.");
            return FAILURE;
        }
        return SUCCESS;
    } else {
        DHLOGE("RespPrepareRemoteInput error, sessionId <= 0.");
        return FAILURE;
    }
}

int32_t DistributedInputSinkTransport::RespUnprepareRemoteInput(
    const int32_t sessionId, std::string &smsg)
{
    if (sessionId > 0) {
        DHLOGI("RespUnprepareRemoteInput sessionId:%d, smsg:%s.", sessionId, smsg.c_str());
        int32_t ret = SendMessage(sessionId, smsg);
        if (ret != SUCCESS) {
            DHLOGE("RespUnprepareRemoteInput error, SendMessage fail.");
            return FAILURE;
        }
        return SUCCESS;
    } else {
        DHLOGE("RespUnprepareRemoteInput error, sessionId <= 0.");
        return FAILURE;
    }
}

int32_t DistributedInputSinkTransport::RespStartRemoteInput(
    const int32_t sessionId, std::string &smsg)
{
    if (sessionId > 0) {
        DHLOGI("RespStartRemoteInput sessionId:%d, result:%d, smsg:%s.", sessionId, smsg.c_str());
        int32_t ret = SendMessage(sessionId, smsg);
        if (ret != SUCCESS) {
            DHLOGE("RespStartRemoteInput error, SendMessage fail.");
            return FAILURE;
        }
        return SUCCESS;
    } else {
        DHLOGE("RespStartRemoteInput error, sessionId <= 0.");
        return FAILURE;
    }
}

int32_t DistributedInputSinkTransport::RespStopRemoteInput(
    const int32_t sessionId, std::string &smsg)
{
    if (sessionId > 0) {
        DHLOGI("RespStopRemoteInput sessionId:%d, smsg:%s.", sessionId, smsg.c_str());
        int32_t ret = SendMessage(sessionId, smsg);
        if (ret != SUCCESS) {
            DHLOGE("RespStopRemoteInput error, SendMessage fail.");
            return FAILURE;
        }
        return SUCCESS;
    } else {
        DHLOGE("RespStopRemoteInput error, sessionId <= 0.");
        return FAILURE;
    }
}

int32_t DistributedInputSinkTransport::SendMessage(int32_t sessionId, std::string &message)
{
    DHLOGI("start SendMessage");
    if (message.size() > MSG_MAX_SIZE) {
        DHLOGE("SendMessage error: message.size() > MSG_MAX_SIZE");
        return FAILURE;
    }
    uint8_t *buf = (uint8_t *)calloc((MSG_MAX_SIZE), sizeof(uint8_t));
    if (buf == nullptr) {
        DHLOGE("SendMessage: malloc memory failed");
        return FAILURE;
    }
    int32_t outLen = 0;
    if (memcpy_s(buf, MSG_MAX_SIZE, (const uint8_t *)message.c_str(), message.size()) != SUCCESS) {
        DHLOGE("SendMessage: memcpy memory failed");
        free(buf);
        return FAILURE;
    }
    outLen = message.size();
    int32_t ret = SendBytes(sessionId, buf, outLen);
    free(buf);
    return ret;
}

int32_t DistributedInputSinkTransport::OnSessionOpened(int32_t sessionId, int32_t result)
{
    if (result != SUCCESS) {
        DHLOGE("session open failed, sessionId %d", sessionId);
        if (sessionIdSet_.count(sessionId) > 0) {
            sessionIdSet_.erase(sessionId);
        }
        return SUCCESS;
    }

    // return 1 is client
    int32_t sessionSide = GetSessionSide(sessionId);
    DHLOGI("session open succeed, sessionId %d, sessionSide %d", sessionId, sessionSide);

    char mySessionName[SESSION_NAME_SIZE_MAX] = "";
    char peerSessionName[SESSION_NAME_SIZE_MAX] = "";
    char peerDevId[DEVICE_ID_SIZE_MAX] = "";
    int ret = GetMySessionName(sessionId, mySessionName, sizeof(mySessionName));
    if (ret != SUCCESS) {
        DHLOGI("get my session name failed, session id is %d.", sessionId);
    }
    // get other device session name
    ret = GetPeerSessionName(sessionId, peerSessionName, sizeof(peerSessionName));
    if (ret != SUCCESS) {
        DHLOGI("get my peer session name failed, session id is %d.", sessionId);
    }

    ret = GetPeerDeviceId(sessionId, peerDevId, sizeof(peerDevId));
    if (ret != SUCCESS) {
        DHLOGI("get my peer device id failed, session id is %d.", sessionId);
    }
    DHLOGI("mySessionName:%s, peerSessionName:%s, peerDevId:%s.",
        mySessionName, peerSessionName, peerDevId);

    return SUCCESS;
}

void DistributedInputSinkTransport::OnSessionClosed(int32_t sessionId)
{
    DHLOGI("OnSessionClosed, sessionId:%d", sessionId);
    if (sessionIdSet_.count(sessionId) > 0) {
        sessionIdSet_.erase(sessionId);
    }

    DistributedInputSinkSwitch::GetInstance().RemoveSession(sessionId);
}

void DistributedInputSinkTransport::OnBytesReceived(int32_t sessionId, const void *data, uint32_t dataLen)
{
    DHLOGI("OnBytesReceived, sessionId:%d, dataLen:%d", sessionId, dataLen);
    if (sessionId < 0 || data == nullptr || dataLen <= 0) {
        DHLOGE("OnBytesReceived param check failed");
        return;
    }

    uint8_t *buf = (uint8_t *)calloc(dataLen + 1, sizeof(uint8_t));
    if (buf == nullptr) {
        DHLOGE("OnBytesReceived: malloc memory failed");
        return;
    }

    if (memcpy_s(buf, dataLen + 1, (const uint8_t*)data, dataLen) != SUCCESS) {
        DHLOGE("OnBytesReceived: memcpy memory failed");
        free(buf);
        return;
    }

    std::string message = (char *)buf;
    DHLOGI("OnBytesReceived message:%s.", message.c_str());
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
        deviceId.c_str());
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
        deviceId.c_str());
    callback_->onUnprepareRemoteInput(sessionId);
}

void DistributedInputSinkTransport::NotifyStartRemoteInput(int32_t sessionId, const nlohmann::json &recMsg)
{
    if (!recMsg[DINPUT_SOFTBUS_KEY_DEVICE_ID].is_string()) {
        DHLOGE("OnBytesReceived cmdType TRANS_SOURCE_MSG_START, data type error.");
        return;
    }
    std::string deviceId = recMsg[DINPUT_SOFTBUS_KEY_DEVICE_ID];
    uint32_t inputTypes = recMsg[DINPUT_SOFTBUS_KEY_INPUT_TYPE];
    DHLOGI("OnBytesRecei,ved cmdType is TRANS_SOURCE_MSG_START deviceId:%s inputTypes:%d .",
        deviceId.c_str(), inputTypes);
    callback_->onStartRemoteInput(sessionId, inputTypes);
}

void DistributedInputSinkTransport::NotifyStopRemoteInput(int32_t sessionId, const nlohmann::json &recMsg)
{
    if (!recMsg[DINPUT_SOFTBUS_KEY_DEVICE_ID].is_string()) {
        DHLOGE("OnBytesReceived cmdType TRANS_SOURCE_MSG_STOP, data type is error.");
        return;
    }
    std::string deviceId = recMsg[DINPUT_SOFTBUS_KEY_DEVICE_ID];
    uint32_t inputTypes = recMsg[DINPUT_SOFTBUS_KEY_INPUT_TYPE];
    DHLOGI("OnBytesReceived cmdType is TRANS_SOURCE_MSG_STOP deviceId:%s.", deviceId.c_str());
    callback_->onStopRemoteInput(sessionId, inputTypes);
}

void DistributedInputSinkTransport::HandleSessionData(int32_t sessionId, const std::string& message)
{
    if (callback_ == nullptr) {
        DHLOGE("OnBytesReceived the callback_ is null, the message:%s abort.", message.c_str());
        return;
    }

    nlohmann::json recMsg = nlohmann::json::parse(message);
    if (recMsg.is_discarded()) {
        DHLOGE("OnBytesReceived jsonStr error.");
        return;
    }

    if (recMsg.contains(DINPUT_SOFTBUS_KEY_CMD_TYPE) != true) {
        DHLOGE("OnBytesReceived message:%s is error, not contain cmdType.", message.c_str());
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
        case TRANS_SOURCE_MSG_START: {
            NotifyStartRemoteInput(sessionId, recMsg);
            break;
        }
        case TRANS_SOURCE_MSG_STOP: {
            NotifyStopRemoteInput(sessionId, recMsg);
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
    DHLOGI("CloseAllSession session vector size is %d.", vecSession.size());
    for (int32_t kIndex = 0; kIndex < vecSession.size(); ++kIndex) {
        CloseSession(vecSession[kIndex]);
        DHLOGI("CloseAllSession [%d] sessionid is %d.", kIndex, vecSession[kIndex]);
    }

    // clear session data
    DistributedInputSinkSwitch::GetInstance().InitSwitch();
}
} // namespace DistributedInput
} // namespace DistributedHardware
} // namespace OHOS
