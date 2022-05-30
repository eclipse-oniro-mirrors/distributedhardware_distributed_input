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
namespace {
    static const uint32_t INPUT_DEVICE_CLASS_KEYBOARD =
        static_cast<uint32_t>(DeviceClasses::INPUT_DEVICE_CLASS_KEYBOARD);
    static const uint32_t INPUT_DEVICE_CLASS_CURSOR = static_cast<uint32_t>(DeviceClasses::INPUT_DEVICE_CLASS_CURSOR);
    static const uint32_t INPUT_DEVICE_CLASS_TOUCH = static_cast<uint32_t>(DeviceClasses::INPUT_DEVICE_CLASS_TOUCH);
}

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
}

void DistributedInputSourceManagerTest::TestRegisterDInputCb::OnResult(
    const std::string& devId, const std::string& dhId, const int32_t& status) const
{
    return;
}

void DistributedInputSourceManagerTest::TestUnregisterDInputCb::OnResult(
    const std::string& devId, const std::string& dhId, const int32_t& status) const
{
    return;
}

void DistributedInputSourceManagerTest::TestPrepareDInputCallback::OnResult(
    const std::string& deviceId, const int32_t& status) const
{
    return;
}

void DistributedInputSourceManagerTest::TestUnprepareDInputCallback::OnResult(
    const std::string& deviceId, const int32_t& status) const
{
    return;
}

void DistributedInputSourceManagerTest::TestStartDInputCallback::OnResult(
    const std::string& deviceId, const uint32_t& inputTypes, const int32_t& status) const
{
    return;
}

void DistributedInputSourceManagerTest::TestStopDInputCallback::OnResult(
    const std::string& deviceId, const uint32_t& inputTypes, const int32_t& status) const
{
    return;
}

void DistributedInputSourceManagerTest::TestAddWhiteListInfosCb::OnResult(
    const std::string &deviceId, const std::string &strJson) const
{
    return;
}

void DistributedInputSourceManagerTest::TestDelWhiteListInfosCb::OnResult(
    const std::string& deviceId) const
{
    return;
}

int32_t DistributedInputSourceManagerTest::StructTransJson(const InputDevice& pBuf, std::string& strDescriptor) const
{
    nlohmann::json tmpJson;
    tmpJson["name"] = pBuf.name;
    tmpJson["location"] = pBuf.location;
    tmpJson["uniqueId"] = pBuf.uniqueId;
    tmpJson["bus"] = pBuf.bus;
    tmpJson["vendor"] = pBuf.vendor;
    tmpJson["product"] = pBuf.product;
    tmpJson["version"] = pBuf.version;
    tmpJson["descriptor"] = pBuf.descriptor;
    tmpJson["nonce"] = pBuf.nonce;
    tmpJson["classes"] = pBuf.classes;

    std::ostringstream stream;
    stream << tmpJson.dump();
    strDescriptor = stream.str();
    return SUCCESS;
}

HWTEST_F(DistributedInputSourceManagerTest, Init01, testing::ext::TestSize.Level0)
{
    std::cout << "Init01"<< std::endl;
    int32_t ret = sourceManager_->Init();
    EXPECT_EQ(SUCCESS, ret);
}

HWTEST_F(DistributedInputSourceManagerTest, RegisterDistributedHardware01, testing::ext::TestSize.Level0)
{
    InputDevice pBuffer;
    pBuffer.name = "uinput_name_touch";
    pBuffer.bus = 0x03;
    pBuffer.vendor = 0x1233;
    pBuffer.product = 0xfedb;
    pBuffer.version = 3;
    pBuffer.location = "usb-hiusb-ehci-2.1/input1";
    pBuffer.uniqueId = "3";
    pBuffer.nonce = 0;
    pBuffer.classes = INPUT_DEVICE_CLASS_TOUCH;
    pBuffer.descriptor = "1ds56v18e1v21v8v1erv15r1v8r1j1ty8";

    std::string devId = "umkyu1b165e1be98151891erbe8r91ev";
    std::string dhId = pBuffer.descriptor;
    std::string parameters;
    StructTransJson(pBuffer, parameters);

    sptr<TestRegisterDInputCb> callback = new TestRegisterDInputCb();
    int32_t ret = sourceManager_->RegisterDistributedHardware(devId, dhId, parameters, callback);

    EXPECT_EQ(SUCCESS, ret);
}

HWTEST_F(DistributedInputSourceManagerTest, RegisterDistributedHardware02, testing::ext::TestSize.Level0)
{
    std::cout << "RegisterDistributedHardware1"<< std::endl;
    InputDevice pBuffer;
    pBuffer.name = "uinput_name_mouse";
    pBuffer.bus = 0x03;
    pBuffer.vendor = 0x1222;
    pBuffer.product = 0xfeda;
    pBuffer.version = 2;
    pBuffer.location = "usb-hiusb-ehci-2.1/input1";
    pBuffer.uniqueId = "2";
    pBuffer.nonce = 0;
    pBuffer.classes = INPUT_DEVICE_CLASS_CURSOR;
    pBuffer.descriptor = "rt12r1nr81n521be8rb1erbe1w8bg1erb18";

    std::string devId = "umkyu1b165e1be98151891erbe8r91ev";
    std::string dhId = pBuffer.descriptor;
    std::string parameters;
    std::cout << "RegisterDistributedHardware2"<< std::endl;
    StructTransJson(pBuffer, parameters);
    std::cout << "RegisterDistributedHardware3"<< std::endl;
    sptr<TestRegisterDInputCb> callback = new TestRegisterDInputCb();
    int32_t ret = sourceManager_->RegisterDistributedHardware(devId, dhId, parameters, callback);
    std::cout << "RegisterDistributedHardware4"<< std::endl;
    EXPECT_EQ(SUCCESS, ret);
}

