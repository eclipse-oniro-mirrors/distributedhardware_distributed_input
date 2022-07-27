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

#ifndef DISTRIBUTED_INPUT_SINK_SWITCH_H
#define DISTRIBUTED_INPUT_SINK_SWITCH_H

#include <mutex>
#include <set>
#include <string>
#include <vector>

#include "dinput_softbus_define.h"

namespace OHOS {
namespace DistributedHardware {
namespace DistributedInput {
class DistributedInputSinkSwitch {
public:
    static DistributedInputSinkSwitch &GetInstance();
    DistributedInputSinkSwitch();
    ~DistributedInputSinkSwitch();
    void InitSwitch();

    int32_t StartSwitch(int32_t sessionId);
    void StopSwitch(int32_t sessionId);
    void AddSession(int32_t sessionId);
    void RemoveSession(int32_t sessionId);
    void StopAllSwitch();
    std::vector<int32_t> GetAllSessionId();
    int32_t GetSwitchOpenedSession();

private:
    std::mutex operationMutex_;
    std::vector<SwitchStateData> switchVector_;
};
} // namespace DistributedInput
} // namespace DistributedHardware
} // namespace OHOS

#endif // DISTRIBUTED_INPUT_SINK_SWITCH_H
