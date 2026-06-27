/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#include "inputcheckparam_fuzzer.h"

#include <cstddef>
#include <cstdint>
#include <memory>
#include <new>
#include <string>
#include <vector>

#include <fuzzer/FuzzedDataProvider.h>

#include "constants_dinput.h"
#include "input_check_param.h"
#include "start_d_input_call_back_stub.h"

namespace OHOS {
namespace DistributedHardware {
namespace DistributedInput {
namespace {
constexpr uint32_t ID_CASE_COUNT = 4U;
constexpr uint32_t TYPE_CASE_COUNT = 5U;
constexpr uint32_t MAX_DHID_COUNT = 6U;
constexpr size_t PARAMETER_MAX_SIZE = 128U;

class TestStartDInputCallback final : public StartDInputCallbackStub {
public:
    void OnResult(const std::string &devId, const uint32_t &inputTypes, const int32_t &status) override
    {
        (void)devId;
        (void)inputTypes;
        (void)status;
    }
};

class TestRegisterCallback final : public OHOS::DistributedHardware::RegisterCallback {
public:
    int32_t OnRegisterResult(const std::string &devId, const std::string &dhId, int32_t status,
        const std::string &data) override
    {
        (void)devId;
        (void)dhId;
        (void)status;
        (void)data;
        return 0;
    }
};

class TestUnregisterCallback final : public OHOS::DistributedHardware::UnregisterCallback {
public:
    int32_t OnUnregisterResult(const std::string &devId, const std::string &dhId, int32_t status,
        const std::string &data) override
    {
        (void)devId;
        (void)dhId;
        (void)status;
        (void)data;
        return 0;
    }
};

std::string ConsumeNonEmptyString(FuzzedDataProvider &provider, size_t maxSize)
{
    std::string value = provider.ConsumeRandomLengthString(maxSize);
    return value.empty() ? "dinput_id" : value;
}

std::string ConsumeDeviceId(FuzzedDataProvider &provider)
{
    uint32_t idCase = provider.ConsumeIntegralInRange<uint32_t>(0U, ID_CASE_COUNT - 1U);
    if (idCase == 0U) {
        return "";
    }
    if (idCase == 1U) {
        return ConsumeNonEmptyString(provider, DEV_ID_LENGTH_MAX);
    }
    if (idCase == 2U) {
        return std::string(DEV_ID_LENGTH_MAX, 'D');
    }
    return std::string(DEV_ID_LENGTH_MAX + 1U, 'E');
}

std::string ConsumeDhId(FuzzedDataProvider &provider)
{
    if (provider.ConsumeBool()) {
        return ConsumeNonEmptyString(provider, DH_ID_LENGTH_MAX);
    }
    return std::string(DH_ID_LENGTH_MAX + 1U, 'H');
}

uint32_t ConsumeInputTypes(FuzzedDataProvider &provider)
{
    uint32_t typeCase = provider.ConsumeIntegralInRange<uint32_t>(0U, TYPE_CASE_COUNT - 1U);
    if (typeCase == 0U) {
        return static_cast<uint32_t>(DInputDeviceType::NONE);
    }
    if (typeCase == 1U) {
        return static_cast<uint32_t>(DInputDeviceType::ALL);
    }
    if (typeCase == 2U) {
        return static_cast<uint32_t>(DInputDeviceType::MOUSE) |
            static_cast<uint32_t>(DInputDeviceType::KEYBOARD);
    }
    if (typeCase == 3U) {
        return static_cast<uint32_t>(DInputDeviceType::ALL) + 1U;
    }
    return provider.ConsumeIntegral<uint32_t>();
}

std::vector<std::string> ConsumeDhIds(FuzzedDataProvider &provider)
{
    if (provider.ConsumeBool()) {
        return {};
    }
    uint32_t count = provider.ConsumeIntegralInRange<uint32_t>(1U, MAX_DHID_COUNT);
    std::vector<std::string> dhIds;
    dhIds.reserve(count);
    for (uint32_t i = 0U; i < count; ++i) {
        dhIds.emplace_back(ConsumeDhId(provider));
    }
    return dhIds;
}

sptr<IRemoteBroker> MakeRemoteCallback(bool enabled)
{
    if (!enabled) {
        return nullptr;
    }
    sptr<IRemoteBroker> callback(new (std::nothrow) TestStartDInputCallback());
    return callback;
}

void ExerciseBaselineChecks(DInputCheckParam &checker)
{
    sptr<IRemoteBroker> callback = MakeRemoteCallback(true);
    std::vector<std::string> dhIds = { "dhid_valid" };
    uint32_t inputTypes = static_cast<uint32_t>(DInputDeviceType::ALL);
    std::shared_ptr<TestRegisterCallback> registerCallback = std::make_shared<TestRegisterCallback>();
    std::shared_ptr<TestUnregisterCallback> unregisterCallback = std::make_shared<TestUnregisterCallback>();

    (void)checker.CheckParam("device_valid", callback);
    (void)checker.CheckParam("device_valid", inputTypes, callback);
    (void)checker.CheckParam("source_valid", "sink_valid", inputTypes, callback);
    (void)checker.CheckParam("source_valid", "sink_valid", callback);
    (void)checker.CheckParam("sink_valid", dhIds, callback);
    (void)checker.CheckParam("source_valid", "sink_valid", dhIds, callback);
    (void)checker.CheckRegisterParam("device_valid", "dhid_valid", "{}", registerCallback);
    (void)checker.CheckUnregisterParam("device_valid", "dhid_valid", unregisterCallback);
}

void ExerciseRemoteBrokerChecks(DInputCheckParam &checker, FuzzedDataProvider &provider)
{
    sptr<IRemoteBroker> callback = MakeRemoteCallback(provider.ConsumeBool());
    std::string devId = ConsumeDeviceId(provider);
    std::string srcId = ConsumeDeviceId(provider);
    std::string sinkId = ConsumeDeviceId(provider);
    uint32_t inputTypes = ConsumeInputTypes(provider);

    (void)checker.CheckParam(devId, callback);
    (void)checker.CheckParam(devId, inputTypes, callback);
    (void)checker.CheckParam(srcId, sinkId, inputTypes, callback);
    (void)checker.CheckParam(srcId, sinkId, callback);
}

void ExerciseDhIdVectorChecks(DInputCheckParam &checker, FuzzedDataProvider &provider)
{
    sptr<IRemoteBroker> callback = MakeRemoteCallback(provider.ConsumeBool());
    std::string srcId = ConsumeDeviceId(provider);
    std::string sinkId = ConsumeDeviceId(provider);
    std::vector<std::string> dhIds = ConsumeDhIds(provider);

    (void)checker.CheckParam(sinkId, dhIds, callback);
    (void)checker.CheckParam(srcId, sinkId, dhIds, callback);
}

void ExerciseRegisterChecks(DInputCheckParam &checker, FuzzedDataProvider &provider)
{
    std::string devId = ConsumeDeviceId(provider);
    std::string dhId = ConsumeDhId(provider);
    std::string parameters = provider.ConsumeBool() ? "" : provider.ConsumeRandomLengthString(PARAMETER_MAX_SIZE);
    std::shared_ptr<TestRegisterCallback> registerCallback =
        provider.ConsumeBool() ? std::make_shared<TestRegisterCallback>() : nullptr;
    std::shared_ptr<TestUnregisterCallback> unregisterCallback =
        provider.ConsumeBool() ? std::make_shared<TestUnregisterCallback>() : nullptr;

    (void)checker.CheckRegisterParam(devId, dhId, parameters, registerCallback);
    (void)checker.CheckUnregisterParam(devId, dhId, unregisterCallback);
}
} // namespace

void InputCheckParamFuzzTest(const uint8_t *data, size_t size)
{
    if (data == nullptr) {
        return;
    }

    FuzzedDataProvider provider(data, size);
    DInputCheckParam &checker = DInputCheckParam::GetInstance();
    ExerciseBaselineChecks(checker);
    ExerciseRemoteBrokerChecks(checker, provider);
    ExerciseDhIdVectorChecks(checker, provider);
    ExerciseRegisterChecks(checker, provider);
}
} // namespace DistributedInput
} // namespace DistributedHardware
} // namespace OHOS

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    OHOS::DistributedHardware::DistributedInput::InputCheckParamFuzzTest(data, size);
    return 0;
}
