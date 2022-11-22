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

#include "dinput_errcode.h"
#include "distributed_input_sinktrans_test.h"

using namespace testing::ext;
using namespace OHOS::DistributedHardware::DistributedInput;
using namespace std;
namespace OHOS {
namespace DistributedHardware {
namespace DistributedInput {
void DistributedInputSinkTransTest::SetUp()
{
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

HWTEST_F(DistributedInputSinkTransTest, GetSessionIdByNetId, testing::ext::TestSize.Level0)
{
    std::string srcId = "";
    int32_t ret = DistributedInputSinkTransport::GetInstance().GetSessionIdByNetId(srcId);
    EXPECT_EQ(ERR_DH_INPUT_SERVER_SINK_TRANSPORT_GET_SESSIONID_FAIL, ret);
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
    int32_t sessionId = 1;
    std::string smsg = "";
    int32_t ret = DistributedInputSinkTransport::GetInstance().RespPrepareRemoteInput(sessionId, smsg);
    EXPECT_EQ(DH_SUCCESS, ret);
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
    int32_t sessionId = 1;
    std::string smsg = "";
    int32_t ret = DistributedInputSinkTransport::GetInstance().RespUnprepareRemoteInput(sessionId, smsg);
    EXPECT_EQ(DH_SUCCESS, ret);
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
    int32_t sessionId = 1;
    std::string smsg = "";
    int32_t ret = DistributedInputSinkTransport::GetInstance().RespStartRemoteInput(sessionId, smsg);
    EXPECT_EQ(DH_SUCCESS, ret);
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
    int32_t sessionId = 1;
    std::string smsg = "";
    int32_t ret = DistributedInputSinkTransport::GetInstance().RespStopRemoteInput(sessionId, smsg);
    EXPECT_EQ(DH_SUCCESS, ret);
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
    int32_t sessionId = 1;
    std::string smsg = "";
    int32_t ret = DistributedInputSinkTransport::GetInstance().RespLatency(sessionId, smsg);
    EXPECT_EQ(DH_SUCCESS, ret);
}

HWTEST_F(DistributedInputSinkTransTest, GetDeviceIdBySessionId, testing::ext::TestSize.Level1)
{
    int32_t sessionId = 1;
    std::string smsg = "";
    DistributedInputSinkTransport::GetInstance().GetDeviceIdBySessionId(sessionId, smsg);
    EXPECT_EQ("", smsg);
}

HWTEST_F(DistributedInputSinkTransTest, SendMessage, testing::ext::TestSize.Level1)
{
    int32_t sessionId = 1;
    std::string smsg = "qwerzsdgertgdfgbrtyhuert6345634tgadsgfq13451234rfaDSFQ34FQQWEFWQERQWEFQWEFASEFQWERQWERQWER123";
    int32_t ret = DistributedInputSinkTransport::GetInstance().SendMessage(sessionId, smsg);
    EXPECT_EQ(DH_SUCCESS, ret);
}
} // namespace DistributedInput
} // namespace DistributedHardware
} // namespace OHOS