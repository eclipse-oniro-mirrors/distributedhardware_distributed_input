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

namespace OHOS {
namespace DistributedHardware {
namespace DistributedInput {
const int32_t DINPUT_LINK_TYPE_MAX = 4;
static SessionAttribute g_sessionAttr = {
    .dataType = SessionType::TYPE_BYTES,
    .linkTypeNum = DINPUT_LINK_TYPE_MAX,
    .linkType = {
        LINK_TYPE_WIFI_P2P,
        LINK_TYPE_WIFI_WLAN_2G,
        LINK_TYPE_WIFI_WLAN_5G,
        LINK_TYPE_BR
    }
};

DistributedInputSourceTransport::~DistributedInputSourceTransport()
{
    DHLOGI("Dtor DistributedInputSourceTransport");
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
    memberFuncMap_[TRANS_SINK_MSG_LATENCY] = &DistributedInputSourceTransport::CalculateLatency;
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
    std::unique_lock<std::mutex> sessionLock(operationMutex_);
    std::for_each(sessionDevMap_.begin(), sessionDevMap_.end(), [](auto item) { CloseSession(item.first); });
    (void)RemoveSessionServer(DINPUT_PKG_NAME.c_str(), mySessionName_.c_str());
    sessionDevMap_.clear();
    channelStatusMap_.clear();
    memberFuncMap_.clear();
    DistributedInputInject::GetInstance().StopInjectThread();
}

int32_t DistributedInputSourceTransport::CheckDeviceSessionState(bool isToSrcSa, const std::string &devId)
{
    std::unique_lock<std::mutex> sessionLock(operationMutex_);
    for (auto tmp : sessionDevMap_) {
        if (tmp.second.isToSrcSa == isToSrcSa && tmp.second.remoteId == devId) {
            return DH_SUCCESS;
        }
    }
    return ERR_DH_INPUT_SERVER_SOURCE_TRANSPORT_DEVICE_SESSION_STATE;
}

int32_t DistributedInputSourceTransport::OpenInputSoftbus(const std::string &remoteDevId)
{
    int32_t ret = CheckDeviceSessionState(false, remoteDevId);
    if (ret == DH_SUCCESS) {
        DHLOGE("Softbus session has already opened, deviceId: %s", GetAnonyString(remoteDevId).c_str());
        return DH_SUCCESS;
    }

    std::string peerSessionName = SESSION_NAME_SINK + remoteDevId.substr(0, INTERCEPT_STRING_LENGTH);
    DHLOGI("OpenInputSoftbus peerSessionName:%s", peerSessionName.c_str());

    StartAsyncTrace(DINPUT_HITRACE_LABEL, DINPUT_OPEN_SESSION_START, DINPUT_OPEN_SESSION_TASK);
    int sessionId = OpenSession(mySessionName_.c_str(), peerSessionName.c_str(), remoteDevId.c_str(),
        GROUP_ID.c_str(), &g_sessionAttr);
    if (sessionId < 0) {
        DHLOGE("OpenSession fail, remoteDevId: %s, sessionId: %d", GetAnonyString(remoteDevId).c_str(), sessionId);
        FinishAsyncTrace(DINPUT_HITRACE_LABEL, DINPUT_OPEN_SESSION_START, DINPUT_OPEN_SESSION_TASK);
        return ERR_DH_INPUT_SERVER_SOURCE_TRANSPORT_OPEN_SESSION_FAIL;
    }

    HiDumper::GetInstance().CreateSessionInfo(remoteDevId, sessionId, mySessionName_, peerSessionName,
        SessionStatus::OPENING);
    {
        std::unique_lock<std::mutex> sessionLock(operationMutex_);
        DInputSessionInfo sessionInfo{false, remoteDevId};
        sessionDevMap_[sessionId] = sessionInfo;
    }

    DHLOGI("Wait for channel session opened.");
    {
        std::unique_lock<std::mutex> waitLock(operationMutex_);
        auto status = openSessionWaitCond_.wait_for(waitLock, std::chrono::seconds(SESSION_WAIT_TIMEOUT_SECOND),
            [this, sessionId] () { return channelStatusMap_[sessionId]; });
        if (!status) {
            DHLOGE("OpenSession timeout, remoteDevId: %s, sessionId: %d",
                GetAnonyString(remoteDevId).c_str(), sessionId);
            return ERR_DH_INPUT_SERVER_SOURCE_TRANSPORT_OPEN_SESSION_TIMEOUT;
        }
    }

    StartLatencyThread(remoteDevId);

    DistributedInputInject::GetInstance().StartInjectThread();
    DHLOGI("OpenSession success, remoteDevId:%s, sessionId: %d", GetAnonyString(remoteDevId).c_str(), sessionId);
    sessionId_ = sessionId;

#ifdef DINPUT_LOW_LATENCY
    DInputLowLatency::GetInstance().EnableSourceLowLatency();
#endif

    HiDumper::GetInstance().SetSessionStatus(remoteDevId, SessionStatus::OPENED);
    return DH_SUCCESS;
}

void DistributedInputSourceTransport::CloseInputSoftbus(const int32_t sessionId)
{
    std::unique_lock<std::mutex> sessionLock(operationMutex_);
    // check this device's all hd is close,this device session close.

    if (sessionDevMap_.count(sessionId) == 0) {
        DHLOGI("sessionDevMap_ Not find sessionId: %d", sessionId);
        return;
    }

    StopLatencyThread();

    DHLOGI("RemoteDevId: %s, sessionId: %d", GetAnonyString(sessionDevMap_[sessionId].remoteId).c_str(), sessionId);
    HiDumper::GetInstance().SetSessionStatus(sessionDevMap_[sessionId].remoteId, SessionStatus::CLOSING);
    CloseSession(sessionId);
    sessionDevMap_.erase(sessionId);
    channelStatusMap_.erase(sessionId);
    DistributedInputInject::GetInstance().StopInjectThread();

#ifdef DINPUT_LOW_LATENCY
    DInputLowLatency::GetInstance().DisableSourceLowLatency();
#endif

    HiDumper::GetInstance().SetSessionStatus(sessionDevMap_[sessionId].remoteId, SessionStatus::CLOSED);
    HiDumper::GetInstance().DeleteSessionInfo(sessionDevMap_[sessionId].remoteId);
}


int32_t DistributedInputSourceTransport::OpenInputSoftbusForRelay(const std::string &srcId)
{
    int32_t ret = CheckDeviceSessionState(true, srcId);
    if (ret == DH_SUCCESS) {
        DHLOGE("Softbus session has already opened, deviceId: %s", GetAnonyString(srcId).c_str());
        return DH_SUCCESS;
    }

    ret = Init();
    if (ret != DH_SUCCESS) {
        return ERR_DH_INPUT_SERVER_SOURCE_TRANSPORT_OPEN_SESSION_FAIL;
    }

    std::string peerSessionName = SESSION_NAME_SOURCE + srcId.substr(0, INTERCEPT_STRING_LENGTH);
    DHLOGI("OpenInputSoftbus peerSessionName:%s", peerSessionName.c_str());

    StartAsyncTrace(DINPUT_HITRACE_LABEL, DINPUT_OPEN_SESSION_START, DINPUT_OPEN_SESSION_TASK);
    int sessionId = OpenSession(mySessionName_.c_str(), peerSessionName.c_str(), srcId.c_str(), GROUP_ID.c_str(),
        &g_sessionAttr);
    if (sessionId < 0) {
        DHLOGE("OpenSession fail, remoteDevId: %s, sessionId: %d", GetAnonyString(srcId).c_str(), sessionId);
        FinishAsyncTrace(DINPUT_HITRACE_LABEL, DINPUT_OPEN_SESSION_START, DINPUT_OPEN_SESSION_TASK);
        return ERR_DH_INPUT_SERVER_SOURCE_TRANSPORT_OPEN_SESSION_FAIL;
    }

    HiDumper::GetInstance().CreateSessionInfo(srcId, sessionId, mySessionName_, peerSessionName,
        SessionStatus::OPENING);
    {
        std::unique_lock<std::mutex> sessionLock(operationMutex_);
        DInputSessionInfo sessionInfo{true, srcId};
        sessionDevMap_[sessionId] = sessionInfo;
    }

    DHLOGI("Wait for channel session opened.");
    {
        std::unique_lock<std::mutex> waitLock(operationMutex_);
        auto status = openSessionWaitCond_.wait_for(waitLock, std::chrono::seconds(SESSION_WAIT_TIMEOUT_SECOND),
            [this, sessionId] () { return channelStatusMap_[sessionId]; });
        if (!status) {
            DHLOGE("OpenSession timeout, remoteDevId: %s, sessionId: %d", GetAnonyString(srcId).c_str(), sessionId);
            return ERR_DH_INPUT_SERVER_SOURCE_TRANSPORT_OPEN_SESSION_TIMEOUT;
        }
    }

#ifdef DINPUT_LOW_LATENCY
    DInputLowLatency::GetInstance().EnableSourceLowLatency();
#endif

    DHLOGI("OpenSession success, remoteDevId:%s, sessionId:%d", GetAnonyString(srcId).c_str(), sessionId);
    HiDumper::GetInstance().SetSessionStatus(srcId, SessionStatus::OPENED);
    return DH_SUCCESS;
}

void DistributedInputSourceTransport::RegisterSourceRespCallback(std::shared_ptr<DInputSourceTransCallback> callback)
{
    DHLOGI("RegisterSourceRespCallback");
    callback_ = callback;
}

int32_t DistributedInputSourceTransport::FindSessionIdByDevId(bool isToSrc, const std::string &deviceId)
{
    std::unique_lock<std::mutex> sessionLock(operationMutex_);
    int32_t sessionId = -1;
    for (auto tmp : sessionDevMap_) {
        if (tmp.second.isToSrcSa == isToSrc && tmp.second.remoteId == deviceId) {
            sessionId = tmp.first;
            break;
        }
    }
    return sessionId;
}

/*
 * PrepareRemoteInput.
 * @param  deviceId is remote device
 * @return Returns 0 is success, other is fail.
 */
int32_t DistributedInputSourceTransport::PrepareRemoteInput(const std::string& deviceId)
{
    int32_t sessionId = FindSessionIdByDevId(false, deviceId);
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
    int32_t ret = SendMsg(sessionId, smsg);
    if (ret != DH_SUCCESS) {
        DHLOGE("PrepareRemoteInput deviceId:%s, sessionId:%d, smsg:%s, SendMsg error, ret:%d.",
            GetAnonyString(deviceId).c_str(), sessionId, SetAnonyId(smsg).c_str(), ret);
        return ERR_DH_INPUT_SERVER_SOURCE_TRANSPORT_PREPARE_FAIL;
    }
    DHLOGI("PrepareRemoteInput devId:%s, sessionId:%d, smsg:%s.",
        GetAnonyString(deviceId).c_str(), sessionId, SetAnonyId(smsg).c_str());
    return DH_SUCCESS;
}

int32_t DistributedInputSourceTransport::UnprepareRemoteInput(const std::string& deviceId)
{
    int32_t sessionId = FindSessionIdByDevId(false, deviceId);
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
    int32_t ret = SendMsg(sessionId, smsg);
    if (ret != DH_SUCCESS) {
        DHLOGE("UnprepareRemoteInput deviceId:%s, sessionId:%d, smsg:%s, SendMsg error, ret:%d.",
            GetAnonyString(deviceId).c_str(), sessionId, SetAnonyId(smsg).c_str(), ret);
        return ERR_DH_INPUT_SERVER_SOURCE_TRANSPORT_UNPREPARE_FAIL;
    }
    DHLOGI("UnprepareRemoteInput deviceId:%s, sessionId:%d, smsg:%s.",
        GetAnonyString(deviceId).c_str(), sessionId, SetAnonyId(smsg).c_str());
    return DH_SUCCESS;
}


int32_t DistributedInputSourceTransport::PrepareRemoteInput(int32_t srcTsrcSeId, const std::string &deviceId)
{
    int32_t sinkSessionId = FindSessionIdByDevId(false, deviceId);
    if (sinkSessionId < 0) {
        DHLOGE("PrepareRemoteInput error, not find this device:%s.", GetAnonyString(deviceId).c_str());
        return ERR_DH_INPUT_SERVER_SOURCE_TRANSPORT_PREPARE_FAIL;
    }
    DHLOGI("PrepareRemoteInput srcTsrcSeId:%d, sinkSessionId:%d.", srcTsrcSeId, sinkSessionId);

    nlohmann::json jsonStr;
    jsonStr[DINPUT_SOFTBUS_KEY_CMD_TYPE] = TRANS_SOURCE_MSG_PREPARE_FOR_REL;
    jsonStr[DINPUT_SOFTBUS_KEY_DEVICE_ID] = deviceId;
    jsonStr[DINPUT_SOFTBUS_KEY_SESSION_ID] = srcTsrcSeId;
    std::string smsg = jsonStr.dump();
    int32_t ret = SendMsg(sinkSessionId, smsg);
    if (ret != DH_SUCCESS) {
        DHLOGE("PrepareRemoteInput deviceId:%s, smsg:%s, SendMsg error, ret:%d.",
            GetAnonyString(deviceId).c_str(), SetAnonyId(smsg).c_str(), ret);
        return ERR_DH_INPUT_SERVER_SOURCE_TRANSPORT_PREPARE_FAIL;
    }
    DHLOGI("PrepareRemoteInput send success, devId:%s, msg:%s.",
        GetAnonyString(deviceId).c_str(), SetAnonyId(smsg).c_str());
    return DH_SUCCESS;
}

int32_t DistributedInputSourceTransport::UnprepareRemoteInput(int32_t srcTsrcSeId, const std::string &deviceId)
{
    int32_t sinkSessionId = FindSessionIdByDevId(false, deviceId);
    if (sinkSessionId < 0) {
        DHLOGE("UnprepareRemoteInput error, not find this device:%s.", GetAnonyString(deviceId).c_str());
        return ERR_DH_INPUT_SERVER_SOURCE_TRANSPORT_UNPREPARE_FAIL;
    }
    DHLOGI("UnprepareRemoteInput srcTsrcSeId:%d, sinkSessionId:%d.", srcTsrcSeId, sinkSessionId);

    nlohmann::json jsonStr;
    jsonStr[DINPUT_SOFTBUS_KEY_CMD_TYPE] = TRANS_SOURCE_MSG_UNPREPARE_FOR_REL;
    jsonStr[DINPUT_SOFTBUS_KEY_DEVICE_ID] = deviceId;
    jsonStr[DINPUT_SOFTBUS_KEY_SESSION_ID] = srcTsrcSeId;
    std::string smsg = jsonStr.dump();
    int32_t ret = SendMsg(sinkSessionId, smsg);
    if (ret != DH_SUCCESS) {
        DHLOGE("UnprepareRemoteInput deviceId:%s, smsg:%s, SendMsg error, ret:%d.",
            GetAnonyString(deviceId).c_str(), SetAnonyId(smsg).c_str(), ret);
        return ERR_DH_INPUT_SERVER_SOURCE_TRANSPORT_UNPREPARE_FAIL;
    }
    DHLOGI("UnprepareRemoteInput send success, devId:%s, msg:%s.",
        GetAnonyString(deviceId).c_str(), SetAnonyId(smsg).c_str());
    return DH_SUCCESS;
}

int32_t DistributedInputSourceTransport::StartRemoteInputDhids(int32_t srcTsrcSeId, const std::string &deviceId,
    const std::string &dhids)
{
    int32_t sinkSessionId = FindSessionIdByDevId(false, deviceId);
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
    int32_t ret = SendMsg(sinkSessionId, smsg);
    if (ret != DH_SUCCESS) {
        DHLOGE("StartRemoteInputDhids deviceId:%s, smsg:%s, SendMsg error, ret:%d.",
            GetAnonyString(deviceId).c_str(), SetAnonyId(smsg).c_str(), ret);
        return ERR_DH_INPUT_SERVER_SOURCE_TRANSPORT_START_FAIL;
    }
    DHLOGI("StartRemoteInputDhids send success, devId:%s, msg:%s.", GetAnonyString(deviceId).c_str(),
        SetAnonyId(smsg).c_str());
    return DH_SUCCESS;
}

int32_t DistributedInputSourceTransport::StopRemoteInputDhids(int32_t srcTsrcSeId, const std::string &deviceId,
    const std::string &dhids)
{
    int32_t sinkSessionId = FindSessionIdByDevId(false, deviceId);
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
    int32_t ret = SendMsg(sinkSessionId, smsg);
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
    int32_t sinkSessionId = FindSessionIdByDevId(false, deviceId);
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
    int32_t ret = SendMsg(sinkSessionId, smsg);
    if (ret != DH_SUCCESS) {
        DHLOGE("StartRemoteInputType deviceId:%s, smsg:%s, SendMsg error, ret:%d.",
            GetAnonyString(deviceId).c_str(), smsg.c_str(), ret);
        return ERR_DH_INPUT_SERVER_SOURCE_TRANSPORT_START_FAIL;
    }
    DHLOGI("StartRemoteInputType send success, devId:%s, smsg:%s.", GetAnonyString(deviceId).c_str(),
        SetAnonyId(smsg).c_str());
    return DH_SUCCESS;
}

int32_t DistributedInputSourceTransport::StopRemoteInputType(int32_t srcTsrcSeId, const std::string &deviceId,
    const uint32_t& inputTypes)
{
    int32_t sinkSessionId = FindSessionIdByDevId(false, deviceId);
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
    int32_t ret = SendMsg(sinkSessionId, smsg);
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
    int32_t sessionId = FindSessionIdByDevId(true, srcId);
    if (sessionId < 0) {
        DHLOGE("SendRelayPrepareRequest error, not find this device:%s.", GetAnonyString(srcId).c_str());
        return ERR_DH_INPUT_SERVER_SOURCE_TRANSPORT_PREPARE_FAIL;
    }
    DHLOGI("SendRelayPrepareRequest sessionId:%d.", sessionId);

    nlohmann::json jsonStr;
    jsonStr[DINPUT_SOFTBUS_KEY_CMD_TYPE] = TRANS_SOURCE_TO_SOURCE_MSG_PREPARE;
    jsonStr[DINPUT_SOFTBUS_KEY_DEVICE_ID] = sinkId;
    std::string smsg = jsonStr.dump();
    int32_t ret = SendMsg(sessionId, smsg);
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
    int32_t sessionId = FindSessionIdByDevId(true, srcId);
    if (sessionId < 0) {
        DHLOGE("SendRelayUnprepareRequest error, not find this device:%s.", GetAnonyString(srcId).c_str());
        return ERR_DH_INPUT_SERVER_SOURCE_TRANSPORT_PREPARE_FAIL;
    }
    DHLOGI("SendRelayUnprepareRequest sessionId:%d.", sessionId);

    nlohmann::json jsonStr;
    jsonStr[DINPUT_SOFTBUS_KEY_CMD_TYPE] = TRANS_SOURCE_TO_SOURCE_MSG_UNPREPARE;
    jsonStr[DINPUT_SOFTBUS_KEY_DEVICE_ID] = sinkId;
    std::string smsg = jsonStr.dump();
    int32_t ret = SendMsg(sessionId, smsg);
    if (ret != DH_SUCCESS) {
        DHLOGE("SendRelayUnprepareRequest srcId:%s, sessionId:%d, smsg:%s, SendMsg error, ret:%d.",
            GetAnonyString(srcId).c_str(), sessionId, SetAnonyId(smsg).c_str(), ret);
        return ERR_DH_INPUT_SERVER_SOURCE_TRANSPORT_PREPARE_FAIL;
    }
    DHLOGI("SendRelayUnprepareRequest srcId:%s, sessionId:%s, smsg:%s.",
        GetAnonyString(srcId).c_str(), GetAnonyInt32(sessionId).c_str(), SetAnonyId(smsg).c_str());
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
    int32_t ret = SendMsg(srcTsrcSeId, smsg);
    if (ret != DH_SUCCESS) {
        DHLOGE("NotifyOriginPrepareResult srcTsrcSeId:%d, smsg:%s, SendMsg error, ret:%d.",
            srcTsrcSeId, SetAnonyId(smsg).c_str(), ret);
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
    int32_t ret = SendMsg(srcTsrcSeId, smsg);
    if (ret != DH_SUCCESS) {
        DHLOGE("NotifyOriginUnprepareResult srcTsrcSeId:%d, smsg:%s, SendMsg error, ret:%d.",
            srcTsrcSeId, SetAnonyId(smsg).c_str(), ret);
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
    int32_t ret = SendMsg(srcTsrcSeId, smsg);
    if (ret != DH_SUCCESS) {
        DHLOGE("NotifyOriginStartDhidResult srcTsrcSeId:%d, smsg:%s, SendMsg error, ret:%d.",
            srcTsrcSeId, SetAnonyId(smsg).c_str(), ret);
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
    int32_t ret = SendMsg(srcTsrcSeId, smsg);
    if (ret != DH_SUCCESS) {
        DHLOGE("NotifyOriginStopDhidResult srcTsrcSeId:%d, smsg:%s, SendMsg error, ret:%d.",
            srcTsrcSeId, SetAnonyId(smsg).c_str(), ret);
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
    int32_t ret = SendMsg(srcTsrcSeId, smsg);
    if (ret != DH_SUCCESS) {
        DHLOGE("NotifyOriginStartTypeResult srcTsrcSeId:%d, smsg:%s, SendMsg error, ret:%d.",
            srcTsrcSeId, SetAnonyId(smsg).c_str(), ret);
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
    int32_t ret = SendMsg(srcTsrcSeId, smsg);
    if (ret != DH_SUCCESS) {
        DHLOGE("NotifyOriginStopTypeResult srcTsrcSeId:%d, smsg:%s, SendMsg error, ret:%d.",
            srcTsrcSeId, SetAnonyId(smsg).c_str(), ret);
        return ERR_DH_INPUT_SERVER_SOURCE_TRANSPORT_STOP_FAIL;
    }
    DHLOGI("NotifyOriginStopTypeResult srcTsrcSeId:%d, smsg:%s.", srcTsrcSeId, SetAnonyId(smsg).c_str());
    return DH_SUCCESS;
}


int32_t DistributedInputSourceTransport::StartRemoteInput(
    const std::string& deviceId, const uint32_t& inputTypes)
{
    int32_t sessionId = FindSessionIdByDevId(false, deviceId);
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
    int32_t ret = SendMsg(sessionId, smsg);
    if (ret != DH_SUCCESS) {
        DHLOGE("StartRemoteInput deviceId:%s, sessionId:%d, smsg:%s, SendMsg error, ret:%d.",
            GetAnonyString(deviceId).c_str(), sessionId, SetAnonyId(smsg).c_str(), ret);
        return ERR_DH_INPUT_SERVER_SOURCE_TRANSPORT_START_FAIL;
    }
    DHLOGI("StartRemoteInput deviceId:%s, sessionId:%d, smsg:%s.",
        GetAnonyString(deviceId).c_str(), sessionId, SetAnonyId(smsg).c_str());
    return DH_SUCCESS;
}

int32_t DistributedInputSourceTransport::StopRemoteInput(
    const std::string& deviceId, const uint32_t& inputTypes)
{
    int32_t sessionId = FindSessionIdByDevId(false, deviceId);
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
    int32_t ret = SendMsg(sessionId, smsg);
    if (ret != DH_SUCCESS) {
        DHLOGE("StopRemoteInput deviceId:%s, sessionId:%d, smsg:%s, SendMsg error, ret:%d.",
            GetAnonyString(deviceId).c_str(), sessionId, SetAnonyId(smsg).c_str(), ret);
        return ERR_DH_INPUT_SERVER_SOURCE_TRANSPORT_STOP_FAIL;
    }
    DHLOGI("StopRemoteInput deviceId:%s, sessionId:%d, smsg:%s.",
        GetAnonyString(deviceId).c_str(), sessionId, SetAnonyId(smsg).c_str());
    return DH_SUCCESS;
}

int32_t DistributedInputSourceTransport::LatencyCount(const std::string& deviceId)
{
    int32_t sessionId = FindSessionIdByDevId(false, deviceId);
    if (sessionId < 0) {
        DHLOGE("LatencyCount error, not find this device:%s.", GetAnonyString(deviceId).c_str());
        return ERR_DH_INPUT_SERVER_SOURCE_TRANSPORT_LATENCY_FAIL;
    }

    nlohmann::json jsonStr;
    jsonStr[DINPUT_SOFTBUS_KEY_CMD_TYPE] = TRANS_SOURCE_MSG_LATENCY;
    jsonStr[DINPUT_SOFTBUS_KEY_DEVICE_ID] = deviceId;
    jsonStr[DINPUT_SOFTBUS_KEY_SESSION_ID] = sessionId;
    std::string smsg = jsonStr.dump();
    int32_t ret = SendMsg(sessionId, smsg);
    if (ret != DH_SUCCESS) {
        DHLOGE("LatencyCount deviceId:%s, sessionId: %d, smsg:%s, SendMsg error, ret:%d.",
            GetAnonyString(deviceId).c_str(), sessionId, SetAnonyId(smsg).c_str(), ret);
        return ERR_DH_INPUT_SERVER_SOURCE_TRANSPORT_LATENCY_FAIL;
    }

    DHLOGI("LatencyCount deviceId:%s, sessionId: %d, smsg:%s.",
        GetAnonyString(deviceId).c_str(), sessionId, SetAnonyId(smsg).c_str());
    return DH_SUCCESS;
}

void DistributedInputSourceTransport::StartLatencyCount(const std::string& deviceId)
{
    DHLOGI("start");
    while (isLatencyThreadRunning_.load()) {
        if (sendNum_ >= INPUT_LATENCY_DELAY_TIMES) {
            uint64_t latency = (uint64_t)(deltaTimeAll_ / 2 / INPUT_LATENCY_DELAY_TIMES);
            DHLOGI("LatencyCount average single-channel latency is %d us, send times is %d, recive times is %d, " +
                "each RTT latency details is %s", latency, sendNum_, recvNum_, eachLatencyDetails_.c_str());
            deltaTimeAll_ = 0;
            sendNum_ = 0;
            recvNum_ = 0;
            eachLatencyDetails_ = "";
        }
        sendTime_ = GetCurrentTime();
        LatencyCount(deviceId);
        sendNum_ += 1;
        usleep(INPUT_LATENCY_DELAYTIME_US);
    }
    DHLOGI("end");
}

void DistributedInputSourceTransport::StartLatencyThread(const std::string& deviceId)
{
    DHLOGI("start");
    isLatencyThreadRunning_.store(true);
    latencyThread_ = std::thread(&DistributedInputSourceTransport::StartLatencyCount, this, deviceId);
    DHLOGI("end");
}

void DistributedInputSourceTransport::StopLatencyThread()
{
    DHLOGI("start");
    isLatencyThreadRunning_.store(false);
    if (latencyThread_.joinable()) {
        latencyThread_.join();
    }
    DHLOGI("end");
}

int32_t DistributedInputSourceTransport::StartRemoteInput(const std::string &deviceId,
    const std::vector<std::string> &dhids)
{
    int32_t sessionId = FindSessionIdByDevId(false, deviceId);
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
    int32_t ret = SendMsg(sessionId, smsg);
    if (ret != DH_SUCCESS) {
        DHLOGE("StartRemoteInput deviceId:%s, sessionId: %d, smsg:%s, SendMsg error, ret:%d.",
            GetAnonyString(deviceId).c_str(), sessionId, SetAnonyId(smsg).c_str(), ret);
        return ERR_DH_INPUT_SERVER_SOURCE_TRANSPORT_START_FAIL;
    }
    DHLOGI("StartRemoteInput deviceId:%s, sessionId: %d, smsg:%s.", GetAnonyString(deviceId).c_str(),
        sessionId, SetAnonyId(smsg).c_str());
    return DH_SUCCESS;
}

int32_t DistributedInputSourceTransport::StopRemoteInput(const std::string &deviceId,
    const std::vector<std::string> &dhids)
{
    int32_t sessionId = FindSessionIdByDevId(false, deviceId);
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
    int32_t ret = SendMsg(sessionId, smsg);
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
    int32_t sessionId = FindSessionIdByDevId(true, srcId);
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
    int32_t ret = SendMsg(sessionId, smsg);
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
    int32_t sessionId = FindSessionIdByDevId(true, srcId);
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
    int32_t ret = SendMsg(sessionId, smsg);
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
    int32_t sessionId = FindSessionIdByDevId(true, srcId);
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
    int32_t ret = SendMsg(sessionId, smsg);
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
    int32_t sessionId = FindSessionIdByDevId(true, srcId);
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
    int32_t ret = SendMsg(sessionId, smsg);
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

std::string DistributedInputSourceTransport::FindDeviceBySession(int32_t sessionId)
{
    std::unique_lock<std::mutex> sessionLock(operationMutex_);
    std::string devId = "";
    if (sessionDevMap_.count(sessionId) == 0) {
        DHLOGE("FindDeviceBySession error, has no this sessionId.");
        return devId;
    }
    devId = sessionDevMap_[sessionId].remoteId;
    return devId;
}

int32_t DistributedInputSourceTransport::OnSessionOpened(int32_t sessionId, int32_t result)
{
    FinishAsyncTrace(DINPUT_HITRACE_LABEL, DINPUT_OPEN_SESSION_START, DINPUT_OPEN_SESSION_TASK);
    if (result != DH_SUCCESS) {
        DHLOGE("session open failed, sessionId:%d, result:%d, deviceId:%s", sessionId, result,
            GetAnonyString(sessionDevMap_[sessionId].remoteId).c_str());
        std::unique_lock<std::mutex> sessionLock(operationMutex_);
        if (sessionDevMap_.count(sessionId) > 0) {
            sessionDevMap_.erase(sessionId);
        }
        return DH_SUCCESS;
    }

    std::string deviceId = FindDeviceBySession(sessionId);
    int32_t sessionSide = GetSessionSide(sessionId);
    DHLOGI("session open succeed, sessionId: %d, sessionSide:%d(1 is "
        "client side), deviceId:%s", sessionId, sessionSide, GetAnonyString(deviceId).c_str());

    char mySessionName[SESSION_NAME_SIZE_MAX] = "";
    char peerSessionName[SESSION_NAME_SIZE_MAX] = "";
    char peerDevId[DEVICE_ID_SIZE_MAX] = "";
    int ret = GetMySessionName(sessionId, mySessionName, sizeof(mySessionName));
    if (ret != DH_SUCCESS) {
        DHLOGI("get my session name failed, session id is %d", sessionId);
    }
    ret = GetPeerSessionName(sessionId, peerSessionName, sizeof(peerSessionName));
    if (ret != DH_SUCCESS) {
        DHLOGI("get peer session name failed, session id is %d", sessionId);
    }
    ret = GetPeerDeviceId(sessionId, peerDevId, sizeof(peerDevId));
    if (ret != DH_SUCCESS) {
        DHLOGI("get peer device id failed, session id is %d", sessionId);
    }
    DHLOGI("mySessionName:%s, peerSessionName:%s, peerDevId:%s",
        mySessionName, peerSessionName, GetAnonyString(peerDevId).c_str());

    if (sessionSide == AUTH_SESSION_SIDE_SERVER) {
        DHLOGI("session open succeed, sessionId:%d, sessionSide:service", sessionId);
        std::lock_guard<std::mutex> notifyLock(operationMutex_);
        DInputSessionInfo sessionInfo{true, peerDevId};
        sessionDevMap_[sessionId] = sessionInfo;
    } else {
        DHLOGI("session open succeed, sessionId:%d, sessionSide:client", sessionId);
        std::lock_guard<std::mutex> notifyLock(operationMutex_);
        channelStatusMap_[sessionId] = true;
        openSessionWaitCond_.notify_all();
    }

    return DH_SUCCESS;
}

void DistributedInputSourceTransport::OnSessionClosed(int32_t sessionId)
{
    std::string deviceId = FindDeviceBySession(sessionId);
    DHLOGI("OnSessionClosed, sessionId: %d, deviceId:%s", sessionId, GetAnonyString(deviceId).c_str());
    std::unique_lock<std::mutex> sessionLock(operationMutex_);
    if (sessionDevMap_.count(sessionId) > 0) {
        sessionDevMap_.erase(sessionId);
    }
    if (channelStatusMap_.count(sessionId) > 0) {
        channelStatusMap_.erase(sessionId);
    }
    StopLatencyThread();
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
    CloseInputSoftbus(sessionId);
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

void DistributedInputSourceTransport::NotifyResponseStartRemoteInputDhid(int32_t sessionId,
    const nlohmann::json &recMsg)
{
    DHLOGI("OnBytesReceived cmdType is TRANS_SINK_MSG_DHID_ONSTART.");
    if (!recMsg[DINPUT_SOFTBUS_KEY_RESP_VALUE].is_boolean()) {
        DHLOGE("OnBytesReceived cmdType is TRANS_SINK_MSG_DHID_ONSTART, data type is error.");
        return;
    }
    std::string deviceId = FindDeviceBySession(sessionId);
    if (deviceId.empty()) {
        DHLOGE("OnBytesReceived cmdType is TRANS_SINK_MSG_DHID_ONSTART, deviceId is error.");
        return;
    }
    callback_->onResponseStartRemoteInputDhid(
        deviceId, recMsg[DINPUT_SOFTBUS_KEY_VECTOR_DHID], recMsg[DINPUT_SOFTBUS_KEY_RESP_VALUE]);
}

void DistributedInputSourceTransport::NotifyResponseStopRemoteInputDhid(int32_t sessionId, const nlohmann::json &recMsg)
{
    DHLOGI("OnBytesReceived cmdType is TRANS_SINK_MSG_DHID_ONSTOP.");
    if (!recMsg[DINPUT_SOFTBUS_KEY_RESP_VALUE].is_boolean()) {
        DHLOGE("OnBytesReceived cmdType is TRANS_SINK_MSG_DHID_ONSTOP, data type is error.");
        return;
    }
    std::string deviceId = FindDeviceBySession(sessionId);
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
    std::string deviceId = FindDeviceBySession(sessionId);
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

    std::string deviceId = FindDeviceBySession(sessionId);
    if (deviceId.empty()) {
        DHLOGE("OnBytesReceived cmdType is TRANS_SINK_MSG_BODY_DATA, deviceId is error.");
        return;
    }
    std::string inputDataStr = recMsg[DINPUT_SOFTBUS_KEY_INPUT_DATA];
    callback_->onReceivedEventRemoteInput(deviceId, inputDataStr);
}

void DistributedInputSourceTransport::CalculateLatency(int32_t sessionId, const nlohmann::json &recMsg)
{
    DHLOGI("OnBytesReceived cmdType is TRANS_SINK_MSG_LATENCY.");
    std::string deviceId = FindDeviceBySession(sessionId);
    if (deviceId.empty()) {
        DHLOGE("OnBytesReceived cmdType is TRANS_SINK_MSG_LATENCY, deviceId is error.");
        return;
    }

    deltaTime_ = GetCurrentTime() - sendTime_;
    deltaTimeAll_ += deltaTime_;
    recvNum_ += 1;
    eachLatencyDetails_ += (std::to_string(deltaTime_) + DINPUT_SPLIT_COMMA);
}

void DistributedInputSourceTransport::ReceiveSrcTSrcRelayPrepare(int32_t sessionId, const nlohmann::json &recMsg)
{
    DHLOGI("OnBytesReceived cmdType is TRANS_SOURCE_TO_SOURCE_MSG_PREPARE.");
    if (!recMsg[DINPUT_SOFTBUS_KEY_DEVICE_ID].is_string()) {
        DHLOGE("OnBytesReceived cmdType is TRANS_SOURCE_TO_SOURCE_MSG_PREPARE, data type is error.");
        return;
    }
    std::string deviceId = recMsg[DINPUT_SOFTBUS_KEY_DEVICE_ID];

    // continue notify to A_sink_trans
    int32_t ret = OpenInputSoftbus(deviceId);
    if (ret != DH_SUCCESS) {
        callback_->onResponseRelayPrepareRemoteInput(sessionId, deviceId, false, "");
        return;
    }

    ret = PrepareRemoteInput(sessionId, deviceId);
    if (ret != DH_SUCCESS) {
        callback_->onResponseRelayPrepareRemoteInput(sessionId, deviceId, false, "");
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

    std::string deviceId = recMsg[DINPUT_SOFTBUS_KEY_DEVICE_ID];
    int32_t ret = UnprepareRemoteInput(sessionId, deviceId);
    if (ret != DH_SUCCESS) {
        DHLOGE("Can not send message by softbus, unprepare fail.");
        callback_->onResponseRelayUnprepareRemoteInput(sessionId, deviceId, false);
        return;
    }
}

void DistributedInputSourceTransport::NotifyResponseRelayPrepareRemoteInput(int32_t sessionId,
    const nlohmann::json &recMsg)
{
    DHLOGI("OnBytesReceived cmdType is TRANS_SINK_MSG_ON_RELAY_PREPARE.");
    if (!recMsg[DINPUT_SOFTBUS_KEY_RESP_VALUE].is_boolean() ||
        !recMsg[DINPUT_SOFTBUS_KEY_SESSION_ID].is_number() ||
        !recMsg[DINPUT_SOFTBUS_KEY_WHITE_LIST].is_string()) {
        DHLOGE("OnBytesReceived cmdType is TRANS_SINK_MSG_ON_RELAY_PREPARE, data type is error.");
        return;
    }
    std::string sinkDevId = FindDeviceBySession(sessionId);
    if (sinkDevId.empty()) {
        DHLOGE("OnBytesReceived cmdType is TRANS_SINK_MSG_ON_RELAY_PREPARE, sinkDevId is error.");
        return;
    }
    callback_->onResponseRelayPrepareRemoteInput(recMsg[DINPUT_SOFTBUS_KEY_SESSION_ID], sinkDevId,
        recMsg[DINPUT_SOFTBUS_KEY_RESP_VALUE], recMsg[DINPUT_SOFTBUS_KEY_WHITE_LIST]);
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
    std::string sinkDevId = FindDeviceBySession(sessionId);
    if (sinkDevId.empty()) {
        DHLOGE("OnBytesReceived cmdType is TRANS_SINK_MSG_ON_RELAY_UNPREPARE, sinkDevId is error.");
        return;
    }
    callback_->onResponseRelayUnprepareRemoteInput(recMsg[DINPUT_SOFTBUS_KEY_SESSION_ID], sinkDevId,
        recMsg[DINPUT_SOFTBUS_KEY_RESP_VALUE]);
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
    CloseInputSoftbus(sessionId);
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
        !recMsg[DINPUT_SOFTBUS_KEY_VECTOR_DHID].is_string()) {
        DHLOGE("OnBytesReceived cmdType is TRANS_SINK_MSG_ON_RELAY_STARTDHID, data type is error.");
        return;
    }
    std::string sinkDevId = FindDeviceBySession(sessionId);
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
    std::string sinkDevId = FindDeviceBySession(sessionId);
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
        !recMsg[DINPUT_SOFTBUS_KEY_INPUT_TYPE].is_number()) {
        DHLOGE("OnBytesReceived cmdType is TRANS_SINK_MSG_ON_RELAY_STARTTYPE, data type is error.");
        return;
    }
    std::string sinkDevId = FindDeviceBySession(sessionId);
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
    std::string sinkDevId = FindDeviceBySession(sessionId);
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

void DistributedInputSourceTransport::HandleSessionData(int32_t sessionId, const std::string& message)
{
    if (callback_ == nullptr) {
        DHLOGE("OnBytesReceived the callback_ is null, the message:%s abort.", SetAnonyId(message).c_str());
        return;
    }
    nlohmann::json recMsg = nlohmann::json::parse(message, nullptr, false);
    if (CheckRecivedData(message) != true) {
        return;
    }

    int cmdType = recMsg[DINPUT_SOFTBUS_KEY_CMD_TYPE];
    auto iter = memberFuncMap_.find(cmdType);
    if (iter == memberFuncMap_.end()) {
        DHLOGE("OnBytesReceived cmdType %d is undefined.", cmdType);
        return;
    }
    SourceTransportFunc &func = iter->second;
    (this->*func)(sessionId, recMsg);
}

bool DistributedInputSourceTransport::CheckRecivedData(const std::string& message)
{
    nlohmann::json recMsg = nlohmann::json::parse(message, nullptr, false);
    if (recMsg.is_discarded()) {
        DHLOGE("OnBytesReceived jsonStr error.");
        return false;
    }

    if (recMsg.contains(DINPUT_SOFTBUS_KEY_CMD_TYPE) != true) {
        DHLOGE("OnBytesReceived message:%s is error, not contain cmdType.", SetAnonyId(message).c_str());
        return false;
    }

    if (recMsg[DINPUT_SOFTBUS_KEY_CMD_TYPE].is_number() != true) {
        DHLOGE("OnBytesReceived cmdType is not number type.");
        return false;
    }

    return true;
}

void DistributedInputSourceTransport::OnBytesReceived(int32_t sessionId, const void *data, uint32_t dataLen)
{
    DHLOGI("OnBytesReceived, sessionId: %d, dataLen:%d", sessionId, dataLen);
    if (sessionId < 0 || data == nullptr || dataLen <= 0 || dataLen > MSG_MAX_SIZE) {
        DHLOGE("OnBytesReceived param check failed");
        return;
    }

    uint8_t *buf = reinterpret_cast<uint8_t *>(calloc(dataLen + 1, sizeof(uint8_t)));
    if (buf == nullptr) {
        DHLOGE("OnBytesReceived: malloc memory failed");
        return;
    }

    if (memcpy_s(buf, dataLen + 1, reinterpret_cast<const uint8_t *>(data), dataLen) != EOK) {
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

int32_t DistributedInputSourceTransport::GetCurrentSessionId()
{
    return sessionId_;
}

// send message by sessionId (channel opened)
int32_t DistributedInputSourceTransport::SendMsg(int32_t sessionId, std::string &message)
{
    DHLOGD("start SendMsg");
    if (message.size() > MSG_MAX_SIZE) {
        DHLOGE("SendMessage error: message.size() > MSG_MAX_SIZE");
        return ERR_DH_INPUT_SERVER_SOURCE_TRANSPORT_SENDMESSSAGE;
    }
    uint8_t *buf = reinterpret_cast<uint8_t *>(calloc((MSG_MAX_SIZE), sizeof(uint8_t)));
    if (buf == nullptr) {
        DHLOGE("SendMsg: malloc memory failed");
        return ERR_DH_INPUT_SERVER_SOURCE_TRANSPORT_SENDMESSSAGE;
    }
    int32_t outLen = 0;
    if (memcpy_s(buf, MSG_MAX_SIZE, reinterpret_cast<const uint8_t *>(message.c_str()), message.size()) != EOK) {
        DHLOGE("SendMsg: memcpy memory failed");
        free(buf);
        return ERR_DH_INPUT_SERVER_SOURCE_TRANSPORT_SENDMESSSAGE;
    }
    outLen = static_cast<int32_t>(message.size());
    int32_t ret = SendBytes(sessionId, buf, outLen);
    free(buf);
    return ret;
}
} // namespace DistributedInput
} // namespace DistributedHardware
} // namespace OHOS