HWTEST_F(DistributedInputSourceManagerTest, RegisterDistributedHardware03, testing::ext::TestSize.Level0)
{
    std::cout << "RegisterDistributedHardware11"<< std::endl;
    InputDevice pBuffer;
    pBuffer.name = "uinput_name_keyboard";
    pBuffer.bus = 0x03;
    pBuffer.vendor = 0x1234;
    pBuffer.product = 0xfedc;
    pBuffer.version = 1;
    pBuffer.location = "usb-hiusb-ehci-2.1/input1";
    pBuffer.uniqueId = "1";
    pBuffer.nonce = 0;
    pBuffer.classes = INPUT_DEVICE_CLASS_KEYBOARD;
    pBuffer.descriptor = "afv4s8b1dr1b8er1bd65fb16redb1dfb18d1b56df1b68d";

    std::string devId = "umkyu1b165e1be98151891erbe8r91ev";
    std::string dhId = pBuffer.descriptor;
    std::string parameters;
    std::cout << "RegisterDistributedHardware12"<< std::endl;
    StructTransJson(pBuffer, parameters);
    std::cout << "RegisterDistributedHardware13"<< std::endl;
    sptr<TestRegisterDInputCb> callback = new TestRegisterDInputCb();
    int32_t ret = sourceManager_->RegisterDistributedHardware(devId, dhId, parameters, callback);
    std::cout << "RegisterDistributedHardware03"<< std::endl;
    EXPECT_EQ(SUCCESS, ret);
}

HWTEST_F(DistributedInputSourceManagerTest, PrepareRemoteInput01, testing::ext::TestSize.Level0)
{
    std::string devId = "umkyu1b165e1be98151891erbe8r91ev";
    sptr<TestPrepareDInputCallback> callback = new TestPrepareDInputCallback();
    sptr<TestAddWhiteListInfosCb> addWhiteListCallback = new TestAddWhiteListInfosCb();
    std::cout << "PrepareRemoteInput011"<< std::endl;
    int32_t ret = sourceManager_->PrepareRemoteInput(devId, callback, addWhiteListCallback);
    std::cout << "PrepareRemoteInput012"<< std::endl;
    EXPECT_EQ(ERR_DH_INPUT_SERVER_SOURCE_MANAGER_PREPARE_FAIL, ret);
}

HWTEST_F(DistributedInputSourceManagerTest, PrepareRemoteInput02, testing::ext::TestSize.Level0)
{
    std::string devId = "umkyu1b165e1be98151891erbe8r91ev";
    sptr<TestPrepareDInputCallback> callback;
    sptr<TestAddWhiteListInfosCb> addWhiteListCallback = new TestAddWhiteListInfosCb();
    int32_t ret = sourceManager_->PrepareRemoteInput(devId, callback, addWhiteListCallback);
    EXPECT_EQ(ERR_DH_INPUT_SERVER_SOURCE_MANAGER_PREPARE_FAIL, ret);
}

HWTEST_F(DistributedInputSourceManagerTest, PrepareRemoteInput03, testing::ext::TestSize.Level0)
{
    std::string devId = "";
    sptr<TestPrepareDInputCallback> callback = new TestPrepareDInputCallback();
    sptr<TestAddWhiteListInfosCb> addWhiteListCallback = new TestAddWhiteListInfosCb();
    int32_t ret = sourceManager_->PrepareRemoteInput(devId, callback, addWhiteListCallback);
    EXPECT_EQ(ERR_DH_INPUT_SERVER_SOURCE_MANAGER_PREPARE_FAIL, ret);
}

HWTEST_F(DistributedInputSourceManagerTest, StartRemoteInput01, testing::ext::TestSize.Level0)
{
    std::string devId = "umkyu1b165e1be98151891erbe8r91ev";
    sptr<TestStartDInputCallback> callback = new TestStartDInputCallback();
    int32_t ret = sourceManager_->StartRemoteInput(devId, INPUT_TYPE_ALL, callback);
    EXPECT_EQ(ERR_DH_INPUT_SERVER_SOURCE_MANAGER_START_FAIL, ret);
}

HWTEST_F(DistributedInputSourceManagerTest, StartRemoteInput02, testing::ext::TestSize.Level0)
{
    std::string devId = "umkyu1b165e1be98151891erbe8r91ev";
    sptr<TestStartDInputCallback> callback;
    int32_t ret = sourceManager_->StartRemoteInput(devId, INPUT_TYPE_ALL, callback);
    EXPECT_EQ(ERR_DH_INPUT_SERVER_SOURCE_MANAGER_START_FAIL, ret);
}

