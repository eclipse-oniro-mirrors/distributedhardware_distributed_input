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

#include "distributed_input_inner_test.h"

using namespace testing::ext;
using namespace OHOS::DistributedHardware::DistributedInput;
using namespace std;
using namespace OHOS;

namespace OHOS {
namespace DistributedHardware {
namespace DistributedInput {
void DistributedInputInnerTest::SetUp()
{
}

void DistributedInputInnerTest::TearDown()
{
}

void DistributedInputInnerTest::SetUpTestCase()
{
}

void DistributedInputInnerTest::TearDownTestCase()
{
}

void DistributedInputInnerTest::TestPrepareDInputCallback::OnResult(
    const std::string& deviceId, const int32_t& status)
{
    (void)deviceId;
    (void)status;
    return;
}

void DistributedInputInnerTest::TestUnprepareDInputCallback::OnResult(
    const std::string& deviceId, const int32_t& status)
{
    (void)deviceId;
    (void)status;
    return;
}

void DistributedInputInnerTest::TestStartDInputCallback::OnResult(
    const std::string& deviceId, const uint32_t& inputTypes, const int32_t& status)
{
    (void)deviceId;
    (void)inputTypes;
    (void)status;
    return;
}

void DistributedInputInnerTest::TestStopDInputCallback::OnResult(
    const std::string& deviceId, const uint32_t& inputTypes, const int32_t& status)
{
    (void)deviceId;
    (void)inputTypes;
    (void)status;
    return;
}

int DistributedInputInnerTest::CheckSourceProxy() const
{
    OHOS::sptr<OHOS::ISystemAbilityManager> systemAbilityManager =
        OHOS::SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (!systemAbilityManager) {
        return SUCCESS;
    }

    OHOS::sptr<OHOS::IRemoteObject> remoteObject = systemAbilityManager->GetSystemAbility(
        IDistributedSourceInput::SA_ID_DISTRIBUTED_INPUT_SOURCE_SERVICE);
    if (!remoteObject) {
        return SUCCESS;
    }

    OHOS::sptr<IDistributedSourceInput> proxyTest;

    proxyTest = OHOS::iface_cast<IDistributedSourceInput>(remoteObject);
    if ((!proxyTest) || (!proxyTest->AsObject())) {
        return SUCCESS;
    }

    return SUCCESS;
}

int DistributedInputInnerTest::CheckSinkProxy() const
{
    OHOS::sptr<OHOS::ISystemAbilityManager> systemAbilityManager =
        OHOS::SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (!systemAbilityManager) {
        return SUCCESS;
    }

    OHOS::sptr<OHOS::IRemoteObject> remoteObject = systemAbilityManager->GetSystemAbility(
        IDistributedSinkInput::SA_ID_DISTRIBUTED_INPUT_SINK_SERVICE);
    if (!remoteObject) {
        return SUCCESS;
    }

    OHOS::sptr<IDistributedSinkInput> proxyTest;

    proxyTest = OHOS::iface_cast<IDistributedSinkInput>(remoteObject);
    if ((!proxyTest) || (!proxyTest->AsObject())) {
        return SUCCESS;
    }

    return SUCCESS;
}

HWTEST_F(DistributedInputInnerTest, PrepareRemoteInput01, testing::ext::TestSize.Level0)
{
    string deviceId = "PrepareRemoteInput01";
    sptr<TestPrepareDInputCallback> callback = new TestPrepareDInputCallback();
    int32_t ret = DistributedInputKit::PrepareRemoteInput(deviceId, callback);
    EXPECT_EQ(SUCCESS, ret);
}

HWTEST_F(DistributedInputInnerTest, PrepareRemoteInput02, testing::ext::TestSize.Level0)
{
    string deviceId = "";
    sptr<TestPrepareDInputCallback> callback = new TestPrepareDInputCallback();
    int32_t ret = DistributedInputKit::PrepareRemoteInput(deviceId, callback);
    EXPECT_EQ(SUCCESS, ret);
}

HWTEST_F(DistributedInputInnerTest, PrepareRemoteInput03, testing::ext::TestSize.Level0)
{
    string deviceId = "PrepareRemoteInput01";
    sptr<TestPrepareDInputCallback> callback = nullptr;
    int32_t ret = DistributedInputKit::PrepareRemoteInput(deviceId, callback);
    EXPECT_EQ(SUCCESS, ret);
}

HWTEST_F(DistributedInputInnerTest, PrepareRemoteInput04, testing::ext::TestSize.Level0)
{
    string deviceId = "";
    sptr<TestPrepareDInputCallback> callback = nullptr;
    int32_t ret = DistributedInputKit::PrepareRemoteInput(deviceId, callback);
    EXPECT_EQ(SUCCESS, ret);
}

HWTEST_F(DistributedInputInnerTest, UnprepareRemoteInput01, testing::ext::TestSize.Level0)
{
    string deviceId = "UnprepareRemoteInput01";
    sptr<TestUnprepareDInputCallback> callback = new TestUnprepareDInputCallback();
    int32_t ret = DistributedInputKit::UnprepareRemoteInput(deviceId, callback);
    EXPECT_EQ(SUCCESS, ret);
}

HWTEST_F(DistributedInputInnerTest, UnprepareRemoteInput02, testing::ext::TestSize.Level0)
{
    string deviceId = "";
    sptr<TestUnprepareDInputCallback> callback = new TestUnprepareDInputCallback();
    int32_t ret = DistributedInputKit::UnprepareRemoteInput(deviceId, callback);
    EXPECT_EQ(SUCCESS, ret);
}

HWTEST_F(DistributedInputInnerTest, UnprepareRemoteInput03, testing::ext::TestSize.Level0)
{
    string deviceId = "UnprepareRemoteInput01";
    sptr<TestUnprepareDInputCallback> callback = nullptr;
    int32_t ret = DistributedInputKit::UnprepareRemoteInput(deviceId, callback);
    EXPECT_EQ(SUCCESS, ret);
}

HWTEST_F(DistributedInputInnerTest, UnprepareRemoteInput04, testing::ext::TestSize.Level0)
{
    string deviceId = "";
    sptr<TestUnprepareDInputCallback> callback = nullptr;
    int32_t ret = DistributedInputKit::UnprepareRemoteInput(deviceId, callback);
    EXPECT_EQ(SUCCESS, ret);
}

HWTEST_F(DistributedInputInnerTest, StartRemoteInput01, testing::ext::TestSize.Level0)
{
    string deviceId = "StartRemoteInput01";
    sptr<TestStartDInputCallback> callback = new TestStartDInputCallback();
    int32_t ret =
        DistributedInputKit::StartRemoteInput(deviceId, static_cast<uint32_t>(DInputDeviceType::ALL), callback);
    EXPECT_EQ(SUCCESS, ret);
}

HWTEST_F(DistributedInputInnerTest, StartRemoteInput02, testing::ext::TestSize.Level0)
{
    string deviceId = "";
    sptr<TestStartDInputCallback> callback = new TestStartDInputCallback();
    int32_t ret =
        DistributedInputKit::StartRemoteInput(deviceId, static_cast<uint32_t>(DInputDeviceType::ALL), callback);
    EXPECT_EQ(SUCCESS, ret);
}

HWTEST_F(DistributedInputInnerTest, StartRemoteInput03, testing::ext::TestSize.Level0)
{
    string deviceId = "StartRemoteInput01";
    sptr<TestStartDInputCallback> callback = nullptr;
    int32_t ret =
        DistributedInputKit::StartRemoteInput(deviceId, static_cast<uint32_t>(DInputDeviceType::ALL), callback);
    EXPECT_EQ(SUCCESS, ret);
}

HWTEST_F(DistributedInputInnerTest, StartRemoteInput04, testing::ext::TestSize.Level0)
{
    string deviceId = "";
    sptr<TestStartDInputCallback> callback = nullptr;
    int32_t ret =
        DistributedInputKit::StartRemoteInput(deviceId, static_cast<uint32_t>(DInputDeviceType::ALL), callback);
    EXPECT_EQ(SUCCESS, ret);
}

HWTEST_F(DistributedInputInnerTest, StopRemoteInput01, testing::ext::TestSize.Level0)
{
    string deviceId = "StopRemoteInput01";
    sptr<TestStopDInputCallback> callback = new TestStopDInputCallback();
    int32_t ret =
        DistributedInputKit::StopRemoteInput(deviceId, static_cast<uint32_t>(DInputDeviceType::ALL), callback);
    EXPECT_EQ(SUCCESS, ret);
}

HWTEST_F(DistributedInputInnerTest, StopRemoteInput02, testing::ext::TestSize.Level0)
{
    string deviceId = "";
    sptr<TestStopDInputCallback> callback = new TestStopDInputCallback();
    int32_t ret =
        DistributedInputKit::StopRemoteInput(deviceId, static_cast<uint32_t>(DInputDeviceType::ALL), callback);
    EXPECT_EQ(SUCCESS, ret);
}

HWTEST_F(DistributedInputInnerTest, StopRemoteInput03, testing::ext::TestSize.Level0)
{
    string deviceId = "StopRemoteInput01";
    sptr<TestStopDInputCallback> callback = nullptr;
    int32_t ret =
        DistributedInputKit::StopRemoteInput(deviceId, static_cast<uint32_t>(DInputDeviceType::ALL), callback);
    EXPECT_EQ(SUCCESS, ret);
}

HWTEST_F(DistributedInputInnerTest, StopRemoteInput04, testing::ext::TestSize.Level0)
{
    string deviceId = "";
    sptr<TestStopDInputCallback> callback = nullptr;
    int32_t ret =
        DistributedInputKit::StopRemoteInput(deviceId, static_cast<uint32_t>(DInputDeviceType::ALL), callback);
    EXPECT_EQ(SUCCESS, ret);
}

HWTEST_F(DistributedInputInnerTest, IsNeedFilterOut01, testing::ext::TestSize.Level0)
{
    string deviceId = "IsNeedFilterOut01";
    BusinessEvent event;
    event.pressedKeys.push_back(29);
    event.pressedKeys.push_back(56);
    event.keyCode = 111;
    event.keyAction = 108;
    bool ret = DistributedInputKit::IsNeedFilterOut(deviceId, event);
    EXPECT_EQ(true, ret);
}

HWTEST_F(DistributedInputInnerTest, IsNeedFilterOut02, testing::ext::TestSize.Level0)
{
    string deviceId;
    BusinessEvent event;
    bool ret = DistributedInputKit::IsNeedFilterOut(deviceId, event);
    EXPECT_EQ(true, ret);
}

HWTEST_F(DistributedInputInnerTest, IsNeedFilterOut03, testing::ext::TestSize.Level0)
{
    string deviceId = "IsNeedFilterOut01";
    BusinessEvent event;
    event.pressedKeys.push_back(29);
    event.pressedKeys.push_back(56);
    event.keyCode = 111;
    event.keyAction = 108;
    bool ret = DistributedInputKit::IsNeedFilterOut(deviceId, event);
    EXPECT_EQ(true, ret);
}

HWTEST_F(DistributedInputInnerTest, IsStartDistributedInput1, testing::ext::TestSize.Level0)
{
    DistributedInputClient::GetInstance().ReleaseSource();
    DistributedInputClient::GetInstance().InitSink();
    DInputServerType ret = DistributedInputKit::IsStartDistributedInput(static_cast<uint32_t>(DInputDeviceType::ALL));

    if (ret == DInputServerType::NULL_SERVER_TYPE) {
        EXPECT_EQ(SUCCESS, 0);
    } else {
        EXPECT_EQ(SUCCESS, -1);
    }
}

HWTEST_F(DistributedInputInnerTest, IsStartDistributedInput2, testing::ext::TestSize.Level0)
{
    DInputServerType ret = DistributedInputKit::IsStartDistributedInput(static_cast<uint32_t>(DInputDeviceType::ALL));

    if (ret == DInputServerType::NULL_SERVER_TYPE) {
        EXPECT_EQ(SUCCESS, 0);
    } else {
        EXPECT_EQ(SUCCESS, -1);
    }
}
}
}
}