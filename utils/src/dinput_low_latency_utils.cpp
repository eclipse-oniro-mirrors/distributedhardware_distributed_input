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

#include "dinput_low_latency_utils.h"

#include <cstdint>
#include <dlfcn.h>
#include <string>

#include "distributed_hardware_log.h"

#include "dinput_errcode.h"

namespace OHOS {
namespace DistributedHardware {
namespace DistributedInput {
IMPLEMENT_SINGLE_INSTANCE(DInputLowLatencyUtils);
const std::string LIB_NAME = "libdinput_low_latency.z.so";
const std::string FUNC_GET_ENABLE_SOURCE = "EnableSourceLowLatency";
const std::string FUNC_GET_DISABLE_SOURCE = "DisableSourceLowLatency";
const std::string FUNC_GET_ENABLE_SINK = "EnableSinkLowLatency";
const std::string FUNC_GET_DISABLE_SINK = "DisableSinkLowLatency";

using GetLowLatencyFunc = void *(*)();

DInputLowLatencyUtils::~DInputLowLatencyUtils()
{
    CloseLibrary();
}

void DInputLowLatencyUtils::EnableSourceLowLatency()
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (LoadLibrary() != DH_SUCCESS) {
        DHLOGE("load library failed");
        return;
    }
    auto enableSourceLowLatency = reinterpret_cast<GetLowLatencyFunc>(dlsym(handler_, FUNC_GET_ENABLE_SOURCE.c_str()));
    if (enableSourceLowLatency == nullptr) {
        DHLOGE("can not find %s, failed reason : %s", FUNC_GET_ENABLE_SOURCE.c_str(), dlerror());
        return;
    }

    enableSourceLowLatency();
}

void DInputLowLatencyUtils::DisableSourceLowLatency()
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (LoadLibrary() != DH_SUCCESS) {
        DHLOGE("load library failed");
        return;
    }
    auto disableSourceLowLatency =
        reinterpret_cast<GetLowLatencyFunc>(dlsym(handler_, FUNC_GET_DISABLE_SOURCE.c_str()));
    if (disableSourceLowLatency == nullptr) {
        DHLOGE("can not find %s, failed reason : %s", FUNC_GET_DISABLE_SOURCE.c_str(), dlerror());
        return;
    }

    disableSourceLowLatency();
}

void DInputLowLatencyUtils::EnableSinkLowLatency()
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (LoadLibrary() != DH_SUCCESS) {
        DHLOGE("load library failed");
        return;
    }
    auto enableSinkLowLatency = reinterpret_cast<GetLowLatencyFunc>(dlsym(handler_, FUNC_GET_ENABLE_SINK.c_str()));
    if (enableSinkLowLatency == nullptr) {
        DHLOGE("can not find %s, failed reason : %s", FUNC_GET_ENABLE_SINK.c_str(), dlerror());
        return;
    }

    enableSinkLowLatency();
}

void DInputLowLatencyUtils::DisableSinkLowLatency()
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (LoadLibrary() != DH_SUCCESS) {
        DHLOGE("load library failed");
        return;
    }
    auto disableSinkLowLatency = reinterpret_cast<GetLowLatencyFunc>(dlsym(handler_, FUNC_GET_DISABLE_SINK.c_str()));
    if (disableSinkLowLatency == nullptr) {
        DHLOGE("can not find %s, failed reason : %s", FUNC_GET_DISABLE_SINK.c_str(), dlerror());
        return;
    }

    disableSinkLowLatency();
}

int32_t DInputLowLatencyUtils::LoadLibrary()
{
    DHLOGI("start.");
    if (handler_ != nullptr) {
        DHLOGE("DInputLowLatencyUtils handler has loaded.");
        return DH_SUCCESS;
    }

    handler_ = dlopen(LIB_NAME.c_str(), RTLD_NOW | RTLD_NODELETE);
    if (handler_ == nullptr) {
        DHLOGE("open %s failed, fail reason : %s", LIB_NAME.c_str(), dlerror());
        return ERR_DH_INPUT_DLOPEN_FAIL;
    }
    return DH_SUCCESS;
}

void DInputLowLatencyUtils::CloseLibrary()
{
    if (handler_ == nullptr) {
        DHLOGI("%s is already closed.", LIB_NAME.c_str());
        return;
    }
    dlclose(handler_);
    handler_ = nullptr;
    DHLOGI("%s is closed.", LIB_NAME.c_str());
}
} // namespace DistributedInput
} // namespace DistributedHardware
} // namespace OHOS