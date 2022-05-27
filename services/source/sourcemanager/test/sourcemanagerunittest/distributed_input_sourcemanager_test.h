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

#ifndef DISRIBUTED_INPUT_INNER_TEST_H
#define DISRIBUTED_INPUT_INNER_TEST_H

#include <functional>
#include <iostream>
#include <thread>

#include <gtest/gtest.h>
#include <refbase.h>

#include "add_white_list_infos_call_back_stub.h"
#include "constants_dinput.h"
#include "del_white_list_infos_call_back_stub.h"
#include "distributed_input_source_manager.h"
#include "prepare_d_input_call_back_stub.h"
#include "register_d_input_call_back_stub.h"
#include "start_d_input_call_back_stub.h"
#include "stop_d_input_call_back_stub.h"
#include "unprepare_d_input_call_back_stub.h"
#include "unregister_d_input_call_back_stub.h"

namespace OHOS {
namespace DistributedHardware {
namespace DistributedInput {
class DistributedInputSourceManagerTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    virtual void SetUp() override;
    virtual void TearDown() override;

    class TestPrepareDInputCallback : public
        OHOS::DistributedHardware::DistributedInput::PrepareDInputCallbackStub {
    public:
        TestPrepareDInputCallback() = default;
        virtual ~TestPrepareDInputCallback() = default;
        void OnResult(const std::string& deviceId, const int32_t& status);
    };

    class TestUnprepareDInputCallback : public
        OHOS::DistributedHardware::DistributedInput::UnprepareDInputCallbackStub {
    public:
        TestUnprepareDInputCallback() = default;
        virtual ~TestUnprepareDInputCallback() = default;
        void OnResult(const std::string& deviceId, const int32_t& status);
    };

    class TestStartDInputCallback : public
        OHOS::DistributedHardware::DistributedInput::StartDInputCallbackStub {
    public:
        TestStartDInputCallback() = default;
        virtual ~TestStartDInputCallback() = default;
        void OnResult(const std::string& deviceId, const uint32_t& inputTypes, const int32_t& status);
    };

    class TestStopDInputCallback : public
        OHOS::DistributedHardware::DistributedInput::StopDInputCallbackStub {
    public:
        TestStopDInputCallback() = default;
        virtual ~TestStopDInputCallback() = default;
        void OnResult(const std::string& deviceId, const uint32_t& inputTypes, const int32_t& status);
    };

    class TestRegisterDInputCb : public OHOS::DistributedHardware::DistributedInput::RegisterDInputCallbackStub {
    public:
        TestRegisterDInputCb() = default;
        virtual ~TestRegisterDInputCb() = default;
        void OnResult(const std::string& devId, const std::string& dhId, const int32_t& status);
    };

    class TestUnregisterDInputCb : public OHOS::DistributedHardware::DistributedInput::UnregisterDInputCallbackStub {
    public:
        TestUnregisterDInputCb() = default;
        virtual ~TestUnregisterDInputCb() = default;
        void OnResult(const std::string& devId, const std::string& dhId, const int32_t& status);
    };

    class TestAddWhiteListInfosCb : public OHOS::DistributedHardware::DistributedInput::AddWhiteListInfosCallbackStub {
    public:
        TestAddWhiteListInfosCb() = default;
        virtual ~TestAddWhiteListInfosCb() = default;
        void OnResult(const std::string &deviceId, const std::string &strJson);
    };

    class TestDelWhiteListInfosCb : public OHOS::DistributedHardware::DistributedInput::DelWhiteListInfosCallbackStub {
    public:
        TestDelWhiteListInfosCb() = default;
        virtual ~TestDelWhiteListInfosCb() = default;
        void OnResult(const std::string &deviceId);
    };

private:
    int32_t StructTransJson(const InputDevice* pBuf, std::string& strDescriptor);
    DistributedInputSourceManager* sourceManager_;
};
} // namespace DistributedInput
} // namespace DistributedHardware
} // namespace OHOS
#endif // DISRIBUTED_INPUT_INNER_TEST_H
