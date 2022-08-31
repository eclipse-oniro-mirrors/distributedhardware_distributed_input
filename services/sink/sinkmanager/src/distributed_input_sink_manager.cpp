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

#include "distributed_input_sink_manager.h"

#include <algorithm>
#include <fcntl.h>
#include <thread>
#include <linux/input.h>

#include "anonymous_string.h"
#include "dinput_softbus_define.h"
#include "distributed_hardware_fwk_kit.h"
#include "distributed_hardware_log.h"
#include "if_system_ability_manager.h"
#include "iservice_registry.h"
#include "nlohmann/json.hpp"
#include "screen_manager.h"
#include "string_ex.h"
#include "system_ability_definition.h"

#include "distributed_input_collector.h"
#include "distributed_input_sink_switch.h"
#include "distributed_input_sink_transport.h"

#include "dinput_errcode.h"
#include "dinput_sa_process_state.h"
#include "dinput_utils_tool.h"
#include "hidumper.h"
#include "white_list_util.h"

namespace OHOS {
namespace DistributedHardware {
namespace DistributedInput {
REGISTER_SYSTEM_ABILITY_BY_ID(DistributedInputSinkManager, DISTRIBUTED_HARDWARE_INPUT_SINK_SA_ID, true);

DistributedInputSinkManager::DistributedInputSinkManager(int32_t saId, bool runOnCreate)
    : SystemAbility(saId, runOnCreate)
{
    DHLOGI("DistributedInputSinkManager ctor!");
    inputTypes_ = DInputDeviceType::NONE;
}

DistributedInputSinkManager::~DistributedInputSinkManager()
{
    DHLOGI("DistributedInputSinkManager dtor!");
    projectWindowListener_ = nullptr;
}

DistributedInputSinkManager::DInputSinkListener::DInputSinkListener(DistributedInputSinkManager *manager)
{
    sinkManagerObj_ = manager;
    sinkManagerObj_->SetInputTypes(static_cast<uint32_t>(DInputDeviceType::NONE));
    DHLOGI("DInputSinkListener init.");
}

DistributedInputSinkManager::DInputSinkListener::~DInputSinkListener()
{
    sinkManagerObj_->SetInputTypes(static_cast<uint32_t>(DInputDeviceType::NONE));
    sinkManagerObj_ = nullptr;
    DHLOGI("DInputSinkListener destory.");
}

void DistributedInputSinkManager::DInputSinkListener::onPrepareRemoteInput(
    const int32_t& sessionId, const std::string &deviceId)
{
    DHLOGI("onPrepareRemoteInput called, sessionId: %s, devId: %s",
        GetAnonyInt32(sessionId).c_str(), GetAnonyString(deviceId).c_str());

    nlohmann::json jsonStr;
    jsonStr[DINPUT_SOFTBUS_KEY_CMD_TYPE] = TRANS_SINK_MSG_ONPREPARE;
    std::string smsg = "";
    int ret = DistributedInputCollector::GetInstance().Init(
        DistributedInputSinkTransport::GetInstance().GetEventHandler());
    if (ret != DH_SUCCESS) {
        DHLOGE("DInputSinkListener init InputCollector error.");
        jsonStr[DINPUT_SOFTBUS_KEY_RESP_VALUE] = false;
        jsonStr[DINPUT_SOFTBUS_KEY_WHITE_LIST] = "";
        smsg = jsonStr.dump();
        DistributedInputSinkTransport::GetInstance().RespPrepareRemoteInput(sessionId, smsg);
        return;
    }

    DistributedInputSinkSwitch::GetInstance().AddSession(sessionId);

    // send prepare result and if result ok, send white list
    TYPE_WHITE_LIST_VEC vecFilter;
    std::string localNetworkId = GetLocalDeviceInfo().networkId;
    if (!localNetworkId.empty()) {
        WhiteListUtil::GetInstance().GetWhiteList(localNetworkId, vecFilter);
    } else {
        DHLOGE("query local network id from softbus failed");
    }
    if (vecFilter.empty() || vecFilter[0].empty() || vecFilter[0][0].empty()) {
        DHLOGE("onPrepareRemoteInput called, white list is null.");
        jsonStr[DINPUT_SOFTBUS_KEY_RESP_VALUE] = true;
        jsonStr[DINPUT_SOFTBUS_KEY_WHITE_LIST] = "";
        smsg = jsonStr.dump();
        DistributedInputSinkTransport::GetInstance().RespPrepareRemoteInput(sessionId, smsg);
        return;
    }
    nlohmann::json filterMsg(vecFilter);
    std::string object = filterMsg.dump();
    jsonStr[DINPUT_SOFTBUS_KEY_RESP_VALUE] = true;
    jsonStr[DINPUT_SOFTBUS_KEY_WHITE_LIST] = object;
    smsg = jsonStr.dump();
    DistributedInputSinkTransport::GetInstance().RespPrepareRemoteInput(sessionId, smsg);
}

void DistributedInputSinkManager::DInputSinkListener::onUnprepareRemoteInput(const int32_t& sessionId)
{
    DHLOGI("onUnprepareRemoteInput called, sessionId: %s", GetAnonyInt32(sessionId).c_str());
    onStopRemoteInput(sessionId, static_cast<uint32_t>(DInputDeviceType::ALL));
    DistributedInputSinkSwitch::GetInstance().RemoveSession(sessionId);

    nlohmann::json jsonStr;
    jsonStr[DINPUT_SOFTBUS_KEY_CMD_TYPE] = TRANS_SINK_MSG_ONUNPREPARE;
    jsonStr[DINPUT_SOFTBUS_KEY_RESP_VALUE] = true;
    std::string smsg = jsonStr.dump();
    DistributedInputSinkTransport::GetInstance().RespUnprepareRemoteInput(sessionId, smsg);
}

void DistributedInputSinkManager::DInputSinkListener::onStartRemoteInput(
    const int32_t& sessionId, const uint32_t& inputTypes)
{
    int32_t curSessionId = DistributedInputSinkSwitch::GetInstance().GetSwitchOpenedSession();
    DHLOGI("onStartRemoteInput called, curSessionId:%s, new sessionId: %s",
        GetAnonyInt32(curSessionId).c_str(), GetAnonyInt32(sessionId).c_str());
    // set new session
    int32_t startRes = DistributedInputSinkSwitch::GetInstance().StartSwitch(sessionId);

    sinkManagerObj_->SetStartTransFlag((startRes == DH_SUCCESS) ? DInputServerType::SINK_SERVER_TYPE
        : DInputServerType::NULL_SERVER_TYPE);

    bool result = (startRes == DH_SUCCESS) ? true : false;
    nlohmann::json jsonStrSta;
    jsonStrSta[DINPUT_SOFTBUS_KEY_CMD_TYPE] = TRANS_SINK_MSG_ONSTART;
    jsonStrSta[DINPUT_SOFTBUS_KEY_INPUT_TYPE] = inputTypes;
    jsonStrSta[DINPUT_SOFTBUS_KEY_RESP_VALUE] = result;
    std::string smsgSta = jsonStrSta.dump();
    DistributedInputSinkTransport::GetInstance().RespStartRemoteInput(sessionId, smsgSta);

    // add the input type
    if (startRes == DH_SUCCESS) {
        sinkManagerObj_->SetInputTypes(sinkManagerObj_->GetInputTypes() | inputTypes);
        DistributedInputCollector::GetInstance().SetSharingTypes(true, sinkManagerObj_->GetInputTypes());
    }
}

void DistributedInputSinkManager::DInputSinkListener::onStopRemoteInput(
    const int32_t& sessionId, const uint32_t& inputTypes)
{
    DHLOGI("onStopRemoteInput called, sessionId: %s, inputTypes: %d, curInputTypes: %d",
        GetAnonyInt32(sessionId).c_str(), inputTypes, sinkManagerObj_->GetInputTypes());

    sinkManagerObj_->SetInputTypes(sinkManagerObj_->GetInputTypes() -
        (sinkManagerObj_->GetInputTypes() & inputTypes));
    DistributedInputCollector::GetInstance().SetSharingTypes(false, inputTypes);

    nlohmann::json jsonStr;
    jsonStr[DINPUT_SOFTBUS_KEY_CMD_TYPE] = TRANS_SINK_MSG_ONSTOP;
    jsonStr[DINPUT_SOFTBUS_KEY_INPUT_TYPE] = inputTypes;
    jsonStr[DINPUT_SOFTBUS_KEY_RESP_VALUE] = true;
    std::string smsg = jsonStr.dump();
    DistributedInputSinkTransport::GetInstance().RespStopRemoteInput(sessionId, smsg);

    bool isAllClosed = DistributedInputCollector::GetInstance().IsAllDevicesStoped();
    if (isAllClosed) {
        DistributedInputSinkSwitch::GetInstance().StopAllSwitch();
        if (DistributedInputSinkSwitch::GetInstance().GetSwitchOpenedSession() ==
            ERR_DH_INPUT_SERVER_SINK_GET_OPEN_SESSION_FAIL) {
            DHLOGI("all session is stop.");
            sinkManagerObj_->SetStartTransFlag(DInputServerType::NULL_SERVER_TYPE);
        }
    }
}

void DistributedInputSinkManager::DInputSinkListener::onStartRemoteInputDhid(const int32_t &sessionId,
    const std::string &strDhids)
{
    int32_t curSessionId = DistributedInputSinkSwitch::GetInstance().GetSwitchOpenedSession();
    DHLOGE("onStartRemoteInputDhid called, curSessionId:%s, new sessionId: %s",
        GetAnonyInt32(curSessionId).c_str(), GetAnonyInt32(sessionId).c_str());
    // set new session
    int32_t startRes = DistributedInputSinkSwitch::GetInstance().StartSwitch(sessionId);
    bool result = (startRes == DH_SUCCESS) ? true : false;
    nlohmann::json jsonStrSta;
    jsonStrSta[DINPUT_SOFTBUS_KEY_CMD_TYPE] = TRANS_SINK_MSG_DHID_ONSTART;
    jsonStrSta[DINPUT_SOFTBUS_KEY_VECTOR_DHID] = strDhids;
    jsonStrSta[DINPUT_SOFTBUS_KEY_RESP_VALUE] = result;
    std::string smsgSta = jsonStrSta.dump();
    DistributedInputSinkTransport::GetInstance().RespStartRemoteInput(sessionId, smsgSta);

    if (startRes != DH_SUCCESS) {
        DHLOGE("onStartRemoteInputDhid StartSwitch error.");
        return;
    }

    if (curSessionId == ERR_DH_INPUT_SERVER_SINK_GET_OPEN_SESSION_FAIL) {
        DHLOGW("onStartRemoteInputDhid called, this is the only session.");
    }

    CheckKeyState(sessionId, strDhids);
    // add the dhids
    if (startRes == DH_SUCCESS) {
        std::set<std::string> setStr;
        StringSplitToSet(strDhids, INPUT_STRING_SPLIT_POINT, setStr);
        sinkManagerObj_->StoreStartDhids(sessionId, setStr);
        std::vector<std::string> vecStr;
        vecStr.assign(setStr.begin(), setStr.end());
        DistributedInputCollector::GetInstance().SetSharingDhIds(true, vecStr);
    }
}

void DistributedInputSinkManager::DInputSinkListener::onStopRemoteInputDhid(const int32_t &sessionId,
    const std::string &strDhids)
{
    DHLOGI("onStopRemoteInputDhid called, sessionId: %s", GetAnonyInt32(sessionId).c_str());
    std::vector<std::string> stopStr;
    std::set<std::string> setStr;
    StringSplitToSet(strDhids, INPUT_STRING_SPLIT_POINT, setStr);
    sinkManagerObj_->DeleteStopDhids(sessionId, setStr, stopStr);
    DistributedInputCollector::GetInstance().SetSharingDhIds(false, stopStr);

    if (DistributedInputCollector::GetInstance().IsAllDevicesStoped()) {
        DHLOGE("onStopRemoteInputDhid called, all dhid stop sharing, sessionId: %d is closed.", sessionId);
        DistributedInputSinkSwitch::GetInstance().StopSwitch(sessionId);
    }

    nlohmann::json jsonStr;
    jsonStr[DINPUT_SOFTBUS_KEY_CMD_TYPE] = TRANS_SINK_MSG_DHID_ONSTOP;
    jsonStr[DINPUT_SOFTBUS_KEY_VECTOR_DHID] = strDhids;
    jsonStr[DINPUT_SOFTBUS_KEY_RESP_VALUE] = true;
    std::string smsg = jsonStr.dump();
    DistributedInputSinkTransport::GetInstance().RespStopRemoteInput(sessionId, smsg);

    bool isAllClosed = DistributedInputCollector::GetInstance().IsAllDevicesStoped();
    if (isAllClosed) {
        DistributedInputSinkSwitch::GetInstance().StopAllSwitch();
        sinkManagerObj_->SetInputTypes(static_cast<uint32_t>(DInputDeviceType::NONE));
        if (DistributedInputSinkSwitch::GetInstance().GetSwitchOpenedSession() ==
            ERR_DH_INPUT_SERVER_SINK_GET_OPEN_SESSION_FAIL) {
            DHLOGI("onStartRemoteInput called, all session is stop.");
            sinkManagerObj_->SetStartTransFlag(DInputServerType::NULL_SERVER_TYPE);
        }
    }
}

void DistributedInputSinkManager::DInputSinkListener::StringSplitToSet(const std::string &str, const char split,
    std::set<std::string> &vecStr)
{
    if (str.empty()) {
        DHLOGE("param str is error.");
        return;
    }
    std::string strTmp = str + split;
    size_t pos = strTmp.find(split);
    while (pos != strTmp.npos) {
        std::string matchTmp = strTmp.substr(0, pos);
        vecStr.insert(matchTmp);
        strTmp = strTmp.substr(pos + 1, strTmp.size());
        pos = strTmp.find(split);
    }
}

void DistributedInputSinkManager::DInputSinkListener::SleepTimeMs()
{
    std::this_thread::sleep_for(std::chrono::milliseconds(READ_SLEEP_TIME_MS));
}

void DistributedInputSinkManager::DInputSinkListener::CheckKeyState(const int32_t &sessionId,
    const std::string &strDhids)
{
    std::set<std::string> setStr;
    StringSplitToSet(strDhids, INPUT_STRING_SPLIT_POINT, setStr);
    std::string mouseNodePath;
    std::string dhid;
    std::vector<std::string> vecStr;
    vecStr.assign(setStr.begin(), setStr.end());
    DistributedInputCollector::GetInstance().GetMouseNodePath(vecStr, mouseNodePath, dhid);
    if (mouseNodePath.empty()) {
        DHLOGE("mouse Node Path is empty.");
        return;
    }

    int fd = open(mouseNodePath.c_str(), O_RDONLY | O_NONBLOCK);
    if (fd < 0) {
        DHLOGE("open mouse Node Path error:", errno);
        return;
    }
    DHLOGI("mouse Node Path fd: %d", fd);

    uint32_t count = 0;
    int rc = 0;
    int keyVal = 0;
    unsigned long keystate[NLONGS(KEY_CNT)] = { 0 };
    while (true) {
        if (count > READ_RETRY_MAX) {
            break;
        }
        // Query all key state
        rc = ioctl(fd, EVIOCGKEY(sizeof(keystate)), keystate);
        if (rc < 0) {
            DHLOGE("read all key state failed, rc: ", rc);
            SleepTimeMs();
            continue;
        }

        keyVal = bit_is_set(keystate, BTN_LEFT);
        if (keyVal != 0) {
            DHLOGI("mouse left button is down.");
            nlohmann::json jsonStr;
            jsonStr[DINPUT_SOFTBUS_KEY_CMD_TYPE] = TRANS_SINK_MSG_KEY_STATE;
            jsonStr[DINPUT_SOFTBUS_KEY_KEYSTATE_DHID] = dhid;
            jsonStr[DINPUT_SOFTBUS_KEY_KEYSTATE_TYPE] = EV_KEY;
            jsonStr[DINPUT_SOFTBUS_KEY_KEYSTATE_CODE] = BTN_LEFT;
            jsonStr[DINPUT_SOFTBUS_KEY_KEYSTATE_VALUE] = KEY_DOWN_STATE;
            std::string smsg = jsonStr.dump();
            DistributedInputSinkTransport::GetInstance().SendKeyStateNodeMsg(sessionId, smsg);
            break;
        }
        SleepTimeMs();
        count += 1;
    }
    if (fd > 0) {
        close(fd);
    }
}

bool DistributedInputSinkManager::IsDeleteDhidExist(int32_t sessionId, const std::string &delstr)
{
    for (auto sessionDhid : sharingDhIdsMap_) {
        if (sessionDhid.first == sessionId) {
            DHLOGW("IsDeleteDhidExist sessionId=%d is self, ignore.", sessionId);
            continue;
        }
        for (auto dhid : sessionDhid.second) {
            if (delstr == dhid) {
                DHLOGI("IsDeleteDhidExist delstr=%s is find.", delstr.c_str());
                return true;
            }
        }
    }
    DHLOGW("IsDeleteDhidExist delstr=%s is not find.", delstr.c_str());
    return false;
}

void DistributedInputSinkManager::DeleteStopDhids(int32_t sessionId, const std::set<std::string> delDhIds,
    std::vector<std::string> &stopDhIds)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (sharingDhIdsMap_.count(sessionId) <= 0) {
        DHLOGE("DeleteStopDhids sessionId:%d is not exist.", sessionId);
        return;
    }
    DHLOGI("DeleteStopDhids sessionId=%d before has dhid.size=%d, delDhIds.size=%d.", sessionId,
        sharingDhIdsMap_[sessionId].size(), delDhIds.size());
    for (auto delstr : delDhIds) {
        sharingDhIdsMap_[sessionId].erase(delstr);
    }
    if (sharingDhIdsMap_[sessionId].size() == 0) {
        sharingDhIdsMap_.erase(sessionId);
        DHLOGI("DeleteStopDhids sessionId=%d is delete.", sessionId);
    } else {
        DHLOGI("DeleteStopDhids sessionId=%d after has dhid.size=%d.", sessionId, sharingDhIdsMap_[sessionId].size());
    }
    // find which dhid can be stop
    bool isFind = false;
    for (auto tmp : delDhIds) {
        isFind = IsDeleteDhidExist(sessionId, tmp);
        if (!isFind) {
            stopDhIds.push_back(tmp);
            sharingDhIds_.erase(tmp);
        }
    }
}

void DistributedInputSinkManager::StoreStartDhids(int32_t sessionId, const std::set<std::string> &dhIds)
{
    std::set<std::string> tmpDhids;
    std::lock_guard<std::mutex> lock(mutex_);
    if (sharingDhIdsMap_.count(sessionId) > 0) {
        tmpDhids = sharingDhIdsMap_[sessionId];
    }
    DHLOGI("StoreStartDhids start tmpDhids.size=%d, add dhIds.size=%d.", tmpDhids.size(), dhIds.size());
    for (auto iter : dhIds) {
        tmpDhids.insert(iter);
        sharingDhIds_.insert(iter);
    }
    sharingDhIdsMap_[sessionId] = tmpDhids;
    DHLOGI("StoreStartDhids end tmpDhids.size=%d", tmpDhids.size());
}

void DistributedInputSinkManager::OnStart()
{
    if (serviceRunningState_ == ServiceSinkRunningState::STATE_RUNNING) {
        DHLOGI("dinput Manager Service has already started.");
        return;
    }
    DHLOGI("dinput Manager Service started.");
    if (!InitAuto()) {
        DHLOGI("failed to init service.");
        return;
    }
    serviceRunningState_ = ServiceSinkRunningState::STATE_RUNNING;
    runner_->Run();

    /*
	 * Publish service maybe failed, so we need call this function at the last,
     * so it can't affect the TDD test program.
     */
    bool ret = Publish(this);
    if (!ret) {
        return;
    }

    DHLOGI("DistributedInputSinkManager  start success.");
}

bool DistributedInputSinkManager::InitAuto()
{
    runner_ = AppExecFwk::EventRunner::Create(true);
    if (runner_ == nullptr) {
        return false;
    }

    handler_ = std::make_shared<DistributedInputSinkEventHandler>(runner_);
    DHLOGI("init success");
    return true;
}

void DistributedInputSinkManager::OnStop()
{
    DHLOGI("stop service");
    runner_.reset();
    handler_.reset();
    serviceRunningState_ = ServiceSinkRunningState::STATE_NOT_START;
}

/*
 * get event handler
 *
 * @return event handler object.
 */
std::shared_ptr<DistributedInputSinkEventHandler> DistributedInputSinkManager::GetEventHandler()
{
    return handler_;
}

int32_t DistributedInputSinkManager::Init()
{
    DHLOGI("enter");
    isStartTrans_ = DInputServerType::NULL_SERVER_TYPE;
    // transport init session
    int32_t ret = DistributedInputSinkTransport::GetInstance().Init();
    if (ret != DH_SUCCESS) {
        return ERR_DH_INPUT_SERVER_SINK_MANAGER_INIT_FAIL;
    }

    statuslistener_ = std::make_shared<DInputSinkListener>(this);
    DistributedInputSinkTransport::GetInstance().RegistSinkRespCallback(statuslistener_);

    serviceRunningState_ = ServiceSinkRunningState::STATE_RUNNING;

    std::shared_ptr<DistributedHardwareFwkKit> dhFwkKit = DInputContext::GetInstance().GetDHFwkKit();
    if (dhFwkKit == nullptr) {
        DHLOGE("dhFwkKit obtain fail!");
        return ERR_DH_INPUT_SERVER_SINK_MANAGER_INIT_FAIL;
    }
    projectWindowListener_ = new ProjectWindowListener;
    dhFwkKit->RegisterPublisherListener(DHTopic::TOPIC_SINK_PROJECT_WINDOW_INFO, projectWindowListener_);
    return DH_SUCCESS;
}

int32_t DistributedInputSinkManager::Release()
{
    DHLOGI("exit");

    // 1.stop all session switch
    DistributedInputSinkSwitch::GetInstance().StopAllSwitch();
    // 2.close all session
    DistributedInputSinkTransport::GetInstance().CloseAllSession();

    {
        std::lock_guard<std::mutex> lock(mutex_);
        sharingDhIds_.clear();
        sharingDhIdsMap_.clear();
    }

    // 3.notify callback servertype
    SetStartTransFlag(DInputServerType::NULL_SERVER_TYPE);
    // 4.Release input collect resource
    DistributedInputCollector::GetInstance().Release();

    serviceRunningState_ = ServiceSinkRunningState::STATE_NOT_START;
    std::shared_ptr<DistributedHardwareFwkKit> dhFwkKit = DInputContext::GetInstance().GetDHFwkKit();
    if (dhFwkKit != nullptr && projectWindowListener_ != nullptr) {
        DHLOGI("UnPublish ProjectWindowListener");
        dhFwkKit->UnregisterPublisherListener(DHTopic::TOPIC_SINK_PROJECT_WINDOW_INFO, projectWindowListener_);
    }
    DHLOGI("exit dinput sink sa.");
    SetSinkProcessExit();

    return DH_SUCCESS;
}

DInputServerType DistributedInputSinkManager::GetStartTransFlag()
{
    return isStartTrans_;
}
void DistributedInputSinkManager::SetStartTransFlag(const DInputServerType flag)
{
    DHLOGI("Set Sink isStartTrans_ %d", (int32_t)flag);
    isStartTrans_ = flag;
}

uint32_t DistributedInputSinkManager::GetInputTypes()
{
    return static_cast<uint32_t>(inputTypes_);
}

void DistributedInputSinkManager::SetInputTypes(const uint32_t& inputTypes)
{
    inputTypes_ = static_cast<DInputDeviceType>(inputTypes);
}

DistributedInputSinkManager::ProjectWindowListener::ProjectWindowListener()
{
    DHLOGI("ProjectWindowListener ctor!");
    std::lock_guard<std::mutex> lock(handleScreenMutex_);
    if (screen_ == nullptr) {
        std::vector<sptr<Rosen::Screen>> screens = Rosen::ScreenManager::GetInstance().GetAllScreens();
        screen_ = screens[SCREEN_ID_DEFAULT];
    }
}

DistributedInputSinkManager::ProjectWindowListener::~ProjectWindowListener()
{
    DHLOGI("ProjectWindowListener dtor!");
    std::lock_guard<std::mutex> lock(handleScreenMutex_);
    screen_ = nullptr;
}

void DistributedInputSinkManager::ProjectWindowListener::OnMessage(const DHTopic topic, const std::string& message)
{
    DHLOGI("ProjectWindowListener OnMessage!");
    if (topic != DHTopic::TOPIC_SINK_PROJECT_WINDOW_INFO) {
        DHLOGE("this topic is wrong, %d", static_cast<uint32_t>(topic));
        return;
    }
    std::string srcDeviceId = "";
    uint64_t srcWinId = 0;
    SinkScreenInfo sinkScreenInfo = {};
    int32_t parseRes = ParseMessage(message, srcDeviceId, srcWinId, sinkScreenInfo);
    if (parseRes != DH_SUCCESS) {
        DHLOGE("message parse failed!");
        return;
    }
    int32_t saveRes = UpdateSinkScreenInfoCache(srcDeviceId, srcWinId, sinkScreenInfo);
    if (saveRes != DH_SUCCESS) {
        DHLOGE("Save sink screen info failed!");
        return;
    }
    sptr<IRemoteObject> dScreenSinkSA = DInputContext::GetInstance().GetRemoteObject(
        DISTRIBUTED_HARDWARE_SCREEN_SINK_SA_ID);
    sptr<DScreenSinkSvrRecipient> dScreenSinkDeathRecipient = new(std::nothrow) DScreenSinkSvrRecipient(srcDeviceId,
        srcWinId);
    dScreenSinkSA->AddDeathRecipient(dScreenSinkDeathRecipient);
    DInputContext::GetInstance().AddRemoteObject(DISTRIBUTED_HARDWARE_SCREEN_SINK_SA_ID, dScreenSinkSA);
}

int32_t DistributedInputSinkManager::ProjectWindowListener::ParseMessage(const std::string& message,
    std::string& srcDeviceId, uint64_t& srcWinId, SinkScreenInfo& sinkScreenInfo)
{
    nlohmann::json jsonObj = nlohmann::json::parse(message, nullptr, false);
    if (jsonObj.is_discarded()) {
        DHLOGE("jsonObj parse failed!");
        return ERR_DH_INPUT_JSON_PARSE_FAIL;
    }
    if (!IsString(jsonObj, SOURCE_DEVICE_ID)) {
        DHLOGE("sourceDevId key is invalid");
        return ERR_DH_INPUT_JSON_PARSE_FAIL;
    }
    srcDeviceId = jsonObj[SOURCE_DEVICE_ID].get<std::string>();
    if (!IsUint64(jsonObj, SOURCE_WINDOW_ID)) {
        DHLOGE("sourceWinId key is invalid");
        return ERR_DH_INPUT_JSON_PARSE_FAIL;
    }
    srcWinId = jsonObj[SOURCE_WINDOW_ID].get<uint64_t>();
    if (!IsUint64(jsonObj, SINK_SHOW_WINDOW_ID)) {
        DHLOGE("sinkWinId key is invalid");
        return ERR_DH_INPUT_JSON_PARSE_FAIL;
    }
    sinkScreenInfo.sinkShowWinId = jsonObj[SINK_SHOW_WINDOW_ID].get<uint64_t>();
    if (!IsUint32(jsonObj, SINK_PROJECT_SHOW_WIDTH)) {
        DHLOGE("sourceWinHeight key is invalid");
        return ERR_DH_INPUT_JSON_PARSE_FAIL;
    }
    sinkScreenInfo.sinkProjShowWidth = jsonObj[SINK_PROJECT_SHOW_WIDTH].get<std::uint32_t>();
    if (!IsUint32(jsonObj, SINK_PROJECT_SHOW_HEIGHT)) {
        DHLOGE("sourceWinHeight key is invalid");
        return ERR_DH_INPUT_JSON_PARSE_FAIL;
    }
    sinkScreenInfo.sinkProjShowHeight = jsonObj[SINK_PROJECT_SHOW_HEIGHT].get<std::uint32_t>();
    if (!IsUint32(jsonObj, SINK_WINDOW_SHOW_X)) {
        DHLOGE("sourceWinHeight key is invalid");
        return ERR_DH_INPUT_JSON_PARSE_FAIL;
    }
    sinkScreenInfo.sinkWinShowX = jsonObj[SINK_WINDOW_SHOW_X].get<std::uint32_t>();
    if (!IsUint32(jsonObj, SINK_WINDOW_SHOW_Y)) {
        DHLOGE("sourceWinHeight key is invalid");
        return ERR_DH_INPUT_JSON_PARSE_FAIL;
    }
    sinkScreenInfo.sinkWinShowY = jsonObj[SINK_WINDOW_SHOW_Y].get<std::uint32_t>();
    return DH_SUCCESS;
}

int32_t DistributedInputSinkManager::ProjectWindowListener::UpdateSinkScreenInfoCache(const std::string& srcDevId,
    const uint64_t srcWinId, const SinkScreenInfo& sinkScreenInfoTmp)
{
    std::string srcScreenInfoKey = DInputContext::GetInstance().GetScreenInfoKey(srcDevId, srcWinId);
    SinkScreenInfo sinkScreenInfo = DInputContext::GetInstance().GetSinkScreenInfo(srcScreenInfoKey);
    sinkScreenInfo.sinkShowWinId = sinkScreenInfoTmp.sinkShowWinId;
    sinkScreenInfo.sinkProjShowWidth = sinkScreenInfoTmp.sinkProjShowWidth;
    sinkScreenInfo.sinkProjShowHeight = sinkScreenInfoTmp.sinkProjShowHeight;
    sinkScreenInfo.sinkWinShowX = sinkScreenInfoTmp.sinkWinShowX;
    sinkScreenInfo.sinkWinShowY = sinkScreenInfoTmp.sinkWinShowY;
    sinkScreenInfo.sinkShowWidth = GetScreenWidth();
    sinkScreenInfo.sinkShowHeight = GetScreenHeight();
    LocalAbsInfo info = DInputContext::GetInstance().GetLocalTouchScreenInfo().localAbsInfo;
    sinkScreenInfo.sinkPhyWidth = (uint32_t)(info.absMtPositionXMax + 1);
    sinkScreenInfo.sinkPhyHeight = (uint32_t)(info.absMtPositionYMax + 1);
    DHLOGI("sinkShowWinId: %d, sinkProjShowWidth: %d, sinkProjShowHeight: %d, sinkWinShowX: %d, sinkWinShowY: %d,"
        "sinkShowWidth: %d, sinkShowHeight: %d, sinkPhyWidth: %d, sinkPhyHeight: %d", sinkScreenInfo.sinkShowWinId,
        sinkScreenInfo.sinkProjShowWidth, sinkScreenInfo.sinkProjShowHeight, sinkScreenInfo.sinkWinShowX,
        sinkScreenInfo.sinkWinShowY, sinkScreenInfo.sinkShowWidth, sinkScreenInfo.sinkShowHeight,
        sinkScreenInfo.sinkPhyWidth, sinkScreenInfo.sinkPhyHeight);
    return DInputContext::GetInstance().UpdateSinkScreenInfo(srcScreenInfoKey, sinkScreenInfo);
}

uint32_t DistributedInputSinkManager::ProjectWindowListener::GetScreenWidth()
{
    std::lock_guard<std::mutex> lock(handleScreenMutex_);
    if (screen_ == nullptr) {
        DHLOGE("screen is nullptr!");
        return DEFAULT_VALUE;
    }
    return screen_->GetWidth();
}

uint32_t DistributedInputSinkManager::ProjectWindowListener::GetScreenHeight()
{
    std::lock_guard<std::mutex> lock(handleScreenMutex_);
    if (screen_ == nullptr) {
        DHLOGE("screen is nullptr!");
        return DEFAULT_VALUE;
    }
    return screen_->GetHeight();
}

DistributedInputSinkManager::DScreenSinkSvrRecipient::DScreenSinkSvrRecipient(const std::string& srcDevId,
    const uint64_t srcWinId)
{
    DHLOGI("DScreenStatusListener ctor!");
    this->srcDevId_ = srcDevId;
    this->srcWinId_ = srcWinId;
}

DistributedInputSinkManager::DScreenSinkSvrRecipient::~DScreenSinkSvrRecipient()
{
    DHLOGI("DScreenStatusListener dtor!");
}

void DistributedInputSinkManager::DScreenSinkSvrRecipient::OnRemoteDied(const wptr<IRemoteObject> &remote)
{
    DHLOGI("DScreenSinkSvrRecipient OnRemoteDied");
    sptr<IRemoteObject> remoteObject = remote.promote();
    if (!remoteObject) {
        DHLOGE("OnRemoteDied remote promoted failed");
        return;
    }
    std::string srcScreenInfoKey = DInputContext::GetInstance().GetScreenInfoKey(srcDevId_, srcWinId_);
    DInputContext::GetInstance().RemoveSinkScreenInfo(srcScreenInfoKey);
    DInputContext::GetInstance().RemoveRemoteObject(DISTRIBUTED_HARDWARE_SCREEN_SINK_SA_ID);
}

int32_t DistributedInputSinkManager::NotifyStartDScreen(const SrcScreenInfo& srcScreenInfo)
{
    DHLOGI("NotifyStartDScreen start!");

    CleanExceptionalInfo(srcScreenInfo);

    std::string screenInfoKey = DInputContext::GetInstance().GetScreenInfoKey(srcScreenInfo.devId,
        srcScreenInfo.sourceWinId);
    SinkScreenInfo sinkScreenInfo = DInputContext::GetInstance().GetSinkScreenInfo(screenInfoKey);
    sinkScreenInfo.srcScreenInfo = srcScreenInfo;
    DHLOGI("OnRemoteRequest the data: devId: %s, sourceWinId: %d, sourceWinWidth: %d, sourceWinHeight: %d,"
        "sourcePhyId: %s, sourcePhyFd: %d, sourcePhyWidth: %d, sourcePhyHeight: %d",
        GetAnonyString(srcScreenInfo.devId).c_str(), srcScreenInfo.sourceWinId, srcScreenInfo.sourceWinWidth,
        srcScreenInfo.sourceWinHeight, GetAnonyString(srcScreenInfo.sourcePhyId).c_str(), srcScreenInfo.sourcePhyFd,
        srcScreenInfo.sourcePhyWidth, srcScreenInfo.sourcePhyHeight);
    return DInputContext::GetInstance().UpdateSinkScreenInfo(screenInfoKey, sinkScreenInfo);
}

void DistributedInputSinkManager::CleanExceptionalInfo(const SrcScreenInfo& srcScreenInfo)
{
    DHLOGI("CleanExceptionalInfo start!");
    std::string uuid = srcScreenInfo.uuid;
    int32_t sessionId = srcScreenInfo.sessionId;
    auto sinkInfos = DInputContext::GetInstance().GetAllSinkScreenInfo();

    for (const auto& [id, sinkInfo] : sinkInfos) {
        auto srcInfo = sinkInfo.srcScreenInfo;
        if ((std::strcmp(srcInfo.uuid.c_str(), uuid.c_str()) == 0) && (srcInfo.sessionId != sessionId)) {
            DInputContext::GetInstance().RemoveSinkScreenInfo(id);
            DHLOGI("CleanExceptionalInfo screenInfoKey: %s, sessionId: %d", id.c_str(), sessionId);
        }
    }
}

int32_t DistributedInputSinkManager::NotifyStopDScreen(const std::string& srcScreenInfoKey)
{
    DHLOGI("NotifyStopDScreen start, srcScreenInfoKey: %s", GetAnonyString(srcScreenInfoKey).c_str());
    if (srcScreenInfoKey.empty()) {
        DHLOGE("srcScreenInfoKey is empty, srcScreenInfoKey: %s", GetAnonyString(srcScreenInfoKey).c_str());
        return ERR_DH_INPUT_SERVER_SINK_SCREEN_INFO_IS_EMPTY;
    }
    return DInputContext::GetInstance().RemoveSinkScreenInfo(srcScreenInfoKey);
}

int32_t DistributedInputSinkManager::RegisterSharingDhIdListener(sptr<ISharingDhIdListener> sharingDhIdListener)
{
    DHLOGI("RegisterSharingDhIdListener");
    DistributedInputCollector::GetInstance().RegisterSharingDhIdListener(sharingDhIdListener);
    return DH_SUCCESS;
}

int32_t DistributedInputSinkManager::Dump(int32_t fd, const std::vector<std::u16string>& args)
{
    DHLOGI("DistributedInputSinkManager Dump.");
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
