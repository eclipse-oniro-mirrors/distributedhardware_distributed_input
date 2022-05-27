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

#ifndef DISTRIBUTED_INPUT_SINK_PROXY_H
#define DISTRIBUTED_INPUT_SINK_PROXY_H

#include "i_distributed_sink_input.h"

#include <iostream>

#include "iremote_proxy.h"

namespace OHOS {
namespace DistributedHardware {
namespace DistributedInput {
class DistributedInputSinkProxy : public IRemoteProxy<IDistributedSinkInput> {
public:

    explicit DistributedInputSinkProxy(const sptr<IRemoteObject> &object);

    virtual ~DistributedInputSinkProxy() override;

    virtual int32_t Init() override;

    virtual int32_t Release() override;

    virtual int32_t IsStartDistributedInput(
        const uint32_t& inputType, sptr<IStartDInputServerCallback> callback) override;

private:
    bool SendRequest(IDistributedSinkInput::MessageCode code, MessageParcel &data, MessageParcel &reply);

    static inline BrokerDelegator<DistributedInputSinkProxy> g_delegator;
};
} // namespace DistributedInput
} // namespace DistributedHardware
} // namespace OHOS

#endif // DISTRIBUTED_INPUT_PROXY_H
