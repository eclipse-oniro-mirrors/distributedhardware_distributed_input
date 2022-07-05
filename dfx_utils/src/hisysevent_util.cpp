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

#include "hisysevent_util.h"

#include "anonymous_string.h"
#include "distributed_hardware_log.h"

namespace OHOS {
namespace DistributedHardware {
namespace DistributedInput {
IMPLEMENT_SINGLE_INSTANCE(HisyseventUtil);

void HisyseventUtil::SysEventWriteBehavior(std::string status, std::string msg)
{
    int32_t res = OHOS::HiviewDFX::HiSysEvent::Write(
        OHOS::HiviewDFX::HiSysEvent::Domain::DISTRIBUTED_INPUT,
        status,
        OHOS::HiviewDFX::HiSysEvent::EventType::BEHAVIOR,
        "MSG", msg);
    if (res != SUCCESS) {
        DHLOGE("Write HiSysEvent error, res:%d", res);
    }
}

void HisyseventUtil::SysEventWriteBehavior(std::string status, std::string devId, std::string msg)
{
    int32_t res = OHOS::HiviewDFX::HiSysEvent::Write(
        OHOS::HiviewDFX::HiSysEvent::Domain::DISTRIBUTED_INPUT,
        status,
        OHOS::HiviewDFX::HiSysEvent::EventType::BEHAVIOR,
        "DEVID", GetAnonyString(devId),
        "MSG", msg);
    if (res != SUCCESS) {
        DHLOGE("Write HiSysEvent error, res:%d", res);
    }
}

void HisyseventUtil::SysEventWriteBehavior(std::string status, std::string devId, std::string dhId, std::string msg)
{
    int32_t res = OHOS::HiviewDFX::HiSysEvent::Write(
        OHOS::HiviewDFX::HiSysEvent::Domain::DISTRIBUTED_INPUT,
        status,
        OHOS::HiviewDFX::HiSysEvent::EventType::BEHAVIOR,
        "DEVID", GetAnonyString(devId),
        "DHID", GetAnonyString(dhId),
        "MSG", msg);
    if (res != SUCCESS) {
        DHLOGE("Write HiSysEvent error, res:%d", res);
    }
}

void HisyseventUtil::SysEventWriteFault(std::string status, std::string msg)
{
    int32_t res = OHOS::HiviewDFX::HiSysEvent::Write(
        OHOS::HiviewDFX::HiSysEvent::Domain::DISTRIBUTED_INPUT,
        status,
        OHOS::HiviewDFX::HiSysEvent::EventType::FAULT,
        "MSG", msg);
    if (res != SUCCESS) {
        DHLOGE("Write HiSysEvent error, res:%d", res);
    }
}

void HisyseventUtil::SysEventWriteFault(std::string status, std::string devId, int32_t errorCode, std::string msg)
{
    int32_t res = OHOS::HiviewDFX::HiSysEvent::Write(
        OHOS::HiviewDFX::HiSysEvent::Domain::DISTRIBUTED_INPUT,
        status,
        OHOS::HiviewDFX::HiSysEvent::EventType::FAULT,
        "DEVID", GetAnonyString(devId),
        "ERROR_CODE", errorCode,
        "MSG", msg);
    if (res != SUCCESS) {
        DHLOGE("Write HiSysEvent error, res:%d", res);
    }
}

void HisyseventUtil::SysEventWriteFault(std::string status, std::string devId, std::string dhId, int32_t errorCode,
    std::string msg)
{
    int32_t res = OHOS::HiviewDFX::HiSysEvent::Write(
        OHOS::HiviewDFX::HiSysEvent::Domain::DISTRIBUTED_INPUT,
        status,
        OHOS::HiviewDFX::HiSysEvent::EventType::FAULT,
        "DEVID", GetAnonyString(devId),
        "DHID", GetAnonyString(dhId),
        "ERROR_CODE", errorCode,
        "MSG", msg);
    if (res != SUCCESS) {
        DHLOGE("Write HiSysEvent error, res:%d", res);
    }
}
} // namespace DistributedInput
} // namespace DistributedHardware
} // namespace OHOS