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

#ifndef ADD_WHITE_LIST_INFOS_CALLBACK_TEST_H
#define ADD_WHITE_LIST_INFOS_CALLBACK_TEST_H

#include <gtest/gtest.h>

#include "add_white_list_infos_call_back_proxy.h"
#include "add_white_list_infos_call_back_stub.h"

namespace OHOS {
namespace DistributedHardware {
namespace DistributedInput {
class AddWhiteListInfosCallbackTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;

    class TestAddWhiteListInfosCallbackStub : public
        OHOS::DistributedHardware::DistributedInput::AddWhiteListInfosCallbackStub {
    public:
        TestAddWhiteListInfosCallbackStub() = default;
        virtual ~TestAddWhiteListInfosCallbackStub() = default;
        void OnResult(const std::string& deviceId, const std::string& strJson);
        std::string deviceId_;
        std::string strJson_;
    };
};
} // namespace DistributedInput
} // namespace DistributedHardware
} // namespace OHOS
#endif // ADD_WHITE_LIST_INFOS_CALLBACK_TEST_H