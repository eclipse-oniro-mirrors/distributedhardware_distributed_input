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

#include "distributed_input_collector_test.h"
#include "event_handler.h"
#include "dinput_errcode.h"

using namespace testing::ext;
using namespace OHOS::DistributedHardware::DistributedInput;
using namespace std;
namespace OHOS {
namespace DistributedHardware {
namespace DistributedInput {
void DistributedInputCollectorTest::SetUp()
{
}

void DistributedInputCollectorTest::TearDown()
{
    DistributedInputCollector::GetInstance().Release();
}

void DistributedInputCollectorTest::SetUpTestCase()
{
}

void DistributedInputCollectorTest::TearDownTestCase()
{
}

DistributedInputCollectorTest::DInputSinkCollectorEventHandler::DInputSinkCollectorEventHandler(
    const std::shared_ptr<AppExecFwk::EventRunner> &runner) : AppExecFwk::EventHandler(runner)
{
}

void DistributedInputCollectorTest::DInputSinkCollectorEventHandler::ProcessEvent(
    const AppExecFwk::InnerEvent::Pointer &event)
{
    (void)event;
}

HWTEST_F(DistributedInputCollectorTest, Init01, testing::ext::TestSize.Level1)
{
    std::shared_ptr<AppExecFwk::EventHandler> eventHandler_ = nullptr;
    int32_t ret = DistributedInputCollector::GetInstance().Init(eventHandler_);
    EXPECT_EQ(ERR_DH_INPUT_SERVER_SINK_COLLECTOR_INIT_FAIL, ret);
}

HWTEST_F(DistributedInputCollectorTest, Init02, testing::ext::TestSize.Level1)
{
    std::shared_ptr<AppExecFwk::EventRunner> runner = AppExecFwk::EventRunner::Create(true);
    std::shared_ptr<AppExecFwk::EventHandler> eventHandler_ =
        std::make_shared<DistributedInputCollectorTest::DInputSinkCollectorEventHandler>(runner);

    int32_t ret = DistributedInputCollector::GetInstance().Init(eventHandler_);
    EXPECT_EQ(DH_SUCCESS, ret);
}

HWTEST_F(DistributedInputCollectorTest, IsAllDevicesStoped, testing::ext::TestSize.Level1)
{
    bool isStop = DistributedInputCollector::GetInstance().IsAllDevicesStoped();
    EXPECT_EQ(true, isStop);
}
} // namespace DistributedInput
} // namespace DistributedHardware
} // namespace OHOS