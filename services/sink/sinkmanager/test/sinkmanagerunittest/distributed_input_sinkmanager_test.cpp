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
#include "distributed_input_sinkmanager_test.h"

#include "if_system_ability_manager.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"


using namespace testing::ext;
using namespace OHOS::DistributedHardware::DistributedInput;
using namespace std;
namespace OHOS {
namespace DistributedHardware {
namespace DistributedInput {
void DistributedInputSinkManagerTest::SetUp()
{
    sinkManager_ = new DistributedInputSinkManager(DISTRIBUTED_HARDWARE_INPUT_SINK_SA_ID, true);
}

void DistributedInputSinkManagerTest::TearDown()
{
}

void DistributedInputSinkManagerTest::SetUpTestCase()
{
}

void DistributedInputSinkManagerTest::TearDownTestCase()
{
}

HWTEST_F(DistributedInputSinkManagerTest, Init01, testing::ext::TestSize.Level0)
{
    std::cout << "Init01"<< std::endl;
    int32_t ret = sinkManager_->Init();
    EXPECT_EQ(SUCCESS, ret);
}

HWTEST_F(DistributedInputSinkManagerTest, Release01, testing::ext::TestSize.Level0)
{
    std::cout << "Release01"<< std::endl;
    int32_t ret = sinkManager_->Release();
    EXPECT_EQ(SUCCESS, ret);
}
}
}
}
