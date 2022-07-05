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

#include "distributed_input_sink_proxy.h"

#include "distributed_hardware_log.h"

#include "dinput_errcode.h"

namespace OHOS {
namespace DistributedHardware {
namespace DistributedInput {
DistributedInputSinkProxy::DistributedInputSinkProxy(const sptr<IRemoteObject> &object)
    : IRemoteProxy<IDistributedSinkInput>(object)
{}

DistributedInputSinkProxy::~DistributedInputSinkProxy()
{}

int32_t DistributedInputSinkProxy::Init()
{
    MessageParcel data;
    MessageParcel reply;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        DHLOGE("DistributedInputSinkProxy write token valid failed");
        return ERR_DH_INPUT_IPC_WRITE_TOKEN_VALID_FAIL;
    }
    int32_t result = ERR_DH_INPUT_SINK_PROXY_INIT_FAIL;
    bool ret = SendRequest(IDistributedSinkInput::MessageCode::INIT, data, reply);
    if (ret) {
        result = reply.ReadInt32();
    }
    return result;
}

int32_t DistributedInputSinkProxy::Release()
{
    MessageParcel data;
    MessageParcel reply;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        DHLOGE("DistributedInputSinkProxy write token valid failed");
        return ERR_DH_INPUT_IPC_WRITE_TOKEN_VALID_FAIL;
    }
    int32_t result = ERR_DH_INPUT_SINK_PROXY_RELEASE_FAIL;
    bool ret = SendRequest(IDistributedSinkInput::MessageCode::RELEASE, data, reply);
    if (ret) {
        result = reply.ReadInt32();
    }
    return result;
}

int32_t DistributedInputSinkProxy::IsStartDistributedInput(
    const uint32_t& inputType, sptr<IStartDInputServerCallback> callback)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        DHLOGE("DistributedInputSinkProxy write token valid failed");
        return ERR_DH_INPUT_IPC_WRITE_TOKEN_VALID_FAIL;
    }
    if (!data.WriteUint32(inputType)) {
        DHLOGE("DistributedInputSinkProxy write inputType failed");
        return ERR_DH_INPUT_IPC_WRITE_TOKEN_VALID_FAIL;
    }
    if (!data.WriteRemoteObject(callback->AsObject())) {
        DHLOGE("DistributedInputSinkProxy write callback failed");
        return ERR_DH_INPUT_IPC_WRITE_TOKEN_VALID_FAIL;
    }
    MessageParcel reply;
    int32_t result = ERR_DH_INPUT_SINK_PROXY_IS_START_INPUT_FAIL;
    bool ret = SendRequest(IDistributedSinkInput::MessageCode::ISSTART_REMOTE_INPUT, data, reply);
    if (ret) {
        result = reply.ReadInt32();
    }
    return result;
}

bool DistributedInputSinkProxy::SendRequest(
    IDistributedSinkInput::MessageCode code, MessageParcel &data, MessageParcel &reply)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        return false;
    }
    MessageOption option(MessageOption::TF_SYNC);
    int32_t result = remote->SendRequest(static_cast<uint32_t>(code), data, reply, option);
    if (result != DH_SUCCESS) {
        return false;
    }
    return true;
}
} // namespace DistributedInput
} // namespace DistributedHardware
} // namespace OHOS
