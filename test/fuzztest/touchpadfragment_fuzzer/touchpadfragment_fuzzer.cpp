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

#include "touchpadfragment_fuzzer.h"

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

#include <fuzzer/FuzzedDataProvider.h>
#include <linux/input.h>

#include "constants_dinput.h"
#include "touchpad_event_fragment.h"
#include "touchpad_event_fragment_mgr.h"

namespace OHOS {
namespace DistributedHardware {
namespace DistributedInput {
namespace {
constexpr size_t DHID_MAX_SIZE = 64U;
constexpr uint32_t EXTRA_EVENT_MAX_COUNT = 10U;
constexpr uint32_t EVENT_KIND_COUNT = 6U;
constexpr uint32_t EVENT_KIND_MIN = 0U;
constexpr uint32_t TOUCH_DOWN_EVENT_KIND = 0U;
constexpr uint32_t TOUCH_UP_EVENT_KIND = 1U;
constexpr uint32_t SYN_REPORT_EVENT_KIND = 2U;
constexpr uint32_t ABS_EVENT_KIND = 3U;
constexpr uint32_t ABS_MT_EVENT_KIND = 4U;
constexpr int32_t SYN_REPORT_VALUE = 0;
constexpr int32_t FUZZ_TEST_SUCCESS = 0;

std::string ConsumeDhId(FuzzedDataProvider &provider)
{
    std::string dhId = provider.ConsumeRandomLengthString(DHID_MAX_SIZE);
    return dhId.empty() ? "touchpad_dhid" : dhId;
}

RawEvent MakeEvent(FuzzedDataProvider &provider, uint32_t type, uint32_t code, int32_t value,
    const std::string &dhId)
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

RawEvent ConsumeEvent(FuzzedDataProvider &provider, const std::string &dhId)
{
    uint32_t eventKind = provider.ConsumeIntegralInRange<uint32_t>(
        EVENT_KIND_MIN, EVENT_KIND_COUNT - 1U);
    if (eventKind == TOUCH_DOWN_EVENT_KIND) {
        return MakeEvent(provider, EV_KEY, BTN_TOUCH, KEY_DOWN_STATE, dhId);
    }
    if (eventKind == TOUCH_UP_EVENT_KIND) {
        return MakeEvent(provider, EV_KEY, BTN_TOUCH, KEY_UP_STATE, dhId);
    }
    if (eventKind == SYN_REPORT_EVENT_KIND) {
        return MakeEvent(provider, EV_SYN, SYN_REPORT, SYN_REPORT_VALUE, dhId);
    }
    if (eventKind == ABS_EVENT_KIND) {
        return MakeEvent(provider, EV_ABS, provider.ConsumeBool() ? ABS_X : ABS_Y,
            provider.ConsumeIntegral<int32_t>(), dhId);
    }
    if (eventKind == ABS_MT_EVENT_KIND) {
        uint32_t code = provider.ConsumeBool() ? ABS_MT_POSITION_X : ABS_MT_POSITION_Y;
        return MakeEvent(provider, EV_ABS, code,
            provider.ConsumeIntegral<int32_t>(), dhId);
    }
    return MakeEvent(provider, provider.ConsumeIntegral<uint32_t>(),
        provider.ConsumeIntegral<uint32_t>(),
        provider.ConsumeIntegral<int32_t>(), dhId);
}

void PushSequence(TouchPadEventFragmentMgr &mgr, const std::string &dhId,
    const std::vector<RawEvent> &events)
{
    for (const RawEvent &event : events) {
        (void)mgr.PushEvent(dhId, event);
    }
}

void ExerciseFragment(FuzzedDataProvider &provider, const std::string &dhId)
{
    TouchPadEventFragment fragment;
    fragment.PushEvent(MakeEvent(provider, EV_KEY, BTN_TOUCH, KEY_DOWN_STATE, dhId));
    fragment.PushEvent(MakeEvent(provider, EV_SYN, SYN_REPORT, SYN_REPORT_VALUE, dhId));
    (void)fragment.IsTouchPadOptStart();
    (void)fragment.IsTouchPadOptFinish();
    (void)fragment.IsShouldDrop();
    (void)fragment.GetEvents();

    TouchPadEventFragment finishFragment;
    finishFragment.PushEvent(MakeEvent(provider, EV_KEY, BTN_TOUCH, KEY_UP_STATE, dhId));
    finishFragment.PushEvent(MakeEvent(provider, EV_SYN, SYN_REPORT, SYN_REPORT_VALUE, dhId));
    (void)finishFragment.IsTouchPadOptStart();
    (void)finishFragment.IsTouchPadOptFinish();
    (void)finishFragment.IsShouldDrop();
    (void)finishFragment.GetEvents();
}

void ExerciseManagerSequences(FuzzedDataProvider &provider, const std::string &dhId)
{
    TouchPadEventFragmentMgr mgr;
    std::vector<RawEvent> wholeTouch = {
        MakeEvent(provider, EV_KEY, BTN_TOUCH, KEY_DOWN_STATE, dhId),
        MakeEvent(provider, EV_KEY, BTN_TOUCH, KEY_UP_STATE, dhId),
        MakeEvent(provider, EV_SYN, SYN_REPORT, SYN_REPORT_VALUE, dhId),
    };
    std::vector<RawEvent> notWholeTouch = {
        MakeEvent(provider, EV_KEY, BTN_TOUCH, KEY_UP_STATE, dhId),
        MakeEvent(provider, EV_SYN, SYN_REPORT, SYN_REPORT_VALUE, dhId),
    };
    std::vector<RawEvent> dropFragment = {
        MakeEvent(provider, EV_KEY, KEY_A, KEY_DOWN_STATE, dhId),
        MakeEvent(provider, EV_SYN, SYN_REPORT, SYN_REPORT_VALUE, dhId),
    };

    (void)mgr.PushEvent(dhId,
        MakeEvent(provider, EV_ABS, ABS_X, provider.ConsumeIntegral<int32_t>(), dhId));
    PushSequence(mgr, wholeTouch);
    PushSequence(mgr, notWholeTouch);
    PushSequence(mgr, dropFragment);
    (void)mgr.GetAndClearEvents(dhId);
    mgr.Clear(dhId);
}

void ExerciseFuzzedManager(FuzzedDataProvider &provider, const std::string &dhId)
{
    TouchPadEventFragmentMgr mgr;
    uint32_t count = provider.ConsumeIntegralInRange<uint32_t>(0U, EXTRA_EVENT_MAX_COUNT);
    for (uint32_t i = 0U; i < count; ++i) {
        (void)mgr.PushEvent(dhId, ConsumeEvent(provider, dhId));
    }
    (void)mgr.GetAndClearEvents(provider.ConsumeBool() ? dhId : dhId + "_missing");
    mgr.Clear(dhId);
}
} // namespace

void TouchPadFragmentFuzzTest(const uint8_t *data, size_t size)
{
    if (data == nullptr) {
        return;
    }

    FuzzedDataProvider provider(data, size);
    std::string dhId = ConsumeDhId(provider);
    ExerciseFragment(provider, dhId);
    ExerciseManagerSequences(provider, dhId);
    ExerciseFuzzedManager(provider, dhId);
}
} // namespace DistributedInput
} // namespace DistributedHardware
} // namespace OHOS

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    OHOS::DistributedHardware::DistributedInput::TouchPadFragmentFuzzTest(data, size);
    return OHOS::DistributedHardware::DistributedInput::FUZZ_TEST_SUCCESS;
}
