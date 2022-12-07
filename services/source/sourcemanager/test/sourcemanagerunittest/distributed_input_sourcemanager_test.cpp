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

using namespace testing::ext;
using namespace OHOS::DistributedHardware::DistributedInput;
using namespace std;
namespace OHOS {
namespace DistributedHardware {
namespace DistributedInput {
void DistributedInputSourceManagerTest::SetUp()
{
    sourceManager_ = new DistributedInputSourceManager(DISTRIBUTED_HARDWARE_INPUT_SOURCE_SA_ID, true);
}

void DistributedInputSourceManagerTest::TearDown()
{
}

void DistributedInputSourceManagerTest::SetUpTestCase()
{
}

void DistributedInputSourceManagerTest::TearDownTestCase()
{
    _Exit(0);
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

HWTEST_F(DistributedInputSourceManagerTest, RegisterDistributedHardware01, testing::ext::TestSize.Level0)
{
    InputDevice pBuffer;
    pBuffer.name = "uinput_name_touch";
    pBuffer.bus = 0x03;
    pBuffer.vendor = 0x1233;
    pBuffer.product = 0xfedb;
    pBuffer.version = 3;
    pBuffer.physicalPath = "usb-hiusb-ehci-2.1/input1";
    pBuffer.uniqueId = "3";
    pBuffer.classes = INPUT_DEVICE_CLASS_TOUCH;
    pBuffer.descriptor = "1ds56v18e1v21v8v1erv15r1v8r1j1ty8";

    std::string devId = "umkyu1b165e1be98151891erbe8r91ev";
    std::string dhId = pBuffer.descriptor;
    std::string parameters;
    StructTransJson(pBuffer, parameters);

    sptr<TestRegisterDInputCb> callback = new TestRegisterDInputCb();
    int32_t ret = sourceManager_->RegisterDistributedHardware(devId, dhId, parameters, callback);

    EXPECT_EQ(DH_SUCCESS, ret);
}

HWTEST_F(DistributedInputSourceManagerTest, RegisterDistributedHardware02, testing::ext::TestSize.Level0)
{
    InputDevice pBuffer;
    pBuffer.name = "uinput_name_mouse";
    pBuffer.bus = 0x03;
    pBuffer.vendor = 0x1222;
    pBuffer.product = 0xfeda;
    pBuffer.version = 2;
    pBuffer.physicalPath = "usb-hiusb-ehci-2.1/input1";
    pBuffer.uniqueId = "2";
    pBuffer.classes = INPUT_DEVICE_CLASS_CURSOR;
    pBuffer.descriptor = "rt12r1nr81n521be8rb1erbe1w8bg1erb18";

    std::string devId = "umkyu1b165e1be98151891erbe8r91ev";
    std::string dhId = pBuffer.descriptor;
    std::string parameters;
    StructTransJson(pBuffer, parameters);
    sptr<TestRegisterDInputCb> callback = new TestRegisterDInputCb();
    int32_t ret = sourceManager_->RegisterDistributedHardware(devId, dhId, parameters, callback);
    EXPECT_EQ(DH_SUCCESS, ret);
}

HWTEST_F(DistributedInputSourceManagerTest, RegisterDistributedHardware03, testing::ext::TestSize.Level0)
{
    InputDevice pBuffer;
    pBuffer.name = "uinput_name_keyboard";
    pBuffer.bus = 0x03;
    pBuffer.vendor = 0x1234;
    pBuffer.product = 0xfedc;
    pBuffer.version = 1;
    pBuffer.physicalPath = "usb-hiusb-ehci-2.1/input1";
    pBuffer.uniqueId = "1";
    pBuffer.classes = INPUT_DEVICE_CLASS_KEYBOARD;
    pBuffer.descriptor = "afv4s8b1dr1b8er1bd65fb16redb1dfb18d1b56df1b68d";

    std::string devId = "umkyu1b165e1be98151891erbe8r91ev";
    std::string dhId = pBuffer.descriptor;
    std::string parameters;
    StructTransJson(pBuffer, parameters);
    sptr<TestRegisterDInputCb> callback = new TestRegisterDInputCb();
    int32_t ret = sourceManager_->RegisterDistributedHardware(devId, dhId, parameters, callback);
    EXPECT_EQ(DH_SUCCESS, ret);
}

HWTEST_F(DistributedInputSourceManagerTest, PrepareRemoteInput01, testing::ext::TestSize.Level0)
{
    std::string devId = "umkyu1b165e1be98151891erbe8r91ev";
    sptr<TestPrepareDInputCallback> callback = new TestPrepareDInputCallback();
    int32_t ret = sourceManager_->PrepareRemoteInput(devId, callback);
    EXPECT_EQ(DH_SUCCESS, ret);
}

HWTEST_F(DistributedInputSourceManagerTest, PrepareRemoteInput02, testing::ext::TestSize.Level0)
{
    std::string devId = "umkyu1b165e1be98151891erbe8r91ev";
    sptr<TestPrepareDInputCallback> callback;
    int32_t ret = sourceManager_->PrepareRemoteInput(devId, callback);
    EXPECT_EQ(ERR_DH_INPUT_SERVER_SOURCE_MANAGER_PREPARE_FAIL, ret);
}

HWTEST_F(DistributedInputSourceManagerTest, PrepareRemoteInput03, testing::ext::TestSize.Level0)
{
    std::string devId = "";
    sptr<TestPrepareDInputCallback> callback = new TestPrepareDInputCallback();
    int32_t ret = sourceManager_->PrepareRemoteInput(devId, callback);
    EXPECT_EQ(ERR_DH_INPUT_SERVER_SOURCE_MANAGER_PREPARE_FAIL, ret);
}

/**
 * @tc.name: PrepareRemoteInput04
 * @tc.desc: verify the function of distributing data from any device.
 * @tc.type: FUNC
 * @tc.require: SR000H9J75
 */
HWTEST_F(DistributedInputSourceManagerTest, PrepareRemoteInput04, testing::ext::TestSize.Level0)
{
    std::string srcId = "networkidc08647073e02e7a78f09473aa122ff57fc81c00";
    std::string sinkId = "umkyu1b165e1be98151891erbe8r91ev";
    sptr<TestPrepareDInputCallback> callback = new TestPrepareDInputCallback();
    int32_t ret = sourceManager_->PrepareRemoteInput(srcId, sinkId, callback);
    EXPECT_EQ(DH_SUCCESS, ret);
}

/**
 * @tc.name: PrepareRemoteInput05
 * @tc.desc: verify the function of distributing data from any device.
 * @tc.type: FUNC
 * @tc.require: SR000H9J75
 */
HWTEST_F(DistributedInputSourceManagerTest, PrepareRemoteInput05, testing::ext::TestSize.Level0)
{
    std::string srcId = "networkidc08647073e02e7a78f09473aa122ff57fc81c00";
    std::string sinkId = "umkyu1b165e1be98151891erbe8r91ev";
    sptr<TestPrepareDInputCallback> callback;
    int32_t ret = sourceManager_->PrepareRemoteInput(srcId, sinkId, callback);
    EXPECT_EQ(ERR_DH_INPUT_SERVER_SOURCE_MANAGER_PREPARE_FAIL, ret);
}

/**
 * @tc.name: PrepareRemoteInput06
 * @tc.desc: verify the function of distributing data from any device.
 * @tc.type: FUNC
 * @tc.require: SR000H9J75
 */
HWTEST_F(DistributedInputSourceManagerTest, PrepareRemoteInput06, testing::ext::TestSize.Level0)
{
    std::string srcId = "";
    std::string sinkId = "";
    sptr<TestPrepareDInputCallback> callback = new TestPrepareDInputCallback();
    int32_t ret = sourceManager_->PrepareRemoteInput(srcId, sinkId, callback);
    EXPECT_EQ(ERR_DH_INPUT_SERVER_SOURCE_MANAGER_PREPARE_FAIL, ret);
}

HWTEST_F(DistributedInputSourceManagerTest, StartRemoteInput01, testing::ext::TestSize.Level0)
{
    std::string devId = "umkyu1b165e1be98151891erbe8r91ev";
    sptr<TestStartDInputCallback> callback = new TestStartDInputCallback();
    int32_t ret = sourceManager_->StartRemoteInput(devId, static_cast<uint32_t>(DInputDeviceType::ALL), callback);
    EXPECT_EQ(DH_SUCCESS, ret);
}

HWTEST_F(DistributedInputSourceManagerTest, StartRemoteInput02, testing::ext::TestSize.Level0)
{
    std::string devId = "umkyu1b165e1be98151891erbe8r91ev";
    sptr<TestStartDInputCallback> callback;
    int32_t ret = sourceManager_->StartRemoteInput(devId, static_cast<uint32_t>(DInputDeviceType::ALL), callback);
    EXPECT_EQ(ERR_DH_INPUT_SERVER_SOURCE_MANAGER_START_FAIL, ret);
}

HWTEST_F(DistributedInputSourceManagerTest, StartRemoteInput03, testing::ext::TestSize.Level0)
{
    std::string devId = "";
    sptr<TestStartDInputCallback> callback = new TestStartDInputCallback();
    int32_t ret = sourceManager_->StartRemoteInput(devId, static_cast<uint32_t>(DInputDeviceType::ALL), callback);
    EXPECT_EQ(ERR_DH_INPUT_SERVER_SOURCE_MANAGER_START_FAIL, ret);
}

/**
 * @tc.name: StartRemoteInput04
 * @tc.desc: verify the function of starting distributed input on InputDeviceType.
 * @tc.type: FUNC
 * @tc.require: SR000H9J75
 */
HWTEST_F(DistributedInputSourceManagerTest, StartRemoteInput04, testing::ext::TestSize.Level0)
{
    std::string srcId = "networkidc08647073e02e7a78f09473aa122ff57fc81c00";
    std::string sinkId = "umkyu1b165e1be98151891erbe8r91ev";
    sptr<TestStartDInputCallback> callback = new TestStartDInputCallback();
    int32_t ret =
        sourceManager_->StartRemoteInput(srcId, sinkId, static_cast<uint32_t>(DInputDeviceType::ALL), callback);
    EXPECT_EQ(DH_SUCCESS, ret);
}

/**
 * @tc.name: StartRemoteInput05
 * @tc.desc: verify the function of starting distributed input on InputDeviceType.
 * @tc.type: FUNC
 * @tc.require: SR000H9J75
 */
HWTEST_F(DistributedInputSourceManagerTest, StartRemoteInput05, testing::ext::TestSize.Level0)
{
    std::string srcId = "networkidc08647073e02e7a78f09473aa122ff57fc81c00";
    std::string sinkId = "umkyu1b165e1be98151891erbe8r91ev";
    sptr<TestStartDInputCallback> callback;
    int32_t ret =
        sourceManager_->StartRemoteInput(srcId, sinkId, static_cast<uint32_t>(DInputDeviceType::ALL), callback);
    EXPECT_EQ(ERR_DH_INPUT_SERVER_SOURCE_MANAGER_START_FAIL, ret);
}

/**
 * @tc.name: StartRemoteInput06
 * @tc.desc: verify the low-latency transmission capability of distributed input.
 * @tc.type: FUNC
 * @tc.require: SR000H9J78
 */
HWTEST_F(DistributedInputSourceManagerTest, StartRemoteInput06, testing::ext::TestSize.Level0)
{
    std::string srcId = "";
    std::string sinkId = "";
    sptr<TestStartDInputCallback> callback = new TestStartDInputCallback();
    int32_t ret =
        sourceManager_->StartRemoteInput(srcId, sinkId, static_cast<uint32_t>(DInputDeviceType::ALL), callback);
    EXPECT_EQ(ERR_DH_INPUT_SERVER_SOURCE_MANAGER_START_FAIL, ret);
}

/**
 * @tc.name: StartRemoteInput10
 * @tc.desc: verify the function of transferring mouse button status.
 * @tc.type: FUNC
 * @tc.require: SR000H9J76
 */
HWTEST_F(DistributedInputSourceManagerTest, StartRemoteInput07, testing::ext::TestSize.Level0)
{
    std::string srcId = "networkidc08647073e02e7a78f09473aa122ff57fc81c00";
    std::string sinkId = "umkyu1b165e1be98151891erbe8r91ev";
    std::vector<std::string> dhIds;
    dhIds.push_back("Input_slkdiek3kddkeojfe");
    sptr<TestStartStopVectorCallbackStub> callback = new TestStartStopVectorCallbackStub();
    int32_t ret = sourceManager_->StartRemoteInput(srcId, sinkId, dhIds, callback);
    EXPECT_EQ(DH_SUCCESS, ret);
}

/**
 * @tc.name: StartRemoteInput08
 * @tc.desc: verify the function of transferring mouse button status.
 * @tc.type: FUNC
 * @tc.require: SR000H9J76
 */
HWTEST_F(DistributedInputSourceManagerTest, StartRemoteInput08, testing::ext::TestSize.Level0)
{
    std::string srcId = "networkidc08647073e02e7a78f09473aa122ff57fc81c00";
    std::string sinkId = "umkyu1b165e1be98151891erbe8r91ev";
    std::vector<std::string> dhIds;
    sptr<TestStartStopVectorCallbackStub> callback;
    int32_t ret = sourceManager_->StartRemoteInput(srcId, sinkId, dhIds, callback);
    EXPECT_EQ(ERR_DH_INPUT_SERVER_SOURCE_MANAGER_START_FAIL, ret);
}

/**
 * @tc.name: StartRemoteInput09
 * @tc.desc: verify the low-latency transmission capability of distributed input
 * @tc.type: FUNC
 * @tc.require: SR000H9J78
 */
HWTEST_F(DistributedInputSourceManagerTest, StartRemoteInput09, testing::ext::TestSize.Level0)
{
    std::string srcId = "";
    std::string sinkId = "";
    std::vector<std::string> dhIds;
    dhIds.push_back("Input_sldkjfldsjf234mdwswo");
    sptr<TestStartStopVectorCallbackStub> callback = new TestStartStopVectorCallbackStub();
    int32_t ret = sourceManager_->StartRemoteInput(srcId, sinkId, dhIds, callback);
    EXPECT_EQ(ERR_DH_INPUT_SERVER_SOURCE_MANAGER_START_FAIL, ret);
}

HWTEST_F(DistributedInputSourceManagerTest, StopRemoteInput01, testing::ext::TestSize.Level0)
{
    std::string devId = "umkyu1b165e1be98151891erbe8r91ev";
    sptr<TestStopDInputCallback> callback = new TestStopDInputCallback();
    int32_t ret = sourceManager_->StopRemoteInput(devId, static_cast<uint32_t>(DInputDeviceType::ALL), callback);
    EXPECT_EQ(DH_SUCCESS, ret);
}

HWTEST_F(DistributedInputSourceManagerTest, StopRemoteInput02, testing::ext::TestSize.Level0)
{
    std::string devId = "";
    sptr<TestStopDInputCallback> callback = new TestStopDInputCallback();
    int32_t ret = sourceManager_->StopRemoteInput(devId, static_cast<uint32_t>(DInputDeviceType::ALL), callback);
    EXPECT_EQ(ERR_DH_INPUT_SERVER_SOURCE_MANAGER_STOP_FAIL, ret);
}

HWTEST_F(DistributedInputSourceManagerTest, StopRemoteInput03, testing::ext::TestSize.Level0)
{
    std::string devId = "umkyu1b165e1be98151891erbe8r91ev";
    sptr<TestStopDInputCallback> callback;
    int32_t ret = sourceManager_->StopRemoteInput(devId, static_cast<uint32_t>(DInputDeviceType::ALL), callback);
    EXPECT_EQ(ERR_DH_INPUT_SERVER_SOURCE_MANAGER_STOP_FAIL, ret);
}

/**
 * @tc.name: StopRemoteInput04
 * @tc.desc: verify the function of stoping distributed input on InputDeviceType.
 * @tc.type: FUNC
 * @tc.require: SR000H9J75
 */
HWTEST_F(DistributedInputSourceManagerTest, StopRemoteInput04, testing::ext::TestSize.Level0)
{
    std::string srcId = "networkidc08647073e02e7a78f09473aa122ff57fc81c00";
    std::string sinkId = "umkyu1b165e1be98151891erbe8r91ev";
    sptr<TestStopDInputCallback> callback = new TestStopDInputCallback();
    int32_t ret =
        sourceManager_->StopRemoteInput(srcId, sinkId, static_cast<uint32_t>(DInputDeviceType::ALL), callback);
    EXPECT_EQ(DH_SUCCESS, ret);
}

/**
 * @tc.name: StopRemoteInput05
 * @tc.desc: verify the function of stoping distributed input on InputDeviceType.
 * @tc.type: FUNC
 * @tc.require: SR000H9J75
 */
HWTEST_F(DistributedInputSourceManagerTest, StopRemoteInput05, testing::ext::TestSize.Level0)
{
    std::string srcId = "";
    std::string sinkId = "";
    sptr<TestStopDInputCallback> callback = new TestStopDInputCallback();
    int32_t ret =
        sourceManager_->StopRemoteInput(srcId, sinkId, static_cast<uint32_t>(DInputDeviceType::ALL), callback);
    EXPECT_EQ(ERR_DH_INPUT_SERVER_SOURCE_MANAGER_STOP_FAIL, ret);
}

/**
 * @tc.name: StopRemoteInput06
 * @tc.desc: verify the function of stoping distributed input on InputDeviceType.
 * @tc.type: FUNC
 * @tc.require: SR000H9J75
 */
HWTEST_F(DistributedInputSourceManagerTest, StopRemoteInput06, testing::ext::TestSize.Level0)
{
    std::string srcId = "networkidc08647073e02e7a78f09473aa122ff57fc81c00";
    std::string sinkId = "umkyu1b165e1be98151891erbe8r91ev";
    sptr<TestStopDInputCallback> callback;
    int32_t ret =
        sourceManager_->StopRemoteInput(srcId, sinkId, static_cast<uint32_t>(DInputDeviceType::ALL), callback);
    EXPECT_EQ(ERR_DH_INPUT_SERVER_SOURCE_MANAGER_STOP_FAIL, ret);
}

/**
 * @tc.name: StopRemoteInput07
 * @tc.desc: verify the function of stoping distributed input with dhid.
 * @tc.type: FUNC
 * @tc.require: SR000H9J74
 */
HWTEST_F(DistributedInputSourceManagerTest, StopRemoteInput07, testing::ext::TestSize.Level0)
{
    std::string srcId = "networkidc08647073e02e7a78f09473aa122ff57fc81c00";
    std::string sinkId = "umkyu1b165e1be98151891erbe8r91ev";
    std::vector<std::string> dhIds;
    dhIds.push_back("Input_slkdiek3kddkeojfe");
    sptr<TestStartStopVectorCallbackStub> callback = new TestStartStopVectorCallbackStub();
    int32_t ret = sourceManager_->StopRemoteInput(srcId, sinkId, dhIds, callback);
    EXPECT_EQ(DH_SUCCESS, ret);
}

/**
 * @tc.name: StopRemoteInput08
 * @tc.desc: verify the function of stoping distributed input with dhid.
 * @tc.type: FUNC
 * @tc.require: SR000H9J74
 */
HWTEST_F(DistributedInputSourceManagerTest, StopRemoteInput08, testing::ext::TestSize.Level0)
{
    std::string srcId = "networkidc08647073e02e7a78f09473aa122ff57fc81c00";
    std::string sinkId = "umkyu1b165e1be98151891erbe8r91ev";
    std::vector<std::string> dhIds;
    sptr<TestStartStopVectorCallbackStub> callback;
    int32_t ret = sourceManager_->StopRemoteInput(srcId, sinkId, dhIds, callback);
    EXPECT_EQ(ERR_DH_INPUT_SERVER_SOURCE_MANAGER_STOP_FAIL, ret);
}

/**
 * @tc.name: StopRemoteInput09
 * @tc.desc: verify the function of stoping distributed input with dhid.
 * @tc.type: FUNC
 * @tc.require: SR000H9J74
 */
HWTEST_F(DistributedInputSourceManagerTest, StopRemoteInput09, testing::ext::TestSize.Level0)
{
    std::string srcId = "";
    std::string sinkId = "";
    std::vector<std::string> dhIds;
    dhIds.push_back("Input_sldkjfldsjf234mdwswo");
    sptr<TestStartStopVectorCallbackStub> callback = new TestStartStopVectorCallbackStub();
    int32_t ret = sourceManager_->StopRemoteInput(srcId, sinkId, dhIds, callback);
    EXPECT_EQ(ERR_DH_INPUT_SERVER_SOURCE_MANAGER_STOP_FAIL, ret);
}

HWTEST_F(DistributedInputSourceManagerTest, UnprepareRemoteInput01, testing::ext::TestSize.Level0)
{
    std::string devId = "umkyu1b165e1be98151891erbe8r91ev";
    sptr<TestUnprepareDInputCallback> callback = new TestUnprepareDInputCallback();
    int32_t ret = sourceManager_->UnprepareRemoteInput(devId, callback);
    EXPECT_EQ(DH_SUCCESS, ret);
}

HWTEST_F(DistributedInputSourceManagerTest, UnprepareRemoteInput02, testing::ext::TestSize.Level0)
{
    std::string devId = "";
    sptr<TestUnprepareDInputCallback> callback = new TestUnprepareDInputCallback();
    int32_t ret = sourceManager_->UnprepareRemoteInput(devId, callback);
    EXPECT_EQ(ERR_DH_INPUT_SERVER_SOURCE_MANAGER_UNPREPARE_FAIL, ret);
}

HWTEST_F(DistributedInputSourceManagerTest, UnprepareRemoteInput03, testing::ext::TestSize.Level0)
{
    std::string devId = "umkyu1b165e1be98151891erbe8r91ev";
    sptr<TestUnprepareDInputCallback> callback;
    int32_t ret = sourceManager_->UnprepareRemoteInput(devId, callback);
    EXPECT_EQ(ERR_DH_INPUT_SERVER_SOURCE_MANAGER_UNPREPARE_FAIL, ret);
}

/**
 * @tc.name: UnprepareRemoteInput04
 * @tc.desc: verify the function of distributing data from any device.
 * @tc.type: FUNC
 * @tc.require: SR000H9J75
 */
HWTEST_F(DistributedInputSourceManagerTest, UnprepareRemoteInput04, testing::ext::TestSize.Level0)
{
    std::string srcId = "networkidc08647073e02e7a78f09473aa122ff57fc81c00";
    std::string sinkId = "umkyu1b165e1be98151891erbe8r91ev";
    sptr<TestUnprepareDInputCallback> callback = new TestUnprepareDInputCallback();
    int32_t ret = sourceManager_->UnprepareRemoteInput(srcId, sinkId, callback);
    EXPECT_EQ(DH_SUCCESS, ret);
}

/**
 * @tc.name: UnprepareRemoteInput05
 * @tc.desc: verify the function of distributing data from any device.
 * @tc.type: FUNC
 * @tc.require: SR000H9J75
 */
HWTEST_F(DistributedInputSourceManagerTest, UnprepareRemoteInput05, testing::ext::TestSize.Level0)
{
    std::string srcId = "";
    std::string sinkId = "";
    sptr<TestUnprepareDInputCallback> callback = new TestUnprepareDInputCallback();
    int32_t ret = sourceManager_->UnprepareRemoteInput(srcId, sinkId, callback);
    EXPECT_EQ(ERR_DH_INPUT_SERVER_SOURCE_MANAGER_UNPREPARE_FAIL, ret);
}

/**
 * @tc.name: UnprepareRemoteInput06
 * @tc.desc: verify the function of distributing data from any device.
 * @tc.type: FUNC
 * @tc.require: SR000H9J75
 */
HWTEST_F(DistributedInputSourceManagerTest, UnprepareRemoteInput06, testing::ext::TestSize.Level0)
{
    std::string srcId = "networkidc08647073e02e7a78f09473aa122ff57fc81c00";
    std::string sinkId = "umkyu1b165e1be98151891erbe8r91ev";
    sptr<TestUnprepareDInputCallback> callback;
    int32_t ret = sourceManager_->UnprepareRemoteInput(srcId, sinkId, callback);
    EXPECT_EQ(ERR_DH_INPUT_SERVER_SOURCE_MANAGER_UNPREPARE_FAIL, ret);
}

/**
 * @tc.name: UnprepareRemoteInput01
 * @tc.desc: verify the function of disabling a peripheral device.
 * @tc.type: FUNC
 * @tc.require: SR000H9J77
 */
HWTEST_F(DistributedInputSourceManagerTest, UnregisterDistributedHardware01, testing::ext::TestSize.Level0)
{
    std::string devId = "umkyu1b165e1be98151891erbe8r91ev";
    std::string dhId = "1ds56v18e1v21v8v1erv15r1v8r1j1ty8";
    sptr<TestUnregisterDInputCb> callback = new TestUnregisterDInputCb();
    int32_t ret = sourceManager_->UnregisterDistributedHardware(devId, dhId, callback);
    EXPECT_EQ(ERR_DH_INPUT_SERVER_SOURCE_MANAGER_UNREGISTER_FAIL, ret);
}

/**
 * @tc.name: UnprepareRemoteInput02
 * @tc.desc: verify the function of disabling a peripheral device.
 * @tc.type: FUNC
 * @tc.require: SR000H9J77
 */
HWTEST_F(DistributedInputSourceManagerTest, UnregisterDistributedHardware02, testing::ext::TestSize.Level0)
{
    std::string devId = "umkyu1b165e1be98151891erbe8r91ev";
    std::string dhId = "rt12r1nr81n521be8rb1erbe1w8bg1erb18";
    sptr<TestUnregisterDInputCb> callback;
    int32_t ret = sourceManager_->UnregisterDistributedHardware(devId, dhId, callback);
    EXPECT_EQ(ERR_DH_INPUT_SERVER_SOURCE_MANAGER_UNREGISTER_FAIL, ret);
}

HWTEST_F(DistributedInputSourceManagerTest, UnregisterDistributedHardware03, testing::ext::TestSize.Level0)
{
    std::string devId = "umkyu1b165e1be98151891erbe8r91ev";
    std::string dhId = "afv4s8b1dr1b8er1bd65fb16redb1dfb18d1b56df1b68d";
    sptr<TestUnregisterDInputCb> callback = new TestUnregisterDInputCb();
    int32_t ret = sourceManager_->UnregisterDistributedHardware(devId, dhId, callback);
    EXPECT_EQ(ERR_DH_INPUT_SERVER_SOURCE_MANAGER_UNREGISTER_FAIL, ret);
}

HWTEST_F(DistributedInputSourceManagerTest, RegisterAddWhiteListCallback01, testing::ext::TestSize.Level0)
{
    sptr<TestAddWhiteListInfosCb> callback;
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
    sptr<TestDelWhiteListInfosCb> callback;
    int32_t ret = sourceManager_->RegisterDelWhiteListCallback(callback);
    EXPECT_EQ(ERR_DH_INPUT_SERVER_SOURCE_MANAGER_REG_CALLBACK_ERR, ret);
}

HWTEST_F(DistributedInputSourceManagerTest, RegisterDelWhiteListCallback02, testing::ext::TestSize.Level0)
{
    sptr<TestDelWhiteListInfosCb> callback = new TestDelWhiteListInfosCb();
    int32_t ret = sourceManager_->RegisterDelWhiteListCallback(callback);
    EXPECT_EQ(DH_SUCCESS, ret);
}

HWTEST_F(DistributedInputSourceManagerTest, RegisterInputNodeListener01, testing::ext::TestSize.Level0)
{
    sptr<TestInputNodeListenerCb> callback = new TestInputNodeListenerCb();
    int32_t ret = sourceManager_->RegisterInputNodeListener(callback);
    EXPECT_EQ(DH_SUCCESS, ret);
}

HWTEST_F(DistributedInputSourceManagerTest, RegisterInputNodeListener02, testing::ext::TestSize.Level0)
{
    sptr<TestInputNodeListenerCb> callback = nullptr;
    int32_t ret = sourceManager_->RegisterInputNodeListener(callback);
    EXPECT_EQ(ERR_DH_INPUT_SERVER_SOURCE_MANAGER_NODE_LISTENER_CALLBACK_ERR, ret);
}

HWTEST_F(DistributedInputSourceManagerTest, UnRegisterInputNodeListener01, testing::ext::TestSize.Level0)
{
    sptr<TestInputNodeListenerCb> callback = new TestInputNodeListenerCb();
    int32_t ret = sourceManager_->UnregisterInputNodeListener(callback);
    EXPECT_EQ(DH_SUCCESS, ret);
}

HWTEST_F(DistributedInputSourceManagerTest, UnRegisterInputNodeListener02, testing::ext::TestSize.Level0)
{
    sptr<TestInputNodeListenerCb> callback = nullptr;
    int32_t ret = sourceManager_->UnregisterInputNodeListener(callback);
    EXPECT_EQ(ERR_DH_INPUT_SERVER_SOURCE_MANAGER_NODE_LISTENER_CALLBACK_ERR, ret);
}

HWTEST_F(DistributedInputSourceManagerTest, RegisterSimulationEventListener, testing::ext::TestSize.Level0)
{
    sptr<TestSimulationEventCb> callback = new TestSimulationEventCb();
    int32_t ret = sourceManager_->RegisterSimulationEventListener(callback);
    EXPECT_EQ(DH_SUCCESS, ret);
}

HWTEST_F(DistributedInputSourceManagerTest, UnregisterSimulationEventListener, testing::ext::TestSize.Level0)
{
    sptr<TestSimulationEventCb> callback = new TestSimulationEventCb();
    int32_t ret = sourceManager_->UnregisterSimulationEventListener(callback);
    EXPECT_EQ(DH_SUCCESS, ret);
}

HWTEST_F(DistributedInputSourceManagerTest, IsStringDataSame01, testing::ext::TestSize.Level0)
{
    std::vector<std::string> oldDhIds;
    std::vector<std::string> newDhIds;
    oldDhIds.push_back("afv4s8b1dr1b8er1bd65fb16redb1dfb18d1b56df1b68d");
    oldDhIds.push_back("rt12r1nr81n521be8rb1erbe1w8bg1erb18");
    newDhIds.push_back("afv4s8b1dr1b8er1bd65fb16redb1dfb18d1b56df1b68d");
    bool ret = sourceManager_->IsStringDataSame(oldDhIds, newDhIds);
    EXPECT_EQ(false, ret);
}

HWTEST_F(DistributedInputSourceManagerTest, IsStringDataSame02, testing::ext::TestSize.Level0)
{
    std::vector<std::string> oldDhIds;
    std::vector<std::string> newDhIds;
    oldDhIds.push_back("afv4s8b1dr1b8er1bd65fb16redb1dfb18d1b56df1b68d");
    oldDhIds.push_back("rt12r1nr81n521be8rb1erbe1w8bg1erb18");
    newDhIds.push_back("afv4s8b1dr1b8er1bd65fb16redb1dfb18d1b56df1b68d");
    newDhIds.push_back("rt12r1nr81n521be8rb1erbe1w8bg1erb18");
    bool ret = sourceManager_->IsStringDataSame(oldDhIds, newDhIds);
    EXPECT_EQ(true, ret);
}

HWTEST_F(DistributedInputSourceManagerTest, StartRemoteInput_010, testing::ext::TestSize.Level0)
{
    std::string sinkId = "umkyu1b165e1be98151891erbe8r91ev";
    std::vector<std::string> dhIds;
    dhIds.push_back("Input_slkdiek3kddkeojfe");
    sptr<TestStartStopDInputsCb> callback = new TestStartStopDInputsCb();
    int32_t ret = sourceManager_->StartRemoteInput(sinkId, dhIds, callback);
    EXPECT_EQ(DH_SUCCESS, ret);
}

HWTEST_F(DistributedInputSourceManagerTest, StartRemoteInput_011, testing::ext::TestSize.Level0)
{
    std::string sinkId = "umkyu1b165e1be98151891erbe8r91ev";
    std::vector<std::string> dhIds;
    dhIds.push_back("Input_slkdiek3kddkeojfe");
    sptr<TestStartStopDInputsCb> callback = new TestStartStopDInputsCb();
    std::string localNetworkId = "networkidc08647073e02e7a78f09473aa122ff57fc81c00";
    DistributedInputSourceManager::DInputClientStartDhidInfo info {localNetworkId, sinkId, dhIds, callback};
    sourceManager_->staStringCallbacks_.push_back(info);
    int32_t ret = sourceManager_->StartRemoteInput(sinkId, dhIds, callback);
    EXPECT_EQ(ERR_DH_INPUT_SERVER_SOURCE_MANAGER_START_FAIL, ret);
}

HWTEST_F(DistributedInputSourceManagerTest, StartRemoteInput_012, testing::ext::TestSize.Level0)
{
    std::string sinkId = "umkyu1b165e1be98151891erbe8r91ev";
    std::vector<std::string> dhIds;
    dhIds.push_back("Input_slkdiek3kddkeojfe");
    sptr<TestStartStopDInputsCb> callback = nullptr;
    int32_t ret = sourceManager_->StartRemoteInput(sinkId, dhIds, callback);
    EXPECT_EQ(ERR_DH_INPUT_SERVER_SOURCE_MANAGER_START_FAIL, ret);
}

HWTEST_F(DistributedInputSourceManagerTest, StopRemoteInput_010, testing::ext::TestSize.Level0)
{
    std::string sinkId = "umkyu1b165e1be98151891erbe8r91ev";
    std::vector<std::string> dhIds;
    dhIds.push_back("Input_slkdiek3kddkeojfe");
    sptr<TestStartStopDInputsCb> callback = new TestStartStopDInputsCb();
    std::string localNetworkId = "networkidc08647073e02e7a78f09473aa122ff57fc81c00";
    int32_t ret = sourceManager_->StopRemoteInput(sinkId, dhIds, callback);
    EXPECT_EQ(DH_SUCCESS, ret);
}

HWTEST_F(DistributedInputSourceManagerTest, StopRemoteInput_011, testing::ext::TestSize.Level0)
{
    std::string sinkId = "umkyu1b165e1be98151891erbe8r91ev";
    std::vector<std::string> dhIds;
    dhIds.push_back("Input_slkdiek3kddkeojfe");
    sptr<TestStartStopDInputsCb> callback = new TestStartStopDInputsCb();
    std::string localNetworkId = "networkidc08647073e02e7a78f09473aa122ff57fc81c00";
    DistributedInputSourceManager::DInputClientStopDhidInfo info {localNetworkId, sinkId, dhIds, callback};
    sourceManager_->stpStringCallbacks_.push_back(info);
    int32_t ret = sourceManager_->StopRemoteInput(sinkId, dhIds, callback);
    EXPECT_EQ(ERR_DH_INPUT_SERVER_SOURCE_MANAGER_STOP_FAIL, ret);
}

HWTEST_F(DistributedInputSourceManagerTest, StartRemoteInput_013, testing::ext::TestSize.Level0)
{
    std::string sinkId = "umkyu1b165e1be98151891erbe8r91ev";
    std::vector<std::string> dhIds;
    dhIds.push_back("Input_slkdiek3kddkeojfe");
    sptr<TestStartStopDInputsCb> callback = nullptr;
    int32_t ret = sourceManager_->StopRemoteInput(sinkId, dhIds, callback);
    EXPECT_EQ(ERR_DH_INPUT_SERVER_SOURCE_MANAGER_STOP_FAIL, ret);
}

HWTEST_F(DistributedInputSourceManagerTest, SyncNodeInfoRemoteInput_01, testing::ext::TestSize.Level1)
{
    std::string userDevId = "umkyu1b165e1be98151891erbe8r91ev";
    std::string dhId = "input_slkdiek3kddkeojfe";
    std::string nodeDesc = "input_deviceid:umkyu1b165e1be98151891erbe8r91ev, input_dhid:slkdiek3kddkeojfe";
    int32_t ret = sourceManager_->SyncNodeInfoRemoteInput(userDevId, dhId, nodeDesc);
    EXPECT_EQ(DH_SUCCESS, ret);
}

HWTEST_F(DistributedInputSourceManagerTest, RelayPrepareRemoteInput_01, testing::ext::TestSize.Level1)
{
    std::string srcId = "networkidc08647073e02e7a78f09473aa122ff57fc81c00";
    std::string sinkId = "umkyu1b165e1be98151891erbe8r91ev";
    sptr<TestPrepareDInputCallback> callback = new TestPrepareDInputCallback();
    DistributedInputSourceManager::DInputClientRelayPrepareInfo info(srcId, sinkId, callback);
    sourceManager_->relayPreCallbacks_.push_back(info);
    int32_t ret = sourceManager_->RelayPrepareRemoteInput(srcId, sinkId, callback);
    EXPECT_EQ(ERR_DH_INPUT_SERVER_SOURCE_MANAGER_PREPARE_FAIL, ret);
}

HWTEST_F(DistributedInputSourceManagerTest, RelayUnprepareRemoteInput_01, testing::ext::TestSize.Level1)
{
    std::string srcId = "networkidc08647073e02e7a78f09473aa122ff57fc81c00";
    std::string sinkId = "umkyu1b165e1be98151891erbe8r91ev";
    sptr<TestUnprepareDInputCallback> callback = new TestUnprepareDInputCallback();
    DistributedInputSourceManager::DInputClientRelayUnprepareInfo info(srcId, sinkId, callback);
    sourceManager_->relayUnpreCallbacks_.push_back(info);
    int32_t ret = sourceManager_->RelayUnprepareRemoteInput(srcId, sinkId, callback);
    EXPECT_EQ(ERR_DH_INPUT_SERVER_SOURCE_MANAGER_UNPREPARE_FAIL, ret);
}

HWTEST_F(DistributedInputSourceManagerTest, RelayStartRemoteInputByDhid_01, testing::ext::TestSize.Level1)
{
    std::string srcId = "networkidc08647073e02e7a78f09473aa122ff57fc81c00";
    std::string sinkId = "umkyu1b165e1be98151891erbe8r91ev";
    std::vector<std::string> dhIds;
    dhIds.push_back("input_slkdiek3kddkeojfe");
    sptr<TestStartStopDInputsCb> callback = new TestStartStopDInputsCb();
    DistributedInputSourceManager::DInputClientStartDhidInfo info{srcId, sinkId, dhIds, callback};
    sourceManager_->relayStaDhidCallbacks_.push_back(info);
    int32_t ret = sourceManager_->RelayStartRemoteInputByDhid(srcId, sinkId, dhIds, callback);
    EXPECT_EQ(ERR_DH_INPUT_SERVER_SOURCE_MANAGER_START_FAIL, ret);
}

HWTEST_F(DistributedInputSourceManagerTest, RelayStopRemoteInputByDhid_01, testing::ext::TestSize.Level1)
{
    std::string srcId = "networkidc08647073e02e7a78f09473aa122ff57fc81c00";
    std::string sinkId = "umkyu1b165e1be98151891erbe8r91ev";
    std::vector<std::string> dhIds;
    dhIds.push_back("input_slkdiek3kddkeojfe");
    sptr<TestStartStopDInputsCb> callback = new TestStartStopDInputsCb();
    DistributedInputSourceManager::DInputClientStopDhidInfo info{srcId, sinkId, dhIds, callback};
    sourceManager_->relayStpDhidCallbacks_.push_back(info);
    int32_t ret = sourceManager_->RelayStopRemoteInputByDhid(srcId, sinkId, dhIds, callback);
    EXPECT_EQ(ERR_DH_INPUT_SERVER_SOURCE_MANAGER_STOP_FAIL, ret);
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
} // namespace DistributedInput
} // namespace DistributedHardware
} // namespace OHOS