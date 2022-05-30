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

#include "distributed_input_sink_switch.h"

#include "anonymous_string.h"
#include "distributed_hardware_log.h"

#include "constants_dinput.h"
#include "dinput_errcode.h"

namespace OHOS {
namespace DistributedHardware {
namespace DistributedInput {
DistributedInputSinkSwitch::DistributedInputSinkSwitch()
{
    InitSwitch();
}

DistributedInputSinkSwitch::~DistributedInputSinkSwitch()
{
    DHLOGI("~DistributedInputSinkSwitch()");
    InitSwitch();
}

DistributedInputSinkSwitch &DistributedInputSinkSwitch::GetInstance()
{
    static DistributedInputSinkSwitch instance;
    return instance;
}

void DistributedInputSinkSwitch::InitSwitch()
{
    DHLOGI("InitSwitch.");
    std::unique_lock<std::mutex> switchLock(operationMutex_);
    switchVector_.clear();
}

int32_t DistributedInputSinkSwitch::StartSwitch(int32_t sessionId)
{
    std::unique_lock<std::mutex> switchLock(operationMutex_);
    if (switchVector_.empty()) {
        DHLOGE("StartSwitch sessionId:%d fail,switchVector_ is null.", sessionId);
        return ERR_DH_INPUT_SERVER_SINK_START_SWITCH_FAIL;
    } else {
        bool findOld = false;
        for (std::vector<SwitchStateData>::iterator it = switchVector_.begin(); it < switchVector_.end(); it++) {
            if ((*it).sessionId == sessionId) {
                (*it).switchState = true;
                findOld = true;
                break;
            }
        }

        if (findOld) {
            DHLOGI("StartSwitch sessionId:%s is find.", GetAnonyInt32(sessionId).c_str());
            return DH_SUCCESS;
        } else {
            DHLOGE("StartSwitch sessionId:%s fail, not found.", GetAnonyInt32(sessionId).c_str());
            return ERR_DH_INPUT_SERVER_SINK_START_SWITCH_FAIL;
        }
    }
}

void DistributedInputSinkSwitch::StopSwitch(int32_t sessionId)
{
    std::unique_lock<std::mutex> switchLock(operationMutex_);
    if (switchVector_.empty()) {
        DHLOGE("StopSwitch sessionId:%s fail,switchVector_ is null.", GetAnonyInt32(sessionId).c_str());
    } else {
        bool findOld = false;
        for (std::vector<SwitchStateData>::iterator it = switchVector_.begin(); it < switchVector_.end(); it++) {
            if ((*it).sessionId == sessionId) {
                (*it).switchState = false;
                findOld = true;
                break;
            }
        }

        if (findOld) {
            DHLOGI("StopSwitch sessionId:%s is success.", GetAnonyInt32(sessionId).c_str());
        } else {
            DHLOGE("StopSwitch sessionId:%s fail,not find it.", GetAnonyInt32(sessionId).c_str());
        }
    }
}

void DistributedInputSinkSwitch::StopAllSwitch()
{
    std::unique_lock<std::mutex> switchLock(operationMutex_);
    if (switchVector_.empty()) {
        DHLOGW("StopAllSwitch switchVector_ is null.");
    } else {
        for (std::vector<SwitchStateData>::iterator it = switchVector_.begin(); it < switchVector_.end(); it++) {
            (*it).switchState = false;
        }
        DHLOGI("StopAllSwitch success.");
    }
}

void DistributedInputSinkSwitch::AddSession(int32_t sessionId)
{
    std::unique_lock<std::mutex> switchLock(operationMutex_);
    if (switchVector_.empty()) {
        SwitchStateData tmp(sessionId, false);
        switchVector_.push_back(tmp);
        DHLOGI("AddSession sessionId:%s add first.", GetAnonyInt32(sessionId).c_str());
    } else {
        bool findOld = false;
        for (std::vector<SwitchStateData>::iterator it = switchVector_.begin(); it < switchVector_.end(); it++) {
            if ((*it).sessionId == sessionId) {
                (*it).switchState = false;
                findOld = true;
                break;
            }
        }

        if (findOld) {
            DHLOGI("AddSession sessionId:%s is find.", GetAnonyInt32(sessionId).c_str());
        } else {
            SwitchStateData tmp(sessionId, false);
            switchVector_.push_back(tmp);
            DHLOGI("AddSession sessionId:%s add new.", GetAnonyInt32(sessionId).c_str());
        }
    }
}

void DistributedInputSinkSwitch::RemoveSession(int32_t sessionId)
{
    std::unique_lock<std::mutex> switchLock(operationMutex_);
    if (switchVector_.empty()) {
        DHLOGE("RemoveSession sessionId:%s fail,switch_vector is null.", GetAnonyInt32(sessionId).c_str());
    } else {
        bool findOld = false;
        for (std::vector<SwitchStateData>::iterator it = switchVector_.begin(); it < switchVector_.end();) {
            if ((*it).sessionId == sessionId) {
                it = switchVector_.erase(it);
                findOld = true;
                break;
            } else {
                it++;
            }
        }
        if (findOld) {
            DHLOGI("RemoveSession sessionId:%s is success.", GetAnonyInt32(sessionId).c_str());
        } else {
            DHLOGE("RemoveSession sessionId:%s fail,not find it.", GetAnonyInt32(sessionId).c_str());
        }
    }
}

std::vector<int32_t> DistributedInputSinkSwitch::GetAllSessionId()
{
    std::unique_lock<std::mutex> switchLock(operationMutex_);
    std::vector<int32_t> tmpVecSession;
    for (std::vector<SwitchStateData>::iterator it = switchVector_.begin(); it < switchVector_.end(); it++) {
        tmpVecSession.push_back((*it).sessionId);
    }
    return tmpVecSession;
}

// get current session which state is on, if error return -1.
int32_t DistributedInputSinkSwitch::GetSwitchOpenedSession()
{
    std::unique_lock<std::mutex> switchLock(operationMutex_);
    if (switchVector_.empty()) {
        DHLOGE("GetSwitchOpenedSession error, no data.");
        return ERR_DH_INPUT_SERVER_SINK_GET_OPEN_SESSION_FAIL;
    }
    for (std::vector<SwitchStateData>::iterator it = switchVector_.begin(); it < switchVector_.end(); it++) {
        if ((*it).switchState == true) {
            return (*it).sessionId;
        }
    }
    DHLOGE("GetSwitchOpenedSession no session is open.");
    return ERR_DH_INPUT_SERVER_SINK_GET_OPEN_SESSION_FAIL;
}
} // namespace DistributedInput
} // namespace DistributedHardware
} // namespace OHOS
