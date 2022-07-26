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

#ifndef DINPUT_LOW_LATENCY_H
#define DINPUT_LOW_LATENCY_H

#include "single_instance.h"

namespace OHOS {
namespace DistributedHardware {
namespace DistributedInput {
class DInputLowLatency {
DECLARE_SINGLE_INSTANCE_BASE(DInputLowLatency);
public:
    void EnableSourceLowLatency();
    void DisableSourceLowLatency();
    void EnableSinkLowLatency();
    void DisableSinkLowLatency();
private:
    DInputLowLatency() = default;
    ~DInputLowLatency() = default;
};
}  // namespace DistributedInput
}  // namespace DistributedHardware
}  // namespace OHOS
#endif  // DINPUT_LOW_LATENCY_H
