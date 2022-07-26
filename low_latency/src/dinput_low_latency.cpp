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

#include "dinput_low_latency.h"

#include "res_sched_client.h"
#include "res_type.h"

namespace OHOS {
namespace DistributedHardware {
namespace DistributedInput {
IMPLEMENT_SINGLE_INSTANCE(DInputLowLatency);

constexpr int32_t MODE_ENABLE = 0;
constexpr int32_t MODE_DISABLE = 1;
const std::string KEY = "identity";
const std::string PKG_SOURCE = "ohos.DistributedHardware.DistributedInput.DistributedInputSourceTransport";
const std::string PKG_SINK = "ohos.DistributedHardware.DistributedInput.DistributedInputSinkTransport";

void DInputLowLatency::EnableSourceLowLatency()
{
    auto &rssClient = OHOS::ResourceSchedule::ResSchedClient::GetInstance();
    // to enable low latency mode: value = 0
    rssClient.ReportData(OHOS::ResourceSchedule::ResType::RES_TYPE_NETWORK_LATENCY_REQUEST, MODE_ENABLE,
        {{KEY, PKG_SOURCE}});
}

void DInputLowLatency::DisableSourceLowLatency()
{
    auto &rssClient = OHOS::ResourceSchedule::ResSchedClient::GetInstance();
    // to restore normal latency mode: value = 1
    rssClient.ReportData(OHOS::ResourceSchedule::ResType::RES_TYPE_NETWORK_LATENCY_REQUEST, MODE_DISABLE,
        {{KEY, PKG_SOURCE}});
}

void DInputLowLatency::EnableSinkLowLatency()
{
    auto &rssClient = OHOS::ResourceSchedule::ResSchedClient::GetInstance();
    // to enable low latency mode: value = 0
    rssClient.ReportData(OHOS::ResourceSchedule::ResType::RES_TYPE_NETWORK_LATENCY_REQUEST, MODE_ENABLE,
        {{KEY, PKG_SINK}});
}

void DInputLowLatency::DisableSinkLowLatency()
{
    auto &rssClient = OHOS::ResourceSchedule::ResSchedClient::GetInstance();
    // to restore normal latency mode: value = 1
    rssClient.ReportData(OHOS::ResourceSchedule::ResType::RES_TYPE_NETWORK_LATENCY_REQUEST, MODE_DISABLE,
        {{KEY, PKG_SINK}});
}
} // namespace DistributedInput
} // namespace DistributedHardware
} // namespace OHOS