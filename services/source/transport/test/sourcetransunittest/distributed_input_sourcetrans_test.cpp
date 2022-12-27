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

#include "distributed_input_sourcetrans_test.h"

#include <cstdlib>

#include "dinput_errcode.h"
#include "dinput_softbus_define.h"
#include "distributed_input_source_manager.h"

using namespace testing::ext;
using namespace OHOS::DistributedHardware::DistributedInput;
using namespace std;
namespace OHOS {
namespace DistributedHardware {
namespace DistributedInput {
void DistributedInputSourceTransTest::SetUp()
{
}

void DistributedInputSourceTransTest::TearDown()
{
}

void DistributedInputSourceTransTest::SetUpTestCase()
{
}

void DistributedInputSourceTransTest::TearDownTestCase()
{
}

HWTEST_F(DistributedInputSourceTransTest, Init01, testing::ext::TestSize.Level0)
{
    int32_t ret = DistributedInputSourceTransport::GetInstance().Init();
    EXPECT_EQ(DH_SUCCESS, ret);
}

HWTEST_F(DistributedInputSourceTransTest, OpenInputSoftbus01, testing::ext::TestSize.Level0)
{
    std::string remoteDevId = "";
    int32_t ret = DistributedInputSourceTransport::GetInstance().OpenInputSoftbus(remoteDevId);
    EXPECT_EQ(ERR_DH_INPUT_SERVER_SOURCE_TRANSPORT_OPEN_SESSION_FAIL, ret);
}

HWTEST_F(DistributedInputSourceTransTest, OpenInputSoftbus02, testing::ext::TestSize.Level0)
{
    std::string remoteDevId = "f6d4c08647073e02e7a78f09473aa122ff57fc81c00981fcf5be989e7d112591";
    int32_t sessionId = 1;
    DistributedInputSourceTransport::DInputSessionInfo sessionInfo{false, remoteDevId};
    DistributedInputSourceTransport::GetInstance().sessionDevMap_[sessionId] = sessionInfo;
    int32_t ret = DistributedInputSourceTransport::GetInstance().OpenInputSoftbus(remoteDevId);
    EXPECT_EQ(DH_SUCCESS, ret);
    DistributedInputSourceTransport::GetInstance().sessionDevMap_.clear();
}

HWTEST_F(DistributedInputSourceTransTest, PrepareRemoteInput_01, testing::ext::TestSize.Level0)
{
    std::string deviceId = "";
    int32_t ret = DistributedInputSourceTransport::GetInstance().PrepareRemoteInput(deviceId);
    EXPECT_EQ(ERR_DH_INPUT_SERVER_SOURCE_TRANSPORT_PREPARE_FAIL, ret);
}

HWTEST_F(DistributedInputSourceTransTest, PrepareRemoteInput_02, testing::ext::TestSize.Level1)
{
    int32_t sessionId = 1;
    std::string deviceId = "f6d4c08647073e02e7a78f09473aa122ff57fc81c00981fcf5be989e7d112591";
    DistributedInputSourceTransport::DInputSessionInfo sessionInfo{false, deviceId};
    DistributedInputSourceTransport::GetInstance().sessionDevMap_[sessionId] = sessionInfo;
    int32_t ret = DistributedInputSourceTransport::GetInstance().PrepareRemoteInput(deviceId);
    EXPECT_EQ(DH_SUCCESS, ret);
}

HWTEST_F(DistributedInputSourceTransTest, PrepareRemoteInput_03, testing::ext::TestSize.Level1)
{
    std::string deviceId = "";
    int32_t srcTsrcSeId = 1;
    int32_t ret = DistributedInputSourceTransport::GetInstance().PrepareRemoteInput(srcTsrcSeId, deviceId);
    EXPECT_EQ(ERR_DH_INPUT_SERVER_SOURCE_TRANSPORT_PREPARE_FAIL, ret);
}

HWTEST_F(DistributedInputSourceTransTest, PrepareRemoteInput_04, testing::ext::TestSize.Level1)
{
    std::string deviceId = "f6d4c08647073e02e7a78f09473aa122ff57fc81c00981fcf5be989e7d112591";
    int32_t srcTsrcSeId = 1;
    int32_t ret = DistributedInputSourceTransport::GetInstance().PrepareRemoteInput(srcTsrcSeId, deviceId);
    EXPECT_EQ(DH_SUCCESS, ret);
}

HWTEST_F(DistributedInputSourceTransTest, UnprepareRemoteInput_01, testing::ext::TestSize.Level0)
{
    std::string deviceId = "";
    int32_t ret = DistributedInputSourceTransport::GetInstance().UnprepareRemoteInput(deviceId);
    EXPECT_EQ(ERR_DH_INPUT_SERVER_SOURCE_TRANSPORT_UNPREPARE_FAIL, ret);
}

HWTEST_F(DistributedInputSourceTransTest, UnprepareRemoteInput_02, testing::ext::TestSize.Level0)
{
    std::string deviceId = "f6d4c08647073e02e7a78f09473aa122ff57fc81c00981fcf5be989e7d112591";
    int32_t ret = DistributedInputSourceTransport::GetInstance().UnprepareRemoteInput(deviceId);
    EXPECT_EQ(DH_SUCCESS, ret);
}

HWTEST_F(DistributedInputSourceTransTest, UnprepareRemoteInput_03, testing::ext::TestSize.Level1)
{
    std::string deviceId = "";
    int32_t srcTsrcSeId = 1;
    int32_t ret = DistributedInputSourceTransport::GetInstance().UnprepareRemoteInput(srcTsrcSeId, deviceId);
    EXPECT_EQ(ERR_DH_INPUT_SERVER_SOURCE_TRANSPORT_UNPREPARE_FAIL, ret);
}

HWTEST_F(DistributedInputSourceTransTest, UnprepareRemoteInput_04, testing::ext::TestSize.Level1)
{
    std::string deviceId = "f6d4c08647073e02e7a78f09473aa122ff57fc81c00981fcf5be989e7d112591";
    int32_t srcTsrcSeId = 1;
    int32_t ret = DistributedInputSourceTransport::GetInstance().UnprepareRemoteInput(srcTsrcSeId, deviceId);
    EXPECT_EQ(DH_SUCCESS, ret);
}

HWTEST_F(DistributedInputSourceTransTest, StartRemoteInput01, testing::ext::TestSize.Level0)
{
    std::string deviceId = "";
    int32_t ret = DistributedInputSourceTransport::GetInstance().StartRemoteInput(
        deviceId, static_cast<uint32_t>(DInputDeviceType::ALL));
    EXPECT_EQ(ERR_DH_INPUT_SERVER_SOURCE_TRANSPORT_START_FAIL, ret);
}

HWTEST_F(DistributedInputSourceTransTest, StartRemoteInput02, testing::ext::TestSize.Level0)
{
    std::string deviceId = "f6d4c08647073e02e7a78f09473aa122ff57fc81c00981fcf5be989e7d112591";
    int32_t ret = DistributedInputSourceTransport::GetInstance().StartRemoteInput(
        deviceId, static_cast<uint32_t>(DInputDeviceType::ALL));
    EXPECT_EQ(DH_SUCCESS, ret);
}

HWTEST_F(DistributedInputSourceTransTest, StartRemoteInput03, testing::ext::TestSize.Level0)
{
    std::string deviceId = "";
    std::string dhid = "Input_1ds56v18e1v21v8v1erv15r1v8r1j1ty8";
    std::vector<std::string> dhids;
    dhids.push_back(dhid);
    int32_t ret = DistributedInputSourceTransport::GetInstance().StartRemoteInput(deviceId, dhids);
    EXPECT_EQ(ERR_DH_INPUT_SERVER_SOURCE_TRANSPORT_START_FAIL, ret);
}

HWTEST_F(DistributedInputSourceTransTest, StartRemoteInput04, testing::ext::TestSize.Level0)
{
    std::string deviceId = "f6d4c08647073e02e7a78f09473aa122ff57fc81c00981fcf5be989e7d112591";
    std::string dhid = "Input_1ds56v18e1v21v8v1erv15r1v8r1j1ty8";
    std::vector<std::string> dhids;
    dhids.push_back(dhid);
    int32_t ret = DistributedInputSourceTransport::GetInstance().StartRemoteInput(deviceId, dhids);
    EXPECT_EQ(DH_SUCCESS, ret);
}

HWTEST_F(DistributedInputSourceTransTest, StartRemoteInputDhids_01, testing::ext::TestSize.Level1)
{
    int32_t srcTsrcSeId = 1;
    const std::string deviceId = "f6d4c08647073e02e7a78f09473aa122ff57fc81c00981fcf5be989e7d112591";
    const std::string dhids = "Input_1ds56v18e1v21v8v1erv15r1v8r1j1ty8";
    int32_t ret = DistributedInputSourceTransport::GetInstance().StartRemoteInputDhids(srcTsrcSeId, deviceId, dhids);
    EXPECT_EQ(DH_SUCCESS, ret);
}

HWTEST_F(DistributedInputSourceTransTest, StartRemoteInputDhids_02, testing::ext::TestSize.Level1)
{
    int32_t srcTsrcSeId = 1;
    const std::string deviceId = "";
    const std::string dhids = "Input_1ds56v18e1v21v8v1erv15r1v8r1j1ty8";
    int32_t ret = DistributedInputSourceTransport::GetInstance().StartRemoteInputDhids(srcTsrcSeId, deviceId, dhids);
    EXPECT_EQ(ERR_DH_INPUT_SERVER_SOURCE_TRANSPORT_START_FAIL, ret);
}

HWTEST_F(DistributedInputSourceTransTest, StartRemoteInputType_01, testing::ext::TestSize.Level1)
{
    int32_t srcTsrcSeId = 1;
    std::string deviceId = "f6d4c08647073e02e7a78f09473aa122ff57fc81c00981fcf5be989e7d112591";
    int32_t ret = DistributedInputSourceTransport::GetInstance().StartRemoteInputType(srcTsrcSeId, deviceId,
        static_cast<uint32_t>(DInputDeviceType::ALL));
    EXPECT_EQ(DH_SUCCESS, ret);
}

HWTEST_F(DistributedInputSourceTransTest, StartRemoteInputType_02, testing::ext::TestSize.Level1)
{
    int32_t srcTsrcSeId = 1;
    std::string deviceId = "";
    int32_t ret = DistributedInputSourceTransport::GetInstance().StartRemoteInputType(srcTsrcSeId, deviceId,
        static_cast<uint32_t>(DInputDeviceType::ALL));
    EXPECT_EQ(ERR_DH_INPUT_SERVER_SOURCE_TRANSPORT_START_FAIL, ret);
}

HWTEST_F(DistributedInputSourceTransTest, StopRemoteInput01, testing::ext::TestSize.Level0)
{
    std::string deviceId = "";
    int32_t ret = DistributedInputSourceTransport::GetInstance().StopRemoteInput(
        deviceId, static_cast<uint32_t>(DInputDeviceType::ALL));
    EXPECT_EQ(ERR_DH_INPUT_SERVER_SOURCE_TRANSPORT_STOP_FAIL, ret);
}

HWTEST_F(DistributedInputSourceTransTest, StopRemoteInput02, testing::ext::TestSize.Level0)
{
    std::string deviceId = "f6d4c08647073e02e7a78f09473aa122ff57fc81c00981fcf5be989e7d112591";
    int32_t sessionId = 2;
    int32_t ret = DistributedInputSourceTransport::GetInstance().StopRemoteInput(
        deviceId, static_cast<uint32_t>(DInputDeviceType::ALL));
    EXPECT_EQ(DH_SUCCESS, ret);
    DistributedInputSourceTransport::GetInstance().CloseInputSoftbus(sessionId);
}

HWTEST_F(DistributedInputSourceTransTest, StopRemoteInput03, testing::ext::TestSize.Level0)
{
    std::string deviceId = "";
    std::vector<std::string> dhids;
    dhids.push_back("Input_1ds56v18e1v21v8v1erv15r1v8r1j1ty8");
    int32_t ret = DistributedInputSourceTransport::GetInstance().StopRemoteInput(deviceId, dhids);
    EXPECT_EQ(ERR_DH_INPUT_SERVER_SOURCE_TRANSPORT_STOP_FAIL, ret);
}

HWTEST_F(DistributedInputSourceTransTest, StopRemoteInput04, testing::ext::TestSize.Level0)
{
    std::string deviceId = "f6d4c08647073e02e7a78f09473aa122ff57fc81c00981fcf5be989e7d112591";
    std::vector<std::string> dhids;
    dhids.push_back("Input_1ds56v18e1v21v8v1erv15r1v8r1j1ty8");
    int32_t ret = DistributedInputSourceTransport::GetInstance().StopRemoteInput(deviceId, dhids);
    EXPECT_EQ(DH_SUCCESS, ret);
}

HWTEST_F(DistributedInputSourceTransTest, StopRemoteInputDhids_01, testing::ext::TestSize.Level1)
{
    int32_t srcTsrcSeId = 1;
    const std::string deviceId = "f6d4c08647073e02e7a78f09473aa122ff57fc81c00981fcf5be989e7d112591";
    const std::string dhids = "Input_1ds56v18e1v21v8v1erv15r1v8r1j1ty8";
    int32_t ret = DistributedInputSourceTransport::GetInstance().StopRemoteInputDhids(srcTsrcSeId, deviceId, dhids);
    EXPECT_EQ(DH_SUCCESS, ret);
}

HWTEST_F(DistributedInputSourceTransTest, StopRemoteInputDhids_02, testing::ext::TestSize.Level1)
{
    int32_t srcTsrcSeId = 1;
    const std::string deviceId = "";
    const std::string dhids = "Input_1ds56v18e1v21v8v1erv15r1v8r1j1ty8";
    int32_t ret = DistributedInputSourceTransport::GetInstance().StopRemoteInputDhids(srcTsrcSeId, deviceId, dhids);
    EXPECT_EQ(ERR_DH_INPUT_SERVER_SOURCE_TRANSPORT_STOP_FAIL, ret);
}

HWTEST_F(DistributedInputSourceTransTest, StopRemoteInputType_01, testing::ext::TestSize.Level1)
{
    int32_t srcTsrcSeId = 1;
    std::string deviceId = "f6d4c08647073e02e7a78f09473aa122ff57fc81c00981fcf5be989e7d112591";
    int32_t ret = DistributedInputSourceTransport::GetInstance().StopRemoteInputType(srcTsrcSeId, deviceId,
        static_cast<uint32_t>(DInputDeviceType::ALL));
    EXPECT_EQ(DH_SUCCESS, ret);
}

HWTEST_F(DistributedInputSourceTransTest, StopRemoteInputType_02, testing::ext::TestSize.Level1)
{
    int32_t srcTsrcSeId = 1;
    std::string deviceId = "";
    int32_t ret = DistributedInputSourceTransport::GetInstance().StopRemoteInputType(srcTsrcSeId, deviceId,
        static_cast<uint32_t>(DInputDeviceType::ALL));
    EXPECT_EQ(ERR_DH_INPUT_SERVER_SOURCE_TRANSPORT_STOP_FAIL, ret);
}

HWTEST_F(DistributedInputSourceTransTest, NotifyOriginPrepareResult01, testing::ext::TestSize.Level0)
{
    std::string srcId = "f6d4c08647073e02e7a78f09473aa122ff57fc81c00981fcf5be989e7d112591";
    std::string sinkId = "networkidc08647073e02e7a78f09473aa122ff57fc81c00";
    int32_t sessionId = 2;
    int32_t ret = DistributedInputSourceTransport::GetInstance().NotifyOriginPrepareResult(
        sessionId, srcId, sinkId, DH_SUCCESS);
    EXPECT_EQ(DH_SUCCESS, ret);
}

HWTEST_F(DistributedInputSourceTransTest, NotifyOriginUnprepareResult01, testing::ext::TestSize.Level0)
{
    std::string srcId = "f6d4c08647073e02e7a78f09473aa122ff57fc81c00981fcf5be989e7d112591";
    std::string sinkId = "networkidc08647073e02e7a78f09473aa122ff57fc81c00";
    int32_t sessionId = 2;
    int32_t ret = DistributedInputSourceTransport::GetInstance().NotifyOriginUnprepareResult(
        sessionId, srcId, sinkId, DH_SUCCESS);
    EXPECT_EQ(DH_SUCCESS, ret);
}

HWTEST_F(DistributedInputSourceTransTest, NotifyOriginStartDhidResult01, testing::ext::TestSize.Level0)
{
    std::string srcId = "f6d4c08647073e02e7a78f09473aa122ff57fc81c00981fcf5be989e7d112591";
    std::string sinkId = "networkidc08647073e02e7a78f09473aa122ff57fc81c00";
    int32_t sessionId = 2;
    std::string dhIds = "Input_1ds56v18e1v21v8v1erv15r1v8r1j1ty8";
    int32_t ret = DistributedInputSourceTransport::GetInstance().NotifyOriginStartDhidResult(
        sessionId, srcId, sinkId, DH_SUCCESS, dhIds);
    EXPECT_EQ(DH_SUCCESS, ret);
}

HWTEST_F(DistributedInputSourceTransTest, NotifyOriginStopDhidResult01, testing::ext::TestSize.Level0)
{
    std::string srcId = "f6d4c08647073e02e7a78f09473aa122ff57fc81c00981fcf5be989e7d112591";
    std::string sinkId = "networkidc08647073e02e7a78f09473aa122ff57fc81c00";
    int32_t sessionId = 2;
    std::string dhIds = "Input_1ds56v18e1v21v8v1erv15r1v8r1j1ty8";
    int32_t ret = DistributedInputSourceTransport::GetInstance().NotifyOriginStopDhidResult(
        sessionId, srcId, sinkId, DH_SUCCESS, dhIds);
    EXPECT_EQ(DH_SUCCESS, ret);
}

HWTEST_F(DistributedInputSourceTransTest, NotifyOriginStartTypeResult_01, testing::ext::TestSize.Level1)
{
    int32_t srcTsrcSeId = 2;
    std::string srcId = "f6d4c08647073e02e7a78f09473aa122ff57fc81c00981fcf5be989e7d112591";
    std::string sinkId = "networkidc08647073e02e7a78f09473aa122ff57fc81c00";
    int32_t ret = DistributedInputSourceTransport::GetInstance().NotifyOriginStartTypeResult(srcTsrcSeId, srcId,
        sinkId, DH_SUCCESS, static_cast<uint32_t>(DInputDeviceType::ALL));
    EXPECT_EQ(DH_SUCCESS, ret);
}

HWTEST_F(DistributedInputSourceTransTest, NotifyOriginStopTypeResult_01, testing::ext::TestSize.Level1)
{
    int32_t srcTsrcSeId = 2;
    std::string srcId = "f6d4c08647073e02e7a78f09473aa122ff57fc81c00981fcf5be989e7d112591";
    std::string sinkId = "networkidc08647073e02e7a78f09473aa122ff57fc81c00";
    int32_t ret = DistributedInputSourceTransport::GetInstance().NotifyOriginStopTypeResult(srcTsrcSeId, srcId,
        sinkId, DH_SUCCESS, static_cast<uint32_t>(DInputDeviceType::ALL));
    EXPECT_EQ(DH_SUCCESS, ret);
}

HWTEST_F(DistributedInputSourceTransTest, OpenInputSoftbusForRelay_01, testing::ext::TestSize.Level1)
{
    std::string srcId = "f6d4c08647073e02e7a78f09473aa122ff57fc81c00981fcf5be989e7d112591";
    int32_t sessionId = 2;
    std::string devId = "networkidc08647073e02e7a78f09473aa122ff57fc81c00";
    DistributedInputSourceTransport::DInputSessionInfo sessionInfo {true, devId};
    DistributedInputSourceTransport::GetInstance().sessionDevMap_[sessionId] = sessionInfo;
    int32_t ret = DistributedInputSourceTransport::GetInstance().OpenInputSoftbusForRelay(srcId);
    EXPECT_EQ(DH_SUCCESS, ret);
}

HWTEST_F(DistributedInputSourceTransTest, SendRelayPrepareRequest_01, testing::ext::TestSize.Level1)
{
    std::string srcId = "f6d4c08647073e02e7a78f09473aa122ff57fc81c00981fcf5be989e7d112591";
    std::string sinkId = "networkidc08647073e02e7a78f09473aa122ff57fc81c00";
    DistributedInputSourceTransport::GetInstance().OpenInputSoftbusForRelay(srcId);
    int32_t ret = DistributedInputSourceTransport::GetInstance().SendRelayPrepareRequest(srcId, sinkId);
    EXPECT_EQ(DH_SUCCESS, ret);
}

HWTEST_F(DistributedInputSourceTransTest, SendRelayPrepareRequest_02, testing::ext::TestSize.Level1)
{
    std::string srcId = "";
    std::string sinkId = "networkidc08647073e02e7a78f09473aa122ff57fc81c00";
    int32_t ret = DistributedInputSourceTransport::GetInstance().SendRelayPrepareRequest(srcId, sinkId);
    EXPECT_EQ(ERR_DH_INPUT_SERVER_SOURCE_TRANSPORT_PREPARE_FAIL, ret);
}

HWTEST_F(DistributedInputSourceTransTest, SendRelayUnprepareRequest_01, testing::ext::TestSize.Level1)
{
    std::string srcId = "f6d4c08647073e02e7a78f09473aa122ff57fc81c00981fcf5be989e7d112591";
    std::string sinkId = "networkidc08647073e02e7a78f09473aa122ff57fc81c00";
    int32_t ret = DistributedInputSourceTransport::GetInstance().SendRelayUnprepareRequest(srcId, sinkId);
    EXPECT_EQ(DH_SUCCESS, ret);
}

HWTEST_F(DistributedInputSourceTransTest, SendRelayUnprepareRequest_02, testing::ext::TestSize.Level1)
{
    std::string srcId = "";
    std::string sinkId = "networkidc08647073e02e7a78f09473aa122ff57fc81c00";
    int32_t ret = DistributedInputSourceTransport::GetInstance().SendRelayUnprepareRequest(srcId, sinkId);
    EXPECT_EQ(ERR_DH_INPUT_SERVER_SOURCE_TRANSPORT_PREPARE_FAIL, ret);
}

HWTEST_F(DistributedInputSourceTransTest, SendRelayStartDhidRequest_01, testing::ext::TestSize.Level1)
{
    std::string srcId = "f6d4c08647073e02e7a78f09473aa122ff57fc81c00981fcf5be989e7d112591";
    std::string sinkId = "networkidc08647073e02e7a78f09473aa122ff57fc81c00";
    std::vector<std::string> dhids;
    dhids.push_back("Input_1ds56v18e1v21v8v1erv15r1v8r1j1ty8");
    int32_t ret = DistributedInputSourceTransport::GetInstance().SendRelayStartDhidRequest(srcId, sinkId, dhids);
    EXPECT_EQ(DH_SUCCESS, ret);
}

HWTEST_F(DistributedInputSourceTransTest, SendRelayStartTypeRequest_01, testing::ext::TestSize.Level1)
{
    std::string srcId = "f6d4c08647073e02e7a78f09473aa122ff57fc81c00981fcf5be989e7d112591";
    std::string sinkId = "networkidc08647073e02e7a78f09473aa122ff57fc81c00";
    int32_t ret = DistributedInputSourceTransport::GetInstance().SendRelayStartTypeRequest(srcId, sinkId,
        static_cast<uint32_t>(DInputDeviceType::ALL));
    EXPECT_EQ(DH_SUCCESS, ret);
}

HWTEST_F(DistributedInputSourceTransTest, SendRelayStopDhidRequest_01, testing::ext::TestSize.Level1)
{
    std::string srcId = "f6d4c08647073e02e7a78f09473aa122ff57fc81c00981fcf5be989e7d112591";
    std::string sinkId = "networkidc08647073e02e7a78f09473aa122ff57fc81c00";
    std::vector<std::string> dhids;
    dhids.push_back("Input_1ds56v18e1v21v8v1erv15r1v8r1j1ty8");
    int32_t ret = DistributedInputSourceTransport::GetInstance().SendRelayStopDhidRequest(srcId, sinkId, dhids);
    EXPECT_EQ(DH_SUCCESS, ret);
}

HWTEST_F(DistributedInputSourceTransTest, SendRelayStopTypeRequest_01, testing::ext::TestSize.Level1)
{
    std::string srcId = "f6d4c08647073e02e7a78f09473aa122ff57fc81c00981fcf5be989e7d112591";
    std::string sinkId = "networkidc08647073e02e7a78f09473aa122ff57fc81c00";
    int32_t ret = DistributedInputSourceTransport::GetInstance().SendRelayStopTypeRequest(srcId, sinkId,
        static_cast<uint32_t>(DInputDeviceType::ALL));
    EXPECT_EQ(DH_SUCCESS, ret);
}

HWTEST_F(DistributedInputSourceTransTest, LatencyCount01, testing::ext::TestSize.Level0)
{
    std::string deviceId = "deviceId";
    int32_t ret = DistributedInputSourceTransport::GetInstance().LatencyCount(deviceId);
    EXPECT_EQ(ERR_DH_INPUT_SERVER_SOURCE_TRANSPORT_LATENCY_FAIL, ret);
}

HWTEST_F(DistributedInputSourceTransTest, LatencyCount02, testing::ext::TestSize.Level0)
{
    std::string deviceId = "f6d4c08647073e02e7a78f09473aa122ff57fc81c00981fcf5be989e7d112591";
    DistributedInputSourceTransport::DInputSessionInfo dInputSessionInfo;
    dInputSessionInfo.isToSrcSa = false;
    dInputSessionInfo.remoteId = deviceId;
    int32_t sessionId = 2;
    DistributedInputSourceTransport::GetInstance().sessionDevMap_[sessionId] = dInputSessionInfo;
    int32_t ret = DistributedInputSourceTransport::GetInstance().LatencyCount(deviceId);
    DistributedInputSourceTransport::GetInstance().sessionDevMap_.clear();
    DistributedInputSourceTransport::GetInstance().StopLatencyThread();
    EXPECT_EQ(DH_SUCCESS, ret);
}

HWTEST_F(DistributedInputSourceTransTest, SendRelayStartDhidRequest_02, testing::ext::TestSize.Level1)
{
    std::string srcId = "f6d4c08647073e02e7a78f09473aa122ff57fc81c00981fcf5be989e7d112591";
    std::string sinkId = "networkidc08647073e02e7a78f09473aa122ff57fc81c00";
    std::vector<std::string> dhids;
    dhids.push_back("Input_1ds56v18e1v21v8v1erv15r1v8r1j1ty8");
    DistributedInputSourceTransport::DInputSessionInfo dInputSessionInfo;
    dInputSessionInfo.isToSrcSa = true;
    dInputSessionInfo.remoteId = srcId;
    int32_t sessionId = -1;
    DistributedInputSourceTransport::GetInstance().sessionDevMap_[sessionId] = dInputSessionInfo;
    int32_t ret = DistributedInputSourceTransport::GetInstance().SendRelayStartDhidRequest(srcId, sinkId, dhids);
    DistributedInputSourceTransport::GetInstance().sessionDevMap_.clear();
    EXPECT_EQ(ERR_DH_INPUT_SERVER_SOURCE_TRANSPORT_START_FAIL, ret);
}

HWTEST_F(DistributedInputSourceTransTest, SendRelayStopDhidRequest_02, testing::ext::TestSize.Level1)
{
    std::string srcId = "f6d4c08647073e02e7a78f09473aa122ff57fc81c00981fcf5be989e7d112591";
    std::string sinkId = "networkidc08647073e02e7a78f09473aa122ff57fc81c00";
    std::vector<std::string> dhids;
    dhids.push_back("Input_1ds56v18e1v21v8v1erv15r1v8r1j1ty8");
    DistributedInputSourceTransport::DInputSessionInfo dInputSessionInfo;
    dInputSessionInfo.isToSrcSa = true;
    dInputSessionInfo.remoteId = srcId;
    int32_t sessionId = -1;
    DistributedInputSourceTransport::GetInstance().sessionDevMap_[sessionId] = dInputSessionInfo;
    int32_t ret = DistributedInputSourceTransport::GetInstance().SendRelayStopDhidRequest(srcId, sinkId, dhids);
    DistributedInputSourceTransport::GetInstance().sessionDevMap_.clear();
    EXPECT_EQ(ERR_DH_INPUT_SERVER_SOURCE_TRANSPORT_STOP_FAIL, ret);
}

HWTEST_F(DistributedInputSourceTransTest, SendRelayStartTypeRequest_02, testing::ext::TestSize.Level1)
{
    std::string srcId = "f6d4c08647073e02e7a78f09473aa122ff57fc81c00981fcf5be989e7d112591";
    std::string sinkId = "networkidc08647073e02e7a78f09473aa122ff57fc81c00";
    std::vector<std::string> dhids;
    dhids.push_back("Input_1ds56v18e1v21v8v1erv15r1v8r1j1ty8");
    DistributedInputSourceTransport::DInputSessionInfo dInputSessionInfo;
    dInputSessionInfo.isToSrcSa = true;
    dInputSessionInfo.remoteId = srcId;
    int32_t sessionId = -1;
    DistributedInputSourceTransport::GetInstance().sessionDevMap_[sessionId] = dInputSessionInfo;
    int32_t ret = DistributedInputSourceTransport::GetInstance().SendRelayStartTypeRequest(srcId, sinkId,
        static_cast<uint32_t>(DInputDeviceType::ALL));
    DistributedInputSourceTransport::GetInstance().sessionDevMap_.clear();
    EXPECT_EQ(ERR_DH_INPUT_SERVER_SOURCE_TRANSPORT_START_FAIL, ret);
}

HWTEST_F(DistributedInputSourceTransTest, SendRelayStopTypeRequest_02, testing::ext::TestSize.Level1)
{
    std::string srcId = "f6d4c08647073e02e7a78f09473aa122ff57fc81c00981fcf5be989e7d112591";
    std::string sinkId = "networkidc08647073e02e7a78f09473aa122ff57fc81c00";
    std::vector<std::string> dhids;
    dhids.push_back("Input_1ds56v18e1v21v8v1erv15r1v8r1j1ty8");
    DistributedInputSourceTransport::DInputSessionInfo dInputSessionInfo;
    dInputSessionInfo.isToSrcSa = true;
    dInputSessionInfo.remoteId = srcId;
    int32_t sessionId = -1;
    DistributedInputSourceTransport::GetInstance().sessionDevMap_[sessionId] = dInputSessionInfo;
    int32_t ret = DistributedInputSourceTransport::GetInstance().SendRelayStopTypeRequest(srcId, sinkId,
        static_cast<uint32_t>(DInputDeviceType::ALL));
    DistributedInputSourceTransport::GetInstance().sessionDevMap_.clear();
    EXPECT_EQ(ERR_DH_INPUT_SERVER_SOURCE_TRANSPORT_STOP_FAIL, ret);
}

HWTEST_F(DistributedInputSourceTransTest, JointDhIds01, testing::ext::TestSize.Level0)
{
    std::vector<std::string> dhids;
    std::string ret = DistributedInputSourceTransport::GetInstance().JointDhIds(dhids);
    EXPECT_EQ("", ret);
}

HWTEST_F(DistributedInputSourceTransTest, FindDeviceBySession01, testing::ext::TestSize.Level0)
{
    int32_t sessionId = 0;
    DistributedInputSourceTransport::DInputSessionInfo dInputSessionInfo;
    dInputSessionInfo.isToSrcSa = true;
    DistributedInputSourceTransport::GetInstance().sessionDevMap_[sessionId] = dInputSessionInfo;
    std::string ret = DistributedInputSourceTransport::GetInstance().FindDeviceBySession(sessionId);
    DistributedInputSourceTransport::GetInstance().sessionDevMap_.clear();
    EXPECT_EQ("", ret);
}

HWTEST_F(DistributedInputSourceTransTest, OnSessionOpened01, testing::ext::TestSize.Level0)
{
    int32_t sessionId = 0;
    int32_t result = ERR_DH_INPUT_SERVER_SOURCE_TRANSPORT_STOP_FAIL;
    DistributedInputSourceTransport::DInputSessionInfo dInputSessionInfo;
    dInputSessionInfo.isToSrcSa = true;
    DistributedInputSourceTransport::GetInstance().sessionDevMap_[sessionId] = dInputSessionInfo;
    int32_t ret = DistributedInputSourceTransport::GetInstance().OnSessionOpened(sessionId, result);
    DistributedInputSourceTransport::GetInstance().sessionDevMap_.clear();
    EXPECT_EQ(DH_SUCCESS, ret);
}

HWTEST_F(DistributedInputSourceTransTest, NotifyResponsePrepareRemoteInput01, testing::ext::TestSize.Level0)
{
    int32_t sessionId = 0;
    nlohmann::json recMsg;
    recMsg[DINPUT_SOFTBUS_KEY_RESP_VALUE] = false;
    recMsg[DINPUT_SOFTBUS_KEY_WHITE_LIST] = false;
    DistributedInputSourceTransport::GetInstance().NotifyResponsePrepareRemoteInput(sessionId, recMsg);
    EXPECT_EQ(false, recMsg[DINPUT_SOFTBUS_KEY_WHITE_LIST].is_string());
}

HWTEST_F(DistributedInputSourceTransTest, NotifyResponsePrepareRemoteInput02, testing::ext::TestSize.Level0)
{
    int32_t sessionId = 0;
    nlohmann::json recMsg;
    recMsg[DINPUT_SOFTBUS_KEY_RESP_VALUE] = false;
    recMsg[DINPUT_SOFTBUS_KEY_WHITE_LIST] = "false";
    DistributedInputSourceTransport::GetInstance().NotifyResponsePrepareRemoteInput(sessionId, recMsg);
    EXPECT_EQ(true, recMsg[DINPUT_SOFTBUS_KEY_WHITE_LIST].is_string());
}

HWTEST_F(DistributedInputSourceTransTest, NotifyResponsePrepareRemoteInput03, testing::ext::TestSize.Level0)
{
    int32_t sessionId = 0;
    nlohmann::json recMsg;
    recMsg[DINPUT_SOFTBUS_KEY_RESP_VALUE] = false;
    recMsg[DINPUT_SOFTBUS_KEY_WHITE_LIST] = "false";
    DistributedInputSourceTransport::DInputSessionInfo dInputSessionInfo;
    dInputSessionInfo.isToSrcSa = true;
    dInputSessionInfo.remoteId = "f6d4c08647073e02e7a78f09473aa122ff57fc81c00981fcf5be989e7d112591";
    DistributedInputSourceTransport::GetInstance().sessionDevMap_[sessionId] = dInputSessionInfo;
    DistributedInputSourceManager srcMgr(4810, false);
    std::shared_ptr<DistributedInputSourceManager::DInputSourceListener> srcListener =
        std::make_shared<DistributedInputSourceManager::DInputSourceListener>(&srcMgr);
    DistributedInputSourceTransport::GetInstance().callback_ = srcListener;
    DistributedInputSourceTransport::GetInstance().NotifyResponsePrepareRemoteInput(sessionId, recMsg);
    EXPECT_EQ(true, recMsg[DINPUT_SOFTBUS_KEY_WHITE_LIST].is_string());
    DistributedInputSourceTransport::GetInstance().sessionDevMap_.clear();
}

HWTEST_F(DistributedInputSourceTransTest, NotifyResponsePrepareRemoteInput04, testing::ext::TestSize.Level0)
{
    int32_t sessionId = 0;
    nlohmann::json recMsg;
    recMsg[DINPUT_SOFTBUS_KEY_RESP_VALUE] = "false";
    recMsg[DINPUT_SOFTBUS_KEY_WHITE_LIST] = false;
    DistributedInputSourceTransport::GetInstance().NotifyResponsePrepareRemoteInput(sessionId, recMsg);
    EXPECT_EQ(false, recMsg[DINPUT_SOFTBUS_KEY_WHITE_LIST].is_string());
}

HWTEST_F(DistributedInputSourceTransTest, NotifyResponseUnprepareRemoteInput01, testing::ext::TestSize.Level0)
{
    int32_t sessionId = 0;
    nlohmann::json recMsg;
    recMsg[DINPUT_SOFTBUS_KEY_RESP_VALUE] = "false";
    DistributedInputSourceTransport::GetInstance().NotifyResponseUnprepareRemoteInput(sessionId, recMsg);

    recMsg[DINPUT_SOFTBUS_KEY_RESP_VALUE] = false;
    DistributedInputSourceTransport::GetInstance().NotifyResponseUnprepareRemoteInput(sessionId, recMsg);
    EXPECT_EQ(true, recMsg[DINPUT_SOFTBUS_KEY_RESP_VALUE].is_boolean());
}

HWTEST_F(DistributedInputSourceTransTest, NotifyResponseStartRemoteInput01, testing::ext::TestSize.Level0)
{
    int32_t sessionId = 0;
    nlohmann::json recMsg;
    recMsg[DINPUT_SOFTBUS_KEY_RESP_VALUE] = "false";
    recMsg[DINPUT_SOFTBUS_KEY_INPUT_TYPE] = "input_type";
    DistributedInputSourceTransport::GetInstance().NotifyResponseStartRemoteInput(sessionId, recMsg);

    recMsg[DINPUT_SOFTBUS_KEY_RESP_VALUE] = false;
    DistributedInputSourceTransport::GetInstance().NotifyResponseStartRemoteInput(sessionId, recMsg);

    recMsg[DINPUT_SOFTBUS_KEY_INPUT_TYPE] = 1;
    DistributedInputSourceTransport::GetInstance().NotifyResponseStartRemoteInput(sessionId, recMsg);
    std::string remoteId = "f6d4c08647073e02e7a78f09473aa122ff57fc81c00981fcf5be989e7d112591";
    DistributedInputSourceTransport::DInputSessionInfo dInputSessionInfo {true, remoteId};
    DistributedInputSourceTransport::GetInstance().sessionDevMap_[sessionId] = dInputSessionInfo;
    DistributedInputSourceTransport::GetInstance().NotifyResponseStartRemoteInput(sessionId, recMsg);
    EXPECT_EQ(true, recMsg[DINPUT_SOFTBUS_KEY_RESP_VALUE].is_boolean());
    DistributedInputSourceTransport::GetInstance().sessionDevMap_.clear();
}

HWTEST_F(DistributedInputSourceTransTest, NotifyResponseStopRemoteInput01, testing::ext::TestSize.Level0)
{
    int32_t sessionId = 0;
    nlohmann::json recMsg;
    recMsg[DINPUT_SOFTBUS_KEY_RESP_VALUE] = "false";
    recMsg[DINPUT_SOFTBUS_KEY_INPUT_TYPE] = "input_type";
    DistributedInputSourceTransport::GetInstance().NotifyResponseStopRemoteInput(sessionId, recMsg);

    recMsg[DINPUT_SOFTBUS_KEY_RESP_VALUE] = false;
    DistributedInputSourceTransport::GetInstance().NotifyResponseStopRemoteInput(sessionId, recMsg);

    recMsg[DINPUT_SOFTBUS_KEY_INPUT_TYPE] = 1;
    DistributedInputSourceTransport::GetInstance().NotifyResponseStopRemoteInput(sessionId, recMsg);
    std::string remoteId = "f6d4c08647073e02e7a78f09473aa122ff57fc81c00981fcf5be989e7d112591";
    DistributedInputSourceTransport::DInputSessionInfo dInputSessionInfo {true, remoteId};
    DistributedInputSourceTransport::GetInstance().sessionDevMap_[sessionId] = dInputSessionInfo;
    DistributedInputSourceTransport::GetInstance().NotifyResponseStopRemoteInput(sessionId, recMsg);
    EXPECT_EQ(true, recMsg[DINPUT_SOFTBUS_KEY_RESP_VALUE].is_boolean());
    DistributedInputSourceTransport::GetInstance().sessionDevMap_.clear();
}

HWTEST_F(DistributedInputSourceTransTest, NotifyResponseStartRemoteInputDhid01, testing::ext::TestSize.Level0)
{
    int32_t sessionId = 0;
    nlohmann::json recMsg;
    recMsg[DINPUT_SOFTBUS_KEY_RESP_VALUE] = "false";
    recMsg[DINPUT_SOFTBUS_KEY_VECTOR_DHID] = 1;
    DistributedInputSourceTransport::GetInstance().NotifyResponseStartRemoteInputDhid(sessionId, recMsg);

    recMsg[DINPUT_SOFTBUS_KEY_RESP_VALUE] = false;
    DistributedInputSourceTransport::GetInstance().NotifyResponseStartRemoteInputDhid(sessionId, recMsg);

    recMsg[DINPUT_SOFTBUS_KEY_VECTOR_DHID] = "Input_1ds56v18e1v21v8v1erv15r1v8r1j1ty8";
    DistributedInputSourceTransport::GetInstance().NotifyResponseStartRemoteInputDhid(sessionId, recMsg);
    EXPECT_EQ(true, recMsg[DINPUT_SOFTBUS_KEY_RESP_VALUE].is_boolean());
}

HWTEST_F(DistributedInputSourceTransTest, NotifyResponseStopRemoteInputDhid01, testing::ext::TestSize.Level0)
{
    int32_t sessionId = 0;
    nlohmann::json recMsg;
    recMsg[DINPUT_SOFTBUS_KEY_RESP_VALUE] = "false";
    recMsg[DINPUT_SOFTBUS_KEY_VECTOR_DHID] = 1;
    DistributedInputSourceTransport::GetInstance().NotifyResponseStopRemoteInputDhid(sessionId, recMsg);

    recMsg[DINPUT_SOFTBUS_KEY_RESP_VALUE] = false;
    DistributedInputSourceTransport::GetInstance().NotifyResponseStopRemoteInputDhid(sessionId, recMsg);

    recMsg[DINPUT_SOFTBUS_KEY_VECTOR_DHID] = "Input_1ds56v18e1v21v8v1erv15r1v8r1j1ty8";
    DistributedInputSourceTransport::GetInstance().NotifyResponseStopRemoteInputDhid(sessionId, recMsg);
    EXPECT_EQ(true, recMsg[DINPUT_SOFTBUS_KEY_RESP_VALUE].is_boolean());
}

HWTEST_F(DistributedInputSourceTransTest, NotifyResponseKeyState01, testing::ext::TestSize.Level0)
{
    int32_t sessionId = 0;
    nlohmann::json recMsg;
    recMsg[DINPUT_SOFTBUS_KEY_KEYSTATE_DHID] = 1;
    recMsg[DINPUT_SOFTBUS_KEY_KEYSTATE_TYPE] = "false";
    recMsg[DINPUT_SOFTBUS_KEY_KEYSTATE_CODE] = "false";
    recMsg[DINPUT_SOFTBUS_KEY_KEYSTATE_VALUE] = "false";
    DistributedInputSourceTransport::GetInstance().NotifyResponseKeyState(sessionId, recMsg);

    recMsg[DINPUT_SOFTBUS_KEY_KEYSTATE_DHID] = "Input_1ds56v18e1v21v8v1erv15r1v8r1j1ty8";
    DistributedInputSourceTransport::GetInstance().NotifyResponseKeyState(sessionId, recMsg);

    recMsg[DINPUT_SOFTBUS_KEY_KEYSTATE_TYPE] = 1;
    DistributedInputSourceTransport::GetInstance().NotifyResponseKeyState(sessionId, recMsg);

    recMsg[DINPUT_SOFTBUS_KEY_KEYSTATE_CODE] = 1;

    DistributedInputSourceTransport::GetInstance().NotifyResponseKeyState(sessionId, recMsg);

    recMsg[DINPUT_SOFTBUS_KEY_KEYSTATE_VALUE] = 1;
    DistributedInputSourceTransport::GetInstance().NotifyResponseKeyState(sessionId, recMsg);
    std::string remoteId = "f6d4c08647073e02e7a78f09473aa122ff57fc81c00981fcf5be989e7d112591";
    DistributedInputSourceTransport::DInputSessionInfo dInputSessionInfo {true, remoteId};
    DistributedInputSourceTransport::GetInstance().sessionDevMap_[sessionId] = dInputSessionInfo;
    DistributedInputSourceTransport::GetInstance().NotifyResponseKeyState(sessionId, recMsg);
    EXPECT_EQ(true, recMsg[DINPUT_SOFTBUS_KEY_KEYSTATE_DHID].is_string());
    DistributedInputSourceTransport::GetInstance().sessionDevMap_.clear();
}

HWTEST_F(DistributedInputSourceTransTest, NotifyReceivedEventRemoteInput01, testing::ext::TestSize.Level1)
{
    DistributedInputSourceTransport::GetInstance().sessionDevMap_.clear();
    int32_t sessionId = 1;
    nlohmann::json recMsg;
    int32_t inputData = 100;
    recMsg[DINPUT_SOFTBUS_KEY_INPUT_DATA] = inputData;
    DistributedInputSourceTransport::GetInstance().NotifyReceivedEventRemoteInput(sessionId, recMsg);
    std::string inputDatas = "inputDatas_test";
    recMsg[DINPUT_SOFTBUS_KEY_INPUT_DATA] = inputDatas;
    DistributedInputSourceTransport::GetInstance().NotifyReceivedEventRemoteInput(sessionId, recMsg);
    DistributedInputSourceTransport::GetInstance().CalculateLatency(sessionId, recMsg);
    std::string remoteId = "f6d4c08647073e02e7a78f09473aa122ff57fc81c00981fcf5be989e7d112591";
    DistributedInputSourceTransport::DInputSessionInfo dInputSessionInfo {false, remoteId};
    DistributedInputSourceTransport::GetInstance().sessionDevMap_[sessionId] = dInputSessionInfo;
    DistributedInputSourceTransport::GetInstance().NotifyReceivedEventRemoteInput(sessionId, recMsg);
    DistributedInputSourceTransport::GetInstance().CalculateLatency(sessionId, recMsg);
    EXPECT_EQ(1, DistributedInputSourceTransport::GetInstance().sessionDevMap_.size());
}

HWTEST_F(DistributedInputSourceTransTest, ReceiveSrcTSrcRelayUnprepare01, testing::ext::TestSize.Level1)
{
    int32_t sessionId = 1;
    int32_t devId = 100;
    nlohmann::json recMsg;
    recMsg[DINPUT_SOFTBUS_KEY_DEVICE_ID] = devId;
    DistributedInputSourceTransport::GetInstance().ReceiveSrcTSrcRelayPrepare(sessionId, recMsg);
    DistributedInputSourceTransport::GetInstance().ReceiveSrcTSrcRelayUnprepare(sessionId, recMsg);
    std::string deviceId = "f6d4c08647073e02e7a78f09473aa122ff57fc81c00981fcf5be989e7d112591";
    recMsg[DINPUT_SOFTBUS_KEY_DEVICE_ID] = deviceId;
    DistributedInputSourceTransport::GetInstance().ReceiveSrcTSrcRelayUnprepare(sessionId, recMsg);
    EXPECT_EQ(1, DistributedInputSourceTransport::GetInstance().sessionDevMap_.size());
}

HWTEST_F(DistributedInputSourceTransTest, NotifyResponseRelayPrepareRemoteInput01, testing::ext::TestSize.Level1)
{
    DistributedInputSourceTransport::GetInstance().sessionDevMap_.clear();
    int32_t sessionId = 1;
    nlohmann::json recMsg;
    recMsg[DINPUT_SOFTBUS_KEY_RESP_VALUE] = "false";
    recMsg[DINPUT_SOFTBUS_KEY_SESSION_ID] = "sessionId";
    recMsg[DINPUT_SOFTBUS_KEY_WHITE_LIST] = 1;
    DistributedInputSourceTransport::GetInstance().NotifyResponseRelayPrepareRemoteInput(sessionId, recMsg);
    recMsg[DINPUT_SOFTBUS_KEY_RESP_VALUE] = false;
    DistributedInputSourceTransport::GetInstance().NotifyResponseRelayPrepareRemoteInput(sessionId, recMsg);
    recMsg[DINPUT_SOFTBUS_KEY_SESSION_ID] = sessionId;
    DistributedInputSourceTransport::GetInstance().NotifyResponseRelayPrepareRemoteInput(sessionId, recMsg);
    recMsg[DINPUT_SOFTBUS_KEY_WHITE_LIST] = "white_list_test";
    DistributedInputSourceTransport::GetInstance().NotifyResponseRelayPrepareRemoteInput(sessionId, recMsg);
    EXPECT_EQ(0, DistributedInputSourceTransport::GetInstance().sessionDevMap_.size());
}

HWTEST_F(DistributedInputSourceTransTest, NotifyResponseRelayUnprepareRemoteInput01, testing::ext::TestSize.Level1)
{
    int32_t sessionId = 1;
    std::string remoteId = "f6d4c08647073e02e7a78f09473aa122ff57fc81c00981fcf5be989e7d112591";
    DistributedInputSourceTransport::DInputSessionInfo dInputSessionInfo {false, remoteId};
    DistributedInputSourceTransport::GetInstance().sessionDevMap_[sessionId] = dInputSessionInfo;
    nlohmann::json recMsg;
    recMsg[DINPUT_SOFTBUS_KEY_RESP_VALUE] = "false";
    recMsg[DINPUT_SOFTBUS_KEY_SESSION_ID] = "sessionId";
    DistributedInputSourceTransport::GetInstance().NotifyResponseRelayUnprepareRemoteInput(sessionId, recMsg);
    recMsg[DINPUT_SOFTBUS_KEY_RESP_VALUE] = false;
    DistributedInputSourceTransport::GetInstance().NotifyResponseRelayUnprepareRemoteInput(sessionId, recMsg);
    recMsg[DINPUT_SOFTBUS_KEY_SESSION_ID] = sessionId;
    sessionId = 2;
    DistributedInputSourceTransport::GetInstance().NotifyResponseRelayUnprepareRemoteInput(sessionId, recMsg);
    EXPECT_EQ(1, DistributedInputSourceTransport::GetInstance().sessionDevMap_.size());
    DistributedInputSourceTransport::GetInstance().sessionDevMap_.clear();
}

HWTEST_F(DistributedInputSourceTransTest, ReceiveRelayPrepareResult01, testing::ext::TestSize.Level1)
{
    int32_t sessionId = 1;
    nlohmann::json recMsg;
    recMsg[DINPUT_SOFTBUS_KEY_SRC_DEV_ID] = 100;
    recMsg[DINPUT_SOFTBUS_KEY_SINK_DEV_ID] = 100;
    recMsg[DINPUT_SOFTBUS_KEY_RESP_VALUE] = "resp_value_test";
    DistributedInputSourceTransport::GetInstance().ReceiveRelayPrepareResult(sessionId, recMsg);
    recMsg[DINPUT_SOFTBUS_KEY_SRC_DEV_ID] = "src_devid_test";
    DistributedInputSourceTransport::GetInstance().ReceiveRelayPrepareResult(sessionId, recMsg);
    recMsg[DINPUT_SOFTBUS_KEY_SINK_DEV_ID] = "sink_devid_test";
    DistributedInputSourceTransport::GetInstance().ReceiveRelayPrepareResult(sessionId, recMsg);
}

HWTEST_F(DistributedInputSourceTransTest, ReceiveRelayUnprepareResult01, testing::ext::TestSize.Level1)
{
    int32_t sessionId = 1;
    nlohmann::json recMsg;
    recMsg[DINPUT_SOFTBUS_KEY_SRC_DEV_ID] = 100;
    recMsg[DINPUT_SOFTBUS_KEY_SINK_DEV_ID] = 100;
    recMsg[DINPUT_SOFTBUS_KEY_RESP_VALUE] = "resp_value_test";
    DistributedInputSourceTransport::GetInstance().ReceiveRelayUnprepareResult(sessionId, recMsg);
    recMsg[DINPUT_SOFTBUS_KEY_SRC_DEV_ID] = "src_devid_test";
    DistributedInputSourceTransport::GetInstance().ReceiveRelayUnprepareResult(sessionId, recMsg);
    recMsg[DINPUT_SOFTBUS_KEY_SINK_DEV_ID] = "sink_devid_test";
    DistributedInputSourceTransport::GetInstance().ReceiveRelayUnprepareResult(sessionId, recMsg);
}

HWTEST_F(DistributedInputSourceTransTest, ReceiveSrcTSrcRelayStartDhid01, testing::ext::TestSize.Level1)
{
    int32_t sessionId = 1;
    std::string deviceId = "f6d4c08647073e02e7a78f09473aa122ff57fc81c00981fcf5be989e7d112591";
    DistributedInputSourceTransport::DInputSessionInfo dInputSessionInfo {false, deviceId};
    DistributedInputSourceTransport::GetInstance().sessionDevMap_[sessionId] = dInputSessionInfo;
    nlohmann::json recMsg;
    recMsg[DINPUT_SOFTBUS_KEY_DEVICE_ID] = 100;
    recMsg[DINPUT_SOFTBUS_KEY_VECTOR_DHID] = 200;
    DistributedInputSourceTransport::GetInstance().ReceiveSrcTSrcRelayStartDhid(sessionId, recMsg);
    recMsg[DINPUT_SOFTBUS_KEY_DEVICE_ID] = deviceId;
    DistributedInputSourceTransport::GetInstance().ReceiveSrcTSrcRelayStartDhid(sessionId, recMsg);
    recMsg[DINPUT_SOFTBUS_KEY_VECTOR_DHID] = "vector_dhId_test";
    DistributedInputSourceTransport::GetInstance().ReceiveSrcTSrcRelayStartDhid(sessionId, recMsg);
    EXPECT_EQ(1, DistributedInputSourceTransport::GetInstance().sessionDevMap_.size());
    DistributedInputSourceTransport::GetInstance().sessionDevMap_.clear();
    DistributedInputSourceTransport::GetInstance().ReceiveSrcTSrcRelayStartDhid(sessionId, recMsg);
    EXPECT_EQ(0, DistributedInputSourceTransport::GetInstance().sessionDevMap_.size());
}

HWTEST_F(DistributedInputSourceTransTest, ReceiveSrcTSrcRelayStopDhid01, testing::ext::TestSize.Level1)
{
    int32_t sessionId = 1;
    std::string deviceId = "f6d4c08647073e02e7a78f09473aa122ff57fc81c00981fcf5be989e7d112591";
    nlohmann::json recMsg;
    recMsg[DINPUT_SOFTBUS_KEY_DEVICE_ID] = 100;
    recMsg[DINPUT_SOFTBUS_KEY_VECTOR_DHID] = 200;
    DistributedInputSourceTransport::GetInstance().ReceiveSrcTSrcRelayStopDhid(sessionId, recMsg);
    recMsg[DINPUT_SOFTBUS_KEY_DEVICE_ID] = deviceId;
    DistributedInputSourceTransport::GetInstance().ReceiveSrcTSrcRelayStopDhid(sessionId, recMsg);
    recMsg[DINPUT_SOFTBUS_KEY_VECTOR_DHID] = "vector_dhId_test";
    DistributedInputSourceTransport::GetInstance().ReceiveSrcTSrcRelayStopDhid(sessionId, recMsg);
    EXPECT_EQ(0, DistributedInputSourceTransport::GetInstance().sessionDevMap_.size());
    DistributedInputSourceTransport::DInputSessionInfo dInputSessionInfo {false, deviceId};
    DistributedInputSourceTransport::GetInstance().sessionDevMap_[sessionId] = dInputSessionInfo;
    DistributedInputSourceTransport::GetInstance().ReceiveSrcTSrcRelayStopDhid(sessionId, recMsg);
    EXPECT_EQ(1, DistributedInputSourceTransport::GetInstance().sessionDevMap_.size());
}

HWTEST_F(DistributedInputSourceTransTest, NotifyResponseRelayStartDhidRemoteInput01, testing::ext::TestSize.Level1)
{
    int32_t sessionId = 1;
    std::string deviceId = "f6d4c08647073e02e7a78f09473aa122ff57fc81c00981fcf5be989e7d112591";
    nlohmann::json recMsg;
    recMsg[DINPUT_SOFTBUS_KEY_RESP_VALUE] = "false";
    recMsg[DINPUT_SOFTBUS_KEY_SESSION_ID] = "1";
    recMsg[DINPUT_SOFTBUS_KEY_VECTOR_DHID] = 1;
    DistributedInputSourceTransport::GetInstance().NotifyResponseRelayStartDhidRemoteInput(sessionId, recMsg);
    recMsg[DINPUT_SOFTBUS_KEY_RESP_VALUE] = false;
    DistributedInputSourceTransport::GetInstance().NotifyResponseRelayStartDhidRemoteInput(sessionId, recMsg);
    recMsg[DINPUT_SOFTBUS_KEY_SESSION_ID] = sessionId;
    DistributedInputSourceTransport::GetInstance().NotifyResponseRelayStartDhidRemoteInput(sessionId, recMsg);
    recMsg[DINPUT_SOFTBUS_KEY_VECTOR_DHID] = "vector_dhId_test";
    DistributedInputSourceTransport::GetInstance().NotifyResponseRelayStartDhidRemoteInput(sessionId, recMsg);
    DistributedInputSourceTransport::GetInstance().sessionDevMap_.clear();
    DistributedInputSourceTransport::GetInstance().NotifyResponseRelayStartDhidRemoteInput(sessionId, recMsg);
    EXPECT_EQ(0, DistributedInputSourceTransport::GetInstance().sessionDevMap_.size());
}

HWTEST_F(DistributedInputSourceTransTest, NotifyResponseRelayStopDhidRemoteInput01, testing::ext::TestSize.Level1)
{
    int32_t sessionId = 1;
    std::string deviceId = "f6d4c08647073e02e7a78f09473aa122ff57fc81c00981fcf5be989e7d112591";
    nlohmann::json recMsg;
    recMsg[DINPUT_SOFTBUS_KEY_RESP_VALUE] = "false";
    recMsg[DINPUT_SOFTBUS_KEY_SESSION_ID] = "1";
    recMsg[DINPUT_SOFTBUS_KEY_VECTOR_DHID] = 1;
    DistributedInputSourceTransport::GetInstance().NotifyResponseRelayStopDhidRemoteInput(sessionId, recMsg);
    recMsg[DINPUT_SOFTBUS_KEY_RESP_VALUE] = false;
    DistributedInputSourceTransport::GetInstance().NotifyResponseRelayStopDhidRemoteInput(sessionId, recMsg);
    recMsg[DINPUT_SOFTBUS_KEY_SESSION_ID] = sessionId;
    DistributedInputSourceTransport::GetInstance().NotifyResponseRelayStopDhidRemoteInput(sessionId, recMsg);
    recMsg[DINPUT_SOFTBUS_KEY_VECTOR_DHID] = "vector_dhId_test";
    DistributedInputSourceTransport::GetInstance().NotifyResponseRelayStopDhidRemoteInput(sessionId, recMsg);
    DistributedInputSourceTransport::DInputSessionInfo dInputSessionInfo {false, deviceId};
    DistributedInputSourceTransport::GetInstance().sessionDevMap_[sessionId] = dInputSessionInfo;
    DistributedInputSourceTransport::GetInstance().NotifyResponseRelayStopDhidRemoteInput(sessionId, recMsg);
    EXPECT_EQ(1, DistributedInputSourceTransport::GetInstance().sessionDevMap_.size());
}

HWTEST_F(DistributedInputSourceTransTest, ReceiveRelayStartDhidResult01, testing::ext::TestSize.Level1)
{
    int32_t sessionId = 1;
    nlohmann::json recMsg;
    recMsg[DINPUT_SOFTBUS_KEY_SRC_DEV_ID] = 100;
    recMsg[DINPUT_SOFTBUS_KEY_SINK_DEV_ID] = 200;
    recMsg[DINPUT_SOFTBUS_KEY_RESP_VALUE] = "value";
    recMsg[DINPUT_SOFTBUS_KEY_VECTOR_DHID] = 300;
    DistributedInputSourceTransport::GetInstance().ReceiveRelayStartDhidResult(sessionId, recMsg);
    recMsg[DINPUT_SOFTBUS_KEY_SRC_DEV_ID] ="src_devId_test";
    DistributedInputSourceTransport::GetInstance().ReceiveRelayStartDhidResult(sessionId, recMsg);
    recMsg[DINPUT_SOFTBUS_KEY_SINK_DEV_ID] = "sink_devId_test";
    DistributedInputSourceTransport::GetInstance().ReceiveRelayStartDhidResult(sessionId, recMsg);
    recMsg[DINPUT_SOFTBUS_KEY_RESP_VALUE] = 1;
    DistributedInputSourceTransport::GetInstance().ReceiveRelayStartDhidResult(sessionId, recMsg);
    recMsg[DINPUT_SOFTBUS_KEY_VECTOR_DHID] = "vector_dhId_test";
    DistributedInputSourceTransport::GetInstance().ReceiveRelayStartDhidResult(sessionId, recMsg);
    EXPECT_EQ("vector_dhId_test", recMsg[DINPUT_SOFTBUS_KEY_VECTOR_DHID]);
}

HWTEST_F(DistributedInputSourceTransTest, ReceiveRelayStopDhidResult01, testing::ext::TestSize.Level1)
{
    int32_t sessionId = 1;
    nlohmann::json recMsg;
    recMsg[DINPUT_SOFTBUS_KEY_SRC_DEV_ID] = 100;
    recMsg[DINPUT_SOFTBUS_KEY_SINK_DEV_ID] = 200;
    recMsg[DINPUT_SOFTBUS_KEY_RESP_VALUE] = "value";
    recMsg[DINPUT_SOFTBUS_KEY_VECTOR_DHID] = 300;
    DistributedInputSourceTransport::GetInstance().ReceiveRelayStopDhidResult(sessionId, recMsg);
    recMsg[DINPUT_SOFTBUS_KEY_SRC_DEV_ID] ="src_devId_test";
    DistributedInputSourceTransport::GetInstance().ReceiveRelayStopDhidResult(sessionId, recMsg);
    recMsg[DINPUT_SOFTBUS_KEY_SINK_DEV_ID] = "sink_devId_test";
    DistributedInputSourceTransport::GetInstance().ReceiveRelayStopDhidResult(sessionId, recMsg);
    recMsg[DINPUT_SOFTBUS_KEY_RESP_VALUE] = 1;
    DistributedInputSourceTransport::GetInstance().ReceiveRelayStopDhidResult(sessionId, recMsg);
    recMsg[DINPUT_SOFTBUS_KEY_VECTOR_DHID] = "vector_dhId_test";
    DistributedInputSourceTransport::GetInstance().ReceiveRelayStopDhidResult(sessionId, recMsg);
    EXPECT_EQ("vector_dhId_test", recMsg[DINPUT_SOFTBUS_KEY_VECTOR_DHID]);
}

HWTEST_F(DistributedInputSourceTransTest, ReceiveSrcTSrcRelayStartType01, testing::ext::TestSize.Level1)
{
    int32_t sessionId = 1;
    std::string deviceId = "f6d4c08647073e02e7a78f09473aa122ff57fc81c00981fcf5be989e7d112591";
    nlohmann::json recMsg;
    recMsg[DINPUT_SOFTBUS_KEY_DEVICE_ID] = 1;
    recMsg[DINPUT_SOFTBUS_KEY_INPUT_TYPE] = "input_type_test";
    DistributedInputSourceTransport::GetInstance().ReceiveSrcTSrcRelayStartType(sessionId, recMsg);
    recMsg[DINPUT_SOFTBUS_KEY_DEVICE_ID] = deviceId;
    DistributedInputSourceTransport::GetInstance().ReceiveSrcTSrcRelayStartType(sessionId, recMsg);
    recMsg[DINPUT_SOFTBUS_KEY_INPUT_TYPE] = 1;
    DistributedInputSourceTransport::GetInstance().ReceiveSrcTSrcRelayStartType(sessionId, recMsg);
    DistributedInputSourceTransport::GetInstance().sessionDevMap_.clear();
    DistributedInputSourceTransport::GetInstance().ReceiveSrcTSrcRelayStartType(sessionId, recMsg);
    EXPECT_EQ(0, DistributedInputSourceTransport::GetInstance().sessionDevMap_.size());
}

HWTEST_F(DistributedInputSourceTransTest, ReceiveSrcTSrcRelayStopType01, testing::ext::TestSize.Level1)
{
    int32_t sessionId = 1;
    std::string deviceId = "f6d4c08647073e02e7a78f09473aa122ff57fc81c00981fcf5be989e7d112591";
    nlohmann::json recMsg;
    recMsg[DINPUT_SOFTBUS_KEY_DEVICE_ID] = 1;
    recMsg[DINPUT_SOFTBUS_KEY_INPUT_TYPE] = "input_type_test";
    DistributedInputSourceTransport::GetInstance().ReceiveSrcTSrcRelayStopType(sessionId, recMsg);
    recMsg[DINPUT_SOFTBUS_KEY_DEVICE_ID] = deviceId;
    DistributedInputSourceTransport::GetInstance().ReceiveSrcTSrcRelayStopType(sessionId, recMsg);
    recMsg[DINPUT_SOFTBUS_KEY_INPUT_TYPE] = 1;
    DistributedInputSourceTransport::GetInstance().ReceiveSrcTSrcRelayStopType(sessionId, recMsg);
    DistributedInputSourceTransport::DInputSessionInfo dInputSessionInfo {false, deviceId};
    DistributedInputSourceTransport::GetInstance().sessionDevMap_[sessionId] = dInputSessionInfo;
    DistributedInputSourceTransport::GetInstance().ReceiveSrcTSrcRelayStopType(sessionId, recMsg);
    EXPECT_EQ(1, DistributedInputSourceTransport::GetInstance().sessionDevMap_.size());
}

HWTEST_F(DistributedInputSourceTransTest, NotifyResponseRelayStartTypeRemoteInput01, testing::ext::TestSize.Level1)
{
    int32_t sessionId = 1;
    nlohmann::json recMsg;
    recMsg[DINPUT_SOFTBUS_KEY_RESP_VALUE] = "false";
    recMsg[DINPUT_SOFTBUS_KEY_SESSION_ID] = "1";
    recMsg[DINPUT_SOFTBUS_KEY_INPUT_TYPE] = "input_type_test";
    DistributedInputSourceTransport::GetInstance().NotifyResponseRelayStartTypeRemoteInput(sessionId, recMsg);
    recMsg[DINPUT_SOFTBUS_KEY_RESP_VALUE] = false;
    DistributedInputSourceTransport::GetInstance().NotifyResponseRelayStartTypeRemoteInput(sessionId, recMsg);
    recMsg[DINPUT_SOFTBUS_KEY_SESSION_ID] = sessionId;
    DistributedInputSourceTransport::GetInstance().NotifyResponseRelayStartTypeRemoteInput(sessionId, recMsg);
    recMsg[DINPUT_SOFTBUS_KEY_INPUT_TYPE] = 1;
    DistributedInputSourceTransport::GetInstance().NotifyResponseRelayStartTypeRemoteInput(sessionId, recMsg);
    DistributedInputSourceTransport::GetInstance().sessionDevMap_.clear();
    DistributedInputSourceTransport::GetInstance().NotifyResponseRelayStartTypeRemoteInput(sessionId, recMsg);
    EXPECT_EQ(0, DistributedInputSourceTransport::GetInstance().sessionDevMap_.size());
}

HWTEST_F(DistributedInputSourceTransTest, NotifyResponseRelayStopTypeRemoteInput01, testing::ext::TestSize.Level1)
{
    int32_t sessionId = 1;
    std::string deviceId = "f6d4c08647073e02e7a78f09473aa122ff57fc81c00981fcf5be989e7d112591";
    nlohmann::json recMsg;
    recMsg[DINPUT_SOFTBUS_KEY_RESP_VALUE] = "false";
    recMsg[DINPUT_SOFTBUS_KEY_SESSION_ID] = "1";
    recMsg[DINPUT_SOFTBUS_KEY_INPUT_TYPE] = "input_type_test";
    DistributedInputSourceTransport::GetInstance().NotifyResponseRelayStopTypeRemoteInput(sessionId, recMsg);
    recMsg[DINPUT_SOFTBUS_KEY_RESP_VALUE] = false;
    DistributedInputSourceTransport::GetInstance().NotifyResponseRelayStopTypeRemoteInput(sessionId, recMsg);
    recMsg[DINPUT_SOFTBUS_KEY_SESSION_ID] = sessionId;
    DistributedInputSourceTransport::GetInstance().NotifyResponseRelayStopTypeRemoteInput(sessionId, recMsg);
    recMsg[DINPUT_SOFTBUS_KEY_INPUT_TYPE] = 1;
    DistributedInputSourceTransport::GetInstance().NotifyResponseRelayStopTypeRemoteInput(sessionId, recMsg);
    DistributedInputSourceTransport::DInputSessionInfo dInputSessionInfo {false, deviceId};
    DistributedInputSourceTransport::GetInstance().sessionDevMap_[sessionId] = dInputSessionInfo;
    DistributedInputSourceTransport::GetInstance().NotifyResponseRelayStopTypeRemoteInput(sessionId, recMsg);
    EXPECT_EQ(1, DistributedInputSourceTransport::GetInstance().sessionDevMap_.size());
}

HWTEST_F(DistributedInputSourceTransTest, HandleSessionData01, testing::ext::TestSize.Level1)
{
    int32_t sessionId = 1;
    std::string message = "";
    DistributedInputSourceTransport::GetInstance().HandleSessionData(sessionId, message);
    message = "message_test";
    DistributedInputSourceTransport::GetInstance().HandleSessionData(sessionId, message);
    nlohmann::json recMsg;
    recMsg[DINPUT_SOFTBUS_KEY_CMD_TYPE] = "cmd_type_test";
    DistributedInputSourceTransport::GetInstance().HandleSessionData(sessionId, recMsg.dump());
    recMsg[DINPUT_SOFTBUS_KEY_CMD_TYPE] = 1;
    DistributedInputSourceTransport::GetInstance().HandleSessionData(sessionId, recMsg.dump());
    DistributedInputSourceTransport::GetInstance().callback_ = nullptr;
    DistributedInputSourceTransport::GetInstance().HandleSessionData(sessionId, recMsg.dump());
    EXPECT_EQ(1, DistributedInputSourceTransport::GetInstance().sessionDevMap_.size());
}

HWTEST_F(DistributedInputSourceTransTest, OnBytesReceived01, testing::ext::TestSize.Level1)
{
    int32_t sessionId = -1;
    char *data = nullptr;
    uint32_t dataLen = -1;
    DistributedInputSourceTransport::GetInstance().OnBytesReceived(sessionId, data, dataLen);
    sessionId = 1;
    DistributedInputSourceTransport::GetInstance().OnBytesReceived(sessionId, data, dataLen);
    char dataMsg[10] = "dataMsg";
    DistributedInputSourceTransport::GetInstance().OnBytesReceived(sessionId, dataMsg, dataLen);
    dataLen = 1;
    DistributedInputSourceTransport::GetInstance().OnBytesReceived(sessionId, dataMsg, dataLen);
    EXPECT_EQ(1, DistributedInputSourceTransport::GetInstance().sessionDevMap_.size());
}

HWTEST_F(DistributedInputSourceTransTest, SendMsg01, testing::ext::TestSize.Level1)
{
    int32_t sessionId = 1;
    std::string message(MSG_MAX_SIZE + 1, 'a');
    int32_t ret = DistributedInputSourceTransport::GetInstance().SendMsg(sessionId, message);
    EXPECT_EQ(ERR_DH_INPUT_SERVER_SOURCE_TRANSPORT_SENDMESSSAGE, ret);

    std::string messageData(MSG_MAX_SIZE - 1, 'a');
    ret = DistributedInputSourceTransport::GetInstance().SendMsg(sessionId, messageData);
    EXPECT_EQ(DH_SUCCESS, ret);
}
} // namespace DistributedInput
} // namespace DistributedHardware
} // namespace OHOS
