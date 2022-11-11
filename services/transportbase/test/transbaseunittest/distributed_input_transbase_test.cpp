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
#include "distributed_input_transbase_test.h"

using namespace testing::ext;
using namespace OHOS::DistributedHardware::DistributedInput;
using namespace std;
namespace OHOS {
namespace DistributedHardware {
namespace DistributedInput {
void DistributedInputTransbaseTest::SetUp()
{
}

void DistributedInputTransbaseTest::TearDown()
{
}

void DistributedInputTransbaseTest::SetUpTestCase()
{
}

void DistributedInputTransbaseTest::TearDownTestCase()
{
}

HWTEST_F(DistributedInputTransbaseTest, StartSession, testing::ext::TestSize.Level0)
{
    std::string remoteDevId = "";
    int32_t ret = DistributedInputTransportBase::GetInstance().StartSession(remoteDevId);

    EXPECT_EQ(ERR_DH_INPUT_SERVER_SOURCE_TRANSPORT_OPEN_SESSION_FAIL, ret);
}

HWTEST_F(DistributedInputTransbaseTest, Init, testing::ext::TestSize.Level0)
{
    int32_t ret = DistributedInputTransportBase::GetInstance().Init();
    EXPECT_EQ(DH_SUCCESS, ret);
}

HWTEST_F(DistributedInputTransbaseTest, GetSessionIdByDevId, testing::ext::TestSize.Level0)
{
    std::string remoteDevId = "";
    int32_t ret = DistributedInputTransportBase::GetInstance().GetSessionIdByDevId(remoteDevId);
    EXPECT_EQ(ERR_DH_INPUT_SERVER_SINK_TRANSPORT_GET_SESSIONID_FAIL, ret);
}
} // namespace DistributedInput
} // namespace DistributedHardware
} // namespace OHOS
