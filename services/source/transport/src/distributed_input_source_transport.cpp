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

#include <cstring>
#include "session.h"
#include "constants_dinput.h"
#include "dinput_softbus_define.h"
#include "distributed_input_source_transport.h"

#include "iservice_registry.h"
#include "ipc_skeleton.h"
#include "system_ability_definition.h"
#include "softbus_bus_center.h"
#include "softbus_common.h"
#include "distributed_hardware_log.h"

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
    std::unique_lock<std::mutex> sessionLock(operationMutex_);
    std::map<std::string, int32_t>::iterator iter = sessionDevMap_.begin();
    while (iter != sessionDevMap_.end()) {
        CloseSession(iter->second);
    }
    sessionDevMap_.clear();
    devHardwareMap_.clear();

    (void)RemoveSessionServer(DINPUT_PKG_NAME.c_str(), mySessionName_.c_str());
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
    if (retCode != SUCCESS) {
        DHLOGE("Init Could not get local device id.");
        return FAILURE;
    }
    std::string networkId = localNode->networkId;
    DHLOGI("Init device local networkId is %s", networkId.c_str());
    mySessionName_ = SESSION_NAME_SOURCE + networkId.substr(0, INTERCEPT_STRING_LENGTH);

    int32_t ret = CreateSessionServer(DINPUT_PKG_NAME.c_str(), mySessionName_.c_str(), &iSessionListener);
    if (ret != SUCCESS) {
        DHLOGE("Init CreateSessionServer failed, error code %d.", ret);
        return FAILURE;
    }
    return SUCCESS;
}

int32_t DistributedInputSourceTransport::CheckDeviceSessionState(const std::string &devId, const std::string &hwId)
{
    std::unique_lock<std::mutex> sessionLock(operationMutex_);
    if (sessionDevMap_.count(devId) != 0) {
        DHLOGI("CheckDeviceSessionState has opened %s", devId.c_str());

        if (devHardwareMap_.count(devId) != 0) {
            std::set<std::string> devHwArray = devHardwareMap_[devId];
            devHwArray.insert(hwId);
            devHardwareMap_[devId] = devHwArray;
        } else {
            std::set<std::string> devHwArray;
            devHwArray.insert(hwId);
            devHardwareMap_[devId] = devHwArray;
        }
        lastDevId_ = devId;
        lastHwId_ = hwId;
        return SUCCESS;
    } else {
        return FAILURE;
    }
}

int32_t DistributedInputSourceTransport::OpenInputSoftbus(const std::string &remoteDevId, const std::string &hwId)
{
    int32_t ret = CheckDeviceSessionState(remoteDevId, hwId);
    if (ret == SUCCESS) {
        if (callback_ != nullptr) {
            callback_->onResponseRegisterDistributedHardware(lastDevId_, lastHwId_, true);
        } else {
            DHLOGE("OpenInputSoftbus callback_ is null.");
        }
        return SUCCESS;
    }

    std::string peerSessionName = SESSION_NAME_SINK + remoteDevId.substr(0, INTERCEPT_STRING_LENGTH);
    DHLOGI("OpenInputSoftbus peerSessionName:%s", peerSessionName.c_str());

    int sessionId = OpenSession(mySessionName_.c_str(), peerSessionName.c_str(), remoteDevId.c_str(),
        GROUP_ID.c_str(), &g_sessionAttr);
    if (sessionId < 0) {
        DHLOGE("OpenInputSoftbus OpenSession fail, remoteDevId:%s, sessionId:%d", remoteDevId.c_str(), sessionId);
        if (callback_ != nullptr) {
            callback_->onResponseRegisterDistributedHardware(remoteDevId, hwId, false);
        } else {
            DHLOGE("OpenInputSoftbus callback_ is null.");
        }
        return FAILURE;
    }
    DHLOGI("OpenInputSoftbus OpenSession success, remoteDevId:%s, sessionId:%d", remoteDevId.c_str(), sessionId);
    {
        std::unique_lock<std::mutex> sessionLock(operationMutex_);
        sessionDevMap_[remoteDevId] = sessionId;
        if (devHardwareMap_.count(remoteDevId) != 0) {
            std::set<std::string> devHwArray = devHardwareMap_[remoteDevId];
            devHwArray.insert(hwId);
            devHardwareMap_[remoteDevId] = devHwArray;
        } else {
            std::set<std::string> devHwArray;
            devHwArray.insert(hwId);
            devHardwareMap_[remoteDevId] = devHwArray;
        }
        lastDevId_ = remoteDevId;
        lastHwId_ = hwId;
    }
    return SUCCESS;
}

