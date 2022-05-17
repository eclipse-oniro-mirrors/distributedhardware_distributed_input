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

#include "distributed_input_sink_handler.h"
#include "i_distributed_sink_input.h"
#include "load_d_input_sink_callback.h"
#include "distributed_hardware_log.h"

namespace OHOS {
namespace DistributedHardware {
namespace DistributedInput {
IMPLEMENT_SINGLE_INSTANCE(DistributedInputSinkHandler);

DistributedInputSinkHandler::~DistributedInputSinkHandler()
{
    DHLOGI("~DistributedInputSinkHandler");
}

int32_t DistributedInputSinkHandler::InitSink(const std::string &params)
{
    DHLOGD("InitSource");
    std::unique_lock<std::mutex> lock(proxyMutex_);
    if (!DistributedInputClient::GetInstance().HasDInputSinkProxy()) {
        sptr<ISystemAbilityManager> samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
        if (!samgr) {
            DHLOGE("Failed to get system ability mgr.");
            return FAILURE_DIS;
        }
        sptr<LoadDInputSinkCallback> loadCallback = new LoadDInputSinkCallback(params);
        int32_t ret = samgr->LoadSystemAbility(DISTRIBUTED_HARDWARE_INPUT_SINK_SA_ID, loadCallback);
        if (ret != ERR_OK) {
            DHLOGE("Failed to Load systemAbility, systemAbilityId:%d, ret code:%d",
                   DISTRIBUTED_HARDWARE_INPUT_SINK_SA_ID, ret);
            return FAILURE_DIS;
        }
    }

    auto waitStatus = proxyConVar_.wait_for(lock, std::chrono::milliseconds(INPUT_LOADSA_TIMEOUT_MS),
                    [this]() { return (DistributedInputClient::GetInstance().HasDInputSinkProxy()); });
    if (!waitStatus) {
        DHLOGE("dinput load sa timeout.");
        return FAILURE_DIS;
    }

    return SUCCESS;
}

void DistributedInputSinkHandler::FinishStartSA(const std::string &params, const sptr<IRemoteObject> &remoteObject)
{
    DHLOGD("FinishStartSA");
    std::unique_lock<std::mutex> lock(proxyMutex_);
    DistributedInputClient::GetInstance().SetDInputSinkProxy(remoteObject);
    DistributedInputClient::GetInstance().InitSink();
    proxyConVar_.notify_all();
}

int32_t DistributedInputSinkHandler::ReleaseSink()
{
    return DistributedInputClient::GetInstance().ReleaseSink();
}

int32_t DistributedInputSinkHandler::SubscribeLocalHardware(const std::string &dhId, const std::string &params)
{
    return SUCCESS;
}

int32_t DistributedInputSinkHandler::UnsubscribeLocalHardware(const std::string &dhId)
{
    return SUCCESS;
}

void DistributedInputSinkHandler::SALoadSinkCb::OnLoadSystemAbilitySuccess(int32_t systemAbilityId,
    const OHOS::sptr<IRemoteObject>& remoteObject)
{
    currSystemAbilityId = systemAbilityId;
    currRemoteObject = remoteObject;
    DHLOGI("DistributedInputSinkHandler OnLoadSystemAbilitySuccess");
}

void DistributedInputSinkHandler::SALoadSinkCb::OnLoadSystemAbilityFail(int32_t systemAbilityId)
{
    currSystemAbilityId = systemAbilityId;
    DHLOGE("DistributedInputSinkHandler OnLoadSystemAbilityFail");
}

IDistributedHardwareSink *GetSinkHardwareHandler()
{
    return &DistributedInputSinkHandler::GetInstance();
}
} // namespace DistributedInput
} // namespace DistributedHardware
} // namespace OHOS