HWTEST_F(DistributedInputSourceManagerTest, StartRemoteInput03, testing::ext::TestSize.Level0)
{
    std::string devId = "";
    sptr<TestStartDInputCallback> callback = new TestStartDInputCallback();
    int32_t ret = sourceManager_->StartRemoteInput(devId, INPUT_TYPE_ALL, callback);
    EXPECT_EQ(ERR_DH_INPUT_SERVER_SOURCE_MANAGER_START_FAIL, ret);
}

HWTEST_F(DistributedInputSourceManagerTest, StopRemoteInput01, testing::ext::TestSize.Level0)
{
    std::string devId = "umkyu1b165e1be98151891erbe8r91ev";
    sptr<TestStopDInputCallback> callback = new TestStopDInputCallback();
    int32_t ret = sourceManager_->StopRemoteInput(devId, INPUT_TYPE_ALL, callback);
    EXPECT_EQ(ERR_DH_INPUT_SERVER_SOURCE_MANAGER_STOP_FAIL, ret);
}

HWTEST_F(DistributedInputSourceManagerTest, StopRemoteInput02, testing::ext::TestSize.Level0)
{
    std::string devId = "";
    sptr<TestStopDInputCallback> callback = new TestStopDInputCallback();
    int32_t ret = sourceManager_->StopRemoteInput(devId, INPUT_TYPE_ALL, callback);
    EXPECT_EQ(ERR_DH_INPUT_SERVER_SOURCE_MANAGER_STOP_FAIL, ret);
}

HWTEST_F(DistributedInputSourceManagerTest, StopRemoteInput03, testing::ext::TestSize.Level0)
{
    std::string devId = "umkyu1b165e1be98151891erbe8r91ev";
    sptr<TestStopDInputCallback> callback;
    int32_t ret = sourceManager_->StopRemoteInput(devId, INPUT_TYPE_ALL, callback);
    EXPECT_EQ(ERR_DH_INPUT_SERVER_SOURCE_MANAGER_STOP_FAIL, ret);
}

HWTEST_F(DistributedInputSourceManagerTest, UnprepareRemoteInput01, testing::ext::TestSize.Level0)
{
    std::string devId = "umkyu1b165e1be98151891erbe8r91ev";
    sptr<TestUnprepareDInputCallback> callback = new TestUnprepareDInputCallback();
    sptr<TestDelWhiteListInfosCb> delWhiteListCallback = new TestDelWhiteListInfosCb();
    int32_t ret = sourceManager_->UnprepareRemoteInput(devId, callback, delWhiteListCallback);
    EXPECT_EQ(ERR_DH_INPUT_SERVER_SOURCE_MANAGER_UNPREPARE_FAIL, ret);
}

HWTEST_F(DistributedInputSourceManagerTest, UnprepareRemoteInput02, testing::ext::TestSize.Level0)
{
    std::string devId = "";
    sptr<TestUnprepareDInputCallback> callback = new TestUnprepareDInputCallback();
    sptr<TestDelWhiteListInfosCb> delWhiteListCallback = new TestDelWhiteListInfosCb();
    int32_t ret = sourceManager_->UnprepareRemoteInput(devId, callback, delWhiteListCallback);
    EXPECT_EQ(ERR_DH_INPUT_SERVER_SOURCE_MANAGER_UNPREPARE_FAIL, ret);
}

HWTEST_F(DistributedInputSourceManagerTest, UnprepareRemoteInput03, testing::ext::TestSize.Level0)
{
    std::string devId = "umkyu1b165e1be98151891erbe8r91ev";
    sptr<TestUnprepareDInputCallback> callback;
    sptr<TestDelWhiteListInfosCb> delWhiteListCallback;
    int32_t ret = sourceManager_->UnprepareRemoteInput(devId, callback, delWhiteListCallback);
    EXPECT_EQ(ERR_DH_INPUT_SERVER_SOURCE_MANAGER_UNPREPARE_FAIL, ret);
}

HWTEST_F(DistributedInputSourceManagerTest, UnregisterDistributedHardware01, testing::ext::TestSize.Level0)
{
    std::string devId = "umkyu1b165e1be98151891erbe8r91ev";
    std::string dhId = "1ds56v18e1v21v8v1erv15r1v8r1j1ty8";
    sptr<TestUnregisterDInputCb> callback = new TestUnregisterDInputCb();
    int32_t ret = sourceManager_->UnregisterDistributedHardware(devId, dhId, callback);
    EXPECT_EQ(ERR_DH_INPUT_SERVER_SOURCE_MANAGER_UNREGISTER_FAIL, ret);
}

HWTEST_F(DistributedInputSourceManagerTest, UnregisterDistributedHardware02, testing::ext::TestSize.Level0)
{
    std::string devId = "umkyu1b165e1be98151891erbe8r91ev";
    std::string dhId = "rt12r1nr81n521be8rb1erbe1w8bg1erb18";
    sptr<TestUnregisterDInputCb> callback = new TestUnregisterDInputCb();
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
}
}
}
