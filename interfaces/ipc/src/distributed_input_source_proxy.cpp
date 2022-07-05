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

#include "distributed_input_source_proxy.h"

#include "distributed_hardware_log.h"

#include "dinput_errcode.h"

namespace OHOS {
namespace DistributedHardware {
namespace DistributedInput {
DistributedInputSourceProxy::DistributedInputSourceProxy(const sptr<IRemoteObject> &object)
    : IRemoteProxy<IDistributedSourceInput>(object)
{}

DistributedInputSourceProxy::~DistributedInputSourceProxy()
{}

int32_t DistributedInputSourceProxy::Init()
{
    MessageParcel data;
    MessageParcel reply;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        DHLOGE("DistributedInputSourceProxy write token valid failed");
        return ERR_DH_INPUT_IPC_WRITE_TOKEN_VALID_FAIL;
    }
    int32_t result = ERR_DH_INPUT_SOURCE_PROXY_INIT_FAIL;
    bool ret = SendRequest(IDistributedSourceInput::MessageCode::INIT, data, reply);
    if (ret) {
        result = reply.ReadInt32();
    }
    return result;
}

int32_t DistributedInputSourceProxy::Release()
{
    MessageParcel data;
    MessageParcel reply;
    int32_t result = ERR_DH_INPUT_SOURCE_PROXY_RELEASE_FAIL;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        DHLOGE("DistributedInputSourceProxy write token valid failed");
        return ERR_DH_INPUT_IPC_WRITE_TOKEN_VALID_FAIL;
    }
    bool ret = SendRequest(IDistributedSourceInput::MessageCode::RELEASE, data, reply);
    if (ret) {
        result = reply.ReadInt32();
    }
    return result;
}

int32_t DistributedInputSourceProxy::RegisterDistributedHardware(const std::string& devId, const std::string& dhId,
    const std::string& parameters, sptr<IRegisterDInputCallback> callback)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        DHLOGE("DistributedInputSourceProxy write token valid failed");
        return ERR_DH_INPUT_IPC_WRITE_TOKEN_VALID_FAIL;
    }
    if (!data.WriteString(devId)) {
        DHLOGE("DistributedInputSourceProxy write devId failed");
        return ERR_DH_INPUT_IPC_WRITE_TOKEN_VALID_FAIL;
    }
    if (!data.WriteString(dhId)) {
        DHLOGE("DistributedInputSourceProxy write dhId failed");
        return ERR_DH_INPUT_IPC_WRITE_TOKEN_VALID_FAIL;
    }
    if (!data.WriteString(parameters)) {
        DHLOGE("DistributedInputSourceProxy write parameters failed");
        return ERR_DH_INPUT_IPC_WRITE_TOKEN_VALID_FAIL;
    }
    if (!data.WriteRemoteObject(callback->AsObject())) {
        DHLOGE("DistributedInputSourceProxy write callback failed");
        return ERR_DH_INPUT_IPC_WRITE_TOKEN_VALID_FAIL;
    }
    MessageParcel reply;
    int32_t result = ERR_DH_INPUT_SOURCE_PROXY_REGISTER_FAIL;
    bool ret = SendRequest(IDistributedSourceInput::MessageCode::REGISTER_REMOTE_INPUT, data, reply);
    if (ret) {
        result = reply.ReadInt32();
    }
    return result;
}

