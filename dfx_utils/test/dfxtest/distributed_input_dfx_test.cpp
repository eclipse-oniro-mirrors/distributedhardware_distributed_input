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

#include "distributed_input_dfx_test.h"

#include "system_ability_definition.h"

#include "dinput_errcode.h"
#include "hidumper.h"
#include "hisysevent_util.h"

namespace OHOS {
namespace DistributedHardware {
namespace DistributedInput {
void DInputDfxUtilsTest::SetUp()
{
}

void DInputDfxUtilsTest::TearDown()
{
}

void DInputDfxUtilsTest::SetUpTestCase()
{
}

void DInputDfxUtilsTest::TearDownTestCase()
{
}

HWTEST_F(DInputDfxUtilsTest, HiDump_001, testing::ext::TestSize.Level1)
{
    std::vector<std::string> args;
    std::string result = "";
    bool ret = HiDumper::GetInstance().HiDump(args, result);
    EXPECT_EQ(false, ret);
}

HWTEST_F(DInputDfxUtilsTest, HiDump_002, testing::ext::TestSize.Level1)
{
    std::vector<std::string> args;
    args.push_back("-h");
    std::string result = "";
    bool ret = HiDumper::GetInstance().HiDump(args, result);
    EXPECT_EQ(true, ret);

    args.clear();
    args.push_back("-nodeinfo");
    ret = HiDumper::GetInstance().HiDump(args, result);
    EXPECT_EQ(true, ret);

    args.clear();
    args.push_back("-sessioninfo");
    ret = HiDumper::GetInstance().HiDump(args, result);
    EXPECT_EQ(true, ret);

    args.clear();
    args.push_back("args_test");
    ret = HiDumper::GetInstance().HiDump(args, result);
    EXPECT_EQ(false, ret);
}

HWTEST_F(DInputDfxUtilsTest, GetAllNodeInfos_001, testing::ext::TestSize.Level1)
{
    std::string devId = "umkyu1b165e1be98151891erbe8r91ev";
    std::string dhId = "1ds56v18e1v21v8v1erv15r1v8r1j1ty8";
    std::string nodeName = "DistributedInput_123456";
    HiDumper::GetInstance().SaveNodeInfo(devId, nodeName, dhId);
    std::string dhId1 = "dhId1_test";
    std::string devId1 = "devId1_test";
    HiDumper::GetInstance().SaveNodeInfo(devId, nodeName, dhId1);
    EXPECT_EQ(2, HiDumper::GetInstance().nodeInfos_.size());

    std::string result = "";
    int32_t ret = HiDumper::GetInstance().GetAllNodeInfos(result);
    EXPECT_EQ(DH_SUCCESS, ret);

    HiDumper::GetInstance().DeleteNodeInfo(devId, dhId1);
    HiDumper::GetInstance().DeleteNodeInfo(devId1, dhId);
    HiDumper::GetInstance().DeleteNodeInfo(devId1, dhId1);
    HiDumper::GetInstance().DeleteNodeInfo(devId, dhId);
    EXPECT_EQ(0, HiDumper::GetInstance().nodeInfos_.size());
    std::string status = "status_ok";
    std::string msg = "msg_test";
    HisyseventUtil::GetInstance().SysEventWriteBehavior(status, msg);
    HisyseventUtil::GetInstance().SysEventWriteBehavior(status, devId, msg);
    HisyseventUtil::GetInstance().SysEventWriteBehavior(status, devId, dhId, msg);
}

HWTEST_F(DInputDfxUtilsTest, GetSessionInfo_001, testing::ext::TestSize.Level1)
{
    std::string remoteDevId = "umkyu1b165e1be98151891erbe8r91ev";
    std::string remoteDevId1 = "remoteDevId_test";
    int32_t sessionId = 1;
    std::string mySessionName = "mySessionName_test";
    std::string peerSessionName = "peerSessionName_test";
    SessionStatus sessionStatus = static_cast<SessionStatus>(0x04);
    std::string result = "";
    HiDumper::GetInstance().CreateSessionInfo(remoteDevId, sessionId, mySessionName, peerSessionName, sessionStatus);
    EXPECT_EQ(1, HiDumper::GetInstance().sessionInfos_.size());
    int32_t ret = HiDumper::GetInstance().GetSessionInfo(result);
    EXPECT_EQ(DH_SUCCESS, ret);

    sessionStatus = SessionStatus::CLOSED;
    HiDumper::GetInstance().CreateSessionInfo(remoteDevId, sessionId, mySessionName, peerSessionName, sessionStatus);
    EXPECT_EQ(1, HiDumper::GetInstance().sessionInfos_.size());

    sessionStatus = SessionStatus::CLOSING;
    HiDumper::GetInstance().SetSessionStatus(remoteDevId1, sessionStatus);
    HiDumper::GetInstance().SetSessionStatus(remoteDevId, sessionStatus);

    ret = HiDumper::GetInstance().GetSessionInfo(result);
    EXPECT_EQ(DH_SUCCESS, ret);

    HiDumper::GetInstance().DeleteSessionInfo(remoteDevId);
    EXPECT_EQ(0, HiDumper::GetInstance().sessionInfos_.size());

    HiDumper::GetInstance().DeleteSessionInfo(remoteDevId1);

    std::string status = "status_ok";
    std::string msg = "msg_test";
    int32_t errorCode = 1;
    std::string dhId = "1ds56v18e1v21v8v1erv15r1v8r1j1ty8";
    HisyseventUtil::GetInstance().SysEventWriteFault(status, msg);
    HisyseventUtil::GetInstance().SysEventWriteFault(status, remoteDevId, errorCode, msg);
    HisyseventUtil::GetInstance().SysEventWriteFault(status, remoteDevId, dhId, errorCode, msg);
}
} // namespace DistributedInput
} // namespace DistributedHardware
} // namespace OHOS