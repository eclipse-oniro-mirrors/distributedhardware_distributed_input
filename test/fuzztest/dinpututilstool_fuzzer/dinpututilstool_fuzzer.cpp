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

#include "dinpututilstool_fuzzer.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

#include <fuzzer/FuzzedDataProvider.h>
#include "nlohmann/json.hpp"

#include "constants_dinput.h"
#include "dinput_softbus_define.h"
#include "dinput_utils_tool.h"

namespace OHOS {
namespace DistributedHardware {
namespace DistributedInput {
namespace {
constexpr size_t STRING_MAX_LEN = 96U;
constexpr uint32_t VECTOR_MAX_COUNT = 8U;
constexpr int32_t INT_MIN_BOUND = -1024;
constexpr int32_t INT_MAX_BOUND = 1024;
constexpr size_t BIT_ARRAY_SIZE = 2U;
constexpr int BIT_INDEX_MIN = 0;
constexpr int32_t FUZZ_TEST_SUCCESS = 0;

std::string ConsumeText(FuzzedDataProvider &provider)
{
    return provider.ConsumeRandomLengthString(STRING_MAX_LEN);
}

std::string ConsumeNonEmptyText(FuzzedDataProvider &provider)
{
    std::string value = ConsumeText(provider);
    return value.empty() ? "dinput_value" : value;
}

nlohmann::json BuildTypedJson(FuzzedDataProvider &provider)
{
    nlohmann::json jsonObj;
    jsonObj["boolKey"] = provider.ConsumeBool();
    jsonObj["stringKey"] = ConsumeText(provider);
    jsonObj["intKey"] = provider.ConsumeIntegralInRange<int32_t>(INT_MIN_BOUND, INT_MAX_BOUND);
    jsonObj["uintKey"] = provider.ConsumeIntegral<uint32_t>();
    jsonObj["arrayKey"] = nlohmann::json::array({
        ConsumeText(provider), provider.ConsumeIntegral<uint32_t>()
    });
    jsonObj[DEVICE_NAME] = ConsumeNonEmptyText(provider);
    jsonObj[PHYSICAL_PATH] = ConsumeNonEmptyText(provider);
    jsonObj[CLASSES] = provider.ConsumeIntegralInRange<int32_t>(INT_MIN_BOUND, INT_MAX_BOUND);
    return jsonObj;
}

nlohmann::json BuildSoftBusJson(FuzzedDataProvider &provider)
{
    nlohmann::json jsonObj;
    jsonObj[DINPUT_SOFTBUS_KEY_DEVICE_ID] = ConsumeNonEmptyText(provider);
    jsonObj[DESCRIPTOR] = ConsumeNonEmptyText(provider);
    jsonObj[DINPUT_SOFTBUS_KEY_INPUT_DATA] = ConsumeNonEmptyText(provider);
    jsonObj[DINPUT_SOFTBUS_KEY_VECTOR_DHID] =
        ConsumeNonEmptyText(provider) + "." + ConsumeNonEmptyText(provider);
    jsonObj[DINPUT_SOFTBUS_KEY_SRC_DEV_ID] = ConsumeNonEmptyText(provider);
    jsonObj[DINPUT_SOFTBUS_KEY_SINK_DEV_ID] = ConsumeNonEmptyText(provider);
    return jsonObj;
}

std::vector<std::string> ConsumeStringVector(FuzzedDataProvider &provider)
{
    uint32_t count = provider.ConsumeIntegralInRange<uint32_t>(0U, VECTOR_MAX_COUNT);
    std::vector<std::string> values;
    values.reserve(count);
    for (uint32_t i = 0U; i < count; ++i) {
        values.emplace_back(ConsumeText(provider));
    }
    return values;
}

void ExerciseJsonHelpers(FuzzedDataProvider &provider)
{
    nlohmann::json typedJson = BuildTypedJson(provider);
    nlohmann::json softbusJson = BuildSoftBusJson(provider);
    std::string randomMessage = ConsumeText(provider);

    static_cast<void>(IsBoolean(typedJson, "boolKey"));
    static_cast<void>(IsString(typedJson, "stringKey"));
    static_cast<void>(IsInt32(typedJson, "intKey"));
    static_cast<void>(IsInt64(typedJson, "intKey"));
    static_cast<void>(IsUInt16(typedJson, "uintKey"));
    static_cast<void>(IsUInt32(typedJson, "uintKey"));
    static_cast<void>(IsUInt64(typedJson, "uintKey"));
    static_cast<void>(IsArray(typedJson, "arrayKey"));
    static_cast<void>(IsString(typedJson, "missingKey"));
    static_cast<void>(GetNodeDesc(typedJson.dump()));
    static_cast<void>(GetNodeDesc(randomMessage));
    static_cast<void>(SetAnonyId(softbusJson.dump()));
    static_cast<void>(SetAnonyId(randomMessage));
}

void ExerciseStringHelpers(FuzzedDataProvider &provider)
{
    std::string value = ConsumeText(provider);
    std::string joinedInput = ConsumeNonEmptyText(provider) + "." +
        ConsumeText(provider) + "." + ConsumeText(provider);
    char split = provider.ConsumeBool() ? '.' : ',';
    std::vector<std::string> splitValues;
    std::vector<std::string> dhIds = ConsumeStringVector(provider);

    static_cast<void>(GetAnonyString(value));
    static_cast<void>(GetAnonyInt32(provider.ConsumeIntegral<int32_t>()));
    static_cast<void>(Sha256(value));
    SplitStringToVector(joinedInput, split, splitValues);
    static_cast<void>(GetString(splitValues));
    static_cast<void>(JointDhIds(dhIds));
    static_cast<void>(SplitDhIdString(joinedInput));
}

void ExerciseDevicePathHelpers(FuzzedDataProvider &provider)
{
    std::string invalidPath = provider.ConsumeBool() ? "" :
        "/not_exist_dinput_" + ConsumeText(provider);
    std::vector<std::string> inputPaths;
    std::vector<std::string> nodePaths = { invalidPath };
    std::array<unsigned long, BIT_ARRAY_SIZE> bits = {
        provider.ConsumeIntegral<unsigned long>(),
        provider.ConsumeIntegral<unsigned long>(),
    };
    int bit = provider.ConsumeIntegralInRange<int>(
        BIT_INDEX_MIN, static_cast<int>(bits.size() * LONG_BITS - 1U));
    int fd = UN_INIT_FD_VALUE;

    static_cast<void>(BitIsSet(bits.data(), bit));
    static_cast<void>(OpenInputDeviceFdByPath(invalidPath));
    static_cast<void>(ConvertErrNo());
    ScanInputDevicesPath(invalidPath, inputPaths);
    ResetVirtualDevicePressedKeys(nodePaths);
    CloseFd(fd);
}
} // namespace

void DInputUtilsToolFuzzTest(const uint8_t *data, size_t size)
{
    if (data == nullptr) {
        return;
    }

    FuzzedDataProvider provider(data, size);
    ExerciseJsonHelpers(provider);
    ExerciseStringHelpers(provider);
    ExerciseDevicePathHelpers(provider);
}
} // namespace DistributedInput
} // namespace DistributedHardware
} // namespace OHOS

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    OHOS::DistributedHardware::DistributedInput::DInputUtilsToolFuzzTest(data, size);
    return OHOS::DistributedHardware::DistributedInput::FUZZ_TEST_SUCCESS;
}
