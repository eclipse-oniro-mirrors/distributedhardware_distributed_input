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

#include "distributed_input_sinktrans_test.h"

#include "nlohmann/json.hpp"
#include "dinput_errcode.h"
#include "distributed_input_sink_manager.h"

using namespace testing::ext;
using namespace OHOS::DistributedHardware::DistributedInput;
using namespace std;
namespace OHOS {
namespace DistributedHardware {
namespace DistributedInput {
namespace {
    const int32_t MESSAGE_MAX_SIZE = 46 * 1024;
    const int32_t SESSIONID = 1;
    const int32_t SRCTSRCSEID = 1;
    const std::string DEVID = "umkyu1b165e1be98151891erbe8r91ev";
    const std::string DHID = "Input_1ds56v18e1v21v8v1erv15r1v8r1j1ty8";
    const std::string SINKID = "networkidc08647073e02e7a78f09473aa122ff57fc81c00";
    const uint32_t INPUTTYPE = static_cast<uint32_t>(DInputDeviceType::ALL);
}

void DistributedInputSinkTransTest::SetUp()
{
    std::string cmdType = "cmdType_test";
    jsonStr_[DINPUT_SOFTBUS_KEY_CMD_TYPE] = cmdType;

    jsonStr1_[DINPUT_SOFTBUS_KEY_CMD_TYPE] = TRANS_SOURCE_MSG_PREPARE;
    jsonStr1_[DINPUT_SOFTBUS_KEY_DEVICE_ID] = DEVID;
    jsonStr1_[DINPUT_SOFTBUS_KEY_SESSION_ID] = SESSIONID;

    jsonStr2_[DINPUT_SOFTBUS_KEY_CMD_TYPE] = TRANS_SOURCE_MSG_UNPREPARE;
    jsonStr2_[DINPUT_SOFTBUS_KEY_DEVICE_ID] = DEVID;
    jsonStr2_[DINPUT_SOFTBUS_KEY_SESSION_ID] = SESSIONID;

    jsonStr3_[DINPUT_SOFTBUS_KEY_CMD_TYPE] = TRANS_SOURCE_MSG_PREPARE_FOR_REL;
    jsonStr3_[DINPUT_SOFTBUS_KEY_DEVICE_ID] = DEVID;
    jsonStr3_[DINPUT_SOFTBUS_KEY_SESSION_ID] = SRCTSRCSEID;

    jsonStr4_[DINPUT_SOFTBUS_KEY_CMD_TYPE] = TRANS_SOURCE_MSG_UNPREPARE_FOR_REL;
    jsonStr4_[DINPUT_SOFTBUS_KEY_DEVICE_ID] = DEVID;
    jsonStr4_[DINPUT_SOFTBUS_KEY_SESSION_ID] = SRCTSRCSEID;

    jsonStr5_[DINPUT_SOFTBUS_KEY_CMD_TYPE] = TRANS_SOURCE_MSG_START_DHID_FOR_REL;
    jsonStr5_[DINPUT_SOFTBUS_KEY_DEVICE_ID] = DEVID;
    jsonStr5_[DINPUT_SOFTBUS_KEY_SESSION_ID] = SRCTSRCSEID;
    jsonStr5_[DINPUT_SOFTBUS_KEY_VECTOR_DHID] = DHID;

    jsonStr6_[DINPUT_SOFTBUS_KEY_CMD_TYPE] = TRANS_SOURCE_MSG_STOP_DHID_FOR_REL;
    jsonStr6_[DINPUT_SOFTBUS_KEY_DEVICE_ID] = DEVID;
    jsonStr6_[DINPUT_SOFTBUS_KEY_SESSION_ID] = SRCTSRCSEID;
    jsonStr6_[DINPUT_SOFTBUS_KEY_VECTOR_DHID] = DHID;

    jsonStr7_[DINPUT_SOFTBUS_KEY_CMD_TYPE] = TRANS_SOURCE_MSG_START_TYPE_FOR_REL;
    jsonStr7_[DINPUT_SOFTBUS_KEY_DEVICE_ID] = DEVID;
    jsonStr7_[DINPUT_SOFTBUS_KEY_SESSION_ID] = SRCTSRCSEID;
    jsonStr7_[DINPUT_SOFTBUS_KEY_INPUT_TYPE] = INPUTTYPE;

    jsonStr8_[DINPUT_SOFTBUS_KEY_CMD_TYPE] = TRANS_SOURCE_MSG_STOP_TYPE_FOR_REL;
    jsonStr8_[DINPUT_SOFTBUS_KEY_DEVICE_ID] = DEVID;
    jsonStr8_[DINPUT_SOFTBUS_KEY_SESSION_ID] = SRCTSRCSEID;
    jsonStr8_[DINPUT_SOFTBUS_KEY_INPUT_TYPE] = INPUTTYPE;

    jsonStr9_[DINPUT_SOFTBUS_KEY_CMD_TYPE] = TRANS_SOURCE_TO_SOURCE_MSG_PREPARE;
    jsonStr9_[DINPUT_SOFTBUS_KEY_DEVICE_ID] = SINKID;

    jsonStr10_[DINPUT_SOFTBUS_KEY_CMD_TYPE] = TRANS_SOURCE_TO_SOURCE_MSG_UNPREPARE;
    jsonStr10_[DINPUT_SOFTBUS_KEY_DEVICE_ID] = SINKID;

    jsonStr11_[DINPUT_SOFTBUS_KEY_CMD_TYPE] = TRANS_SOURCE_MSG_START_TYPE;
    jsonStr11_[DINPUT_SOFTBUS_KEY_DEVICE_ID] = DEVID;
    jsonStr11_[DINPUT_SOFTBUS_KEY_SESSION_ID] = SESSIONID;
    jsonStr11_[DINPUT_SOFTBUS_KEY_INPUT_TYPE] = INPUTTYPE;

    jsonStr12_[DINPUT_SOFTBUS_KEY_CMD_TYPE] = TRANS_SOURCE_MSG_STOP_TYPE;
    jsonStr12_[DINPUT_SOFTBUS_KEY_DEVICE_ID] = DEVID;
    jsonStr12_[DINPUT_SOFTBUS_KEY_SESSION_ID] = SESSIONID;
    jsonStr12_[DINPUT_SOFTBUS_KEY_INPUT_TYPE] = INPUTTYPE;

    jsonStr13_[DINPUT_SOFTBUS_KEY_CMD_TYPE] = TRANS_SOURCE_MSG_LATENCY;
    jsonStr13_[DINPUT_SOFTBUS_KEY_DEVICE_ID] = DEVID;
    jsonStr13_[DINPUT_SOFTBUS_KEY_SESSION_ID] = SESSIONID;
}

void DistributedInputSinkTransTest::TearDown()
{
}

void DistributedInputSinkTransTest::SetUpTestCase()
{
}

void DistributedInputSinkTransTest::TearDownTestCase()
{
}

HWTEST_F(DistributedInputSinkTransTest, Init, testing::ext::TestSize.Level0)
{
    int32_t ret = DistributedInputSinkTransport::GetInstance().Init();
    EXPECT_EQ(DH_SUCCESS, ret);
}

HWTEST_F(DistributedInputSinkTransTest, GetSessionIdByNetId_001, testing::ext::TestSize.Level0)
{
    std::string srcId = "";
    int32_t ret = DistributedInputSinkTransport::GetInstance().GetSessionIdByNetId(srcId);
    EXPECT_EQ(ERR_DH_INPUT_SERVER_SINK_TRANSPORT_GET_SESSIONID_FAIL, ret);
}

HWTEST_F(DistributedInputSinkTransTest, GetSessionIdByNetId_002, testing::ext::TestSize.Level0)
{
    std::string srcId = "umkyu1b165e1be98151891erbe8r91ev";
    DistributedInputSinkTransport::GetInstance().sessionDevMap_[srcId] = 1;
    DistributedInputSinkTransport::GetInstance().GetDeviceIdBySessionId(1, srcId);
    int32_t ret = DistributedInputSinkTransport::GetInstance().GetSessionIdByNetId(srcId);
    EXPECT_EQ(1, ret);
    DistributedInputSinkTransport::GetInstance().GetDeviceIdBySessionId(100, srcId);
}

HWTEST_F(DistributedInputSinkTransTest, RespPrepareRemoteInput01, testing::ext::TestSize.Level1)
{
    int32_t sessionId = -1;
    std::string smsg = "";
    int32_t ret = DistributedInputSinkTransport::GetInstance().RespPrepareRemoteInput(sessionId, smsg);
    EXPECT_EQ(ERR_DH_INPUT_SERVER_SINK_TRANSPORT_RESPPREPARE_FAIL, ret);
}

HWTEST_F(DistributedInputSinkTransTest, RespPrepareRemoteInput02, testing::ext::TestSize.Level1)
{
    int32_t sessionId = 0;
    std::string smsg = "";
    int32_t ret = DistributedInputSinkTransport::GetInstance().RespPrepareRemoteInput(sessionId, smsg);
    EXPECT_EQ(ERR_DH_INPUT_SERVER_SINK_TRANSPORT_RESPPREPARE_FAIL, ret);
}

HWTEST_F(DistributedInputSinkTransTest, RespPrepareRemoteInput03, testing::ext::TestSize.Level1)
{
    DistributedInputSinkManager sinkMgr(4810, false);
    std::shared_ptr<DistributedInputSinkManager::DInputSinkListener> statuslistener =
        std::make_shared<DistributedInputSinkManager::DInputSinkListener>(&sinkMgr);
    DistributedInputSinkTransport::GetInstance().callback_ = statuslistener;
    std::string message = "";
    DistributedInputSinkTransport::GetInstance().HandleSessionData(1, message);
    message = "message_test";
    DistributedInputSinkTransport::GetInstance().HandleSessionData(1, message);
    DistributedInputSinkTransport::GetInstance().HandleSessionData(1, jsonStr_.dump());
    DistributedInputSinkTransport::GetInstance().HandleSessionData(1, jsonStr1_.dump());
    DistributedInputSinkTransport::GetInstance().HandleSessionData(1, jsonStr3_.dump());
    int32_t sessionId = 1;
    std::string smsg = "";
    int32_t ret = DistributedInputSinkTransport::GetInstance().RespPrepareRemoteInput(sessionId, smsg);
    EXPECT_EQ(DH_SUCCESS, ret);
}

HWTEST_F(DistributedInputSinkTransTest, RespPrepareRemoteInput04, testing::ext::TestSize.Level1)
{
    int32_t sessionId = 1;
    std::string smsg(MESSAGE_MAX_SIZE, 'a');
    int32_t ret = DistributedInputSinkTransport::GetInstance().RespPrepareRemoteInput(sessionId, smsg);
    EXPECT_EQ(ERR_DH_INPUT_SERVER_SINK_TRANSPORT_RESPPREPARE_FAIL, ret);
}

HWTEST_F(DistributedInputSinkTransTest, RespUnprepareRemoteInput01, testing::ext::TestSize.Level0)
{
    int32_t sessionId = -1;
    std::string smsg = "";
    int32_t ret = DistributedInputSinkTransport::GetInstance().RespUnprepareRemoteInput(sessionId, smsg);
    EXPECT_EQ(ERR_DH_INPUT_SERVER_SINK_TRANSPORT_RESPUNPREPARE_FAIL, ret);
}

HWTEST_F(DistributedInputSinkTransTest, RespUnprepareRemoteInput02, testing::ext::TestSize.Level1)
{
    int32_t sessionId = 0;
    std::string smsg = "";
    int32_t ret = DistributedInputSinkTransport::GetInstance().RespUnprepareRemoteInput(sessionId, smsg);
    EXPECT_EQ(ERR_DH_INPUT_SERVER_SINK_TRANSPORT_RESPUNPREPARE_FAIL, ret);
}

HWTEST_F(DistributedInputSinkTransTest, RespUnprepareRemoteInput03, testing::ext::TestSize.Level1)
{
    DistributedInputSinkManager sinkMgr(4810, false);
    std::shared_ptr<DistributedInputSinkManager::DInputSinkListener> statuslistener =
        std::make_shared<DistributedInputSinkManager::DInputSinkListener>(&sinkMgr);
    DistributedInputSinkTransport::GetInstance().callback_ = statuslistener;
    DistributedInputSinkTransport::GetInstance().HandleSessionData(1, jsonStr2_.dump());
    DistributedInputSinkTransport::GetInstance().HandleSessionData(1, jsonStr4_.dump());
    int32_t sessionId = 1;
    std::string smsg = "";
    int32_t ret = DistributedInputSinkTransport::GetInstance().RespUnprepareRemoteInput(sessionId, smsg);
    EXPECT_EQ(DH_SUCCESS, ret);
}

HWTEST_F(DistributedInputSinkTransTest, RespUnprepareRemoteInput04, testing::ext::TestSize.Level1)
{
    int32_t sessionId = 1;
    std::string smsg(MESSAGE_MAX_SIZE, 'a');
    int32_t ret = DistributedInputSinkTransport::GetInstance().RespUnprepareRemoteInput(sessionId, smsg);
    EXPECT_EQ(ERR_DH_INPUT_SERVER_SINK_TRANSPORT_RESPUNPREPARE_FAIL, ret);
}

HWTEST_F(DistributedInputSinkTransTest, RespStartRemoteInput01, testing::ext::TestSize.Level1)
{
    int32_t sessionId = -1;
    std::string smsg = "";
    int32_t ret = DistributedInputSinkTransport::GetInstance().RespStartRemoteInput(sessionId, smsg);
    EXPECT_EQ(ERR_DH_INPUT_SERVER_SINK_TRANSPORT_RESPSTART_FAIL, ret);
}

HWTEST_F(DistributedInputSinkTransTest, RespStartRemoteInput02, testing::ext::TestSize.Level1)
{
    int32_t sessionId = 0;
    std::string smsg = "";
    int32_t ret = DistributedInputSinkTransport::GetInstance().RespStartRemoteInput(sessionId, smsg);
    EXPECT_EQ(ERR_DH_INPUT_SERVER_SINK_TRANSPORT_RESPSTART_FAIL, ret);
}

HWTEST_F(DistributedInputSinkTransTest, RespStartRemoteInput03, testing::ext::TestSize.Level1)
{
    DistributedInputSinkManager sinkMgr(4810, false);
    std::shared_ptr<DistributedInputSinkManager::DInputSinkListener> statuslistener =
        std::make_shared<DistributedInputSinkManager::DInputSinkListener>(&sinkMgr);
    DistributedInputSinkTransport::GetInstance().callback_ = statuslistener;
    DistributedInputSinkTransport::GetInstance().HandleSessionData(1, jsonStr5_.dump());
    DistributedInputSinkTransport::GetInstance().HandleSessionData(1, jsonStr7_.dump());
    DistributedInputSinkTransport::GetInstance().HandleSessionData(1, jsonStr11_.dump());
    int32_t sessionId = 1;
    std::string smsg = "";
    int32_t ret = DistributedInputSinkTransport::GetInstance().RespStartRemoteInput(sessionId, smsg);
    EXPECT_EQ(DH_SUCCESS, ret);
}

HWTEST_F(DistributedInputSinkTransTest, RespStartRemoteInput04, testing::ext::TestSize.Level1)
{
    int32_t sessionId = 1;
    std::string smsg(MESSAGE_MAX_SIZE, 'a');
    int32_t ret = DistributedInputSinkTransport::GetInstance().RespStartRemoteInput(sessionId, smsg);
    EXPECT_EQ(ERR_DH_INPUT_SERVER_SINK_TRANSPORT_RESPSTART_FAIL, ret);
}

HWTEST_F(DistributedInputSinkTransTest, RespStopRemoteInput01, testing::ext::TestSize.Level1)
{
    int32_t sessionId = -1;
    std::string smsg = "";
    int32_t ret = DistributedInputSinkTransport::GetInstance().RespStopRemoteInput(sessionId, smsg);
    EXPECT_EQ(ERR_DH_INPUT_SERVER_SINK_TRANSPORT_RESPSTOP_FAIL, ret);
}

HWTEST_F(DistributedInputSinkTransTest, RespStopRemoteInput02, testing::ext::TestSize.Level1)
{
    int32_t sessionId = 0;
    std::string smsg = "";
    int32_t ret = DistributedInputSinkTransport::GetInstance().RespStopRemoteInput(sessionId, smsg);
    EXPECT_EQ(ERR_DH_INPUT_SERVER_SINK_TRANSPORT_RESPSTOP_FAIL, ret);
}

HWTEST_F(DistributedInputSinkTransTest, RespStopRemoteInput03, testing::ext::TestSize.Level1)
{
    DistributedInputSinkManager sinkMgr(4810, false);
    std::shared_ptr<DistributedInputSinkManager::DInputSinkListener> statuslistener =
        std::make_shared<DistributedInputSinkManager::DInputSinkListener>(&sinkMgr);
    DistributedInputSinkTransport::GetInstance().callback_ = statuslistener;
    DistributedInputSinkTransport::GetInstance().HandleSessionData(1, jsonStr6_.dump());
    DistributedInputSinkTransport::GetInstance().HandleSessionData(1, jsonStr8_.dump());
    DistributedInputSinkTransport::GetInstance().HandleSessionData(1, jsonStr12_.dump());
    int32_t sessionId = 1;
    std::string smsg = "";
    int32_t ret = DistributedInputSinkTransport::GetInstance().RespStopRemoteInput(sessionId, smsg);
    EXPECT_EQ(DH_SUCCESS, ret);
}

HWTEST_F(DistributedInputSinkTransTest, RespStopRemoteInput04, testing::ext::TestSize.Level1)
{
    int32_t sessionId = 1;
    std::string smsg(MESSAGE_MAX_SIZE, 'a');
    int32_t ret = DistributedInputSinkTransport::GetInstance().RespStopRemoteInput(sessionId, smsg);
    EXPECT_EQ(ERR_DH_INPUT_SERVER_SINK_TRANSPORT_RESPSTOP_FAIL, ret);
}

HWTEST_F(DistributedInputSinkTransTest, GetEventHandler, testing::ext::TestSize.Level1)
{
    std::shared_ptr<OHOS::AppExecFwk::EventHandler> eventHd =
        DistributedInputSinkTransport::GetInstance().GetEventHandler();
    EXPECT_NE(nullptr, eventHd);
}

HWTEST_F(DistributedInputSinkTransTest, StartSwitch01, testing::ext::TestSize.Level1)
{
    int32_t sessionId = 1000;
    DistributedInputSinkSwitch::GetInstance().AddSession(sessionId);
    int32_t ret = DistributedInputSinkSwitch::GetInstance().StartSwitch(sessionId);
    EXPECT_EQ(DH_SUCCESS, ret);
}

HWTEST_F(DistributedInputSinkTransTest, StartSwitch02, testing::ext::TestSize.Level1)
{
    int32_t sessionId = 1000;
    DistributedInputSinkSwitch::GetInstance().AddSession(sessionId);
    int32_t ret = DistributedInputSinkSwitch::GetInstance().StartSwitch(sessionId+10);
    EXPECT_EQ(ERR_DH_INPUT_SERVER_SINK_START_SWITCH_FAIL, ret);
}

HWTEST_F(DistributedInputSinkTransTest, StartSwitch03, testing::ext::TestSize.Level1)
{
    DistributedInputSinkSwitch::GetInstance().InitSwitch();
    int32_t sessionId = 1002;
    int32_t ret = DistributedInputSinkSwitch::GetInstance().StartSwitch(sessionId);
    EXPECT_EQ(ERR_DH_INPUT_SERVER_SINK_START_SWITCH_FAIL, ret);
}

HWTEST_F(DistributedInputSinkTransTest, StopSwitch01, testing::ext::TestSize.Level1)
{
    int32_t sessionId = 1000;
    SwitchStateData switchStateData {sessionId, true};
    DistributedInputSinkSwitch::GetInstance().switchVector_.clear();
    DistributedInputSinkSwitch::GetInstance().StopSwitch(sessionId);
    DistributedInputSinkSwitch::GetInstance().switchVector_.push_back(switchStateData);
    DistributedInputSinkSwitch::GetInstance().StopSwitch(sessionId);
    EXPECT_EQ(false, DistributedInputSinkSwitch::GetInstance().switchVector_[0].switchState);
    sessionId = 2000;
    DistributedInputSinkSwitch::GetInstance().StopSwitch(sessionId);
}

HWTEST_F(DistributedInputSinkTransTest, StopAllSwitch01, testing::ext::TestSize.Level1)
{
    int32_t sessionId = 1000;
    SwitchStateData switchStateData {sessionId, true};
    DistributedInputSinkSwitch::GetInstance().switchVector_.push_back(switchStateData);
    DistributedInputSinkSwitch::GetInstance().StopAllSwitch();
    EXPECT_EQ(false, DistributedInputSinkSwitch::GetInstance().switchVector_[0].switchState);
}

HWTEST_F(DistributedInputSinkTransTest, RemoveSession01, testing::ext::TestSize.Level1)
{
    DistributedInputSinkSwitch::GetInstance().switchVector_.clear();
    int32_t sessionId = 1000;
    SwitchStateData switchStateData {sessionId, true};
    DistributedInputSinkSwitch::GetInstance().RemoveSession(sessionId);
    DistributedInputSinkSwitch::GetInstance().switchVector_.push_back(switchStateData);
    DistributedInputSinkSwitch::GetInstance().RemoveSession(sessionId);
    EXPECT_EQ(0, DistributedInputSinkSwitch::GetInstance().switchVector_.size());
}

HWTEST_F(DistributedInputSinkTransTest, GetAllSessionId, testing::ext::TestSize.Level1)
{
    DistributedInputSinkSwitch::GetInstance().InitSwitch();
    int32_t sessionId = 1000;
    DistributedInputSinkSwitch::GetInstance().AddSession(sessionId);
    DistributedInputSinkSwitch::GetInstance().AddSession(sessionId + 1);
    DistributedInputSinkSwitch::GetInstance().AddSession(sessionId + 2);
    std::vector<int32_t> tmpVecSession = DistributedInputSinkSwitch::GetInstance().GetAllSessionId();
    EXPECT_EQ(3, tmpVecSession.size());
}

HWTEST_F(DistributedInputSinkTransTest, GetSwitchOpenedSession01, testing::ext::TestSize.Level1)
{
    int32_t sessionId = 1010;
    DistributedInputSinkSwitch::GetInstance().AddSession(sessionId);
    DistributedInputSinkSwitch::GetInstance().AddSession(sessionId + 1);
    DistributedInputSinkSwitch::GetInstance().AddSession(sessionId + 2);
    int32_t ret = DistributedInputSinkSwitch::GetInstance().GetSwitchOpenedSession();
    EXPECT_EQ(ERR_DH_INPUT_SERVER_SINK_GET_OPEN_SESSION_FAIL, ret);
}

HWTEST_F(DistributedInputSinkTransTest, GetSwitchOpenedSession02, testing::ext::TestSize.Level1)
{
    DistributedInputSinkSwitch::GetInstance().switchVector_.clear();
    int32_t ret = DistributedInputSinkSwitch::GetInstance().GetSwitchOpenedSession();
    EXPECT_EQ(ERR_DH_INPUT_SERVER_SINK_GET_OPEN_SESSION_FAIL, ret);
}

HWTEST_F(DistributedInputSinkTransTest, GetSwitchOpenedSession03, testing::ext::TestSize.Level1)
{
    int32_t sessionId = 1013;
    DistributedInputSinkSwitch::GetInstance().AddSession(sessionId);
    DistributedInputSinkSwitch::GetInstance().StartSwitch(sessionId);
    int32_t ret = DistributedInputSinkSwitch::GetInstance().GetSwitchOpenedSession();
    EXPECT_EQ(sessionId, ret);
}

HWTEST_F(DistributedInputSinkTransTest, RespLatency01, testing::ext::TestSize.Level1)
{
    int32_t sessionId = 0;
    std::string smsg = "";
    int32_t ret = DistributedInputSinkTransport::GetInstance().RespLatency(sessionId, smsg);
    EXPECT_EQ(ERR_DH_INPUT_SERVER_SINK_TRANSPORT_RESP_LATENCY_FAIL, ret);
}

HWTEST_F(DistributedInputSinkTransTest, RespLatency02, testing::ext::TestSize.Level1)
{
    DistributedInputSinkTransport::GetInstance().HandleSessionData(1, jsonStr13_.dump());
    int32_t sessionId = 1;
    std::string smsg = "";
    int32_t ret = DistributedInputSinkTransport::GetInstance().RespLatency(sessionId, smsg);
    EXPECT_EQ(DH_SUCCESS, ret);
}

HWTEST_F(DistributedInputSinkTransTest, RespLatency03, testing::ext::TestSize.Level1)
{
    int32_t sessionId = 1;
    std::string smsg(MESSAGE_MAX_SIZE, 'a');
    int32_t ret = DistributedInputSinkTransport::GetInstance().RespLatency(sessionId, smsg);
    EXPECT_EQ(ERR_DH_INPUT_SERVER_SINK_TRANSPORT_RESP_LATENCY_FAIL, ret);
}

HWTEST_F(DistributedInputSinkTransTest, SendMessage_001, testing::ext::TestSize.Level1)
{
    int32_t sessionId = 1;
    std::string smsg = "qwerzsdgertgdfgbrtyhuert6345634tgadsgfq13451234rfaDSFQ34FQQWEFWQERQWEFQWEFASEFQWERQWERQWER123";
    int32_t ret = DistributedInputSinkTransport::GetInstance().SendMessage(sessionId, smsg);
    EXPECT_EQ(DH_SUCCESS, ret);
}

HWTEST_F(DistributedInputSinkTransTest, OnSessionOpened_001, testing::ext::TestSize.Level1)
{
    int32_t sessionId = 0;
    int32_t result = 1;
    int32_t ret = DistributedInputSinkTransport::GetInstance().OnSessionOpened(sessionId, result);
    EXPECT_EQ(DH_SUCCESS, ret);
}

HWTEST_F(DistributedInputSinkTransTest, OnSessionOpened_002, testing::ext::TestSize.Level1)
{
    int32_t sessionId = 1;
    int32_t result = 0;
    int32_t ret = DistributedInputSinkTransport::GetInstance().OnSessionOpened(sessionId, result);
    EXPECT_EQ(DH_SUCCESS, ret);
}

HWTEST_F(DistributedInputSinkTransTest, OnSessionClosed_001, testing::ext::TestSize.Level1)
{
    int32_t sessionId = -1;
    std::string networkId = "umkyu1b165e1be98151891erbe8r91ev";
    char *data = nullptr;
    uint32_t dataLen = 100;
    std::string dhId = "dhId_test";
    uint32_t btnCode = 0;
    DistributedInputSinkTransport::GetInstance().sessionDevMap_.clear();
    DistributedInputSinkTransport::GetInstance().OnBytesReceived(sessionId, data, dataLen);
    DistributedInputSinkTransport::GetInstance().sessionDevMap_[networkId] = sessionId;
    DistributedInputSinkTransport::GetInstance().OnSessionClosed(sessionId);
    EXPECT_EQ(0, DistributedInputSinkTransport::GetInstance().sessionDevMap_.size());
    DistributedInputSinkTransport::GetInstance().SendKeyStateNodeMsg(sessionId, dhId, btnCode);
    sessionId = 0;
    DistributedInputSinkTransport::GetInstance().SendKeyStateNodeMsg(sessionId, dhId, btnCode);
}
} // namespace DistributedInput
} // namespace DistributedHardware
} // namespace OHOS