void DistributedInputSourceTransport::CloseInputSoftbus(const std::string &remoteDevId, const std::string &hwId)
{
    std::unique_lock<std::mutex> sessionLock(operationMutex_);
    // check this device's all hd is close,this device session close.
    if (devHardwareMap_.count(remoteDevId) == 0) {
        DHLOGI("CloseInputSoftbus device:%s is not found in devHardware.", remoteDevId.c_str());
        return;
    }

    std::set<std::string> devHwArray = devHardwareMap_[remoteDevId];
    devHwArray.erase(hwId);
    if (lastHwId_ == hwId) {
        lastHwId_ = "";
        DHLOGI("CloseInputSoftbus lastHwId_ is same hwId [%s]", hwId.c_str());
    }
    if (devHwArray.size() == 0) {
        devHardwareMap_.erase(remoteDevId);
        if (sessionDevMap_.count(remoteDevId) != 0) {
            int32_t sessionId = sessionDevMap_[remoteDevId];
            sessionDevMap_.erase(remoteDevId);
            DHLOGI("CloseInputSoftbus remoteDevId:%s, sessionId:%d",
                remoteDevId.c_str(), sessionId);
            if (lastDevId_ == remoteDevId) {
                lastDevId_ = "";
                DHLOGI("CloseInputSoftbus lastDevId_ is same remoteDevId [%s]",
                    remoteDevId.c_str());
            }
            CloseSession(sessionId);
        } else {
            DHLOGI("CloseInputSoftbus not find remoteDevId:%s",
                remoteDevId.c_str());
            return;
        }
    }
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
        if (ret != SUCCESS) {
            DHLOGE("PrepareRemoteInput deviceId:%s, sessionId:%d, smsg:%s, SendMsg error, ret:%d.",
                deviceId.c_str(), sessionId, smsg.c_str(), ret);
            return FAILURE;
        }
        DHLOGI("PrepareRemoteInput devId:%s, sessionId:%d, msg:%s.",
            deviceId.c_str(), sessionId, smsg.c_str());
        return SUCCESS;
    } else {
        DHLOGE("PrepareRemoteInput error, not find this device:%s.",
            deviceId.c_str());
        return FAILURE;
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
        if (ret != SUCCESS) {
            DHLOGE("UnprepareRemoteInput deviceId:%s, sessionId:%d, smsg:%s, SendMsg error, ret:%d.",
                deviceId.c_str(), sessionId, smsg.c_str(), ret);
            return FAILURE;
        }
        DHLOGI("UnprepareRemoteInput deviceId:%s, sessionId:%d, smsg:%s.",
            deviceId.c_str(), sessionId, smsg.c_str());
        return SUCCESS;
    } else {
        DHLOGE("UnprepareRemoteInput error, not find this device:%s.",
            deviceId.c_str());
        return FAILURE;
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
        if (ret != SUCCESS) {
            DHLOGE("StartRemoteInput deviceId:%s, sessionId:%d, smsg:%s, SendMsg error, ret:%d.",
                deviceId.c_str(), sessionId, smsg.c_str(), ret);
            return FAILURE;
        }
        DHLOGI("StartRemoteInput deviceId:%s, sessionId:%d, smsg:%s.", deviceId.c_str(), sessionId, smsg.c_str());
        return SUCCESS;
    } else {
        DHLOGE("StartRemoteInput error, not find this device:%s.",
            deviceId.c_str());
        return FAILURE;
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
        if (ret != SUCCESS) {
            DHLOGE("StopRemoteInput deviceId:%s, sessionId:%d, smsg:%s, SendMsg error, ret:%d.",
                deviceId.c_str(), sessionId, smsg.c_str(), ret);
            return FAILURE;
        }
        DHLOGI("StopRemoteInput deviceId:%s, sessionId:%d, smsg:%s.", deviceId.c_str(), sessionId, smsg.c_str());
        return SUCCESS;
    } else {
        DHLOGE("StopRemoteInput error, not find this device:%s.", deviceId.c_str());
        return FAILURE;
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
    if (result != SUCCESS) {
        std::string deviceId = FindDeviceBySession(sessionId);
        DHLOGE("session open failed, sessionId:%d, result:%d, "
            "deviceId:%s", sessionId, result, deviceId.c_str());
        std::unique_lock<std::mutex> sessionLock(operationMutex_);
        if (sessionDevMap_.count(deviceId) > 0) {
            sessionDevMap_.erase(deviceId);
        }
        if (devHardwareMap_.count(deviceId)) {
            std::set<std::string> devHwArray = devHardwareMap_[deviceId];
            for (auto elem : devHwArray) {
                if (callback_ != nullptr) {
                    callback_->onResponseRegisterDistributedHardware(deviceId, elem, false);
                }
            }

            devHardwareMap_.erase(deviceId);
        }
        return SUCCESS;
    }

    std::string deviceId = FindDeviceBySession(sessionId);
    int32_t sessionSide = GetSessionSide(sessionId);
    DHLOGI("session open succeed, sessionId:%d, sessionSide:%d(1 is "
        "client side), deviceId:%s", sessionId, sessionSide, deviceId.c_str());

    if (callback_ != nullptr) {
        callback_->onResponseRegisterDistributedHardware(lastDevId_, lastHwId_, true);
    }

    char mySessionName[SESSION_NAME_SIZE_MAX] = "";
    char peerSessionName[SESSION_NAME_SIZE_MAX] = "";
    char peerDevId[DEVICE_ID_SIZE_MAX] = "";
    int ret = GetMySessionName(sessionId, mySessionName, sizeof(mySessionName));
    if (ret != SUCCESS) {
        DHLOGI("get my session name failed, session id is %d.", sessionId);
    }
    ret = GetPeerSessionName(sessionId, peerSessionName, sizeof(peerSessionName));
    if (ret != SUCCESS) {
        DHLOGI("get peer session name failed, session id is %d.", sessionId);
    }
    ret = GetPeerDeviceId(sessionId, peerDevId, sizeof(peerDevId));
    if (ret != SUCCESS) {
        DHLOGI("get peer device id failed, session id is %d.", sessionId);
    }
    DHLOGI("mySessionName:%s, peerSessionName:%s, peerDevId:%s.",
        mySessionName, peerSessionName, peerDevId);

    return SUCCESS;
}

