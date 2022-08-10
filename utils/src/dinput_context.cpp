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

#include "dinput_context.h"

#include "anonymous_string.h"
#include "constants.h"

#include "dinput_errcode.h"

namespace OHOS {
namespace DistributedHardware {
namespace DistributedInput {
IMPLEMENT_SINGLE_INSTANCE(DInputContext);

DInputContext::~DInputContext()
{
    sinkScreenInfoMap_.clear();
    srcScreenInfoMap_.clear();
}

std::string DInputContext::GetScreenInfoKey(const std::string &devId, const uint64_t sourceWinId)
{
    DHLOGI("GetScreenInfoKey screenInfoKey: %s, sourceWinId: %d", GetAnonyString(devId).c_str(), sourceWinId);
    return devId + RESOURCE_SEPARATOR + std::to_string(sourceWinId);
}

int32_t DInputContext::RemoveSinkScreenInfo(const std::string &screenInfoKey)
{
    DHLOGI("RemoveSinkScreenInfo screenInfoKey: %s", GetAnonyString(screenInfoKey).c_str());
    std::lock_guard<std::mutex> lock(sinkMapMutex_);
    sinkScreenInfoMap_.erase(screenInfoKey);
    return DH_SUCCESS;
}

int32_t DInputContext::UpdateSinkScreenInfo(const std::string &screenInfoKey, const SinkScreenInfo &sinkScreenInfo)
{
    DHLOGI("UpdateSinkScreenInfo screenInfoKey: %s", GetAnonyString(screenInfoKey).c_str());
    std::lock_guard<std::mutex> lock(sinkMapMutex_);
    if (sinkScreenInfoMap_.count(screenInfoKey) <= 0) {
        DHLOGE("source window id not exist");
        return ERR_DH_INPUT_CONTEXT_KEY_NOT_EXIST;
    }

    SinkScreenInfo tmp = sinkScreenInfo;
    if (CalculateTransformInfo(tmp) != DH_SUCCESS) {
        DHLOGE("calculate transform infomation failed");
    }

    sinkScreenInfoMap_[screenInfoKey] = tmp;
    return DH_SUCCESS;
}

SinkScreenInfo DInputContext::GetSinkScreenInfo(const std::string &screenInfoKey)
{
    DHLOGI("GetSinkScreenInfo screenInfoKey: %s", GetAnonyString(screenInfoKey).c_str());
    std::lock_guard<std::mutex> lock(sinkMapMutex_);
    if (sinkScreenInfoMap_.count(screenInfoKey) <= 0) {
        DHLOGE("screenInfoKey not exist");
        SinkScreenInfo sinkScreenInfo;
        sinkScreenInfoMap_[screenInfoKey] = sinkScreenInfo;
    }

    return sinkScreenInfoMap_[screenInfoKey];
}

const std::unordered_map<std::string, SinkScreenInfo>& DInputContext::GetAllSinkScreenInfo()
{
    std::lock_guard<std::mutex> lock(sinkMapMutex_);
    return sinkScreenInfoMap_;
}

int32_t DInputContext::RemoveSrcScreenInfo(const std::string &screenInfoKey)
{
    DHLOGI("RemoveSrcScreenInfo screenInfoKey: %s", GetAnonyString(screenInfoKey).c_str());
    std::lock_guard<std::mutex> lock(srcMapMutex_);
    srcScreenInfoMap_.erase(screenInfoKey);
    return DH_SUCCESS;
}

int32_t DInputContext::UpdateSrcScreenInfo(const std::string &screenInfoKey, const SrcScreenInfo &srcScreenInfo)
{
    std::lock_guard<std::mutex> lock(srcMapMutex_);
    DHLOGI("UpdateSrcScreenInfo screenInfoKey: %s", GetAnonyString(screenInfoKey).c_str());
    if (srcScreenInfoMap_.count(screenInfoKey) <= 0) {
        DHLOGE("source window id not exist");
        return ERR_DH_INPUT_CONTEXT_KEY_NOT_EXIST;
    }

    srcScreenInfoMap_[screenInfoKey] = srcScreenInfo;
    return DH_SUCCESS;
}

SrcScreenInfo DInputContext::GetSrcScreenInfo(const std::string &screenInfoKey)
{
    DHLOGI("GetSrcScreenInfo screenInfoKey: %s", GetAnonyString(screenInfoKey).c_str());
    std::lock_guard<std::mutex> lock(srcMapMutex_);
    if (srcScreenInfoMap_.count(screenInfoKey) <= 0) {
        DHLOGE("source window id not exist");
        SrcScreenInfo srcScreenInfo;
        srcScreenInfoMap_[screenInfoKey] = srcScreenInfo;
    }

    return srcScreenInfoMap_[screenInfoKey];
}

void DInputContext::SetLocalTouchScreenInfo(const LocalTouchScreenInfo &localTouchScreenInfo)
{
    std::lock_guard<std::mutex> lock(localTouchScreenInfoMutex_);
    localTouchScreenInfo_ = localTouchScreenInfo;
}

LocalTouchScreenInfo DInputContext::GetLocalTouchScreenInfo()
{
    std::lock_guard<std::mutex> lock(localTouchScreenInfoMutex_);
    return localTouchScreenInfo_;
}

int32_t DInputContext::CalculateTransformInfo(SinkScreenInfo &sinkScreenInfo)
{
    if (sinkScreenInfo.sinkShowHeight == 0 || sinkScreenInfo.sinkShowWidth == 0) {
        DHLOGE("can not calculate transform infomation");
        return ERR_DH_INPUT_CONTEXT_CALCULATE_FAIL;
    }
    TransformInfo transformInfo;
    transformInfo.sinkWinPhyX = (uint32_t)(sinkScreenInfo.sinkWinShowX / (double)(sinkScreenInfo.sinkShowWidth)) *
        sinkScreenInfo.sinkPhyWidth;
    transformInfo.sinkWinPhyY = (uint32_t)(sinkScreenInfo.sinkWinShowY / (double)(sinkScreenInfo.sinkShowHeight)) *
        sinkScreenInfo.sinkPhyHeight;
    transformInfo.sinkProjPhyWidth = (uint32_t)((sinkScreenInfo.sinkProjShowWidth /
        (double)sinkScreenInfo.sinkShowWidth) * sinkScreenInfo.sinkPhyWidth);
    transformInfo.sinkProjPhyHeight = (uint32_t)((sinkScreenInfo.sinkProjShowHeight /
        (double)sinkScreenInfo.sinkShowHeight) * sinkScreenInfo.sinkPhyHeight);
    if (transformInfo.sinkProjPhyWidth == 0 || transformInfo.sinkProjPhyHeight == 0) {
        DHLOGE("can not calculate transform infomation");
        return ERR_DH_INPUT_CONTEXT_CALCULATE_FAIL;
    }

    // coefficient of the sink projection area in the source touch driver
    transformInfo.coeffWidth = (double)(sinkScreenInfo.srcScreenInfo.sourcePhyWidth /
        (double)(transformInfo.sinkProjPhyWidth));
    transformInfo.coeffHeight = (double)(sinkScreenInfo.srcScreenInfo.sourcePhyHeight /
        (double)(transformInfo.sinkProjPhyHeight));

    DHLOGI("CalculateTransformInfo sinkWinPhyX = %d, sinkWinPhyY = %d, sinkProjPhyWidth = %d, " +
        "sinkProjPhyHeight = %d, coeffWidth = %f, coeffHeight = %f", transformInfo.sinkWinPhyX,
        transformInfo.sinkWinPhyY, transformInfo.sinkProjPhyWidth, transformInfo.sinkProjPhyHeight,
        transformInfo.coeffWidth, transformInfo.coeffHeight);
    sinkScreenInfo.transformInfo = transformInfo;
    return DH_SUCCESS;
}

std::shared_ptr<DistributedHardwareFwkKit> DInputContext::GetDHFwkKit()
{
    std::lock_guard<std::mutex> lock(dhFwkKitMutex_);
    if (dhFwkKit_ == nullptr) {
        dhFwkKit_ = std::make_shared<DistributedHardwareFwkKit>();
    }
    return dhFwkKit_;
}
} // namespace DistributedInput
} // namespace DistributedHardware
} // namespace OHOS