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

#ifndef DISTRIBUTED_INPUT_SOURCE_PROXY_H
#define DISTRIBUTED_INPUT_SOURCE_PROXY_H

#include <iostream>
#include "i_distributed_source_input.h"
#include "iremote_proxy.h"

namespace OHOS {
namespace DistributedHardware {
namespace DistributedInput {
class DistributedInputSourceProxy : public IRemoteProxy<IDistributedSourceInput> {
public:

    explicit DistributedInputSourceProxy(const sptr<IRemoteObject> &object);

    virtual ~DistributedInputSourceProxy() override;

    virtual int32_t Init() override;

    virtual int32_t Release() override;

    virtual int32_t RegisterDistributedHardware(const std::string& devId, const std::string& dhId,
        const std::string& parameters, sptr<IRegisterDInputCallback> callback) override;

    virtual int32_t UnregisterDistributedHardware(const std::string& devId, const std::string& dhId,
        sptr<IUnregisterDInputCallback> callback) override;

    virtual int32_t PrepareRemoteInput(const std::string& deviceId,
        sptr<IPrepareDInputCallback> callback, sptr<IAddWhiteListInfosCallback> addWhiteListCallback) override;

    virtual int32_t UnprepareRemoteInput(const std::string& deviceId,
        sptr<IUnprepareDInputCallback> callback, sptr<IDelWhiteListInfosCallback> delWhiteListCallback) override;

    virtual int32_t StartRemoteInput(
        const std::string& deviceId, const uint32_t& inputTypes, sptr<IStartDInputCallback> callback) override;

    virtual int32_t StopRemoteInput(
        const std::string& deviceId, const uint32_t& inputTypes, sptr<IStopDInputCallback> callback) override;

    virtual int32_t IsStartDistributedInput(
        const uint32_t& inputType, sptr<IStartDInputServerCallback> callback) override;

private:
    bool SendRequest(const IDistributedSourceInput::MessageCode code, MessageParcel &data, MessageParcel &reply);

    static inline BrokerDelegator<DistributedInputSourceProxy> g_delegator;
};
} // namespace DistributedInput
} // namespace DistributedHardware
} // namespace OHOS

#endif // DISTRIBUTED_INPUT_PROXY_H
