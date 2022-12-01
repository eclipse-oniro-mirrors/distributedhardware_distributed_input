/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "distributed_input_ipc_test.h"

#include "dinput_errcode.h"

using namespace testing::ext;
using namespace OHOS::DistributedHardware::DistributedInput;
using namespace std;
namespace OHOS {
namespace DistributedHardware {
namespace DistributedInput {
void DistributedInputIpcTest::SetUp()
{
}

void DistributedInputIpcTest::TearDown()
{
}

void DistributedInputIpcTest::SetUpTestCase()
{
}

void DistributedInputIpcTest::TearDownTestCase()
{
}

void DistributedInputIpcTest::TestInputNodeListener::OnNodeOnLine(const std::string srcDevId,
    const std::string sinkDevId, const std::string sinkNodeId, const std::string sinkNodeDesc)
{
    (void)srcDevId;
    (void)sinkDevId;
    (void)sinkNodeId;
    (void)sinkNodeDesc;
    return;
}

void DistributedInputIpcTest::TestInputNodeListener::OnNodeOffLine(const std::string srcDevId,
    const std::string sinkDevId, const std::string sinkNodeId)
{
    (void)srcDevId;
    (void)sinkDevId;
    (void)sinkNodeId;
    return;
}

int32_t DistributedInputIpcTest::TestSimulationEventListenerStub::OnSimulationEvent(
    uint32_t type, uint32_t code, int32_t value)
{
    (void)type;
    (void)code;
    (void)value;
    return DH_SUCCESS;
}

HWTEST_F(DistributedInputIpcTest, GetDInputSinkProxy01, testing::ext::TestSize.Level1)
{
    bool ret = DInputSAManager::GetInstance().GetDInputSinkProxy();
    EXPECT_EQ(false, ret);
}

HWTEST_F(DistributedInputIpcTest, CheckSourceRegisterCallback01, testing::ext::TestSize.Level1)
{
    std::string deviceId = "46qweqwe46q5qw4e";
    BusinessEvent event;
    DistributedInputClient::GetInstance().CheckSourceRegisterCallback();
    bool ret = DistributedInputClient::GetInstance().IsNeedFilterOut(deviceId, event);
    EXPECT_EQ(false, ret);
}

HWTEST_F(DistributedInputIpcTest, IsNeedFilterOut01, testing::ext::TestSize.Level1)
{
    std::string deviceId = "46qweqwe46q5qw4e";
    BusinessEvent event;
    bool ret = DistributedInputClient::GetInstance().IsNeedFilterOut(deviceId, event);
    EXPECT_EQ(false, ret);
}

HWTEST_F(DistributedInputIpcTest, IsNeedFilterOut02, testing::ext::TestSize.Level1)
{
    std::string deviceId;
    BusinessEvent event;
    bool ret = DistributedInputClient::GetInstance().IsNeedFilterOut(deviceId, event);
    EXPECT_EQ(false, ret);
}

HWTEST_F(DistributedInputIpcTest, IsTouchEventNeedFilterOut01, testing::ext::TestSize.Level1)
{
    TouchScreenEvent event = {100, 100};
    bool ret = DistributedInputClient::GetInstance().IsTouchEventNeedFilterOut(event);
    EXPECT_EQ(false, ret);
}

HWTEST_F(DistributedInputIpcTest, IsStartDistributedInput01, testing::ext::TestSize.Level1)
{
    std::string dhId;
    bool ret = DistributedInputClient::GetInstance().IsStartDistributedInput(dhId);
    EXPECT_EQ(false, ret);
}

HWTEST_F(DistributedInputIpcTest, IsStartDistributedInput02, testing::ext::TestSize.Level1)
{
    std::string dhId = "654ew6qw4f6w1e6f1w6e5f";
    bool ret = DistributedInputClient::GetInstance().IsStartDistributedInput(dhId);
    EXPECT_EQ(false, ret);
}

HWTEST_F(DistributedInputIpcTest, RegisterInputNodeListener01, testing::ext::TestSize.Level1)
{
    sptr<TestInputNodeListener> listener = nullptr;
    int32_t ret = DistributedInputClient::GetInstance().RegisterInputNodeListener(listener);
    EXPECT_EQ(ERR_DH_INPUT_CLIENT_REG_NODE_CB_FAIL, ret);
}

HWTEST_F(DistributedInputIpcTest, RegisterInputNodeListener02, testing::ext::TestSize.Level1)
{
    sptr<TestInputNodeListener> listener = new TestInputNodeListener();
    int32_t ret = DistributedInputClient::GetInstance().RegisterInputNodeListener(listener);
    EXPECT_EQ(ERR_DH_INPUT_CLIENT_GET_SOURCE_PROXY_FAIL, ret);
}

HWTEST_F(DistributedInputIpcTest, UnregisterInputNodeListener01, testing::ext::TestSize.Level1)
{
    sptr<TestInputNodeListener> listener = nullptr;
    int32_t ret = DistributedInputClient::GetInstance().UnregisterInputNodeListener(listener);
    EXPECT_EQ(ERR_DH_INPUT_CLIENT_UNREG_NODE_CB_FAIL, ret);
}

HWTEST_F(DistributedInputIpcTest, UnregisterInputNodeListener02, testing::ext::TestSize.Level1)
{
    sptr<TestInputNodeListener> listener = new TestInputNodeListener();
    int32_t ret = DistributedInputClient::GetInstance().UnregisterInputNodeListener(listener);
    EXPECT_EQ(ERR_DH_INPUT_CLIENT_GET_SOURCE_PROXY_FAIL, ret);
}

HWTEST_F(DistributedInputIpcTest, RegisterSimulationEventListener01, testing::ext::TestSize.Level1)
{
    sptr<TestSimulationEventListenerStub> listener = nullptr;
    int32_t ret = DistributedInputClient::GetInstance().RegisterSimulationEventListener(listener);
    EXPECT_EQ(ERR_DH_INPUT_CLIENT_REG_UNREG_KEY_STATE_FAIL, ret);
}

HWTEST_F(DistributedInputIpcTest, RegisterSimulationEventListener02, testing::ext::TestSize.Level1)
{
    sptr<TestSimulationEventListenerStub> listener = new TestSimulationEventListenerStub();
    int32_t ret = DistributedInputClient::GetInstance().RegisterSimulationEventListener(listener);
    EXPECT_EQ(ERR_DH_INPUT_CLIENT_GET_SOURCE_PROXY_FAIL, ret);
}

HWTEST_F(DistributedInputIpcTest, UnregisterSimulationEventListener01, testing::ext::TestSize.Level1)
{
    sptr<TestSimulationEventListenerStub> listener = nullptr;
    int32_t ret = DistributedInputClient::GetInstance().UnregisterSimulationEventListener(listener);
    EXPECT_EQ(ERR_DH_INPUT_CLIENT_REG_UNREG_KEY_STATE_FAIL, ret);
}

HWTEST_F(DistributedInputIpcTest, UnregisterSimulationEventListener02, testing::ext::TestSize.Level1)
{
    sptr<TestSimulationEventListenerStub> listener = new TestSimulationEventListenerStub();
    int32_t ret = DistributedInputClient::GetInstance().UnregisterSimulationEventListener(listener);
    EXPECT_EQ(ERR_DH_INPUT_CLIENT_GET_SOURCE_PROXY_FAIL, ret);
}

HWTEST_F(DistributedInputIpcTest, IsJsonData01, testing::ext::TestSize.Level1)
{
    std::string strData = "123";
    bool ret = DistributedInputClient::GetInstance().IsJsonData(strData);
    EXPECT_EQ(false, ret);
}

HWTEST_F(DistributedInputIpcTest, IsJsonData02, testing::ext::TestSize.Level1)
{
    std::string strData = "{{{}}}";
    bool ret = DistributedInputClient::GetInstance().IsJsonData(strData);
    EXPECT_EQ(true, ret);
}

HWTEST_F(DistributedInputIpcTest, IsJsonData03, testing::ext::TestSize.Level1)
{
    std::string strData = "{3413";
    bool ret = DistributedInputClient::GetInstance().IsJsonData(strData);
    EXPECT_EQ(false, ret);
}

HWTEST_F(DistributedInputIpcTest, AddWhiteListInfos01, testing::ext::TestSize.Level1)
{
    std::string deviceId = "4646565465asdqweqwe";
    std::string strJson = "[[[2000][2000][1]]]";
    DistributedInputClient::GetInstance().AddWhiteListInfos(deviceId, strJson);
    std::string strData = "{3413";
    bool ret = DistributedInputClient::GetInstance().IsJsonData(strData);
    EXPECT_EQ(false, ret);
}

HWTEST_F(DistributedInputIpcTest, AddWhiteListInfos02, testing::ext::TestSize.Level1)
{
    std::string deviceId = "4646565465asdqweqwe";
    std::string strJson = "";
    DistributedInputClient::GetInstance().AddWhiteListInfos(deviceId, strJson);
    std::string strData = "{3413";
    bool ret = DistributedInputClient::GetInstance().IsJsonData(strData);
    EXPECT_EQ(false, ret);
}

HWTEST_F(DistributedInputIpcTest, DelWhiteListInfos01, testing::ext::TestSize.Level1)
{
    std::string deviceId = "4646565465asdqweqwe";
    DistributedInputClient::GetInstance().DelWhiteListInfos(deviceId);
    std::string strData = "{3413";
    bool ret = DistributedInputClient::GetInstance().IsJsonData(strData);
    EXPECT_EQ(false, ret);
}

HWTEST_F(DistributedInputIpcTest, UpdateSinkScreenInfos01, testing::ext::TestSize.Level1)
{
    std::string strJson = "[[1080][720][10][10]]";
    DistributedInputClient::GetInstance().UpdateSinkScreenInfos(strJson);
    std::string strData = "{3413";
    bool ret = DistributedInputClient::GetInstance().IsJsonData(strData);
    EXPECT_EQ(false, ret);
}

HWTEST_F(DistributedInputIpcTest, NotifyStartDScreen01, testing::ext::TestSize.Level1)
{
    std::string sinkDevId = "46qw4e61dq6w1dq6w5e4q6";
    std::string srcDevId = "erq6w54e9q8w4eqw19q6d1";
    uint64_t srcWinId = 5;
    int32_t ret = DistributedInputClient::GetInstance().NotifyStartDScreen(sinkDevId, srcDevId, srcWinId);
    EXPECT_EQ(ERR_DH_INPUT_RPC_GET_REMOTE_DINPUT_FAIL, ret);
}

HWTEST_F(DistributedInputIpcTest, NotifyStopDScreen01, testing::ext::TestSize.Level1)
{
    std::string networkId = "46qw4e61dq6w1dq6w5e4q6";
    std::string srcScreenInfoKey = "q65we46qw54e6q5we46q";
    int32_t ret = DistributedInputClient::GetInstance().NotifyStopDScreen(networkId, srcScreenInfoKey);
    EXPECT_EQ(ERR_DH_INPUT_RPC_GET_REMOTE_DINPUT_FAIL, ret);
}

} // namespace DistributedInput
} // namespace DistributedHardware
} // namespace OHOS