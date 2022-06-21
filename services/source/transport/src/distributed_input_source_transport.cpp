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
#include "dinput_softbus_define.h"
#include "distributed_input_inject.h"
#include "session.h"
#include "softbus_bus_center.h"
#include "softbus_common.h"

namespace OHOS {
namespace DistributedHardware {
namespace DistributedInput {
const int32_t DINPUT_LINK_TYPE_MAX = 4;
static SessionAttribute g_sessionAttr = {
    .dataType = SessionType::TYPE_BYTES,
    .linkTypeNum = DINPUT_LINK_TYPE_MAX,
    .linkType = {
        LINK_TYPE_WIFI_WLAN_2G,
        LINK_TYPE_WIFI_WLAN_5G,
        LINK_TYPE_WIFI_P2P,
        LINK_TYPE_BR
    }
};

DistributedInputSourceTransport::~DistributedInputSourceTransport()
{
    Release();
}

static int32_t SessionOpened(int32_t sessionId, int32_t result)
{
    return DistributedInput::DistributedInputSourceTransport::GetInstance().OnSessionOpened(sessionId, result);
}

static void SessionClosed(int32_t sessionId)
{
    DistributedInput::DistributedInputSourceTransport::GetInstance().OnSessionClosed(sessionId);
}

static void BytesReceived(int32_t sessionId, const void *data, uint32_t dataLen)
{
    DistributedInput::DistributedInputSourceTransport::GetInstance().OnBytesReceived(sessionId, data, dataLen);
}

static void MessageReceived(int32_t sessionId, const void *data, uint32_t dataLen)
{
    (void)sessionId;
    (void)data;
    (void)dataLen;
    DHLOGI("sessionId:%s, dataLen:%d", GetAnonyInt32(sessionId).c_str(), dataLen);
}

static void StreamReceived(int32_t sessionId, const StreamData *data, const StreamData *ext,
    const StreamFrameInfo *param)
{
    (void)sessionId;
    (void)data;
    (void)ext;
    (void)param;
    DHLOGI("sessionId:%s", GetAnonyInt32(sessionId).c_str());
}
DistributedInputSourceTransport &DistributedInputSourceTransport::GetInstance()
{
    static DistributedInputSourceTransport instance;
    return instance;
}

int32_t DistributedInputSourceTransport::Init()
{
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
        DHLOGE("Init Could not get local device id.");
        return ERR_DH_INPUT_SERVER_SOURCE_TRANSPORT_INIT_FAIL;
    }
    std::string networkId = localNode->networkId;
    DHLOGI("Init device local networkId is %s", GetAnonyString(networkId).c_str());
    mySessionName_ = SESSION_NAME_SOURCE + networkId.substr(0, INTERCEPT_STRING_LENGTH);

