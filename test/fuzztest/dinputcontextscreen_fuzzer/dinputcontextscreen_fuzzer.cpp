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

#include "dinputcontextscreen_fuzzer.h"

#include <cstddef>
#include <cstdint>
#include <string>

#include <fuzzer/FuzzedDataProvider.h>
#include <iremote_object.h>

#include "dinput_context.h"

namespace OHOS {
namespace DistributedHardware {
namespace DistributedInput {
namespace {
constexpr size_t ID_MAX_SIZE = 64U;
constexpr uint32_t DIMENSION_MIN = 1U;
constexpr uint32_t DIMENSION_MAX = 4096U;
constexpr int32_t SA_ID_MIN = -16;
constexpr int32_t SA_ID_MAX = 4096;
constexpr uint32_t ZERO_DIMENSION = 0U;
constexpr int32_t FUZZ_TEST_SUCCESS = 0;

std::string ConsumeId(FuzzedDataProvider &provider)
{
    std::string id = provider.ConsumeRandomLengthString(ID_MAX_SIZE);
    return id.empty() ? "screen_device" : id;
}

uint32_t ConsumeDimension(FuzzedDataProvider &provider, bool allowZero)
{
    if (allowZero && provider.ConsumeBool()) {
        return ZERO_DIMENSION;
    }
    return provider.ConsumeIntegralInRange<uint32_t>(DIMENSION_MIN, DIMENSION_MAX);
}

SrcScreenInfo BuildSrcScreenInfo(FuzzedDataProvider &provider)
{
    SrcScreenInfo info;
    info.devId = ConsumeId(provider);
    info.uuid = ConsumeId(provider);
    info.sessionId = provider.ConsumeIntegral<int32_t>();
    info.sourceWinId = provider.ConsumeIntegral<uint64_t>();
    info.sourceWinWidth = ConsumeDimension(provider, false);
    info.sourceWinHeight = ConsumeDimension(provider, false);
    info.sourcePhyId = ConsumeId(provider);
    info.sourcePhyFd = provider.ConsumeIntegral<uint32_t>();
    info.sourcePhyWidth = ConsumeDimension(provider, false);
    info.sourcePhyHeight = ConsumeDimension(provider, false);
    return info;
}

SinkScreenInfo BuildSinkScreenInfo(FuzzedDataProvider &provider)
{
    SinkScreenInfo info;
    info.sinkShowWidth = ConsumeDimension(provider, true);
    info.sinkShowHeight = ConsumeDimension(provider, true);
    info.sinkPhyWidth = ConsumeDimension(provider, false);
    info.sinkPhyHeight = ConsumeDimension(provider, false);
    info.sinkShowWinId = provider.ConsumeIntegral<uint64_t>();
    info.sinkWinShowX = ConsumeDimension(provider, true);
    info.sinkWinShowY = ConsumeDimension(provider, true);
    info.sinkProjShowWidth = ConsumeDimension(provider, true);
    info.sinkProjShowHeight = ConsumeDimension(provider, true);
    info.srcScreenInfo = BuildSrcScreenInfo(provider);
    return info;
}

LocalTouchScreenInfo BuildLocalTouchScreenInfo(FuzzedDataProvider &provider)
{
    LocalTouchScreenInfo info;
    info.sinkShowWidth = ConsumeDimension(provider, true);
    info.sinkShowHeight = ConsumeDimension(provider, true);
    info.sinkPhyWidth = ConsumeDimension(provider, true);
    info.sinkPhyHeight = ConsumeDimension(provider, true);
    info.localAbsInfo.absXMin = provider.ConsumeIntegral<int32_t>();
    info.localAbsInfo.absXMax = provider.ConsumeIntegral<int32_t>();
    info.localAbsInfo.absYMin = provider.ConsumeIntegral<int32_t>();
    info.localAbsInfo.absYMax = provider.ConsumeIntegral<int32_t>();
    info.localAbsInfo.deviceInfo.name = ConsumeId(provider);
    info.localAbsInfo.deviceInfo.physicalPath = ConsumeId(provider);
    info.localAbsInfo.deviceInfo.descriptor = ConsumeId(provider);
    return info;
}

void ExerciseSinkInfo(DInputContext &context, FuzzedDataProvider &provider)
{
    std::string devId = ConsumeId(provider);
    uint64_t sourceWinId = provider.ConsumeIntegral<uint64_t>();
    std::string key = context.GetScreenInfoKey(devId, sourceWinId);
    SinkScreenInfo sinkInfo = BuildSinkScreenInfo(provider);

    (void)context.UpdateSinkScreenInfo(key + "_missing", sinkInfo);
    (void)context.GetSinkScreenInfo(key);
    (void)context.UpdateSinkScreenInfo(key, sinkInfo);
    (void)context.GetAllSinkScreenInfo();
    context.CleanExceptionalInfo(sinkInfo.srcScreenInfo);
    (void)context.RemoveSinkScreenInfo(key);
}

void ExerciseSrcInfo(DInputContext &context, FuzzedDataProvider &provider)
{
    std::string key = context.GetScreenInfoKey(ConsumeId(provider),
        provider.ConsumeIntegral<uint64_t>());
    SrcScreenInfo srcInfo = BuildSrcScreenInfo(provider);

    (void)context.UpdateSrcScreenInfo(key + "_missing", srcInfo);
    (void)context.GetSrcScreenInfo(key);
    (void)context.UpdateSrcScreenInfo(key, srcInfo);
    context.CleanExceptionalInfo(srcInfo);
    (void)context.RemoveSrcScreenInfo(key);
}

void ExerciseLocalAndRemoteInfo(DInputContext &context, FuzzedDataProvider &provider)
{
    int32_t saId = provider.ConsumeIntegralInRange<int32_t>(SA_ID_MIN, SA_ID_MAX);
    sptr<IRemoteObject> remoteObject = nullptr;

    context.SetLocalTouchScreenInfo(BuildLocalTouchScreenInfo(provider));
    (void)context.GetLocalTouchScreenInfo();
    (void)context.GetDHFwkKit();
    context.AddRemoteObject(saId, remoteObject);
    (void)context.GetRemoteObject(saId);
    context.RemoveRemoteObject(saId);
}
} // namespace

void DInputContextScreenFuzzTest(const uint8_t *data, size_t size)
{
    if (data == nullptr) {
        return;
    }

    FuzzedDataProvider provider(data, size);
    DInputContext &context = DInputContext::GetInstance();
    ExerciseSinkInfo(context, provider);
    ExerciseSrcInfo(context, provider);
    ExerciseLocalAndRemoteInfo(context, provider);
}
} // namespace DistributedInput
} // namespace DistributedHardware
} // namespace OHOS

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    OHOS::DistributedHardware::DistributedInput::DInputContextScreenFuzzTest(data, size);
    return OHOS::DistributedHardware::DistributedInput::FUZZ_TEST_SUCCESS;
}
