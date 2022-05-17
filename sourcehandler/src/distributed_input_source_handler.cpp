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

#include "distributed_input_source_handler.h"
#include "i_distributed_source_input.h"

namespace OHOS {
namespace DistributedHardware {
namespace DistributedInput {
IMPLEMENT_SINGLE_INSTANCE(DistributedInputSourceHandler);

DistributedInputSourceHandler::~DistributedInputSourceHandler()
{
}

int32_t DistributedInputSourceHandler::InitSource(const std::string &params)
{
    return DistributedInputClient::GetInstance().InitSource();
}

int32_t DistributedInputSourceHandler::ReleaseSource()
{
    return DistributedInputClient::GetInstance().ReleaseSource();
}

int32_t DistributedInputSourceHandler::RegisterDistributedHardware(const std::string &devId,
    const std::string &dhId, const EnableParam &param, std::shared_ptr<RegisterCallback> callback)
{
    return DistributedInputClient::GetInstance().RegisterDistributedHardware(devId, dhId, param.attrs, callback);
}

int32_t DistributedInputSourceHandler::UnregisterDistributedHardware(const std::string &devId,
    const std::string &dhId, std::shared_ptr<UnregisterCallback> callback)
{
    return DistributedInputClient::GetInstance().UnregisterDistributedHardware(devId, dhId, callback);
}

int32_t DistributedInputSourceHandler::ConfigDistributedHardware(const std::string &devId,
    const std::string &dhId, const std::string &key, const std::string &value)
{
    return SUCCESS;
}

void DistributedInputSourceHandler::SALoadSourceCb::OnLoadSystemAbilitySuccess(int32_t systemAbilityId,
    const OHOS::sptr<IRemoteObject>& remoteObject)
{
    currSystemAbilityId = systemAbilityId;
    currRemoteObject = remoteObject;
}

void DistributedInputSourceHandler::SALoadSourceCb::OnLoadSystemAbilityFail(int32_t systemAbilityId)
{
    currSystemAbilityId = systemAbilityId;
}

IDistributedHardwareSource *GetSourceHardwareHandler()
{
    return &DistributedInputSourceHandler::GetInstance();
}
} // namespace DistributedInput
} // namespace DistributedHardware
} // namespace OHOS
