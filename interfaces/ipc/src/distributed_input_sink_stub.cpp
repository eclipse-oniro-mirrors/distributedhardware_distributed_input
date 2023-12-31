/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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

#include "distributed_input_sink_stub.h"

#include "anonymous_string.h"

#include "constants_dinput.h"
#include "dinput_errcode.h"
#include "dinput_log.h"
#include "i_sharing_dhid_listener.h"

namespace OHOS {
namespace DistributedHardware {
namespace DistributedInput {
DistributedInputSinkStub::DistributedInputSinkStub()
{
    DHLOGI("DistributedInputSinkStub ctor!");
    memberFuncMap_[INIT] = &DistributedInputSinkStub::InitInner;
    memberFuncMap_[RELEASE] = &DistributedInputSinkStub::ReleaseInner;
    memberFuncMap_[NOTIFY_START_DSCREEN] = &DistributedInputSinkStub::NotifyStartDScreenInner;
    memberFuncMap_[NOTIFY_STOP_DSCREEN] = &DistributedInputSinkStub::NotifyStopDScreenInner;
    memberFuncMap_[REGISTER_SHARING_DHID_LISTENER] = &DistributedInputSinkStub::RegisterSharingDhIdListenerInner;
    memberFuncMap_[GET_SINK_SCREEN_INFOS] = &DistributedInputSinkStub::RegisterGetSinkScreenInfosInner;
}

DistributedInputSinkStub::~DistributedInputSinkStub()
{
    DHLOGI("DistributedInputSinkStub dtor!");
    memberFuncMap_.clear();
}

int32_t DistributedInputSinkStub::OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply,
    MessageOption &option)
{
    if (data.ReadInterfaceToken() != GetDescriptor()) {
        DHLOGE("DistributedInputSinkStub read token valid failed");
        return ERR_DH_INPUT_IPC_READ_TOKEN_VALID_FAIL;
    }
    auto iter = memberFuncMap_.find(code);
    if (iter == memberFuncMap_.end()) {
        DHLOGE("invalid request code is %d.", code);
        return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
    }
    DistributedInputSinkFunc &func = iter->second;
    return (this->*func)(data, reply, option);
}

int32_t DistributedInputSinkStub::InitInner(MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    DHLOGI("DistributedInputSinkStub InitInner start");
    int32_t ret = Init();
    if (!reply.WriteInt32(ret)) {
        DHLOGE("DistributedInputSinkStub write ret failed, ret = %d", ret);
        return ERR_DH_INPUT_IPC_WRITE_TOKEN_VALID_FAIL;
    }
    return ret;
}

int32_t DistributedInputSinkStub::ReleaseInner(MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    int32_t ret = Release();
    if (!reply.WriteInt32(ret)) {
        DHLOGE("DistributedInputSinkStub write ret failed, ret = %d", ret);
        return ERR_DH_INPUT_IPC_WRITE_TOKEN_VALID_FAIL;
    }
    return ret;
}

int32_t DistributedInputSinkStub::NotifyStartDScreenInner(MessageParcel &data, MessageParcel &reply,
    MessageOption &option)
{
    std::string devId = data.ReadString();
    int32_t sessionId = data.ReadInt32();
    std::string uuid = data.ReadString();
    uint64_t sourceWinId = data.ReadUint64();
    uint32_t sourceWinWidth = data.ReadUint32();
    uint32_t sourceWinHeight = data.ReadUint32();
    std::string sourcePhyId = data.ReadString();
    uint32_t sourcePhyFd = data.ReadUint32();
    uint32_t sourcePhyWidth = data.ReadUint32();
    uint32_t sourcePhyHeight = data.ReadUint32();
    DHLOGI("OnRemoteRequest the data: devId: %s, sourceWinId: %d, sourceWinWidth: %d, sourceWinHeight: %d,"
        "sourcePhyId: %s, sourcePhyFd: %d, sourcePhyWidth: %d, sourcePhyHeight: %d", GetAnonyString(devId).c_str(),
        sourceWinId, sourceWinWidth, sourceWinHeight, GetAnonyString(sourcePhyId).c_str(), sourcePhyFd, sourcePhyWidth,
        sourcePhyHeight);
    SrcScreenInfo srcScreenInfo = {
        .devId = devId,
        .sessionId = sessionId,
        .uuid = uuid,
        .sourceWinId = sourceWinId,
        .sourceWinWidth = sourceWinWidth,
        .sourceWinHeight = sourceWinHeight,
        .sourcePhyId = sourcePhyId,
        .sourcePhyFd = sourcePhyFd,
        .sourcePhyWidth = sourcePhyWidth,
        .sourcePhyHeight = sourcePhyHeight,
    };
    int32_t ret = NotifyStartDScreen(srcScreenInfo);
    if (!reply.WriteInt32(ret)) {
        DHLOGE("write reply failed ret = %d", ret);
        return ERR_DH_INPUT_RPC_REPLY_FAIL;
    }
    return ret;
}

int32_t DistributedInputSinkStub::NotifyStopDScreenInner(MessageParcel &data, MessageParcel &reply,
    MessageOption &option)
{
    std::string srcScreenInfoKey = data.ReadString();
    DHLOGI("OnRemoteRequest srcScreenInfoKey: %s", GetAnonyString(srcScreenInfoKey).c_str());
    int ret = NotifyStopDScreen(srcScreenInfoKey);
    if (!reply.WriteInt32(ret)) {
        DHLOGE("write version failed, ret = %d", ret);
        return ERR_DH_INPUT_RPC_REPLY_FAIL;
    }
    return ret;
}

int32_t DistributedInputSinkStub::RegisterSharingDhIdListenerInner(MessageParcel &data, MessageParcel &reply,
    MessageOption &option)
{
    sptr<ISharingDhIdListener> listener = iface_cast<ISharingDhIdListener>(data.ReadRemoteObject());
    int32_t ret = RegisterSharingDhIdListener(listener);
    if (!reply.WriteInt32(ret)) {
        DHLOGE("RegisterSharingDhIdListenerInner write ret failed, ret = %d", ret);
        return ERR_DH_INPUT_SINK_STUB_REGISTER_SHARING_DHID_LISTENER_FAIL;
    }

    return DH_SUCCESS;
}

int32_t DistributedInputSinkStub::RegisterGetSinkScreenInfosInner(MessageParcel &data, MessageParcel &reply,
    MessageOption &option)
{
    sptr<IGetSinkScreenInfosCallback> callback =
        iface_cast<IGetSinkScreenInfosCallback>(data.ReadRemoteObject());
    int32_t ret = RegisterGetSinkScreenInfosCallback(callback);
    if (!reply.WriteInt32(ret)) {
        DHLOGE("write ret failed, ret = %d", ret);
        return ERR_DH_INPUT_IPC_WRITE_TOKEN_VALID_FAIL;
    }
    return ret;
}
} // namespace DistributedInput
} // namespace DistributedHardware
} // namespace OHOS
