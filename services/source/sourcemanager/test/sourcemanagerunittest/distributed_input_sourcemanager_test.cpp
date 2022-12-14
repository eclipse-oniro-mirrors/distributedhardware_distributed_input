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

#include "distributed_input_sourcemanager_test.h"

#include <cstdlib>
#include <fcntl.h>
#include <iostream>
#include <thread>
#include <unistd.h>

#include <linux/input.h>

#include "event_handler.h"
#include "if_system_ability_manager.h"
#include "iservice_registry.h"
#include "nlohmann/json.hpp"
#include "system_ability_definition.h"

#include "dinput_errcode.h"
#include "distributed_input_inject.h"
#include "distributed_input_source_transport.h"

using namespace testing::ext;
using namespace OHOS::DistributedHardware::DistributedInput;
using namespace std;
namespace OHOS {
namespace DistributedHardware {
namespace DistributedInput {
void DistributedInputSourceManagerTest::SetUp()
{
    sourceManager_ = new DistributedInputSourceManager(DISTRIBUTED_HARDWARE_INPUT_SOURCE_SA_ID, true);
    statuslistener_ = std::make_shared<DistributedInputSourceManager::DInputSourceListener>(sourceManager_);
    DistributedInputSourceManagerTest::RegisterSourceRespCallback(statuslistener_);
}

void DistributedInputSourceManagerTest::TearDown()
{
}

void DistributedInputSourceManagerTest::SetUpTestCase()
{
}

void DistributedInputSourceManagerTest::TearDownTestCase()
{
}

void DistributedInputSourceManagerTest::RegisterSourceRespCallback(std::shared_ptr<DInputSourceTransCallback> callback)
{
    callback_ = callback;
    return;
}

void DistributedInputSourceManagerTest::TestRegisterDInputCb::OnResult(
    const std::string& devId, const std::string& dhId, const int32_t& status)
{
    (void)devId;
    (void)dhId;
    (void)status;
    return;
}

void DistributedInputSourceManagerTest::TestUnregisterDInputCb::OnResult(
    const std::string& devId, const std::string& dhId, const int32_t& status)
{
    (void)devId;
    (void)dhId;
    (void)status;
    return;
}

void DistributedInputSourceManagerTest::TestPrepareDInputCallback::OnResult(
    const std::string& deviceId, const int32_t& status)
{
    (void)deviceId;
    (void)status;
    return;
}

void DistributedInputSourceManagerTest::TestUnprepareDInputCallback::OnResult(
    const std::string& deviceId, const int32_t& status)
{
    (void)deviceId;
    (void)status;
    return;
}

void DistributedInputSourceManagerTest::TestStartDInputCallback::OnResult(
    const std::string& deviceId, const uint32_t& inputTypes, const int32_t& status)
{
    (void)deviceId;
    (void)inputTypes;
    (void)status;
    return;
}

void DistributedInputSourceManagerTest::TestStopDInputCallback::OnResult(
    const std::string& deviceId, const uint32_t& inputTypes, const int32_t& status)
{
    (void)deviceId;
    (void)inputTypes;
    (void)status;
    return;
}

void DistributedInputSourceManagerTest::TestStartStopVectorCallbackStub::OnResultDhids(const std::string &devId,
    const int32_t &status)
{
    (void)devId;
    (void)status;
    return;
}

void DistributedInputSourceManagerTest::TestAddWhiteListInfosCb::OnResult(
    const std::string &deviceId, const std::string &strJson)
{
    (void)deviceId;
    (void)strJson;
    return;
}

void DistributedInputSourceManagerTest::TestDelWhiteListInfosCb::OnResult(
    const std::string& deviceId)
{
    (void)deviceId;
    return;
}

void DistributedInputSourceManagerTest::TestStartStopResultCb::OnStart(const std::string &srcId,
    const std::string &sinkId, std::vector<std::string> &devData)
{
    (void)srcId;
    (void)sinkId;
    (void)devData;
    return;
}

void DistributedInputSourceManagerTest::TestStartStopResultCb::OnStop(const std::string &srcId,
    const std::string &sinkId, std::vector<std::string> &devData)
{
    (void)srcId;
    (void)sinkId;
    (void)devData;
    return;
}

void DistributedInputSourceManagerTest::TestInputNodeListenerCb::OnNodeOnLine(const std::string srcDevId,
    const std::string sinkDevId, const std::string sinkNodeId, const std::string sinkNodeDesc)
{
    (void)srcDevId;
    (void)sinkDevId;
    (void)sinkNodeId;
    (void)sinkNodeDesc;
    return;
}

void DistributedInputSourceManagerTest::TestInputNodeListenerCb::OnNodeOffLine(const std::string srcDevId,
    const std::string sinkDevId, const std::string sinkNodeId)
{
    (void)srcDevId;
    (void)sinkDevId;
    (void)sinkNodeId;
    return;
}

int32_t DistributedInputSourceManagerTest::TestSimulationEventCb::OnSimulationEvent(uint32_t type, uint32_t code,
    int32_t value)
{
    (void)type;
    (void)code;
    (void)value;
    return DH_SUCCESS;
}

void DistributedInputSourceManagerTest::TestStartStopDInputsCb::OnResultDhids(const std::string &devId,
    const int32_t &status)
{
    (void)devId;
    (void)status;
    return;
}

int32_t DistributedInputSourceManagerTest::StructTransJson(const InputDevice& pBuf, std::string& strDescriptor) const
{
    nlohmann::json tmpJson;
    tmpJson["name"] = pBuf.name;
    tmpJson["physicalPath"] = pBuf.physicalPath;
    tmpJson["uniqueId"] = pBuf.uniqueId;
    tmpJson["bus"] = pBuf.bus;
    tmpJson["vendor"] = pBuf.vendor;
    tmpJson["product"] = pBuf.product;
    tmpJson["version"] = pBuf.version;
    tmpJson["descriptor"] = pBuf.descriptor;
    tmpJson["classes"] = pBuf.classes;

    std::ostringstream stream;
    stream << tmpJson.dump();
    strDescriptor = stream.str();
    return DH_SUCCESS;
}

HWTEST_F(DistributedInputSourceManagerTest, Init01, testing::ext::TestSize.Level0)
{
    int32_t ret = sourceManager_->Init();
    EXPECT_EQ(DH_SUCCESS, ret);
}

HWTEST_F(DistributedInputSourceManagerTest, RegisterDistributedHardware_01, testing::ext::TestSize.Level1)
{
    std::string devId = "";
    std::string dhId = "1ds56v18e1v21v8v1erv15r1v8r1j1ty8";
    std::string parameters = "";
    sptr<TestRegisterDInputCb> callback = new TestRegisterDInputCb();
    int32_t ret = sourceManager_->RegisterDistributedHardware(devId, dhId, parameters, callback);
    EXPECT_EQ(ERR_DH_INPUT_SERVER_SOURCE_MANAGER_REGISTER_FAIL, ret);
}

/**
 * @tc.name: PrepareRemoteInput
 * @tc.desc: verify the function of distributing data from any device.
 * @tc.type: FUNC
 * @tc.require: SR000H9J75
 */
HWTEST_F(DistributedInputSourceManagerTest, PrepareRemoteInput_01, testing::ext::TestSize.Level1)
{
    std::string devId = "";
    sptr<TestPrepareDInputCallback> callback = new TestPrepareDInputCallback();
    int32_t ret = sourceManager_->PrepareRemoteInput(devId, callback);
    EXPECT_EQ(ERR_DH_INPUT_SERVER_SOURCE_MANAGER_PREPARE_FAIL, ret);
}

HWTEST_F(DistributedInputSourceManagerTest, UnprepareRemoteInput_01, testing::ext::TestSize.Level1)
{
    std::string devId = "";
    sptr<TestUnprepareDInputCallback> callback = new TestUnprepareDInputCallback();
    int32_t ret = sourceManager_->UnprepareRemoteInput(devId, callback);
    EXPECT_EQ(ERR_DH_INPUT_SERVER_SOURCE_MANAGER_UNPREPARE_FAIL, ret);
}

HWTEST_F(DistributedInputSourceManagerTest, UnprepareRemoteInput_02, testing::ext::TestSize.Level1)
{
    std::string devId = "umkyu1b165e1be98151891erbe8r91ev";
    int32_t sessionId = 1;
    sptr<TestUnprepareDInputCallback> callback = new TestUnprepareDInputCallback();
    DistributedInputSourceTransport::DInputSessionInfo sessionInfo{false, devId};
    DistributedInputSourceTransport::GetInstance().sessionDevMap_[sessionId] = sessionInfo;
    int32_t ret = sourceManager_->UnprepareRemoteInput(devId, callback);
    EXPECT_EQ(DH_SUCCESS, ret);
}

/**
 * @tc.name: StartRemoteInput
 * @tc.desc: verify the function of starting distributed input on InputDeviceType.
 * @tc.type: FUNC
 * @tc.require: SR000H9J75
 */
HWTEST_F(DistributedInputSourceManagerTest, StartRemoteInput_01, testing::ext::TestSize.Level1)
{
    std::string devId = "";
    sptr<TestStartDInputCallback> callback = new TestStartDInputCallback();
    int32_t ret = sourceManager_->StartRemoteInput(devId, static_cast<uint32_t>(DInputDeviceType::ALL), callback);
    EXPECT_EQ(ERR_DH_INPUT_SERVER_SOURCE_MANAGER_START_FAIL, ret);
}

HWTEST_F(DistributedInputSourceManagerTest, StartRemoteInput_02, testing::ext::TestSize.Level1)
{
    std::string devId = "umkyu1b165e1be98151891erbe8r91ev";
    sptr<TestStartDInputCallback> callback = new TestStartDInputCallback();
    int32_t ret = sourceManager_->StartRemoteInput(devId, static_cast<uint32_t>(DInputDeviceType::ALL), callback);
    EXPECT_EQ(DH_SUCCESS, ret);
}

/**
 * @tc.name: StopRemoteInput
 * @tc.desc: verify the function of stoping distributed input on InputDeviceType.
 * @tc.type: FUNC
 * @tc.require: SR000H9J75
 */
HWTEST_F(DistributedInputSourceManagerTest, StopRemoteInput_01, testing::ext::TestSize.Level1)
{
    std::string devId = "";
    sptr<TestStopDInputCallback> callback = new TestStopDInputCallback();
    int32_t ret = sourceManager_->StopRemoteInput(devId, static_cast<uint32_t>(DInputDeviceType::ALL), callback);
    EXPECT_EQ(ERR_DH_INPUT_SERVER_SOURCE_MANAGER_STOP_FAIL, ret);
}

HWTEST_F(DistributedInputSourceManagerTest, StopRemoteInput_02, testing::ext::TestSize.Level1)
{
    std::string devId = "umkyu1b165e1be98151891erbe8r91ev";
    sptr<TestStopDInputCallback> callback = new TestStopDInputCallback();
    int32_t ret = sourceManager_->StopRemoteInput(devId, static_cast<uint32_t>(DInputDeviceType::ALL), callback);
    EXPECT_EQ(DH_SUCCESS, ret);
}

/**
 * @tc.name: StartRemoteInput
 * @tc.desc: verify the function of starting distributed input on InputDeviceType.
 * @tc.type: FUNC
 * @tc.require: SR000H9J75
 */
HWTEST_F(DistributedInputSourceManagerTest, StartRemoteInput_03, testing::ext::TestSize.Level1)
{
    std::string srcId = "";
    std::string sinkId = "";
    sptr<TestStartDInputCallback> callback = new TestStartDInputCallback();
    int32_t ret = sourceManager_->StartRemoteInput(srcId, sinkId,
        static_cast<uint32_t>(DInputDeviceType::ALL), callback);
    EXPECT_EQ(ERR_DH_INPUT_SERVER_SOURCE_MANAGER_START_FAIL, ret);
}

HWTEST_F(DistributedInputSourceManagerTest, StartRemoteInput_04, testing::ext::TestSize.Level1)
{
    std::string srcId = "networkidc08647073e02e7a78f09473aa122ff57fc81c00";
    std::string sinkId = "umkyu1b165e1be98151891erbe8r91ev";
    sptr<TestStartDInputCallback> callback = new TestStartDInputCallback();
    int32_t ret = sourceManager_->StartRemoteInput(srcId, sinkId,
        static_cast<uint32_t>(DInputDeviceType::ALL), callback);
    EXPECT_EQ(DH_SUCCESS, ret);
}

/**
 * @tc.name: StopRemoteInput
 * @tc.desc: verify the function of stoping distributed input on InputDeviceType.
 * @tc.type: FUNC
 * @tc.require: SR000H9J75
 */
HWTEST_F(DistributedInputSourceManagerTest, StopRemoteInput_03, testing::ext::TestSize.Level1)
{
    std::string srcId = "";
    std::string sinkId = "";
    sptr<TestStopDInputCallback> callback = new TestStopDInputCallback();
    int32_t ret = sourceManager_->StopRemoteInput(srcId, sinkId,
        static_cast<uint32_t>(DInputDeviceType::ALL), callback);
    EXPECT_EQ(ERR_DH_INPUT_SERVER_SOURCE_MANAGER_STOP_FAIL, ret);
}

HWTEST_F(DistributedInputSourceManagerTest, StopRemoteInput_04, testing::ext::TestSize.Level1)
{
    std::string srcId = "networkidc08647073e02e7a78f09473aa122ff57fc81c00";
    std::string sinkId = "umkyu1b165e1be98151891erbe8r91ev";
    sptr<TestStopDInputCallback> callback = new TestStopDInputCallback();
    int32_t ret = sourceManager_->StopRemoteInput(srcId, sinkId,
        static_cast<uint32_t>(DInputDeviceType::ALL), callback);
    EXPECT_EQ(DH_SUCCESS, ret);
}

/**
 * @tc.name: PrepareRemoteInput
 * @tc.desc: verify the function of distributing data from any device.
 * @tc.type: FUNC
 * @tc.require: SR000H9J77
 */
HWTEST_F(DistributedInputSourceManagerTest, PrepareRemoteInput_02, testing::ext::TestSize.Level1)
{
    std::string srcId = "";
    std::string sinkId = "";
    sptr<TestPrepareDInputCallback> callback = new TestPrepareDInputCallback();
    int32_t ret = sourceManager_->PrepareRemoteInput(srcId, sinkId, callback);
    EXPECT_EQ(ERR_DH_INPUT_SERVER_SOURCE_MANAGER_PREPARE_FAIL, ret);
}

/**
 * @tc.name: UnprepareRemoteInput
 * @tc.desc: verify the function of disabling a peripheral device.
 * @tc.type: FUNC
 * @tc.require: SR000H9J77
 */
HWTEST_F(DistributedInputSourceManagerTest, UnprepareRemoteInput_03, testing::ext::TestSize.Level1)
{
    std::string srcId = "";
    std::string sinkId = "";
    sptr<TestUnprepareDInputCallback> callback = new TestUnprepareDInputCallback();
    int32_t ret = sourceManager_->UnprepareRemoteInput(srcId, sinkId, callback);
    EXPECT_EQ(ERR_DH_INPUT_SERVER_SOURCE_MANAGER_UNPREPARE_FAIL, ret);
}

/**
 * @tc.name: StartRemoteInput
 * @tc.desc: verify the function of starting distributed input with dhid.
 * @tc.type: FUNC
 * @tc.require: SR000H9J74
 */
HWTEST_F(DistributedInputSourceManagerTest, StartRemoteInput_05, testing::ext::TestSize.Level1)
{
    std::string sinkId = "";
    std::vector<std::string> dhIds;
    sptr<TestStartStopVectorCallbackStub> callback = new TestStartStopVectorCallbackStub();
    int32_t ret = sourceManager_->StartRemoteInput(sinkId, dhIds, callback);
    EXPECT_EQ(ERR_DH_INPUT_SERVER_SOURCE_MANAGER_START_FAIL, ret);
}

HWTEST_F(DistributedInputSourceManagerTest, StartRemoteInput_06, testing::ext::TestSize.Level1)
{
    std::vector<std::string> dhIds;
    std::string sinkId = "umkyu1b165e1be98151891erbe8r91ev";
    sptr<TestStartStopVectorCallbackStub> callback = new TestStartStopVectorCallbackStub();
    dhIds.push_back("Input_slkdiek3kddkeojfe");
    int32_t ret = sourceManager_->StartRemoteInput(sinkId, dhIds, callback);
    EXPECT_EQ(DH_SUCCESS, ret);
}

/**
 * @tc.name: StopRemoteInput
 * @tc.desc: verify the function of stoping distributed input with dhid.
 * @tc.type: FUNC
 * @tc.require: SR000H9J74
 */
HWTEST_F(DistributedInputSourceManagerTest, StopRemoteInput_05, testing::ext::TestSize.Level1)
{
    std::string sinkId = "";
    std::vector<std::string> dhIds;
    sptr<TestStartStopVectorCallbackStub> callback = new TestStartStopVectorCallbackStub();
    int32_t ret = sourceManager_->StopRemoteInput(sinkId, dhIds, callback);
    EXPECT_EQ(ERR_DH_INPUT_SERVER_SOURCE_MANAGER_STOP_FAIL, ret);
}

HWTEST_F(DistributedInputSourceManagerTest, StopRemoteInput_06, testing::ext::TestSize.Level1)
{
    std::vector<std::string> dhIds;
    sptr<TestStartStopVectorCallbackStub> callback = new TestStartStopVectorCallbackStub();
    std::string sinkId = "umkyu1b165e1be98151891erbe8r91ev";
    dhIds.push_back("Input_slkdiek3kddkeojfe");
    int32_t ret = sourceManager_->StopRemoteInput(sinkId, dhIds, callback);
    EXPECT_EQ(DH_SUCCESS, ret);
}

/**
 * @tc.name: StartRemoteInput
 * @tc.desc: verify the function of starting distributed input with dhid.
 * @tc.type: FUNC
 * @tc.require: SR000H9J74
 */
HWTEST_F(DistributedInputSourceManagerTest, StartRemoteInput_07, testing::ext::TestSize.Level1)
{
    std::string srcId = "";
    std::string sinkId = "";
    std::vector<std::string> dhIds;
    sptr<TestStartStopVectorCallbackStub> callback = new TestStartStopVectorCallbackStub();
    int32_t ret = sourceManager_->StartRemoteInput(srcId, sinkId, dhIds, callback);
    EXPECT_EQ(ERR_DH_INPUT_SERVER_SOURCE_MANAGER_START_FAIL, ret);
}

HWTEST_F(DistributedInputSourceManagerTest, StartRemoteInput_08, testing::ext::TestSize.Level1)
{
    std::vector<std::string> dhIds;
    sptr<TestStartStopVectorCallbackStub> callback = new TestStartStopVectorCallbackStub();
    std::string srcId = "networkidc08647073e02e7a78f09473aa122ff57fc81c00";
    std::string sinkId = "umkyu1b165e1be98151891erbe8r91ev";
    dhIds.push_back("Input_slkdiek3kddkeojfe");
    int32_t ret = sourceManager_->StartRemoteInput(srcId, sinkId, dhIds, callback);
    EXPECT_EQ(DH_SUCCESS, ret);
}

/**
 * @tc.name: StopRemoteInput
 * @tc.desc: verify the function of stoping distributed input with dhid.
 * @tc.type: FUNC
 * @tc.require: SR000H9J74
 */
HWTEST_F(DistributedInputSourceManagerTest, StopRemoteInput_07, testing::ext::TestSize.Level1)
{
    std::string srcId = "";
    std::string sinkId = "";
    std::vector<std::string> dhIds;
    sptr<TestStartStopVectorCallbackStub> callback = new TestStartStopVectorCallbackStub();
    int32_t ret = sourceManager_->StopRemoteInput(srcId, sinkId, dhIds, callback);
    EXPECT_EQ(ERR_DH_INPUT_SERVER_SOURCE_MANAGER_STOP_FAIL, ret);
}

HWTEST_F(DistributedInputSourceManagerTest, StopRemoteInput_08, testing::ext::TestSize.Level1)
{
    std::vector<std::string> dhIds;
    sptr<TestStartStopVectorCallbackStub> callback = new TestStartStopVectorCallbackStub();
    std::string srcId = "networkidc08647073e02e7a78f09473aa122ff57fc81c00";
    std::string sinkId = "umkyu1b165e1be98151891erbe8r91ev";
    dhIds.push_back("Input_slkdiek3kddkeojfe");
    int32_t ret = sourceManager_->StopRemoteInput(srcId, sinkId, dhIds, callback);
    EXPECT_EQ(DH_SUCCESS, ret);
}

HWTEST_F(DistributedInputSourceManagerTest, IsStringDataSame_01, testing::ext::TestSize.Level1)
{
    std::vector<std::string> oldDhIds;
    std::vector<std::string> newDhIds;
    oldDhIds.push_back("afv4s8b1dr1b8er1bd65fb16redb1dfb18d1b56df1b68d");
    bool ret = sourceManager_->IsStringDataSame(oldDhIds, newDhIds);
    EXPECT_EQ(false, ret);
}

HWTEST_F(DistributedInputSourceManagerTest, IsStringDataSame_02, testing::ext::TestSize.Level1)
{
    std::vector<std::string> oldDhIds;
    std::vector<std::string> newDhIds;
    oldDhIds.push_back("afv4s8b1dr1b8er1bd65fb16redb1dfb18d1b56df1b68d");
    newDhIds.push_back("rt12r1nr81n521be8rb1erbe1w8bg1erb18");
    bool ret = sourceManager_->IsStringDataSame(oldDhIds, newDhIds);
    EXPECT_EQ(false, ret);
    oldDhIds.push_back("rt12r1nr81n521be8rb1erbe1w8bg1erb18");
    newDhIds.push_back("afv4s8b1dr1b8er1bd65fb16redb1dfb18d1b56df1b68d");
    ret = sourceManager_->IsStringDataSame(oldDhIds, newDhIds);
    EXPECT_EQ(true, ret);
}

HWTEST_F(DistributedInputSourceManagerTest, RegisterAddWhiteListCallback01, testing::ext::TestSize.Level0)
{
    sptr<TestAddWhiteListInfosCb> callback = nullptr;
    int32_t ret = sourceManager_->RegisterAddWhiteListCallback(callback);
    EXPECT_EQ(ERR_DH_INPUT_SERVER_SOURCE_MANAGER_REG_CALLBACK_ERR, ret);
}

HWTEST_F(DistributedInputSourceManagerTest, RegisterAddWhiteListCallback02, testing::ext::TestSize.Level0)
{
    sptr<TestAddWhiteListInfosCb> callback = new TestAddWhiteListInfosCb();
    int32_t ret = sourceManager_->RegisterAddWhiteListCallback(callback);
    EXPECT_EQ(DH_SUCCESS, ret);
}

HWTEST_F(DistributedInputSourceManagerTest, RegisterDelWhiteListCallback01, testing::ext::TestSize.Level0)
{
    sptr<TestDelWhiteListInfosCb> callback = nullptr;
    int32_t ret = sourceManager_->RegisterDelWhiteListCallback(callback);
    EXPECT_EQ(ERR_DH_INPUT_SERVER_SOURCE_MANAGER_REG_CALLBACK_ERR, ret);
}

HWTEST_F(DistributedInputSourceManagerTest, RegisterDelWhiteListCallback02, testing::ext::TestSize.Level0)
{
    sptr<TestDelWhiteListInfosCb> callback = new TestDelWhiteListInfosCb();
    int32_t ret = sourceManager_->RegisterDelWhiteListCallback(callback);
    EXPECT_EQ(DH_SUCCESS, ret);
}

HWTEST_F(DistributedInputSourceManagerTest, RegisterInputNodeListener_01, testing::ext::TestSize.Level1)
{
    sptr<TestInputNodeListenerCb> callback = nullptr;
    int32_t ret = sourceManager_->RegisterInputNodeListener(callback);
    EXPECT_EQ(ERR_DH_INPUT_SERVER_SOURCE_MANAGER_NODE_LISTENER_CALLBACK_ERR, ret);

    ret = sourceManager_->UnregisterInputNodeListener(callback);
    EXPECT_EQ(ERR_DH_INPUT_SERVER_SOURCE_MANAGER_NODE_LISTENER_CALLBACK_ERR, ret);
}

HWTEST_F(DistributedInputSourceManagerTest, RegisterSimulationEventListener_01, testing::ext::TestSize.Level1)
{
    sptr<TestSimulationEventCb> callback = nullptr;
    int32_t ret = sourceManager_->RegisterSimulationEventListener(callback);
    EXPECT_EQ(ERR_DH_INPUT_SERVER_SOURCE_MANAGER_SIMULATION_EVENT_CALLBACK_ERR, ret);

    ret = sourceManager_->UnregisterSimulationEventListener(callback);
    EXPECT_EQ(ERR_DH_INPUT_SERVER_SOURCE_MANAGER_SIMULATION_EVENT_CALLBACK_ERR, ret);
}

HWTEST_F(DistributedInputSourceManagerTest, UnregisterSimulationEventListener_02, testing::ext::TestSize.Level1)
{
    sptr<TestSimulationEventCb> callback = new TestSimulationEventCb();
    int32_t ret = sourceManager_->RegisterSimulationEventListener(callback);
    EXPECT_EQ(DH_SUCCESS, ret);

    ret = sourceManager_->UnregisterSimulationEventListener(callback);
    EXPECT_EQ(DH_SUCCESS, ret);
}

HWTEST_F(DistributedInputSourceManagerTest, RelayPrepareRemoteInput_01, testing::ext::TestSize.Level1)
{
    std::string srcId = "networkidc08647073e02e7a78f09473aa122ff57fc81c00";
    std::string sinkId = "umkyu1b165e1be98151891erbe8r91ev";
    sptr<TestPrepareDInputCallback> callback = new TestPrepareDInputCallback();
    int32_t sessionId = 1;
    DistributedInputSourceTransport::DInputSessionInfo sessionInfo{true, srcId};
    DistributedInputSourceTransport::GetInstance().sessionDevMap_[sessionId] = sessionInfo;
    int32_t ret = sourceManager_->RelayPrepareRemoteInput(srcId, sinkId, callback);
    EXPECT_EQ(DH_SUCCESS, ret);
}

HWTEST_F(DistributedInputSourceManagerTest, RelayUnprepareRemoteInput_01, testing::ext::TestSize.Level1)
{
    std::string srcId = "networkidc08647073e02e7a78f09473aa122ff57fc81c00";
    std::string sinkId = "umkyu1b165e1be98151891erbe8r91ev";
    sptr<TestUnprepareDInputCallback> callback = new TestUnprepareDInputCallback();
    int32_t ret = sourceManager_->RelayUnprepareRemoteInput(srcId, sinkId, callback);
    EXPECT_EQ(DH_SUCCESS, ret);
}

HWTEST_F(DistributedInputSourceManagerTest, RelayStartRemoteInputByDhid_01, testing::ext::TestSize.Level1)
{
    std::string srcId = "networkidc08647073e02e7a78f09473aa122ff57fc81c00";
    std::string sinkId = "umkyu1b165e1be98151891erbe8r91ev";
    std::vector<std::string> dhIds;
    dhIds.push_back("input_slkdiek3kddkeojfe");
    sptr<TestStartStopDInputsCb> callback = new TestStartStopDInputsCb();
    int32_t ret = sourceManager_->RelayStartRemoteInputByDhid(srcId, sinkId, dhIds, callback);
    EXPECT_EQ(DH_SUCCESS, ret);
}

HWTEST_F(DistributedInputSourceManagerTest, RelayStopRemoteInputByDhid_01, testing::ext::TestSize.Level1)
{
    std::string srcId = "networkidc08647073e02e7a78f09473aa122ff57fc81c00";
    std::string sinkId = "umkyu1b165e1be98151891erbe8r91ev";
    std::vector<std::string> dhIds;
    dhIds.push_back("input_slkdiek3kddkeojfe");
    sptr<TestStartStopDInputsCb> callback = new TestStartStopDInputsCb();
    int32_t ret = sourceManager_->RelayStopRemoteInputByDhid(srcId, sinkId, dhIds, callback);
    EXPECT_EQ(DH_SUCCESS, ret);
}

HWTEST_F(DistributedInputSourceManagerTest, RunRegisterCallback_01, testing::ext::TestSize.Level1)
{
    std::string devId = "umkyu1b165e1be98151891erbe8r91ev";
    std::string dhId = "input_slkdiek3kddkeojfe";
    int32_t status = 0;
    sptr<TestRegisterDInputCb> callback = new TestRegisterDInputCb();
    DistributedInputSourceManager::DInputClientRegistInfo info {devId, dhId, callback};
    sourceManager_->regCallbacks_.push_back(info);
    sourceManager_->RunRegisterCallback(devId, dhId, status);
    EXPECT_EQ(0, sourceManager_->regCallbacks_.size());
}

HWTEST_F(DistributedInputSourceManagerTest, RunUnregisterCallback_01, testing::ext::TestSize.Level1)
{
    std::string devId = "umkyu1b165e1be98151891erbe8r91ev";
    std::string dhId = "input_slkdiek3kddkeojfe";
    int32_t status = 0;
    sptr<TestUnregisterDInputCb> callback = new TestUnregisterDInputCb();
    DistributedInputSourceManager::DInputClientUnregistInfo info {devId, dhId, callback};
    sourceManager_->unregCallbacks_.push_back(info);
    sourceManager_->RunUnregisterCallback(devId, dhId, status);
    EXPECT_EQ(0, sourceManager_->unregCallbacks_.size());
}

HWTEST_F(DistributedInputSourceManagerTest, RunPrepareCallback_01, testing::ext::TestSize.Level1)
{
    std::string devId = "umkyu1b165e1be98151891erbe8r91ev";
    int32_t status = 0;
    std::string object = "runprepareobject";
    sptr<TestPrepareDInputCallback> callback = new TestPrepareDInputCallback();
    DistributedInputSourceManager::DInputClientPrepareInfo info {devId, callback};
    sourceManager_->preCallbacks_.push_back(info);
    sourceManager_->RunPrepareCallback(devId, status, object);
    EXPECT_EQ(0, sourceManager_->preCallbacks_.size());
}

HWTEST_F(DistributedInputSourceManagerTest, RunRelayPrepareCallback_01, testing::ext::TestSize.Level1)
{
    std::string srcId = "networkidc08647073e02e7a78f09473aa122ff57fc81c00";
    std::string sinkId = "umkyu1b165e1be98151891erbe8r91ev";
    int32_t status = 0;
    sptr<TestPrepareDInputCallback> callback = new TestPrepareDInputCallback();
    DistributedInputSourceManager::DInputClientRelayPrepareInfo info {srcId, sinkId, callback};
    sourceManager_->relayPreCallbacks_.push_back(info);
    sourceManager_->RunRelayPrepareCallback(srcId, sinkId, status);
    EXPECT_EQ(0, sourceManager_->relayPreCallbacks_.size());
}

HWTEST_F(DistributedInputSourceManagerTest, RunRelayUnprepareCallback_01, testing::ext::TestSize.Level1)
{
    sourceManager_->relayUnpreCallbacks_.clear();
    std::string srcId = "networkidc08647073e02e7a78f09473aa122ff57fc81c00";
    std::string sinkId = "umkyu1b165e1be98151891erbe8r91ev";
    int32_t status = 0;
    sptr<TestUnprepareDInputCallback> callback = new TestUnprepareDInputCallback();
    DistributedInputSourceManager::DInputClientRelayUnprepareInfo info {srcId, sinkId, callback};
    sourceManager_->relayUnpreCallbacks_.push_back(info);
    sourceManager_->RunRelayUnprepareCallback(srcId, sinkId, status);
    EXPECT_EQ(0, sourceManager_->relayUnpreCallbacks_.size());
}

HWTEST_F(DistributedInputSourceManagerTest, RunUnprepareCallback_01, testing::ext::TestSize.Level1)
{
    std::string devId = "umkyu1b165e1be98151891erbe8r91ev";
    int32_t status = 0;
    std::string object = "runprepareobject";
    sptr<TestUnprepareDInputCallback> callback = new TestUnprepareDInputCallback();
    DistributedInputSourceManager::DInputClientUnprepareInfo info {devId, callback};
    sourceManager_->unpreCallbacks_.push_back(info);
    sourceManager_->RunUnprepareCallback(devId, status);
    EXPECT_EQ(0, sourceManager_->unpreCallbacks_.size());
}

HWTEST_F(DistributedInputSourceManagerTest, RunStartCallback_01, testing::ext::TestSize.Level1)
{
    std::string devId = "umkyu1b165e1be98151891erbe8r91ev";
    int32_t status = 0;
    uint32_t inputTypes = 1;
    sptr<TestStartDInputCallback> callback = new TestStartDInputCallback();
    DistributedInputSourceManager::DInputClientStartInfo info {devId, inputTypes, callback};
    sourceManager_->staCallbacks_.push_back(info);
    sourceManager_->RunStartCallback(devId, inputTypes, status);
    EXPECT_EQ(0, sourceManager_->staCallbacks_.size());
}

HWTEST_F(DistributedInputSourceManagerTest, RunStopCallback_01, testing::ext::TestSize.Level1)
{
    std::string devId = "umkyu1b165e1be98151891erbe8r91ev";
    int32_t status = 0;
    uint32_t inputTypes = 1;
    sptr<TestStopDInputCallback> callback = new TestStopDInputCallback();
    DistributedInputSourceManager::DInputClientStopInfo info {devId, inputTypes, callback};
    sourceManager_->stpCallbacks_.push_back(info);
    sourceManager_->RunStopCallback(devId, inputTypes, status);
    EXPECT_EQ(0, sourceManager_->stpCallbacks_.size());
}

HWTEST_F(DistributedInputSourceManagerTest, RunStartDhidCallback_01, testing::ext::TestSize.Level1)
{
    std::string sinkId = "umkyu1b165e1be98151891erbe8r91ev";
    int32_t status = 0;
    std::string dhId = "input_slkdiek3kddkeojfe";
    std::string localNetworkId = "networkidc08647073e02e7a78f09473aa122ff57fc81c00";
    std::vector<std::string> dhIds;
    dhIds.push_back("input_slkdiek3kddkeojfe");
    sptr<TestStartStopDInputsCb> callback = new TestStartStopDInputsCb();
    DistributedInputSourceManager::DInputClientStartDhidInfo info {localNetworkId, sinkId, dhIds, callback};
    sourceManager_->staStringCallbacks_.push_back(info);
    sourceManager_->RunStartDhidCallback(sinkId, dhId, status);
    EXPECT_EQ(0, sourceManager_->staStringCallbacks_.size());
}

HWTEST_F(DistributedInputSourceManagerTest, RunStopDhidCallback_01, testing::ext::TestSize.Level1)
{
    std::string sinkId = "umkyu1b165e1be98151891erbe8r91ev";
    int32_t status = 0;
    std::string dhId = "input_slkdiek3kddkeojfe";
    std::string localNetworkId = "networkidc08647073e02e7a78f09473aa122ff57fc81c00";
    std::vector<std::string> dhIds;
    dhIds.push_back(dhId);
    sptr<TestStartStopDInputsCb> callback = new TestStartStopDInputsCb();
    DistributedInputSourceManager::DInputClientStopDhidInfo info {localNetworkId, sinkId, dhIds, callback};
    sourceManager_->stpStringCallbacks_.push_back(info);
    sourceManager_->RunStopDhidCallback(sinkId, dhId, status);
    EXPECT_EQ(0, sourceManager_->stpStringCallbacks_.size());
}

HWTEST_F(DistributedInputSourceManagerTest, RunRelayStartDhidCallback_01, testing::ext::TestSize.Level1)
{
    std::string srcId = "networkidc08647073e02e7a78f09473aa122ff57fc81c00";
    std::string sinkId = "umkyu1b165e1be98151891erbe8r91ev";
    int32_t status = 0;
    std::string dhId = "input_slkdiek3kddkeojfe";
    std::vector<std::string> dhIds;
    dhIds.push_back(dhId);
    sptr<TestStartStopDInputsCb> callback = new TestStartStopDInputsCb();
    DistributedInputSourceManager::DInputClientStartDhidInfo info{srcId, sinkId, dhIds, callback};
    sourceManager_->relayStaDhidCallbacks_.push_back(info);
    sourceManager_->RunRelayStartDhidCallback(srcId, sinkId, status, dhId);
    EXPECT_EQ(0, sourceManager_->relayStaDhidCallbacks_.size());
}

HWTEST_F(DistributedInputSourceManagerTest, RunRelayStopDhidCallback_01, testing::ext::TestSize.Level1)
{
    std::string srcId = "networkidc08647073e02e7a78f09473aa122ff57fc81c00";
    std::string sinkId = "umkyu1b165e1be98151891erbe8r91ev";
    int32_t status = 0;
    std::string dhId = "input_slkdiek3kddkeojfe";
    std::vector<std::string> dhIds;
    dhIds.push_back(dhId);
    sptr<TestStartStopDInputsCb> callback = new TestStartStopDInputsCb();
    DistributedInputSourceManager::DInputClientStopDhidInfo info{srcId, sinkId, dhIds, callback};
    sourceManager_->relayStpDhidCallbacks_.push_back(info);
    sourceManager_->RunRelayStopDhidCallback(srcId, sinkId, status, dhId);
    EXPECT_EQ(0, sourceManager_->relayStpDhidCallbacks_.size());
}

HWTEST_F(DistributedInputSourceManagerTest, RunRelayStartTypeCallback_01, testing::ext::TestSize.Level1)
{
    std::string srcId = "networkidc08647073e02e7a78f09473aa122ff57fc81c00";
    std::string sinkId = "umkyu1b165e1be98151891erbe8r91ev";
    int32_t status = 0;
    uint32_t inputTypes = 1;
    sptr<TestStartDInputCallback> callback = new TestStartDInputCallback();
    DistributedInputSourceManager::DInputClientStartTypeInfo info(srcId, sinkId, inputTypes, callback);
    sourceManager_->relayStaTypeCallbacks_.push_back(info);
    sourceManager_->RunRelayStartTypeCallback(srcId, sinkId, status, inputTypes);
    EXPECT_EQ(0, sourceManager_->relayStaTypeCallbacks_.size());
}

HWTEST_F(DistributedInputSourceManagerTest, RunRelayStopTypeCallback_01, testing::ext::TestSize.Level1)
{
    std::string srcId = "networkidc08647073e02e7a78f09473aa122ff57fc81c00";
    std::string sinkId = "umkyu1b165e1be98151891erbe8r91ev";
    int32_t status = 0;
    uint32_t inputTypes = 1;
    sptr<TestStopDInputCallback> callback = new TestStopDInputCallback();
    DistributedInputSourceManager::DInputClientStopTypeInfo info(srcId, sinkId, inputTypes, callback);
    sourceManager_->relayStpTypeCallbacks_.push_back(info);
    sourceManager_->RunRelayStopTypeCallback(srcId, sinkId, status, inputTypes);
    EXPECT_EQ(0, sourceManager_->relayStpTypeCallbacks_.size());
}

HWTEST_F(DistributedInputSourceManagerTest, GetDeviceMapAllDevSwitchOff_01, testing::ext::TestSize.Level1)
{
    std::string devId = "umkyu1b165e1be98151891erbe8r91ev";
    sourceManager_->DeviceMap_[devId] = DINPUT_SOURCE_SWITCH_OFF;
    bool ret = sourceManager_->GetDeviceMapAllDevSwitchOff();
    EXPECT_EQ(true, ret);
}

HWTEST_F(DistributedInputSourceManagerTest, GetDeviceMapAllDevSwitchOff_02, testing::ext::TestSize.Level1)
{
    std::string devId = "umkyu1b165e1be98151891erbe8r91ev";
    sourceManager_->DeviceMap_[devId] = DINPUT_SOURCE_SWITCH_ON;
    bool ret = sourceManager_->GetDeviceMapAllDevSwitchOff();
    EXPECT_EQ(false, ret);
}

HWTEST_F(DistributedInputSourceManagerTest, GetInputTypesMap_01, testing::ext::TestSize.Level1)
{
    std::string devId = "umkyu1b165e1be98151891erbe8r91ev";
    sourceManager_->InputTypesMap_[devId] = 1;
    uint32_t ret = sourceManager_->GetInputTypesMap(devId);
    EXPECT_EQ(static_cast<uint32_t>(DInputDeviceType::MOUSE), ret);
}

HWTEST_F(DistributedInputSourceManagerTest, GetAllInputTypesMap_01, testing::ext::TestSize.Level1)
{
    std::string devId = "umkyu1b165e1be98151891erbe8r91ev";
    sourceManager_->InputTypesMap_[devId] = 1;
    uint32_t ret = sourceManager_->GetAllInputTypesMap();
    EXPECT_EQ(static_cast<uint32_t>(DInputDeviceType::MOUSE), ret);
}

HWTEST_F(DistributedInputSourceManagerTest, SetInputTypesMap_01, testing::ext::TestSize.Level1)
{
    std::string devId = "umkyu1b165e1be98151891erbe8r91ev";
    uint32_t value = 0;
    sourceManager_->InputTypesMap_[devId] = 1;
    sourceManager_->SetInputTypesMap(devId, value);
    EXPECT_EQ(true, sourceManager_->InputTypesMap_.empty());
}

HWTEST_F(DistributedInputSourceManagerTest, GetSyncNodeInfo_01, testing::ext::TestSize.Level1)
{
    std::string userDevId = "umkyu1b165e1be98151891erbe8r91ev";
    std::string dhId = "input_slkdiek3kddkeojfe";
    std::string nodeDesc = "input_deviceid:umkyu1b165e1be98151891erbe8r91ev, input_dhid:slkdiek3kddkeojfe";
    sourceManager_->syncNodeInfoMap_[userDevId].insert({userDevId, dhId, nodeDesc});
    std::string devId = "sd6f4s6d5f46s5d4f654564sdfdfsdfsdfd55";
    sourceManager_->GetSyncNodeInfo(devId);
    EXPECT_EQ(1, sourceManager_->GetSyncNodeInfo(userDevId).size());
}

HWTEST_F(DistributedInputSourceManagerTest, UpdateSyncNodeInfo_01, testing::ext::TestSize.Level1)
{
    std::string userDevId = "umkyu1b165e1be98151891erbe8r91ev";
    std::string dhId = "input_slkdiek3kddkeojfe";
    std::string nodeDesc = "input_deviceid:umkyu1b165e1be98151891erbe8r91ev, input_dhid:slkdiek3kddkeojfe";
    sourceManager_->syncNodeInfoMap_[userDevId].insert({userDevId, dhId, nodeDesc});
    sourceManager_->UpdateSyncNodeInfo(userDevId, dhId, nodeDesc);
    EXPECT_EQ(1, sourceManager_->syncNodeInfoMap_.size());
}

HWTEST_F(DistributedInputSourceManagerTest, UpdateSyncNodeInfo_02, testing::ext::TestSize.Level1)
{
    std::string userDevId = "umkyu1b165e1be98151891erbe8r91ev";
    std::string dhId = "input_slkdiek3kddkeojfe";
    std::string nodeDesc = "input_deviceid:umkyu1b165e1be98151891erbe8r91ev, input_dhid:slkdiek3kddkeojfe";
    sourceManager_->syncNodeInfoMap_.clear();
    sourceManager_->UpdateSyncNodeInfo(userDevId, dhId, nodeDesc);
    EXPECT_EQ(1, sourceManager_->syncNodeInfoMap_.size());
}

HWTEST_F(DistributedInputSourceManagerTest, DeleteSyncNodeInfo_01, testing::ext::TestSize.Level1)
{
    std::string userDevId = "umkyu1b165e1be98151891erbe8r91ev";
    std::string dhId = "input_slkdiek3kddkeojfe";
    std::string nodeDesc = "input_deviceid:umkyu1b165e1be98151891erbe8r91ev, input_dhid:slkdiek3kddkeojfe";
    sourceManager_->syncNodeInfoMap_[userDevId].insert({userDevId, dhId, nodeDesc});
    sourceManager_->DeleteSyncNodeInfo(userDevId);
    EXPECT_EQ(0, sourceManager_->syncNodeInfoMap_.size());
}

HWTEST_F(DistributedInputSourceManagerTest, Dump_01, testing::ext::TestSize.Level1)
{
    int32_t fd = 1;
    std::vector<std::u16string> args;
    int32_t ret = sourceManager_->Dump(fd, args);
    EXPECT_EQ(ERR_DH_INPUT_HIDUMP_DUMP_PROCESS_FAIL, ret);
}

HWTEST_F(DistributedInputSourceManagerTest, OnResponseRegisterDistributedHardware_01, testing::ext::TestSize.Level1)
{
    std::string deviceId = "djfhskjdhf5465456ds4f654sdf6";
    std::string dhId = "Input_s4df65s5d6f56asd5f6asdfasdfasdfv";
    bool result = true;
    callback_->OnResponseRegisterDistributedHardware(deviceId, dhId, result);

    int32_t fd = 1;
    std::vector<std::u16string> args;
    int32_t ret = sourceManager_->Dump(fd, args);
    EXPECT_EQ(ERR_DH_INPUT_HIDUMP_DUMP_PROCESS_FAIL, ret);
}

HWTEST_F(DistributedInputSourceManagerTest, OnResponsePrepareRemoteInput_01, testing::ext::TestSize.Level1)
{
    std::string deviceId = "djfhskjdhf5465456ds4f654sdf6";
    bool result = true;
    std::string object = "46sdf5g454dfsdfg4sd6fg";
    callback_->OnResponsePrepareRemoteInput(deviceId, result, object);

    int32_t fd = 1;
    std::vector<std::u16string> args;
    int32_t ret = sourceManager_->Dump(fd, args);
    EXPECT_EQ(ERR_DH_INPUT_HIDUMP_DUMP_PROCESS_FAIL, ret);
}

HWTEST_F(DistributedInputSourceManagerTest, OnResponseUnprepareRemoteInput_01, testing::ext::TestSize.Level1)
{
    std::string deviceId = "djfhskjdhf5465456ds4f654sdf6";
    bool result = true;
    callback_->OnResponseUnprepareRemoteInput(deviceId, result);

    int32_t fd = 1;
    std::vector<std::u16string> args;
    int32_t ret = sourceManager_->Dump(fd, args);
    EXPECT_EQ(ERR_DH_INPUT_HIDUMP_DUMP_PROCESS_FAIL, ret);
}

HWTEST_F(DistributedInputSourceManagerTest, OnResponseStartRemoteInput_01, testing::ext::TestSize.Level1)
{
    std::string deviceId = "djfhskjdhf5465456ds4f654sdf6";
    uint32_t inputTypes = 7;
    bool result = true;
    callback_->OnResponseStartRemoteInput(deviceId, inputTypes, result);

    int32_t fd = 1;
    std::vector<std::u16string> args;
    int32_t ret = sourceManager_->Dump(fd, args);
    EXPECT_EQ(ERR_DH_INPUT_HIDUMP_DUMP_PROCESS_FAIL, ret);
}

HWTEST_F(DistributedInputSourceManagerTest, OnResponseStopRemoteInput_01, testing::ext::TestSize.Level1)
{
    std::string deviceId = "djfhskjdhf5465456ds4f654sdf6";
    uint32_t inputTypes = 7;
    bool result = true;
    callback_->OnResponseStopRemoteInput(deviceId, inputTypes, result);

    int32_t fd = 1;
    std::vector<std::u16string> args;
    int32_t ret = sourceManager_->Dump(fd, args);
    EXPECT_EQ(ERR_DH_INPUT_HIDUMP_DUMP_PROCESS_FAIL, ret);
}

HWTEST_F(DistributedInputSourceManagerTest, OnResponseStartRemoteInputDhid_01, testing::ext::TestSize.Level1)
{
    std::string deviceId = "djfhskjdhf5465456ds4f654sdf6";
    std::string dhids = "Input_s4df65s5d6f56asd5f6asdfasdfasdfv";
    bool result = true;
    callback_->OnResponseStartRemoteInputDhid(deviceId, dhids, result);

    int32_t fd = 1;
    std::vector<std::u16string> args;
    int32_t ret = sourceManager_->Dump(fd, args);
    EXPECT_EQ(ERR_DH_INPUT_HIDUMP_DUMP_PROCESS_FAIL, ret);
}

HWTEST_F(DistributedInputSourceManagerTest, OnResponseStopRemoteInputDhid_01, testing::ext::TestSize.Level1)
{
    std::string deviceId = "djfhskjdhf5465456ds4f654sdf6";
    std::string dhids = "Input_s4df65s5d6f56asd5f6asdfasdfasdfv";
    bool result = true;
    callback_->OnResponseStopRemoteInputDhid(deviceId, dhids, result);

    int32_t fd = 1;
    std::vector<std::u16string> args;
    int32_t ret = sourceManager_->Dump(fd, args);
    EXPECT_EQ(ERR_DH_INPUT_HIDUMP_DUMP_PROCESS_FAIL, ret);
}

HWTEST_F(DistributedInputSourceManagerTest, OnResponseRelayPrepareRemoteInput_01, testing::ext::TestSize.Level1)
{
    int32_t toSrcSessionId = 1;
    std::string deviceId = "as5d4a65sd4a65sd456as4d";
    bool result = true;
    std::string object = "46sdf5g454dfsdfg4sd6fg";
    callback_->OnResponseRelayPrepareRemoteInput(toSrcSessionId, deviceId, result, object);

    int32_t fd = 1;
    std::vector<std::u16string> args;
    int32_t ret = sourceManager_->Dump(fd, args);
    EXPECT_EQ(ERR_DH_INPUT_HIDUMP_DUMP_PROCESS_FAIL, ret);
}

HWTEST_F(DistributedInputSourceManagerTest, OnResponseRelayUnprepareRemoteInput_01, testing::ext::TestSize.Level1)
{
    int32_t toSrcSessionId = 1;
    std::string deviceId = "as5d4a65sd4a65sd456as4d";
    bool result = true;
    callback_->OnResponseRelayUnprepareRemoteInput(toSrcSessionId, deviceId, result);

    int32_t fd = 1;
    std::vector<std::u16string> args;
    int32_t ret = sourceManager_->Dump(fd, args);
    EXPECT_EQ(ERR_DH_INPUT_HIDUMP_DUMP_PROCESS_FAIL, ret);
}

HWTEST_F(DistributedInputSourceManagerTest, OnReceiveRelayPrepareResult_01, testing::ext::TestSize.Level1)
{
    int32_t status = 1;
    std::string srcId = "djfhskjdhf5465456ds4f654sdf6";
    std::string sinkId = "asd4a65sd46as4da6s4d6asdasdafwebrb";
    callback_->OnReceiveRelayPrepareResult(status, srcId, sinkId);

    int32_t fd = 1;
    std::vector<std::u16string> args;
    int32_t ret = sourceManager_->Dump(fd, args);
    EXPECT_EQ(ERR_DH_INPUT_HIDUMP_DUMP_PROCESS_FAIL, ret);
}

HWTEST_F(DistributedInputSourceManagerTest, OnReceiveRelayUnprepareResult_01, testing::ext::TestSize.Level1)
{
    int32_t status = 1;
    std::string srcId = "djfhskjdhf5465456ds4f654sdf6";
    std::string sinkId = "asd4a65sd46as4da6s4d6asdasdafwebrb";
    callback_->OnReceiveRelayUnprepareResult(status, srcId, sinkId);

    int32_t fd = 1;
    std::vector<std::u16string> args;
    int32_t ret = sourceManager_->Dump(fd, args);
    EXPECT_EQ(ERR_DH_INPUT_HIDUMP_DUMP_PROCESS_FAIL, ret);
}

HWTEST_F(DistributedInputSourceManagerTest, OnReceiveRelayStartDhidResult_01, testing::ext::TestSize.Level1)
{
    int32_t status = 1;
    std::string srcId = "djfhskjdhf5465456ds4f654sdf6";
    std::string sinkId = "asd4a65sd46as4da6s4d6asdasdafwebrb";
    std::string dhid = "Input_s4df65s5d6f56asd5f6asdfasdfasdfv";
    callback_->OnReceiveRelayStartDhidResult(status, srcId, sinkId, dhid);

    int32_t fd = 1;
    std::vector<std::u16string> args;
    int32_t ret = sourceManager_->Dump(fd, args);
    EXPECT_EQ(ERR_DH_INPUT_HIDUMP_DUMP_PROCESS_FAIL, ret);
}

HWTEST_F(DistributedInputSourceManagerTest, OnReceiveRelayStopDhidResult_01, testing::ext::TestSize.Level1)
{
    int32_t status = 1;
    std::string srcId = "djfhskjdhf5465456ds4f654sdf6";
    std::string sinkId = "asd4a65sd46as4da6s4d6asdasdafwebrb";
    std::string dhid = "Input_s4df65s5d6f56asd5f6asdfasdfasdfv";
    callback_->OnReceiveRelayStopDhidResult(status, srcId, sinkId, dhid);

    int32_t fd = 1;
    std::vector<std::u16string> args;
    int32_t ret = sourceManager_->Dump(fd, args);
    EXPECT_EQ(ERR_DH_INPUT_HIDUMP_DUMP_PROCESS_FAIL, ret);
}

HWTEST_F(DistributedInputSourceManagerTest, OnReceiveRelayStartTypeResult_01, testing::ext::TestSize.Level1)
{
    int32_t status = 1;
    std::string srcId = "djfhskjdhf5465456ds4f654sdf6";
    std::string sinkId = "asd4a65sd46as4da6s4d6asdasdafwebrb";
    uint32_t inputTypes = 7;
    callback_->OnReceiveRelayStartTypeResult(status, srcId, sinkId, inputTypes);

    int32_t fd = 1;
    std::vector<std::u16string> args;
    int32_t ret = sourceManager_->Dump(fd, args);
    EXPECT_EQ(ERR_DH_INPUT_HIDUMP_DUMP_PROCESS_FAIL, ret);
}

HWTEST_F(DistributedInputSourceManagerTest, OnReceiveRelayStopTypeResult_01, testing::ext::TestSize.Level1)
{
    int32_t status = 1;
    std::string srcId = "djfhskjdhf5465456ds4f654sdf6";
    std::string sinkId = "asd4a65sd46as4da6s4d6asdasdafwebrb";
    uint32_t inputTypes = 7;
    callback_->OnReceiveRelayStopTypeResult(status, srcId, sinkId, inputTypes);

    int32_t fd = 1;
    std::vector<std::u16string> args;
    int32_t ret = sourceManager_->Dump(fd, args);
    EXPECT_EQ(ERR_DH_INPUT_HIDUMP_DUMP_PROCESS_FAIL, ret);
}

HWTEST_F(DistributedInputSourceManagerTest, InitAuto_01, testing::ext::TestSize.Level1)
{
    bool ret = sourceManager_->InitAuto();
    EXPECT_EQ(true, ret);
}

HWTEST_F(DistributedInputSourceManagerTest, OnResponseRegisterDistributedHardware_02, testing::ext::TestSize.Level1)
{
    sourceManager_->InitAuto();

    std::string deviceId = "djfhskjdhf5465456ds4f654sdf6";
    std::string dhId = "Input_s4df65s5d6f56asd5f6asdfasdfasdfv";
    bool result = true;
    callback_->OnResponseRegisterDistributedHardware(deviceId, dhId, result);

    int32_t fd = 1;
    std::vector<std::u16string> args;
    int32_t ret = sourceManager_->Dump(fd, args);
    EXPECT_EQ(ERR_DH_INPUT_HIDUMP_DUMP_PROCESS_FAIL, ret);
}

HWTEST_F(DistributedInputSourceManagerTest, OnResponsePrepareRemoteInput_02, testing::ext::TestSize.Level1)
{
    sourceManager_->InitAuto();

    std::string deviceId = "djfhskjdhf5465456ds4f654sdf6";
    bool result = true;
    std::string object = "46sdf5g454dfsdfg4sd6fg";
    callback_->OnResponsePrepareRemoteInput(deviceId, result, object);

    int32_t fd = 1;
    std::vector<std::u16string> args;
    int32_t ret = sourceManager_->Dump(fd, args);
    EXPECT_EQ(ERR_DH_INPUT_HIDUMP_DUMP_PROCESS_FAIL, ret);
}

HWTEST_F(DistributedInputSourceManagerTest, OnResponseUnprepareRemoteInput_02, testing::ext::TestSize.Level1)
{
    sourceManager_->InitAuto();

    std::string deviceId = "djfhskjdhf5465456ds4f654sdf6";
    bool result = true;
    callback_->OnResponseUnprepareRemoteInput(deviceId, result);

    int32_t fd = 1;
    std::vector<std::u16string> args;
    int32_t ret = sourceManager_->Dump(fd, args);
    EXPECT_EQ(ERR_DH_INPUT_HIDUMP_DUMP_PROCESS_FAIL, ret);
}

HWTEST_F(DistributedInputSourceManagerTest, OnResponseStartRemoteInput_02, testing::ext::TestSize.Level1)
{
    sourceManager_->InitAuto();

    std::string deviceId = "djfhskjdhf5465456ds4f654sdf6";
    uint32_t inputTypes = 7;
    bool result = true;
    callback_->OnResponseStartRemoteInput(deviceId, inputTypes, result);

    int32_t fd = 1;
    std::vector<std::u16string> args;
    int32_t ret = sourceManager_->Dump(fd, args);
    EXPECT_EQ(ERR_DH_INPUT_HIDUMP_DUMP_PROCESS_FAIL, ret);
}

HWTEST_F(DistributedInputSourceManagerTest, OnResponseStopRemoteInput_02, testing::ext::TestSize.Level1)
{
    sourceManager_->InitAuto();

    std::string deviceId = "djfhskjdhf5465456ds4f654sdf6";
    uint32_t inputTypes = 7;
    bool result = true;
    callback_->OnResponseStopRemoteInput(deviceId, inputTypes, result);

    int32_t fd = 1;
    std::vector<std::u16string> args;
    int32_t ret = sourceManager_->Dump(fd, args);
    EXPECT_EQ(ERR_DH_INPUT_HIDUMP_DUMP_PROCESS_FAIL, ret);
}

HWTEST_F(DistributedInputSourceManagerTest, OnResponseStartRemoteInputDhid_02, testing::ext::TestSize.Level1)
{
    sourceManager_->InitAuto();

    std::string deviceId = "djfhskjdhf5465456ds4f654sdf6";
    std::string dhids = "Input_s4df65s5d6f56asd5f6asdfasdfasdfv";
    bool result = true;
    callback_->OnResponseStartRemoteInputDhid(deviceId, dhids, result);

    int32_t fd = 1;
    std::vector<std::u16string> args;
    int32_t ret = sourceManager_->Dump(fd, args);
    EXPECT_EQ(ERR_DH_INPUT_HIDUMP_DUMP_PROCESS_FAIL, ret);
}

HWTEST_F(DistributedInputSourceManagerTest, OnResponseStopRemoteInputDhid_02, testing::ext::TestSize.Level1)
{
    sourceManager_->InitAuto();

    std::string deviceId = "djfhskjdhf5465456ds4f654sdf6";
    std::string dhids = "Input_s4df65s5d6f56asd5f6asdfasdfasdfv";
    bool result = true;
    callback_->OnResponseStopRemoteInputDhid(deviceId, dhids, result);

    int32_t fd = 1;
    std::vector<std::u16string> args;
    int32_t ret = sourceManager_->Dump(fd, args);
    EXPECT_EQ(ERR_DH_INPUT_HIDUMP_DUMP_PROCESS_FAIL, ret);
}

HWTEST_F(DistributedInputSourceManagerTest, OnResponseRelayPrepareRemoteInput_02, testing::ext::TestSize.Level1)
{
    sourceManager_->InitAuto();

    int32_t toSrcSessionId = 1;
    std::string deviceId = "as5d4a65sd4a65sd456as4d";
    bool result = true;
    std::string object = "46sdf5g454dfsdfg4sd6fg";
    callback_->OnResponseRelayPrepareRemoteInput(toSrcSessionId, deviceId, result, object);

    int32_t fd = 1;
    std::vector<std::u16string> args;
    int32_t ret = sourceManager_->Dump(fd, args);
    EXPECT_EQ(ERR_DH_INPUT_HIDUMP_DUMP_PROCESS_FAIL, ret);
}

HWTEST_F(DistributedInputSourceManagerTest, OnResponseRelayUnprepareRemoteInput_02, testing::ext::TestSize.Level1)
{
    sourceManager_->InitAuto();

    int32_t toSrcSessionId = 1;
    std::string deviceId = "as5d4a65sd4a65sd456as4d";
    bool result = true;
    callback_->OnResponseRelayUnprepareRemoteInput(toSrcSessionId, deviceId, result);

    int32_t fd = 1;
    std::vector<std::u16string> args;
    int32_t ret = sourceManager_->Dump(fd, args);
    EXPECT_EQ(ERR_DH_INPUT_HIDUMP_DUMP_PROCESS_FAIL, ret);
}

HWTEST_F(DistributedInputSourceManagerTest, OnReceiveRelayPrepareResult_02, testing::ext::TestSize.Level1)
{
    sourceManager_->InitAuto();

    int32_t status = 1;
    std::string srcId = "djfhskjdhf5465456ds4f654sdf6";
    std::string sinkId = "asd4a65sd46as4da6s4d6asdasdafwebrb";
    callback_->OnReceiveRelayPrepareResult(status, srcId, sinkId);

    int32_t fd = 1;
    std::vector<std::u16string> args;
    int32_t ret = sourceManager_->Dump(fd, args);
    EXPECT_EQ(ERR_DH_INPUT_HIDUMP_DUMP_PROCESS_FAIL, ret);
}

HWTEST_F(DistributedInputSourceManagerTest, OnReceiveRelayUnprepareResult_02, testing::ext::TestSize.Level1)
{
    sourceManager_->InitAuto();

    int32_t status = 1;
    std::string srcId = "djfhskjdhf5465456ds4f654sdf6";
    std::string sinkId = "asd4a65sd46as4da6s4d6asdasdafwebrb";
    callback_->OnReceiveRelayUnprepareResult(status, srcId, sinkId);

    int32_t fd = 1;
    std::vector<std::u16string> args;
    int32_t ret = sourceManager_->Dump(fd, args);
    EXPECT_EQ(ERR_DH_INPUT_HIDUMP_DUMP_PROCESS_FAIL, ret);
}

HWTEST_F(DistributedInputSourceManagerTest, OnReceiveRelayStartDhidResult_02, testing::ext::TestSize.Level1)
{
    sourceManager_->InitAuto();

    int32_t status = 1;
    std::string srcId = "djfhskjdhf5465456ds4f654sdf6";
    std::string sinkId = "asd4a65sd46as4da6s4d6asdasdafwebrb";
    std::string dhid = "Input_s4df65s5d6f56asd5f6asdfasdfasdfv";
    callback_->OnReceiveRelayStartDhidResult(status, srcId, sinkId, dhid);

    int32_t fd = 1;
    std::vector<std::u16string> args;
    int32_t ret = sourceManager_->Dump(fd, args);
    EXPECT_EQ(ERR_DH_INPUT_HIDUMP_DUMP_PROCESS_FAIL, ret);
}

HWTEST_F(DistributedInputSourceManagerTest, OnReceiveRelayStopDhidResult_02, testing::ext::TestSize.Level1)
{
    sourceManager_->InitAuto();

    int32_t status = 1;
    std::string srcId = "djfhskjdhf5465456ds4f654sdf6";
    std::string sinkId = "asd4a65sd46as4da6s4d6asdasdafwebrb";
    std::string dhid = "Input_s4df65s5d6f56asd5f6asdfasdfasdfv";
    callback_->OnReceiveRelayStopDhidResult(status, srcId, sinkId, dhid);

    int32_t fd = 1;
    std::vector<std::u16string> args;
    int32_t ret = sourceManager_->Dump(fd, args);
    EXPECT_EQ(ERR_DH_INPUT_HIDUMP_DUMP_PROCESS_FAIL, ret);
}

HWTEST_F(DistributedInputSourceManagerTest, OnReceiveRelayStartTypeResult_02, testing::ext::TestSize.Level1)
{
    sourceManager_->InitAuto();

    int32_t status = 1;
    std::string srcId = "djfhskjdhf5465456ds4f654sdf6";
    std::string sinkId = "asd4a65sd46as4da6s4d6asdasdafwebrb";
    uint32_t inputTypes = 7;
    callback_->OnReceiveRelayStartTypeResult(status, srcId, sinkId, inputTypes);

    int32_t fd = 1;
    std::vector<std::u16string> args;
    int32_t ret = sourceManager_->Dump(fd, args);
    EXPECT_EQ(ERR_DH_INPUT_HIDUMP_DUMP_PROCESS_FAIL, ret);
}

HWTEST_F(DistributedInputSourceManagerTest, OnReceiveRelayStopTypeResult_02, testing::ext::TestSize.Level1)
{
    sourceManager_->InitAuto();

    int32_t status = 1;
    std::string srcId = "djfhskjdhf5465456ds4f654sdf6";
    std::string sinkId = "asd4a65sd46as4da6s4d6asdasdafwebrb";
    uint32_t inputTypes = 7;
    callback_->OnReceiveRelayStopTypeResult(status, srcId, sinkId, inputTypes);

    int32_t fd = 1;
    std::vector<std::u16string> args;
    int32_t ret = sourceManager_->Dump(fd, args);
    EXPECT_EQ(ERR_DH_INPUT_HIDUMP_DUMP_PROCESS_FAIL, ret);
}
} // namespace DistributedInput
} // namespace DistributedHardware
} // namespace OHOS