int32_t DistributedInputSourceProxy::UnregisterDistributedHardware(const std::string& devId, const std::string& dhId,
    sptr<IUnregisterDInputCallback> callback)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        DHLOGE("DistributedInputSourceProxy write token valid failed");
        return ERR_DH_INPUT_IPC_WRITE_TOKEN_VALID_FAIL;
    }
    if (!data.WriteString(devId)) {
        DHLOGE("DistributedInputSourceProxy write devId failed");
        return ERR_DH_INPUT_IPC_WRITE_TOKEN_VALID_FAIL;
    }
    if (!data.WriteString(dhId)) {
        DHLOGE("DistributedInputSourceProxy write dhId failed");
        return ERR_DH_INPUT_IPC_WRITE_TOKEN_VALID_FAIL;
    }
    if (!data.WriteRemoteObject(callback->AsObject())) {
        DHLOGE("DistributedInputSourceProxy write callback failed");
        return ERR_DH_INPUT_IPC_WRITE_TOKEN_VALID_FAIL;
    }
    MessageParcel reply;
    int32_t result = ERR_DH_INPUT_SOURCE_PROXY_UNREGISTER_FAIL;
    bool ret = SendRequest(IDistributedSourceInput::MessageCode::UNREGISTER_REMOTE_INPUT, data, reply);
    if (ret) {
        result = reply.ReadInt32();
    }
    return result;
}

int32_t DistributedInputSourceProxy::PrepareRemoteInput(
    const std::string& deviceId, sptr<IPrepareDInputCallback> callback,
    sptr<IAddWhiteListInfosCallback> addWhiteListCallback)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        DHLOGE("DistributedInputSourceProxy write token valid failed");
        return ERR_DH_INPUT_IPC_WRITE_TOKEN_VALID_FAIL;
    }
    if (!data.WriteString(deviceId)) {
        DHLOGE("DistributedInputSourceProxy write deviceId failed");
        return ERR_DH_INPUT_IPC_WRITE_TOKEN_VALID_FAIL;
    }
    if (!data.WriteRemoteObject(callback->AsObject())) {
        DHLOGE("DistributedInputSourceProxy write callback failed");
        return ERR_DH_INPUT_IPC_WRITE_TOKEN_VALID_FAIL;
    }
    if (!data.WriteRemoteObject(addWhiteListCallback->AsObject())) {
        DHLOGE("DistributedInputSourceProxy write addWhiteListCallback failed");
        return ERR_DH_INPUT_IPC_WRITE_TOKEN_VALID_FAIL;
    }
    MessageParcel reply;
    int32_t result = ERR_DH_INPUT_SOURCE_PROXY_PREPARE_FAIL;
    bool ret = SendRequest(IDistributedSourceInput::MessageCode::PREPARE_REMOTE_INPUT, data, reply);
    if (ret) {
        result = reply.ReadInt32();
    }
    return result;
}

int32_t DistributedInputSourceProxy::UnprepareRemoteInput(
    const std::string& deviceId, sptr<IUnprepareDInputCallback> callback,
    sptr<IDelWhiteListInfosCallback> delWhiteListCallback)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        DHLOGE("DistributedInputSourceProxy write token valid failed");
        return ERR_DH_INPUT_IPC_WRITE_TOKEN_VALID_FAIL;
    }
    if (!data.WriteString(deviceId)) {
        DHLOGE("DistributedInputSourceProxy write deviceId failed");
        return ERR_DH_INPUT_IPC_WRITE_TOKEN_VALID_FAIL;
    }
    if (!data.WriteRemoteObject(callback->AsObject())) {
        DHLOGE("DistributedInputSourceProxy write callback failed");
        return ERR_DH_INPUT_IPC_WRITE_TOKEN_VALID_FAIL;
    }
    if (!data.WriteRemoteObject(delWhiteListCallback->AsObject())) {
        DHLOGE("DistributedInputSourceProxy write delWhiteListCallback failed");
        return ERR_DH_INPUT_IPC_WRITE_TOKEN_VALID_FAIL;
    }
    MessageParcel reply;
    int32_t result = ERR_DH_INPUT_SOURCE_PROXY_UNPREPARE_FAIL;
    bool ret = SendRequest(IDistributedSourceInput::MessageCode::UNPREPARE_REMOTE_INPUT, data, reply);
    if (ret) {
        result = reply.ReadInt32();
    }
    return result;
}

