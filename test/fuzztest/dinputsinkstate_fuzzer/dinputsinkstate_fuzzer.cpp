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

#include "dinputsinkstate_fuzzer.h"

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

#include <fuzzer/FuzzedDataProvider.h>
#include <linux/input.h>

#include "constants_dinput.h"
#include "dinput_sink_state.h"

namespace OHOS {
namespace DistributedHardware {
namespace DistributedInput {
namespace {
constexpr size_t DHID_MAX_SIZE = 64U;
constexpr uint32_t DHID_MAX_COUNT = 5U;
constexpr uint32_t EVENT_MAX_COUNT = 8U;
constexpr int32_t INVALID_SESSION_ID = -1;
constexpr int32_t SYN_REPORT_VALUE = 0;
constexpr int32_t FUZZ_TEST_SUCCESS = 0;

std::string ConsumeDhId(FuzzedDataProvider &provider)
{
    std::string dhId = provider.ConsumeRandomLengthString(DHID_MAX_SIZE);
    return dhId.empty() ? "sink_state_dhid" : dhId;
}

std::vector<std::string> ConsumeDhIds(FuzzedDataProvider &provider)
{
    uint32_t count = provider.ConsumeIntegralInRange<uint32_t>(1U, DHID_MAX_COUNT);
    std::vector<std::string> dhIds;
    dhIds.reserve(count);
    for (uint32_t i = 0U; i < count; ++i) {
        dhIds.emplace_back(ConsumeDhId(provider));
    }
    return dhIds;
}

RawEvent MakeEvent(FuzzedDataProvider &provider, const std::string &dhId, uint32_t type,
    uint32_t code, int32_t value)
{
    RawEvent event;
    event.when = provider.ConsumeIntegral<int64_t>();
    event.type = type;
    event.code = code;
    event.value = value;
    event.descriptor = dhId;
    event.path = "/dev/input/" + dhId;
    return event;
}

RawEvent ConsumeKeyEvent(FuzzedDataProvider &provider, const std::string &dhId)
{
    uint32_t code = provider.ConsumeBool() ? KEY_A :
        provider.ConsumeIntegralInRange<uint32_t>(KEY_ESC, KEY_MAX);
    int32_t value = provider.ConsumeBool() ? KEY_DOWN_STATE : KEY_REPEAT;
    return MakeEvent(provider, dhId, EV_KEY, code, value);
}

void ExerciseDhIdState(DInputSinkState &state, FuzzedDataProvider &provider,
    const std::vector<std::string> &dhIds)
{
    int32_t sessionId = provider.ConsumeBool() ? INVALID_SESSION_ID :
        provider.ConsumeIntegral<int32_t>();
    DhIdState dhIdState = provider.ConsumeBool() ? DhIdState::THROUGH_IN : DhIdState::THROUGH_OUT;

    (void)state.RecordDhIds(dhIds, DhIdState::THROUGH_IN, sessionId);
    (void)state.RecordDhIds(dhIds, dhIdState, INVALID_SESSION_ID);
    for (const std::string &dhId : dhIds) {
        (void)state.GetStateByDhid(dhId);
    }
    (void)state.GetStateByDhid(ConsumeDhId(provider) + "_missing");
    (void)state.RemoveDhIds({ dhIds.front() });
}

void ExerciseKeyStates(DInputSinkState &state, FuzzedDataProvider &provider,
    const std::string &dhId)
{
    std::vector<RawEvent> events;
    uint32_t count = provider.ConsumeIntegralInRange<uint32_t>(1U, EVENT_MAX_COUNT);
    events.reserve(count);
    for (uint32_t i = 0U; i < count; ++i) {
        events.emplace_back(ConsumeKeyEvent(provider, dhId));
    }

    for (const RawEvent &event : events) {
        state.AddKeyDownState(event);
        state.CheckAndSetLongPressedKeyOrder(event);
    }
    (void)state.IsDhIdDown(dhId);
    for (const RawEvent &event : events) {
        state.RemoveKeyDownState(event);
    }
    (void)state.IsDhIdDown(dhId);
}

void ExerciseTouchPadManager(DInputSinkState &state, FuzzedDataProvider &provider,
    const std::string &dhId)
{
    auto mgr = state.GetTouchPadEventFragMgr();
    if (mgr == nullptr) {
        return;
    }
    std::vector<RawEvent> touchEvents = {
        MakeEvent(provider, dhId, EV_KEY, BTN_TOUCH, KEY_DOWN_STATE),
        MakeEvent(provider, dhId, EV_SYN, SYN_REPORT, SYN_REPORT_VALUE),
        MakeEvent(provider, dhId, EV_KEY, BTN_TOUCH, KEY_UP_STATE),
        MakeEvent(provider, dhId, EV_SYN, SYN_REPORT, SYN_REPORT_VALUE),
    };
    for (const RawEvent &event : touchEvents) {
        (void)mgr->PushEvent(dhId, event);
    }
    (void)mgr->GetAndClearEvents(dhId);
    mgr->Clear(dhId);
}
} // namespace

void DInputSinkStateFuzzTest(const uint8_t *data, size_t size)
{
    if (data == nullptr) {
        return;
    }

    FuzzedDataProvider provider(data, size);
    DInputSinkState &state = DInputSinkState::GetInstance();
    std::vector<std::string> dhIds = ConsumeDhIds(provider);
    std::string primaryDhId = dhIds.front();

    (void)state.Init();
    ExerciseDhIdState(state, provider, dhIds);
    ExerciseKeyStates(state, provider, primaryDhId);
    ExerciseTouchPadManager(state, provider, primaryDhId);
    state.ClearDeviceStates();
    (void)state.RemoveDhIds(dhIds);
    (void)state.Release();
}
} // namespace DistributedInput
} // namespace DistributedHardware
} // namespace OHOS

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    OHOS::DistributedHardware::DistributedInput::DInputSinkStateFuzzTest(data, size);
    return OHOS::DistributedHardware::DistributedInput::FUZZ_TEST_SUCCESS;
}
