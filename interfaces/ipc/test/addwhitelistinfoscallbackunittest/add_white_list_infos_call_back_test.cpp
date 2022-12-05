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

#include "add_white_list_infos_call_back_test.h"

namespace OHOS {
namespace DistributedHardware {
namespace DistributedInput {
void AddWhiteListInfosCallbackTest::SetUp()
{
}

void AddWhiteListInfosCallbackTest::TearDown()
{
}

void AddWhiteListInfosCallbackTest::SetUpTestCase()
{
}

void AddWhiteListInfosCallbackTest::TearDownTestCase()
{
}

void AddWhiteListInfosCallbackTest::TestAddWhiteListInfosCallbackStub::OnResult(const std::string& deviceId,
    const std::string& strJson)
{
    deviceId_ = deviceId;
    strJson_ = strJson;
}

HWTEST_F(AddWhiteListInfosCallbackTest, AddWhiteListInfosCallback01, testing::ext::TestSize.Level1)
{
    sptr<IRemoteObject> callBackStubPtr = new TestAddWhiteListInfosCallbackStub();
    AddWhiteListInfosCallbackProxy callBackProxy(callBackStubPtr);
    std::string deviceId = "deviceId0";
    std::string json = "json0";
    callBackProxy.OnResult(deviceId, json);
    EXPECT_STREQ(deviceId.c_str(), ((sptr<TestAddWhiteListInfosCallbackStub> &)callBackStubPtr)->deviceId_.c_str());
    EXPECT_STREQ(json.c_str(), ((sptr<TestAddWhiteListInfosCallbackStub> &)callBackStubPtr)->strJson_.c_str());
}
} // namespace DistributedInput
} // namespace DistributedHardware
} // namespace OHOS