int32_t DistributedInputSourceProxy::StartRemoteInput(
    const std::string& deviceId, const uint32_t& inputTypes, sptr<IStartDInputCallback> callback)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        DHLOGE("DistributedInputSourceProxy write token valid failed");
        return ERR_DH_INPUT_IPC_WRITE_TOKEN_VALID_FAIL;
    }
    if (!data.WriteString(deviceId)) {
        DHLOGE("DistributedInputSourceProxy write deviceId failed");
        return ERR_DH_INPUT_IPC_WRITE_TOKEN_VALID_FAIL;
    }
    if (!data.WriteUint32(inputTypes)) {
        DHLOGE("DistributedInputSourceProxy write inputTypes failed");
        return ERR_DH_INPUT_IPC_WRITE_TOKEN_VALID_FAIL;
    }
    if (!data.WriteRemoteObject(callback->AsObject())) {
        DHLOGE("DistributedInputSourceProxy write callback failed");
        return ERR_DH_INPUT_IPC_WRITE_TOKEN_VALID_FAIL;
    }
    MessageParcel reply;
    int32_t result = ERR_DH_INPUT_SOURCE_PROXY_START_FAIL;
    bool ret = SendRequest(IDistributedSourceInput::MessageCode::START_REMOTE_INPUT, data, reply);
    if (ret) {
        result = reply.ReadInt32();
    }
    return result;
}

int32_t DistributedInputSourceProxy::StopRemoteInput(
    const std::string& deviceId, const uint32_t& inputTypes, sptr<IStopDInputCallback> callback)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        DHLOGE("DistributedInputSourceProxy write token valid failed");
        return ERR_DH_INPUT_IPC_WRITE_TOKEN_VALID_FAIL;
    }
    if (!data.WriteString(deviceId)) {
        DHLOGE("DistributedInputSourceProxy write deviceId failed");
        return ERR_DH_INPUT_IPC_WRITE_TOKEN_VALID_FAIL;
    }
    if (!data.WriteUint32(inputTypes)) {
        DHLOGE("DistributedInputSourceProxy write inputTypes failed");
        return ERR_DH_INPUT_IPC_WRITE_TOKEN_VALID_FAIL;
    }
    if (!data.WriteRemoteObject(callback->AsObject())) {
        DHLOGE("DistributedInputSourceProxy write callback failed");
        return ERR_DH_INPUT_IPC_WRITE_TOKEN_VALID_FAIL;
    }
    MessageParcel reply;
    int32_t result = ERR_DH_INPUT_SOURCE_PROXY_STOP_FAIL;
    bool ret = SendRequest(IDistributedSourceInput::MessageCode::STOP_REMOTE_INPUT, data, reply);
    if (ret) {
        result = reply.ReadInt32();
    }
    return result;
}

int32_t DistributedInputSourceProxy::IsStartDistributedInput(
    const uint32_t& inputType, sptr<IStartDInputServerCallback> callback)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        DHLOGE("DistributedInputSourceProxy write token valid failed");
        return ERR_DH_INPUT_IPC_WRITE_TOKEN_VALID_FAIL;
    }
    if (!data.WriteUint32(inputType)) {
        DHLOGE("DistributedInputSourceProxy write inputType failed");
        return ERR_DH_INPUT_IPC_WRITE_TOKEN_VALID_FAIL;
    }
    if (!data.WriteRemoteObject(callback->AsObject())) {
        DHLOGE("DistributedInputSourceProxy write callback failed");
        return ERR_DH_INPUT_IPC_WRITE_TOKEN_VALID_FAIL;
    }
    MessageParcel reply;
    int32_t result = ERR_DH_INPUT_SOURCE_PROXY_IS_START_INPUT_FAIL;
    bool ret = SendRequest(IDistributedSourceInput::MessageCode::ISSTART_REMOTE_INPUT, data, reply);
    if (ret) {
        result = reply.ReadInt32();
    }
    return result;
}

bool DistributedInputSourceProxy::SendRequest(
    const IDistributedSourceInput::MessageCode code, MessageParcel &data, MessageParcel &reply)
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