    int32_t ret = CreateSessionServer(DINPUT_PKG_NAME.c_str(), mySessionName_.c_str(), &iSessionListener);
    if (ret != DH_SUCCESS) {
        DHLOGE("Init CreateSessionServer failed, error code %d.", ret);
        return ERR_DH_INPUT_SERVER_SOURCE_TRANSPORT_INIT_FAIL;
    }
    return DH_SUCCESS;
}

void DistributedInputSourceTransport::Release()
{
    std::unique_lock<std::mutex> sessionLock(operationMutex_);
    std::for_each(sessionDevMap_.begin(), sessionDevMap_.end(), [](auto item) { CloseSession(item.second); });
    (void)RemoveSessionServer(DINPUT_PKG_NAME.c_str(), mySessionName_.c_str());
    sessionDevMap_.clear();
    channelStatusMap_.clear();
    DistributedInputInject::GetInstance().StopInjectThread();
}

int32_t DistributedInputSourceTransport::CheckDeviceSessionState(const std::string &devId)
{
    std::unique_lock<std::mutex> sessionLock(operationMutex_);
    if (sessionDevMap_.count(devId) != 0) {
        DHLOGI("CheckDeviceSessionState has opened %s", GetAnonyString(devId).c_str());
        return DH_SUCCESS;
    } else {
        return ERR_DH_INPUT_SERVER_SOURCE_TRANSPORT_DEVICE_SESSION_STATE;
    }
}

int32_t DistributedInputSourceTransport::OpenInputSoftbus(const std::string &remoteDevId)
{
    int32_t ret = CheckDeviceSessionState(remoteDevId);
    if (ret == DH_SUCCESS) {
        DHLOGE("Softbus session has already opened, deviceId: %s", GetAnonyString(remoteDevId).c_str());
        return DH_SUCCESS;
    }

    std::string peerSessionName = SESSION_NAME_SINK + remoteDevId.substr(0, INTERCEPT_STRING_LENGTH);
    DHLOGI("OpenInputSoftbus peerSessionName:%s", peerSessionName.c_str());

    int sessionId = OpenSession(mySessionName_.c_str(), peerSessionName.c_str(), remoteDevId.c_str(),
        GROUP_ID.c_str(), &g_sessionAttr);
    if (sessionId < 0) {
        DHLOGE("OpenSession fail, remoteDevId: %s, sessionId: %s",
            GetAnonyString(remoteDevId).c_str(), GetAnonyInt32(sessionId).c_str());
        return ERR_DH_INPUT_SERVER_SOURCE_TRANSPORT_OPEN_SESSION_FAIL;
    }

    {
        std::unique_lock<std::mutex> sessionLock(operationMutex_);
        sessionDevMap_[remoteDevId] = sessionId;
    }

    DHLOGI("Wait for channel session opened.");
    {
        std::unique_lock<std::mutex> waitLock(operationMutex_);
        auto status = openSessionWaitCond_.wait_for(waitLock, std::chrono::seconds(SESSION_WAIT_TIMEOUT_SECOND),
            [this, remoteDevId] () { return channelStatusMap_[remoteDevId]; });
        if (!status) {
            DHLOGE("OpenSession timeout, remoteDevId: %s, sessionId: %s",
                GetAnonyString(remoteDevId).c_str(), GetAnonyInt32(sessionId).c_str());
            return ERR_DH_INPUT_SERVER_SOURCE_TRANSPORT_OPEN_SESSION_TIMEOUT;
        }
    }

    DHLOGI("OpenSession success, remoteDevId:%s, sessionId:%s",
        GetAnonyString(remoteDevId).c_str(), GetAnonyInt32(sessionId).c_str());
    return DH_SUCCESS;
}

void DistributedInputSourceTransport::CloseInputSoftbus(const std::string &remoteDevId)
{
    std::unique_lock<std::mutex> sessionLock(operationMutex_);
    // check this device's all hd is close,this device session close.

    if (sessionDevMap_.count(remoteDevId) == 0) {
        DHLOGI("SessionDevIdMap Not find remoteDevId: %s", GetAnonyString(remoteDevId).c_str());
        return;
    }
    int32_t sessionId = sessionDevMap_[remoteDevId];

    DHLOGI("RemoteDevId: %s, sessionId: %s", GetAnonyString(remoteDevId).c_str(), GetAnonyInt32(sessionId).c_str());
    CloseSession(sessionId);
    sessionDevMap_.erase(remoteDevId);
    channelStatusMap_.erase(remoteDevId);
    DistributedInputInject::GetInstance().StopInjectThread();
}

void DistributedInputSourceTransport::RegisterSourceRespCallback(std::shared_ptr<DInputSourceTransCallback> callback)
{
    DHLOGI("RegisterSourceRespCallback");
    callback_ = callback;
}

/**
* PrepareRemoteInput.
* @param  deviceId is remote device
* @return Returns 0 is success, other is fail.
*/
int32_t DistributedInputSourceTransport::PrepareRemoteInput(const std::string& deviceId)
{
    std::unique_lock<std::mutex> sessionLock(operationMutex_);
    if (sessionDevMap_.count(deviceId) > 0) {
        int32_t sessionId = sessionDevMap_[deviceId];
        nlohmann::json jsonStr;
        jsonStr[DINPUT_SOFTBUS_KEY_CMD_TYPE] = TRANS_SOURCE_MSG_PREPARE;
        jsonStr[DINPUT_SOFTBUS_KEY_DEVICE_ID] = deviceId;
        jsonStr[DINPUT_SOFTBUS_KEY_SESSION_ID] = sessionId;
        std::string smsg = jsonStr.dump();
        int32_t ret = SendMsg(sessionId, smsg);
        if (ret != DH_SUCCESS) {
            DHLOGE("PrepareRemoteInput deviceId:%s, sessionId:%s, smsg:%s, SendMsg error, ret:%d.",
                GetAnonyString(deviceId).c_str(), GetAnonyInt32(sessionId).c_str(), smsg.c_str(), ret);
            return ERR_DH_INPUT_SERVER_SOURCE_TRANSPORT_PREPARE_FAIL;
        }
        DHLOGI("PrepareRemoteInput devId:%s, sessionId:%s, msg:%s.",
            GetAnonyString(deviceId).c_str(), GetAnonyInt32(sessionId).c_str(), smsg.c_str());
        return DH_SUCCESS;
    } else {
        DHLOGE("PrepareRemoteInput error, not find this device:%s.",
            GetAnonyString(deviceId).c_str());
        return ERR_DH_INPUT_SERVER_SOURCE_TRANSPORT_PREPARE_FAIL;
    }
}

int32_t DistributedInputSourceTransport::UnprepareRemoteInput(const std::string& deviceId)
{
    std::unique_lock<std::mutex> sessionLock(operationMutex_);
    if (sessionDevMap_.count(deviceId) > 0) {
        int32_t sessionId = sessionDevMap_[deviceId];
        nlohmann::json jsonStr;
        jsonStr[DINPUT_SOFTBUS_KEY_CMD_TYPE] = TRANS_SOURCE_MSG_UNPREPARE;
        jsonStr[DINPUT_SOFTBUS_KEY_DEVICE_ID] = deviceId;
        jsonStr[DINPUT_SOFTBUS_KEY_SESSION_ID] = sessionId;
        std::string smsg = jsonStr.dump();
        int32_t ret = SendMsg(sessionId, smsg);
        if (ret != DH_SUCCESS) {
            DHLOGE("UnprepareRemoteInput deviceId:%s, sessionId:%s, smsg:%s, SendMsg error, ret:%d.",
                GetAnonyString(deviceId).c_str(), GetAnonyInt32(sessionId).c_str(), smsg.c_str(), ret);
            return ERR_DH_INPUT_SERVER_SOURCE_TRANSPORT_UNPREPARE_FAIL;
        }
        DHLOGI("UnprepareRemoteInput deviceId:%s, sessionId:%s, smsg:%s.",
            GetAnonyString(deviceId).c_str(), GetAnonyInt32(sessionId).c_str(), smsg.c_str());
        return DH_SUCCESS;
    } else {
        DHLOGE("UnprepareRemoteInput error, not find this device:%s.",
            GetAnonyString(deviceId).c_str());
        return ERR_DH_INPUT_SERVER_SOURCE_TRANSPORT_UNPREPARE_FAIL;
    }
}

int32_t DistributedInputSourceTransport::StartRemoteInput(
    const std::string& deviceId, const uint32_t& inputTypes)
{
    std::unique_lock<std::mutex> sessionLock(operationMutex_);
    if (sessionDevMap_.count(deviceId) > 0) {
        int32_t sessionId = sessionDevMap_[deviceId];
        nlohmann::json jsonStr;
        jsonStr[DINPUT_SOFTBUS_KEY_CMD_TYPE] = TRANS_SOURCE_MSG_START;
        jsonStr[DINPUT_SOFTBUS_KEY_DEVICE_ID] = deviceId;
        jsonStr[DINPUT_SOFTBUS_KEY_SESSION_ID] = sessionId;
        jsonStr[DINPUT_SOFTBUS_KEY_INPUT_TYPE] = inputTypes;
        std::string smsg = jsonStr.dump();
        int32_t ret = SendMsg(sessionId, smsg);
        if (ret != DH_SUCCESS) {
            DHLOGE("StartRemoteInput deviceId:%s, sessionId:%s, smsg:%s, SendMsg error, ret:%d.",
                GetAnonyString(deviceId).c_str(), GetAnonyInt32(sessionId).c_str(), smsg.c_str(), ret);
            return ERR_DH_INPUT_SERVER_SOURCE_TRANSPORT_START_FAIL;
        }
        DHLOGI("StartRemoteInput deviceId:%s, sessionId:%s, smsg:%s.",
            GetAnonyString(deviceId).c_str(), GetAnonyInt32(sessionId).c_str(), smsg.c_str());
        return DH_SUCCESS;
    } else {
        DHLOGE("StartRemoteInput error, not find this device:%s.",
            GetAnonyString(deviceId).c_str());
        return ERR_DH_INPUT_SERVER_SOURCE_TRANSPORT_START_FAIL;
    }
}

int32_t DistributedInputSourceTransport::StopRemoteInput(
    const std::string& deviceId, const uint32_t& inputTypes)
{
    std::unique_lock<std::mutex> sessionLock(operationMutex_);
    if (sessionDevMap_.count(deviceId) > 0) {
        int32_t sessionId = sessionDevMap_[deviceId];
        nlohmann::json jsonStr;
        jsonStr[DINPUT_SOFTBUS_KEY_CMD_TYPE] = TRANS_SOURCE_MSG_STOP;
        jsonStr[DINPUT_SOFTBUS_KEY_DEVICE_ID] = deviceId;
        jsonStr[DINPUT_SOFTBUS_KEY_SESSION_ID] = sessionId;
        jsonStr[DINPUT_SOFTBUS_KEY_INPUT_TYPE] = inputTypes;
        std::string smsg = jsonStr.dump();
        int32_t ret = SendMsg(sessionId, smsg);
        if (ret != DH_SUCCESS) {
            DHLOGE("StopRemoteInput deviceId:%s, sessionId:%s, smsg:%s, SendMsg error, ret:%d.",
                GetAnonyString(deviceId).c_str(), GetAnonyInt32(sessionId).c_str(), smsg.c_str(), ret);
            return ERR_DH_INPUT_SERVER_SOURCE_TRANSPORT_STOP_FAIL;
        }
        DHLOGI("StopRemoteInput deviceId:%s, sessionId:%s, smsg:%s.",
            GetAnonyString(deviceId).c_str(), GetAnonyInt32(sessionId).c_str(), smsg.c_str());
        return DH_SUCCESS;
    } else {
        DHLOGE("StopRemoteInput error, not find this device:%s.", GetAnonyString(deviceId).c_str());
        return ERR_DH_INPUT_SERVER_SOURCE_TRANSPORT_STOP_FAIL;
    }
}

std::string DistributedInputSourceTransport::FindDeviceBySession(int32_t sessionId)
{
    std::unique_lock<std::mutex> sessionLock(operationMutex_);
    auto find_item = std::find_if(sessionDevMap_.begin(), sessionDevMap_.end(),
        [sessionId](const std::map<std::string, int>::value_type item) {
        return item.second == sessionId;
    });

    std::string devId = "";
    if (find_item != sessionDevMap_.end()) {
        devId = (*find_item).first;
    } else {
        DHLOGE("findKeyByValue error.");
    }
    return devId;
}

int32_t DistributedInputSourceTransport::OnSessionOpened(int32_t sessionId, int32_t result)
{
    if (result != DH_SUCCESS) {
        std::string deviceId = FindDeviceBySession(sessionId);
        DHLOGE("session open failed, sessionId:%s, result:%d, "
            "deviceId:%s", GetAnonyInt32(sessionId).c_str(), result, GetAnonyString(deviceId).c_str());
        std::unique_lock<std::mutex> sessionLock(operationMutex_);
        if (sessionDevMap_.count(deviceId) > 0) {
            sessionDevMap_.erase(deviceId);
        }
        return DH_SUCCESS;
    }

    std::string deviceId = FindDeviceBySession(sessionId);
    int32_t sessionSide = GetSessionSide(sessionId);
    DHLOGI("session open succeed, sessionId:%s, sessionSide:%d(1 is "
        "client side), deviceId:%s", GetAnonyInt32(sessionId).c_str(), sessionSide, GetAnonyString(deviceId).c_str());

    char mySessionName[SESSION_NAME_SIZE_MAX] = "";
    char peerSessionName[SESSION_NAME_SIZE_MAX] = "";
    char peerDevId[DEVICE_ID_SIZE_MAX] = "";
    int ret = GetMySessionName(sessionId, mySessionName, sizeof(mySessionName));
    if (ret != DH_SUCCESS) {
        DHLOGI("get my session name failed, session id is %s", GetAnonyInt32(sessionId).c_str());
    }
    ret = GetPeerSessionName(sessionId, peerSessionName, sizeof(peerSessionName));
    if (ret != DH_SUCCESS) {
        DHLOGI("get peer session name failed, session id is %s", GetAnonyInt32(sessionId).c_str());
    }
    ret = GetPeerDeviceId(sessionId, peerDevId, sizeof(peerDevId));
    if (ret != DH_SUCCESS) {
        DHLOGI("get peer device id failed, session id is %s", GetAnonyInt32(sessionId).c_str());
    }
    DHLOGI("mySessionName:%s, peerSessionName:%s, peerDevId:%s",
        mySessionName, peerSessionName, GetAnonyString(peerDevId).c_str());
    {
        std::lock_guard<std::mutex> notifyLock(operationMutex_);
        channelStatusMap_[peerDevId] = true;
        openSessionWaitCond_.notify_all();
    }
    DistributedInputInject::GetInstance().StartInjectThread();
    return DH_SUCCESS;
}

void DistributedInputSourceTransport::OnSessionClosed(int32_t sessionId)
{
    std::string deviceId = FindDeviceBySession(sessionId);
    DHLOGI("OnSessionClosed, sessionId:%s, deviceId:%s",
        GetAnonyInt32(sessionId).c_str(), GetAnonyString(deviceId).c_str());
    std::unique_lock<std::mutex> sessionLock(operationMutex_);
    if (sessionDevMap_.count(deviceId) > 0) {
        sessionDevMap_.erase(deviceId);
    }
    channelStatusMap_.erase(deviceId);
    DistributedInputInject::GetInstance().StopInjectThread();
}

void DistributedInputSourceTransport::NotifyResponsePrepareRemoteInput(int32_t sessionId,
    const nlohmann::json &recMsg)
{
    DHLOGI("OnBytesReceived cmdType is TRANS_SINK_MSG_ONPREPARE.");
    if (!recMsg[DINPUT_SOFTBUS_KEY_RESP_VALUE].is_boolean() ||
        !recMsg[DINPUT_SOFTBUS_KEY_WHITE_LIST].is_string()) {
        DHLOGE("OnBytesReceived cmdType is TRANS_SINK_MSG_ONPREPARE, data type is error.");
        return;
    }
    std::string deviceId = FindDeviceBySession(sessionId);
    if (deviceId.empty()) {
        DHLOGE("OnBytesReceived cmdType is TRANS_SINK_MSG_ONPREPARE, deviceId is error.");
        return;
    }
    callback_->onResponsePrepareRemoteInput(deviceId, recMsg[DINPUT_SOFTBUS_KEY_RESP_VALUE],
        recMsg[DINPUT_SOFTBUS_KEY_WHITE_LIST]);
}

void DistributedInputSourceTransport::NotifyResponseUnprepareRemoteInput(int32_t sessionId,
    const nlohmann::json &recMsg)
{
    DHLOGI("OnBytesReceived cmdType is TRANS_SINK_MSG_ONUNPREPARE.");
    if (!recMsg[DINPUT_SOFTBUS_KEY_RESP_VALUE].is_boolean()) {
        DHLOGE("OnBytesReceived cmdType is TRANS_SINK_MSG_ONUNPREPARE data type is error.");
        return;
    }
    std::string deviceId = FindDeviceBySession(sessionId);
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
    if (!recMsg[DINPUT_SOFTBUS_KEY_RESP_VALUE].is_boolean()) {
        DHLOGE("OnBytesReceived cmdType is TRANS_SINK_MSG_ONSTART, data type is error.");
        return;
    }
    std::string deviceId = FindDeviceBySession(sessionId);
    if (deviceId.empty()) {
        DHLOGE("OnBytesReceived cmdType is TRANS_SINK_MSG_ONSTART, deviceId is error.");
        return;
    }
    callback_->onResponseStartRemoteInput(
        deviceId, recMsg[DINPUT_SOFTBUS_KEY_INPUT_TYPE], recMsg[DINPUT_SOFTBUS_KEY_RESP_VALUE]);
}

void DistributedInputSourceTransport::NotifyResponseStopRemoteInput(int32_t sessionId, const nlohmann::json &recMsg)
{
    DHLOGI("OnBytesReceived cmdType is TRANS_SINK_MSG_ONSTOP.");
    if (!recMsg[DINPUT_SOFTBUS_KEY_RESP_VALUE].is_boolean()) {
        DHLOGE("OnBytesReceived cmdType TRANS_SINK_MSG_ONSTOP data type is error.");
        return;
    }
    std::string deviceId = FindDeviceBySession(sessionId);
    if (deviceId.empty()) {
        DHLOGE("OnBytesReceived cmdType TRANS_SINK_MSG_ONSTOP, deviceId is error.");
        return;
    }
    callback_->onResponseStopRemoteInput(
        deviceId, recMsg[DINPUT_SOFTBUS_KEY_INPUT_TYPE], recMsg[DINPUT_SOFTBUS_KEY_RESP_VALUE]);
}

void DistributedInputSourceTransport::NotifyReceivedEventRemoteInput(int32_t sessionId, const nlohmann::json &recMsg)
{
    DHLOGI("OnBytesReceived cmdType is TRANS_SINK_MSG_BODY_DATA.");
    if (!recMsg[DINPUT_SOFTBUS_KEY_INPUT_DATA].is_string()) {
        DHLOGE("OnBytesReceived cmdType is TRANS_SINK_MSG_BODY_DATA, data type is error.");
        return;
    }

    std::string deviceId = FindDeviceBySession(sessionId);
    if (deviceId.empty()) {
        DHLOGE("OnBytesReceived cmdType is TRANS_SINK_MSG_BODY_DATA, deviceId is error.");
        return;
    }
    std::string inputDataStr = recMsg[DINPUT_SOFTBUS_KEY_INPUT_DATA];
    callback_->onReceivedEventRemoteInput(deviceId, inputDataStr);
}

void DistributedInputSourceTransport::HandleSessionData(int32_t sessionId, const std::string& message)
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
        DHLOGE("OnBytesReceived message:%s is error, not contain cmdType.",
            message.c_str());
        return;
    }

