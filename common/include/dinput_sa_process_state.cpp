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

#include "dinput_sa_process_state.h"

#include <mutex>

#include "distributed_hardware_log.h"

#include "constants_dinput.h"
#include "hisysevent_util.h"

namespace OHOS {
namespace DistributedHardware {
namespace DistributedInput {
enum DInputSAState : uint32_t {
    DINPUT_SA_EXIT_STATE_START = 0,
    DINPUT_SA_EXIT_STATE_STOP = 1
};

DInputSAState g_sinkSAState = DINPUT_SA_EXIT_STATE_START;
DInputSAState g_sourceSAState = DINPUT_SA_EXIT_STATE_START;
std::mutex g_saProcessState;

void SetSinkProcessExit()
{
    DHLOGI("set sink process exit.");
    std::lock_guard<std::mutex> autoLock(g_saProcessState);
    g_sinkSAState = DINPUT_SA_EXIT_STATE_STOP;
    DHLOGI("g_sourceSAState = %d g_sinkSAState = %d", g_sourceSAState, g_sinkSAState);
    if (g_sourceSAState == DINPUT_SA_EXIT_STATE_START || g_sinkSAState == DINPUT_SA_EXIT_STATE_START) {
        return;
    }
    DHLOGI("exit sa process success.");
    HisyseventUtil::GetInstance().SysEventWriteBehavior(DINPUT_EXIT, "dinput sink sa exit success.");
    exit(0);
}

void SetSourceProcessExit()
{
    DHLOGI("set sources process exit.");
    std::lock_guard<std::mutex> autoLock(g_saProcessState);
    g_sourceSAState = DINPUT_SA_EXIT_STATE_STOP;
    DHLOGI("g_sourceSAState = %d g_sinkSAState = %d", g_sourceSAState, g_sinkSAState);
    if (g_sourceSAState == DINPUT_SA_EXIT_STATE_START || g_sinkSAState == DINPUT_SA_EXIT_STATE_START) {
        return;
    }
    DHLOGI("exit sa process success.");
    HisyseventUtil::GetInstance().SysEventWriteBehavior(DINPUT_EXIT, "dinput source sa exit success.");
    exit(0);
}
} // namespace DistributedInput
} // namespace DistributedHardware
} // namespace OHOS