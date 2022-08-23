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
#include "distributed_input_sourcetrans_test.h"

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

HWTEST_F(DistributedInputSourceTransTest, Init, testing::ext::TestSize.Level0)
{
    int32_t ret = DistributedInputSourceTransport::GetInstance().Init();
    EXPECT_EQ(ERR_DH_INPUT_SERVER_SOURCE_TRANSPORT_INIT_FAIL, ret);
}

HWTEST_F(DistributedInputSourceTransTest, PrepareRemoteInput01, testing::ext::TestSize.Level0)
{
    std::string deviceId = "";
    int32_t ret = DistributedInputSourceTransport::GetInstance().PrepareRemoteInput(deviceId);
    EXPECT_EQ(ERR_DH_INPUT_SERVER_SOURCE_TRANSPORT_PREPARE_FAIL, ret);
}

HWTEST_F(DistributedInputSourceTransTest, PrepareRemoteInput02, testing::ext::TestSize.Level0)
{
    std::string deviceId = "tttt0001";
    int32_t ret = DistributedInputSourceTransport::GetInstance().PrepareRemoteInput(deviceId);
    EXPECT_EQ(ERR_DH_INPUT_SERVER_SOURCE_TRANSPORT_PREPARE_FAIL, ret);
}

HWTEST_F(DistributedInputSourceTransTest, PrepareRemoteInput03, testing::ext::TestSize.Level1)
{
    std::string deviceId = "f6d4c08647073e02e7a78f09473aa122ff57fc81c00981fcf5be989e7d112591";
    int32_t ret = DistributedInputSourceTransport::GetInstance().PrepareRemoteInput(deviceId);
    EXPECT_EQ(ERR_DH_INPUT_SERVER_SOURCE_TRANSPORT_PREPARE_FAIL, ret);
}

HWTEST_F(DistributedInputSourceTransTest, UnprepareRemoteInput01, testing::ext::TestSize.Level0)
{
    std::string deviceId = "";
    int32_t ret = DistributedInputSourceTransport::GetInstance().UnprepareRemoteInput(deviceId);
    EXPECT_EQ(ERR_DH_INPUT_SERVER_SOURCE_TRANSPORT_UNPREPARE_FAIL, ret);
}

HWTEST_F(DistributedInputSourceTransTest, UnprepareRemoteInput02, testing::ext::TestSize.Level0)
{
    std::string deviceId = "tttt0001";
    int32_t ret = DistributedInputSourceTransport::GetInstance().UnprepareRemoteInput(deviceId);
    EXPECT_EQ(ERR_DH_INPUT_SERVER_SOURCE_TRANSPORT_UNPREPARE_FAIL, ret);
}

HWTEST_F(DistributedInputSourceTransTest, UnprepareRemoteInput03, testing::ext::TestSize.Level0)
{
    std::string deviceId = "f6d4c08647073e02e7a78f09473aa122ff57fc81c00981fcf5be989e7d112591";
    int32_t ret = DistributedInputSourceTransport::GetInstance().UnprepareRemoteInput(deviceId);
    EXPECT_EQ(ERR_DH_INPUT_SERVER_SOURCE_TRANSPORT_UNPREPARE_FAIL, ret);
}

HWTEST_F(DistributedInputSourceTransTest, StartRemoteInput01, testing::ext::TestSize.Level0)
{
    std::string deviceId = "tttt0001";
    int32_t ret = DistributedInputSourceTransport::GetInstance().StartRemoteInput(
        deviceId, static_cast<uint32_t>(DInputDeviceType::ALL));
    EXPECT_EQ(ERR_DH_INPUT_SERVER_SOURCE_TRANSPORT_START_FAIL, ret);
}

HWTEST_F(DistributedInputSourceTransTest, StartRemoteInput02, testing::ext::TestSize.Level0)
{
    std::string deviceId = "f6d4c08647073e02e7a78f09473aa122ff57fc81c00981fcf5be989e7d112591";
    int32_t ret = DistributedInputSourceTransport::GetInstance().StartRemoteInput(
        deviceId, static_cast<uint32_t>(DInputDeviceType::ALL));
    EXPECT_EQ(ERR_DH_INPUT_SERVER_SOURCE_TRANSPORT_START_FAIL, ret);
}

HWTEST_F(DistributedInputSourceTransTest, StopRemoteInput01, testing::ext::TestSize.Level0)
{
    std::string deviceId = "tttt0001";
    int32_t ret = DistributedInputSourceTransport::GetInstance().StopRemoteInput(
        deviceId, static_cast<uint32_t>(DInputDeviceType::ALL));
    EXPECT_EQ(ERR_DH_INPUT_SERVER_SOURCE_TRANSPORT_STOP_FAIL, ret);
}

HWTEST_F(DistributedInputSourceTransTest, StopRemoteInput02, testing::ext::TestSize.Level0)
{
    std::string deviceId = "f6d4c08647073e02e7a78f09473aa122ff57fc81c00981fcf5be989e7d112591";
    int32_t ret = DistributedInputSourceTransport::GetInstance().StopRemoteInput(
        deviceId, static_cast<uint32_t>(DInputDeviceType::ALL));
    EXPECT_EQ(ERR_DH_INPUT_SERVER_SOURCE_TRANSPORT_STOP_FAIL, ret);
    DistributedInputSourceTransport::GetInstance().CloseInputSoftbus(deviceId);
}
} // namespace DistributedInput
} // namespace DistributedHardware
} // namespace OHOS