    if (recMsg[DINPUT_SOFTBUS_KEY_CMD_TYPE].is_number() != true) {
        DHLOGE("OnBytesReceived cmdType is not number type.");
        return;
    }

    int cmdType = recMsg[DINPUT_SOFTBUS_KEY_CMD_TYPE];
    switch (cmdType) {
        case TRANS_SINK_MSG_ONPREPARE: {
            NotifyResponsePrepareRemoteInput(sessionId, recMsg);
            break;
        }
        case TRANS_SINK_MSG_ONUNPREPARE: {
            NotifyResponseUnprepareRemoteInput(sessionId, recMsg);
            break;
        }
        case TRANS_SINK_MSG_ONSTART: {
            NotifyResponseStartRemoteInput(sessionId, recMsg);
            break;
        }
        case TRANS_SINK_MSG_ONSTOP: {
            NotifyResponseStopRemoteInput(sessionId, recMsg);
            break;
        }
        case TRANS_SINK_MSG_BODY_DATA: {
            NotifyReceivedEventRemoteInput(sessionId, recMsg);
            break;
        }
        default: {
            DHLOGE("OnBytesReceived cmdType is undefined.");
            break;
        }
    }
}

void DistributedInputSourceTransport::OnBytesReceived(int32_t sessionId, const void *data, uint32_t dataLen)
{
    DHLOGI("OnBytesReceived, sessionId:%s, dataLen:%d", GetAnonyInt32(sessionId).c_str(), dataLen);
    if (sessionId < 0 || data == nullptr || dataLen <= 0) {
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
    DHLOGI("OnBytesReceived message:%s.", message.c_str());
    HandleSessionData(sessionId, message);

    free(buf);
    DHLOGI("OnBytesReceived completed");
    return;
}

// send message by sessionId (channel opened)
int32_t DistributedInputSourceTransport::SendMsg(int32_t sessionId, std::string &message)
{
    DHLOGI("start SendMsg");
    if (message.size() > MSG_MAX_SIZE) {
        DHLOGE("SendMessage error: message.size() > MSG_MAX_SIZE");
        return ERR_DH_INPUT_SERVER_SOURCE_TRANSPORT_SENDMESSSAGE;
    }
    uint8_t *buf = (uint8_t *)calloc((MSG_MAX_SIZE), sizeof(uint8_t));
    if (buf == nullptr) {
        DHLOGE("SendMsg: malloc memory failed");
        return ERR_DH_INPUT_SERVER_SOURCE_TRANSPORT_SENDMESSSAGE;
    }
    int32_t outLen = 0;
    if (memcpy_s(buf, MSG_MAX_SIZE, (const uint8_t *)message.c_str(), message.size()) != DH_SUCCESS) {
        DHLOGE("SendMsg: memcpy memory failed");
        free(buf);
        return ERR_DH_INPUT_SERVER_SOURCE_TRANSPORT_SENDMESSSAGE;
    }
    outLen = (int32_t)message.size();
    int32_t ret = SendBytes(sessionId, buf, outLen);
    free(buf);
    return ret;
}
} // namespace DistributedInput
} // namespace DistributedHardware
} // namespace OHOS
