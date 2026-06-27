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

#include "whitelistutilstate_fuzzer.h"

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

#include <fuzzer/FuzzedDataProvider.h>

#include "constants_dinput.h"
#include "white_list_util.h"

namespace OHOS {
namespace DistributedHardware {
namespace DistributedInput {
namespace {
constexpr uint32_t COMBINATION_MAX_COUNT = 3U;
constexpr uint32_t KEY_GROUP_MAX_COUNT = 2U;
constexpr uint32_t KEY_CODE_MAX_COUNT = 2U;
constexpr uint32_t DEVICE_ID_CASE_COUNT = 4U;
constexpr int32_t KEY_CODE_MIN = 1;
constexpr int32_t KEY_CODE_MAX = 8;

std::string ConsumeDeviceId(FuzzedDataProvider &provider)
{
    uint32_t deviceCase = provider.ConsumeIntegralInRange<uint32_t>(0U, DEVICE_ID_CASE_COUNT - 1U);
    return "white_list_device_" + std::to_string(deviceCase);
}

int32_t ConsumeKeyCode(FuzzedDataProvider &provider)
{
    return provider.ConsumeIntegralInRange<int32_t>(KEY_CODE_MIN, KEY_CODE_MAX);
}

TYPE_KEY_CODE_VEC ConsumeKeyGroup(FuzzedDataProvider &provider)
{
    uint32_t count = provider.ConsumeIntegralInRange<uint32_t>(1U, KEY_CODE_MAX_COUNT);
    TYPE_KEY_CODE_VEC group;
    group.reserve(count);
    for (uint32_t i = 0U; i < count; ++i) {
        group.emplace_back(ConsumeKeyCode(provider));
    }
    return group;
}

TYPE_COMBINATION_KEY_VEC BuildValidCombination(FuzzedDataProvider &provider)
{
    uint32_t modifierCount = provider.ConsumeIntegralInRange<uint32_t>(0U, KEY_GROUP_MAX_COUNT);
    TYPE_COMBINATION_KEY_VEC combination;
    combination.reserve(modifierCount + 2U);
    for (uint32_t i = 0U; i < modifierCount; ++i) {
        combination.emplace_back(ConsumeKeyGroup(provider));
    }
    combination.push_back({ ConsumeKeyCode(provider) });
    int32_t keyAction = static_cast<int32_t>(provider.ConsumeBool() ? KEY_DOWN_STATE : KEY_UP_STATE);
    combination.push_back({ keyAction });
    return combination;
}

TYPE_COMBINATION_KEY_VEC BuildInvalidCombination(FuzzedDataProvider &provider)
{
    if (provider.ConsumeBool()) {
        return { ConsumeKeyGroup(provider) };
    }
    TYPE_COMBINATION_KEY_VEC combination = BuildValidCombination(provider);
    if (provider.ConsumeBool()) {
        combination.back().push_back(ConsumeKeyCode(provider));
    } else if (combination.size() >= 2U) {
        combination[combination.size() - 2U].push_back(ConsumeKeyCode(provider));
    }
    return combination;
}

TYPE_WHITE_LIST_VEC ConsumeWhiteList(FuzzedDataProvider &provider)
{
    uint32_t count = provider.ConsumeIntegralInRange<uint32_t>(0U, COMBINATION_MAX_COUNT);
    TYPE_WHITE_LIST_VEC whiteList;
    whiteList.reserve(count + 2U);
    whiteList.emplace_back(BuildValidCombination(provider));
    whiteList.emplace_back(BuildInvalidCombination(provider));
    for (uint32_t i = 0U; i < count; ++i) {
        whiteList.emplace_back(provider.ConsumeBool() ? BuildValidCombination(provider) :
            BuildInvalidCombination(provider));
    }
    return whiteList;
}

BusinessEvent BuildMatchingEvent(const TYPE_WHITE_LIST_VEC &whiteList)
{
    BusinessEvent event;
    for (const auto &combination : whiteList) {
        if (combination.size() < 2U || combination.back().empty()) {
            continue;
        }
        const TYPE_KEY_CODE_VEC &lastKey = combination[combination.size() - 2U];
        if (lastKey.size() != 1U) {
            continue;
        }
        for (size_t i = 0U; i + 2U < combination.size(); ++i) {
            for (int32_t keyCode : combination[i]) {
                event.pressedKeys.emplace_back(keyCode);
            }
        }
        event.keyCode = lastKey.front();
        event.keyAction = combination.back().front();
        return event;
    }
    event.keyCode = KEY_A;
    event.keyAction = static_cast<int32_t>(KEY_DOWN_STATE);
    return event;
}

BusinessEvent BuildFuzzedEvent(FuzzedDataProvider &provider)
{
    BusinessEvent event;
    uint32_t pressedCount = provider.ConsumeIntegralInRange<uint32_t>(0U, KEY_CODE_MAX_COUNT);
    for (uint32_t i = 0U; i < pressedCount; ++i) {
        event.pressedKeys.emplace_back(ConsumeKeyCode(provider));
    }
    event.keyCode = ConsumeKeyCode(provider);
    event.keyAction = provider.ConsumeIntegralInRange<int32_t>(static_cast<int32_t>(KEY_UP_STATE),
        static_cast<int32_t>(KEY_REPEAT));
    return event;
}

void ExerciseWhiteListState(FuzzedDataProvider &provider)
{
    WhiteListUtil &whiteListUtil = WhiteListUtil::GetInstance();
    std::string deviceId = ConsumeDeviceId(provider);
    std::string missingDeviceId = ConsumeDeviceId(provider) + "_missing";
    TYPE_WHITE_LIST_VEC whiteList = ConsumeWhiteList(provider);
    TYPE_WHITE_LIST_VEC outputList;

    (void)whiteListUtil.SyncWhiteList(deviceId, whiteList);
    (void)whiteListUtil.GetWhiteList(deviceId, outputList);
    (void)whiteListUtil.GetWhiteList(missingDeviceId, outputList);
    (void)whiteListUtil.IsNeedFilterOut(deviceId, BuildMatchingEvent(whiteList));
    (void)whiteListUtil.IsNeedFilterOut(deviceId, BuildFuzzedEvent(provider));
    (void)whiteListUtil.IsNeedFilterOut(missingDeviceId, BuildFuzzedEvent(provider));
    (void)whiteListUtil.ClearWhiteList(missingDeviceId);
    (void)whiteListUtil.ClearWhiteList(deviceId);
    (void)whiteListUtil.IsNeedFilterOut(deviceId, BuildFuzzedEvent(provider));
    if (provider.ConsumeBool()) {
        (void)whiteListUtil.ClearWhiteList();
    }
}
} // namespace

void WhiteListUtilStateFuzzTest(const uint8_t *data, size_t size)
{
    if (data == nullptr) {
        return;
    }

    FuzzedDataProvider provider(data, size);
    ExerciseWhiteListState(provider);
}
} // namespace DistributedInput
} // namespace DistributedHardware
} // namespace OHOS

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    OHOS::DistributedHardware::DistributedInput::WhiteListUtilStateFuzzTest(data, size);
    return 0;
}
