/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#include "nlohmann/json.hpp"
#include "distributed_hardware_log.h"

#include "constants_dinput.h"
#include "distributed_input_client.h"
#include "white_list_util.h"

namespace OHOS {
namespace DistributedHardware {
namespace DistributedInput {
DistributedInputClient &DistributedInputClient::GetInstance()
{
    static DistributedInputClient instance;
    return instance;
}

void DistributedInputClient::RegisterDInputCb::OnResult(
    const std::string& devId, const std::string& dhId, const int32_t& status)
{
    for (std::vector<DHardWareFwkRegistInfo>::iterator iter =
        DistributedInputClient::GetInstance().dHardWareFwkRstInfos.begin();
        iter != DistributedInputClient::GetInstance().dHardWareFwkRstInfos.end();
        iter++) {
        if (iter->devId == devId && iter->dhId == dhId) {
            iter->callback->OnRegisterResult(devId, dhId, status, "");
            DistributedInputClient::GetInstance().dHardWareFwkRstInfos.erase(iter);
            return;
        }
    }
}

void DistributedInputClient::UnregisterDInputCb::OnResult(
    const std::string& devId, const std::string& dhId, const int32_t& status)

{
    for (std::vector<DHardWareFwkUnRegistInfo>::iterator iter =
        DistributedInputClient::GetInstance().dHardWareFwkUnRstInfos.begin();
        iter != DistributedInputClient::GetInstance().dHardWareFwkUnRstInfos.end();
        iter++) {
        if (iter->devId == devId && iter->dhId == dhId) {
            iter->callback->OnUnregisterResult(devId, dhId, status, "");
            DistributedInputClient::GetInstance().dHardWareFwkUnRstInfos.erase(iter);
            return;
        }
    }
}

void DistributedInputClient::StartDInputServerCb::OnResult(const int32_t& status, const uint32_t& inputTypes)
{
    if (DInputServerType::SOURCE_SERVER_TYPE == static_cast<DInputServerType>(status)) {
        DistributedInputClient::GetInstance().serverType = DInputServerType::SOURCE_SERVER_TYPE;
        DistributedInputClient::GetInstance().inputTypes_ = static_cast<DInputDeviceType>(inputTypes);
    } else if (DInputServerType::SINK_SERVER_TYPE == static_cast<DInputServerType>(status)) {
        DistributedInputClient::GetInstance().serverType = DInputServerType::SINK_SERVER_TYPE;
        DistributedInputClient::GetInstance().inputTypes_ = static_cast<DInputDeviceType>(inputTypes);
    } else {
        DistributedInputClient::GetInstance().serverType = DInputServerType::NULL_SERVER_TYPE;
        DistributedInputClient::GetInstance().inputTypes_ = DInputDeviceType::NONE;
    }
}

void DistributedInputClient::AddWhiteListInfosCb::OnResult(const std::string &deviceId, const std::string &strJson)
{
    nlohmann::json inputData = nlohmann::json::parse(strJson);
    int jsonSize = inputData.size();
    DHLOGI("AddWhiteListInfosCb OnResult json size:%d.\n", jsonSize);
    TYPE_WHITE_LIST_VEC vecWhiteList = inputData;
    WhiteListUtil::GetInstance().SyncWhiteList(deviceId, vecWhiteList);
}

void DistributedInputClient::DelWhiteListInfosCb::OnResult(const std::string& deviceId)
{
    WhiteListUtil::GetInstance().ClearWhiteList(deviceId);
}

int32_t DistributedInputClient::InitSource()
{
    return SUCCESS;
}

int32_t DistributedInputClient::InitSink()
{
    return SUCCESS;
}

int32_t DistributedInputClient::ReleaseSource()
{
    return SUCCESS;
}

int32_t DistributedInputClient::ReleaseSink()
{
    return SUCCESS;
}

int32_t DistributedInputClient::RegisterDistributedHardware(const std::string& devId, const std::string& dhId,
    const std::string& parameters, const std::shared_ptr<RegisterCallback>& callback)
{
    return SUCCESS;
}

int32_t DistributedInputClient::UnregisterDistributedHardware(const std::string& devId, const std::string& dhId,
    const std::shared_ptr<UnregisterCallback>& callback)
{
    return SUCCESS;
}

int32_t DistributedInputClient::PrepareRemoteInput(
    const std::string& deviceId, sptr<IPrepareDInputCallback> callback)
{
    return SUCCESS;
}

int32_t DistributedInputClient::UnprepareRemoteInput(
    const std::string& deviceId, sptr<IUnprepareDInputCallback> callback)
{
    return SUCCESS;
}

int32_t DistributedInputClient::StartRemoteInput(
    const std::string& deviceId, const uint32_t& inputTypes, sptr<IStartDInputCallback> callback)
{
    return SUCCESS;
}

int32_t DistributedInputClient::StopRemoteInput(
    const std::string& deviceId, const uint32_t& inputTypes, sptr<IStopDInputCallback> callback)
{
    return SUCCESS;
}

bool DistributedInputClient::IsNeedFilterOut(const std::string& deviceId, const BusinessEvent& event)
{
    return true;
}

DInputServerType DistributedInputClient::IsStartDistributedInput(const uint32_t& inputType)
{
    return serverType;
}

bool DistributedInputClient::IsJsonData(std::string strData) const
{
    return true;
}
} // namespace DistributedInput
} // namespace DistributedHardware
} // namespace OHOS
