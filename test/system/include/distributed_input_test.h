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

#ifndef DISRIBUTED_INPUT_TEST_H
#define DISRIBUTED_INPUT_TEST_H

#include <thread>
#include <functional>
#include <iostream>

#include <gtest/gtest.h>
#include <refbase.h>

#include "constants_dinput.h"
#include "distributed_input_handler.h"
#include "distributed_input_kit.h"
#include "distributed_input_sink_handler.h"
#include "distributed_input_source_handler.h"
#include "i_distributed_sink_input.h"
#include "i_distributed_source_input.h"
#include "prepare_d_input_call_back_stub.h"
#include "unprepare_d_input_call_back_stub.h"
#include "start_d_input_call_back_stub.h"
#include "stop_d_input_call_back_stub.h"

namespace OHOS {
namespace DistributedHardware {
namespace DistributedInput {
class DistributedInputTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    virtual void SetUp() override;
    virtual void TearDown() override;

    void StartSourceSa();
    void StartSinkSa();
    void GetHardWhereInfo();
    void GetDeviceInfo();
    void RegisterDistributedHardware();
    void UnregisterDistributedHardware();
    void PrepareRemoteInput();
    void UnprepareRemoteInput();
    void StartRemoteInput();
    void StopRemoteInput();
    void IsStartDistributedInput();
    void IsNeedFilterOut();
    void StopSourceSa();
    void StopSinkSa();
    void Help();
    void SwitchCase(int32_t in);

    class TestRegisterInputCallback : public OHOS::DistributedHardware::RegisterCallback {
    public:
        TestRegisterInputCallback() = default;
        virtual ~TestRegisterInputCallback() = default;
        virtual int32_t OnRegisterResult(const std::string &devId, const std::string &dhId, int32_t status,
            const std::string &data) override;
    };

    class TestUnregisterInputCallback : public OHOS::DistributedHardware::UnregisterCallback {
    public:
        TestUnregisterInputCallback() = default;
        virtual ~TestUnregisterInputCallback() = default;
        virtual int32_t OnUnregisterResult(const std::string &devId, const std::string &dhId, int32_t status,
            const std::string &data) override;
    };

    class TestPluginListener : public OHOS::DistributedHardware::PluginListener {
    public:
        TestPluginListener() = default;
        virtual ~TestPluginListener() = default;
        virtual void PluginHardware(const std::string &dhId, const std::string &attrs) override;
        virtual void UnPluginHardware(const std::string &dhId) override;
    };

    class TestPrepareDInputCallback :
        public OHOS::DistributedHardware::DistributedInput::PrepareDInputCallbackStub {
    public:
        TestPrepareDInputCallback() = default;
        virtual ~TestPrepareDInputCallback() = default;
        void OnResult(const std::string& deviceId, const int32_t& status);
    };

    class TestUnprepareDInputCallback :
        public OHOS::DistributedHardware::DistributedInput::UnprepareDInputCallbackStub {
    public:
        TestUnprepareDInputCallback() = default;
        virtual ~TestUnprepareDInputCallback() = default;
        void OnResult(const std::string& deviceId, const int32_t& status);
    };

    class TestStartDInputCallback :
        public OHOS::DistributedHardware::DistributedInput::StartDInputCallbackStub {
    public:
        void OnResult(const std::string& deviceId, const uint32_t& inputTypes, const int32_t& status);
    };

    class TestStopDInputCallback :
        public OHOS::DistributedHardware::DistributedInput::StopDInputCallbackStub {
    public:
        TestStopDInputCallback() = default;
        virtual ~TestStopDInputCallback() = default;
        void OnResult(const std::string& deviceId, const uint32_t& inputTypes, const int32_t& status);
    };

    OHOS::DistributedHardware::EnableParam enableParam;

    std::shared_ptr<TestRegisterInputCallback> registerCb = nullptr;
    std::shared_ptr<TestUnregisterInputCallback> unregisterCb = nullptr;
    std::shared_ptr<TestPluginListener> pluginListener;
    OHOS::sptr<TestPrepareDInputCallback> prepareCb;
    OHOS::sptr<TestUnprepareDInputCallback> unprepareCb;
    OHOS::sptr<TestStartDInputCallback> startCb;
    OHOS::sptr<TestStopDInputCallback> stopCb;
};
} // namespace DistributedInput
} // namespace DistributedHardware
} // namespace OHOS
#endif // DISRIBUTED_INPUT_TEST_H
