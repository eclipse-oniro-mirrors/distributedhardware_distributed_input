/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#include "distributed_input_kit.h"

namespace OHOS {
namespace DistributedHardware {
namespace DistributedInput {
int32_t DistributedInputKit::PrepareRemoteInput(
    const std::string& deviceId, sptr<IPrepareDInputCallback> callback)
{
    return DistributedInputClient::GetInstance().PrepareRemoteInput(deviceId, callback);
}

int32_t DistributedInputKit::UnprepareRemoteInput(
    const std::string& deviceId, sptr<IUnprepareDInputCallback> callback)
{
    return DistributedInputClient::GetInstance().UnprepareRemoteInput(deviceId, callback);
}

int32_t DistributedInputKit::StartRemoteInput(
    const std::string& deviceId, const uint32_t& inputTypes, sptr<IStartDInputCallback> callback)
{
    return DistributedInputClient::GetInstance().StartRemoteInput(deviceId, inputTypes, callback);
}

int32_t DistributedInputKit::StopRemoteInput(
    const std::string& deviceId, const uint32_t& inputTypes, sptr<IStopDInputCallback> callback)
{
    return DistributedInputClient::GetInstance().StopRemoteInput(deviceId, inputTypes, callback);
}

bool DistributedInputKit::IsNeedFilterOut(const std::string& deviceId, const BusinessEvent& event)
{
    return DistributedInputClient::GetInstance().IsNeedFilterOut(deviceId, event);
}

DInputServerType DistributedInputKit::IsStartDistributedInput(const uint32_t& inputType)
{
    return DistributedInputClient::GetInstance().IsStartDistributedInput(inputType);
}
} // namespace DistributedInput
} // namespace DistributedHardware
} // namespace OHOS
