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
    EXPECT_EQ(SUCCESS, ret);
}

HWTEST_F(DistributedInputSinkTransTest, RespPrepareRemoteInput01, testing::ext::TestSize.Level1)
{
    int32_t sessionId = -1;
    int32_t result = SUCCESS;
    string object = "tttt";
    int32_t ret = DistributedInputSinkTransport::GetInstance().RespPrepareRemoteInput(sessionId, result, object);
    EXPECT_EQ(FAILURE, ret);
}

HWTEST_F(DistributedInputSinkTransTest, RespPrepareRemoteInput02, testing::ext::TestSize.Level1)
{
    int32_t sessionId = 0;
    int32_t result = SUCCESS;
    string object = "tttt";
    int32_t ret = DistributedInputSinkTransport::GetInstance().RespPrepareRemoteInput(sessionId, result, object);
    EXPECT_EQ(FAILURE, ret);
}

HWTEST_F(DistributedInputSinkTransTest, RespPrepareRemoteInput03, testing::ext::TestSize.Level1)
{
    int32_t sessionId = 100;
    int32_t result = SUCCESS;
    string object = "tttt";
    int32_t ret = DistributedInputSinkTransport::GetInstance().RespPrepareRemoteInput(sessionId, result, object);
    EXPECT_EQ(SUCCESS, ret);
}

HWTEST_F(DistributedInputSinkTransTest, RespUnprepareRemoteInput01, testing::ext::TestSize.Level0)
{
    int32_t sessionId = -1;
    int32_t result = SUCCESS;
    int32_t ret = DistributedInputSinkTransport::GetInstance().RespUnprepareRemoteInput(sessionId, result);
    EXPECT_EQ(FAILURE, ret);
}

HWTEST_F(DistributedInputSinkTransTest, RespUnprepareRemoteInput02, testing::ext::TestSize.Level1)
{
    int32_t sessionId = 0;
    int32_t result = SUCCESS;
    int32_t ret = DistributedInputSinkTransport::GetInstance().RespUnprepareRemoteInput(sessionId, result);
    EXPECT_EQ(FAILURE, ret);
}

HWTEST_F(DistributedInputSinkTransTest, RespUnprepareRemoteInput03, testing::ext::TestSize.Level1)
{
    int32_t sessionId = 100;
    int32_t result = SUCCESS;
    int32_t ret = DistributedInputSinkTransport::GetInstance().RespUnprepareRemoteInput(sessionId, result);
    EXPECT_EQ(SUCCESS, ret);
}

HWTEST_F(DistributedInputSinkTransTest, RespStartRemoteInput01, testing::ext::TestSize.Level1)
{
    int32_t sessionId = -1;
    std::string smsg = "";
    int32_t ret = DistributedInputSinkTransport::GetInstance().RespStartRemoteInput(sessionId, smsg);
    EXPECT_EQ(FAILURE, ret);
}

HWTEST_F(DistributedInputSinkTransTest, RespStartRemoteInput02, testing::ext::TestSize.Level1)
{
    int32_t sessionId = 0;
    std::string smsg = "";
    int32_t ret = DistributedInputSinkTransport::GetInstance().RespStartRemoteInput(sessionId, smsg);
    EXPECT_EQ(FAILURE, ret);
}

HWTEST_F(DistributedInputSinkTransTest, RespStartRemoteInput03, testing::ext::TestSize.Level0)
{
    int32_t sessionId = 100;
    std::string smsg = "";
    int32_t ret = DistributedInputSinkTransport::GetInstance().RespStartRemoteInput(sessionId, smsg);
    EXPECT_EQ(SUCCESS, ret);
}

HWTEST_F(DistributedInputSinkTransTest, RespStopRemoteInput01, testing::ext::TestSize.Level1)
{
    int32_t sessionId = -1;
    std::string smsg = "";
    int32_t ret = DistributedInputSinkTransport::GetInstance().RespStopRemoteInput(sessionId, smsg);
    EXPECT_EQ(FAILURE, ret);
}

HWTEST_F(DistributedInputSinkTransTest, RespStopRemoteInput02, testing::ext::TestSize.Level1)
{
    int32_t sessionId = 0;
    std::string smsg = "";
    int32_t ret = DistributedInputSinkTransport::GetInstance().RespStopRemoteInput(sessionId, smsg);
    EXPECT_EQ(FAILURE, ret);
}

HWTEST_F(DistributedInputSinkTransTest, RespStopRemoteInput03, testing::ext::TestSize.Level0)
{
    int32_t sessionId = 100;
    std::string smsg = "";
    int32_t ret = DistributedInputSinkTransport::GetInstance().RespStopRemoteInput(sessionId, smsg);
    EXPECT_EQ(SUCCESS, ret);
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
    EXPECT_EQ(SUCCESS, ret);
}
HWTEST_F(DistributedInputSinkTransTest, StartSwitch02, testing::ext::TestSize.Level1)
{
    int32_t sessionId = 1000;
    DistributedInputSinkSwitch::GetInstance().AddSession(sessionId);
    int32_t ret = DistributedInputSinkSwitch::GetInstance().StartSwitch(sessionId+10);
    EXPECT_EQ(FAILURE, ret);
}
}
}
}
