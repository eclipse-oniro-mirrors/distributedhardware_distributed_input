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

#include "del_white_list_infos_call_back_proxy.h"
#include "ipc_types.h"
#include "parcel.h"

namespace OHOS {
namespace DistributedHardware {
namespace DistributedInput {
DelWhiteListInfosCallbackProxy::DelWhiteListInfosCallbackProxy(const sptr<IRemoteObject> &object)
    : IRemoteProxy<IDelWhiteListInfosCallback>(object)
{
}

DelWhiteListInfosCallbackProxy::~DelWhiteListInfosCallbackProxy()
{
}

void DelWhiteListInfosCallbackProxy::OnResult(const std::string& deviceId)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        return;
    }

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    data.WriteInterfaceToken(IDelWhiteListInfosCallback::GetDescriptor());
    if (!data.WriteString(deviceId)) {
        return;
    }
    remote->SendRequest(static_cast<int32_t>(IDelWhiteListInfosCallback::Message::RESULT), data, reply, option);
}
}  // namespace DistributedHardware
}  // namespace DistributedInput
}  // namespace OHOS