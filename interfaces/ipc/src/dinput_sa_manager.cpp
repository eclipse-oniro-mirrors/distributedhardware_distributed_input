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

#include "dinput_sa_manager.h"

#include "distributed_hardware_log.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"

#include "constants_dinput.h"
#include "dinput_errcode.h"

namespace OHOS {
namespace DistributedHardware {
namespace DistributedInput {
IMPLEMENT_SINGLE_INSTANCE(DinputSAManager);

void DinputSAManager::SystemAbilityListener::OnRemoveSystemAbility(int32_t systemAbilityId,
    const std::string& deviceId)
{
    if (systemAbilityId == DISTRIBUTED_HARDWARE_INPUT_SOURCE_SA_ID) {
        DinputSAManager::GetInstance().dInputSourceSAOnline.store(false);
        std::lock_guard<std::mutex> lock(DinputSAManager::GetInstance().sourceMutex_);
        DinputSAManager::GetInstance().dInputSourceProxy_ = nullptr;
    } else if (systemAbilityId == DISTRIBUTED_HARDWARE_INPUT_SINK_SA_ID) {
        DinputSAManager::GetInstance().dInputSinkSAOnline.store(false);
        std::lock_guard<std::mutex> lock(DinputSAManager::GetInstance().sinkMutex_);
        DinputSAManager::GetInstance().dInputSinkProxy_ = nullptr;
    }
    DHLOGI("sa %d is removed.", systemAbilityId);
}

void DinputSAManager::SystemAbilityListener::OnAddSystemAbility(int32_t systemAbilityId, const std::string& deviceId)
{
    if (systemAbilityId == DISTRIBUTED_HARDWARE_INPUT_SOURCE_SA_ID) {
        DinputSAManager::GetInstance().dInputSourceSAOnline.store(true);
    } else if (systemAbilityId == DISTRIBUTED_HARDWARE_INPUT_SINK_SA_ID) {
        DinputSAManager::GetInstance().dInputSinkSAOnline.store(true);
    }
    DHLOGI("sa %d is added.", systemAbilityId);
}

void DinputSAManager::Init()
{
    saListenerCallback = new(std::nothrow) SystemAbilityListener();
    sptr<ISystemAbilityManager> systemAbilityManager =
        SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();

    if (!systemAbilityManager) {
        DHLOGE("get system ability manager failed.");
        return;
    }

    if (!isSubscribeSrcSAChangeListener.load()) {
        DHLOGI("try subscribe source sa change listener, sa id: %d", DISTRIBUTED_HARDWARE_INPUT_SOURCE_SA_ID);
        int32_t ret = systemAbilityManager->SubscribeSystemAbility(DISTRIBUTED_HARDWARE_INPUT_SOURCE_SA_ID,
            saListenerCallback);
        if (ret != DH_SUCCESS) {
            DHLOGE("subscribe source sa change failed: %d", ret);
            return;
        }
        isSubscribeSrcSAChangeListener.store(true);
    }

    if (!isSubscribeSinkSAChangeListener.load()) {
        DHLOGI("try subscribe sink sa change listener, sa id: %d", DISTRIBUTED_HARDWARE_INPUT_SINK_SA_ID);
        int32_t ret = systemAbilityManager->SubscribeSystemAbility(DISTRIBUTED_HARDWARE_INPUT_SINK_SA_ID,
            saListenerCallback);
        if (ret != DH_SUCCESS) {
            DHLOGE("subscribe sink sa change failed: %d", ret);
            return;
        }
        isSubscribeSinkSAChangeListener.store(true);
    }
}

bool DinputSAManager::GetDInputSourceProxy()
{
    if (!isSubscribeSrcSAChangeListener.load()) {
        std::lock_guard<std::mutex> lock(DinputSAManager::GetInstance().sourceMutex_);
        if (!isSubscribeSrcSAChangeListener.load()) {
            sptr<ISystemAbilityManager> systemAbilityManager =
                SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
            if (!systemAbilityManager) {
                DHLOGE("get system ability manager failed.");
                return false;
            }

            DHLOGI("try subscribe source sa change listener, sa id: %d", DISTRIBUTED_HARDWARE_INPUT_SOURCE_SA_ID);
            int32_t ret = systemAbilityManager->SubscribeSystemAbility(DISTRIBUTED_HARDWARE_INPUT_SOURCE_SA_ID,
                saListenerCallback);
            if (ret != DH_SUCCESS) {
                DHLOGE("subscribe source sa change failed: %d", ret);
                return false;
            }
            isSubscribeSrcSAChangeListener.store(true);
        }
    }

    if (dInputSourceSAOnline.load() && !dInputSourceProxy_) {
        std::lock_guard<std::mutex> lock(DinputSAManager::GetInstance().sourceMutex_);
        if (dInputSourceProxy_ != nullptr) {
            DHLOGI("dinput source proxy has already got.");
            return true;
        }
        sptr<ISystemAbilityManager> systemAbilityManager =
            SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();

        if (!systemAbilityManager) {
            DHLOGE("get system ability manager failed.");
            return false;
        }
        DHLOGI("%s try get sa: %d", __func__, DISTRIBUTED_HARDWARE_INPUT_SOURCE_SA_ID);
        sptr<IRemoteObject> remoteObject = systemAbilityManager->GetSystemAbility(
            DISTRIBUTED_HARDWARE_INPUT_SOURCE_SA_ID);
        if (!remoteObject) {
            return false;
        }

        dInputSourceProxy_ = iface_cast<IDistributedSourceInput>(remoteObject);

        if ((!dInputSourceProxy_) || (!dInputSourceProxy_->AsObject())) {
            return false;
        }
    }
    std::lock_guard<std::mutex> lock(DinputSAManager::GetInstance().sourceMutex_);
    return dInputSourceProxy_ != nullptr;
}

bool DinputSAManager::HasDInputSourceProxy()
{
    std::lock_guard<std::mutex> lock(DinputSAManager::GetInstance().sourceMutex_);
    return dInputSourceProxy_ != nullptr;
}

bool DinputSAManager::SetDInputSourceProxy(const sptr<IRemoteObject> &remoteObject)
{
    std::lock_guard<std::mutex> lock(DinputSAManager::GetInstance().sourceMutex_);
    dInputSourceProxy_ = iface_cast<IDistributedSourceInput>(remoteObject);

    if ((!dInputSourceProxy_) || (!dInputSourceProxy_->AsObject())) {
        DHLOGE("Failed to get dinput source proxy.");
        return false;
    }
    return true;
}

bool DinputSAManager::GetDInputSinkProxy()
{
    if (!isSubscribeSinkSAChangeListener.load()) {
        std::lock_guard<std::mutex> lock(DinputSAManager::GetInstance().sinkMutex_);
        if (!isSubscribeSinkSAChangeListener.load()) {
            sptr<ISystemAbilityManager> systemAbilityManager =
                SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
            if (!systemAbilityManager) {
                DHLOGE("get system ability manager failed.");
                return false;
            }

            DHLOGI("try subscribe sink sa change listener, sa id: %d", DISTRIBUTED_HARDWARE_INPUT_SINK_SA_ID);
            int32_t ret = systemAbilityManager->SubscribeSystemAbility(DISTRIBUTED_HARDWARE_INPUT_SINK_SA_ID,
                saListenerCallback);
            if (ret != DH_SUCCESS) {
                DHLOGE("subscribe sink sa change failed: %d", ret);
                return false;
            }
            isSubscribeSinkSAChangeListener.store(true);
        }
    }

    if (dInputSinkSAOnline.load() && !dInputSinkProxy_) {
        std::lock_guard<std::mutex> lock(DinputSAManager::GetInstance().sinkMutex_);
        if (dInputSinkProxy_ != nullptr) {
            DHLOGI("dinput sink proxy has already got.");
            return true;
        }
        sptr<ISystemAbilityManager> systemAbilityManager =
            SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
        if (!systemAbilityManager) {
            DHLOGE("get system ability manager failed.");
            return false;
        }

        sptr<IRemoteObject> remoteObject = systemAbilityManager->GetSystemAbility(
            DISTRIBUTED_HARDWARE_INPUT_SINK_SA_ID);
        if (!remoteObject) {
            return false;
        }

        dInputSinkProxy_ = iface_cast<IDistributedSinkInput>(remoteObject);

        if ((!dInputSinkProxy_) || (!dInputSinkProxy_->AsObject())) {
            return false;
        }
    }
    std::lock_guard<std::mutex> lock(DinputSAManager::GetInstance().sinkMutex_);
    return dInputSinkProxy_ != nullptr;
}

bool DinputSAManager::HasDInputSinkProxy()
{
    std::lock_guard<std::mutex> lock(DinputSAManager::GetInstance().sinkMutex_);
    return dInputSinkProxy_ != nullptr;
}

bool DinputSAManager::SetDInputSinkProxy(const sptr<IRemoteObject> &remoteObject)
{
    std::lock_guard<std::mutex> lock(DinputSAManager::GetInstance().sinkMutex_);
    dInputSinkProxy_ = iface_cast<IDistributedSinkInput>(remoteObject);

    if ((!dInputSinkProxy_) || (!dInputSinkProxy_->AsObject())) {
        DHLOGE("Failed to get dinput sink proxy.");
        return false;
    }
    return true;
}
} // namespace DistributedInput
} // namespace DistributedHardware
} // namespace OHOS