void DistributedInputSourceTransport::OnSessionClosed(int32_t sessionId)
{
    std::string deviceId = FindDeviceBySession(sessionId);
    DHLOGI("OnSessionClosed, sessionId:%d, deviceId:%s",
        sessionId, deviceId.c_str());
    std::unique_lock<std::mutex> sessionLock(operationMutex_);
    if (sessionDevMap_.count(deviceId) > 0) {
        sessionDevMap_.erase(deviceId);
    }
    if (devHardwareMap_.count(deviceId)) {
        devHardwareMap_.erase(deviceId);
    }
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

// send message by sessionId (channel opened)
int32_t DistributedInputSourceTransport::SendMsg(int32_t sessionId, std::string &message)
{
    DHLOGI("start SendMsg");
    if (message.size() > MSG_MAX_SIZE) {
        DHLOGE("SendMessage error: message.size() > MSG_MAX_SIZE");
        return FAILURE;
    }
    uint8_t *buf = (uint8_t *)calloc((MSG_MAX_SIZE), sizeof(uint8_t));
    if (buf == nullptr) {
        DHLOGE("SendMsg: malloc memory failed");
        return FAILURE;
    }
    int32_t outLen = 0;
    if (memcpy_s(buf, MSG_MAX_SIZE, (const uint8_t *)message.c_str(), message.size()) != SUCCESS) {
        DHLOGE("SendMsg: memcpy memory failed");
        free(buf);
        return FAILURE;
    }
    outLen = message.size();
    int32_t ret = SendBytes(sessionId, buf, outLen);
    free(buf);
    return ret;
}
} // namespace DistributedInput
} // namespace DistributedHardware
} // namespace OHOS
