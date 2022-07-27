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

#ifndef OHOS_DINPUT_HITRACE_H
#define OHOS_DINPUT_HITRACE_H

#include <string>

#include "hitrace_meter.h"

namespace OHOS {
namespace DistributedHardware {
constexpr uint64_t DINPUT_HITRACE_LABEL = HITRACE_TAG_DISTRIBUTED_INPUT;

const std::string DINPUT_PREPARE_START = "DINPUT_PREPARE_START";
const std::string DINPUT_START_START = "DINPUT_START_START";
const std::string DINPUT_OPEN_SESSION_START = "DINPUT_OPEN_SESSION_START";
const std::string DINPUT_STOP_START = "DINPUT_STOP_START";
const std::string DINPUT_UNPREPARE_START = "DINPUT_UNPREPARE_START";

constexpr int32_t DINPUT_PREPARE_TASK = 0;
constexpr int32_t DINPUT_START_TASK = 1;
constexpr int32_t DINPUT_OPEN_SESSION_TASK = 2;
constexpr int32_t DINPUT_STOP_TASK = 3;
constexpr int32_t DINPUT_UNPREPARE_TASK = 4;
} // namespace DistributedHardware
} // namespace OHOS

#endif // OHOS_DINPUT_HITRACE_H