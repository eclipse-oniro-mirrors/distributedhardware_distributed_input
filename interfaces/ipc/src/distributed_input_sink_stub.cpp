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

#include "distributed_input_sink_stub.h"

#include "constants_dinput.h"
#include "dinput_errcode.h"

namespace OHOS {
namespace DistributedHardware {
namespace DistributedInput {
DistributedInputSinkStub::DistributedInputSinkStub()
{}

DistributedInputSinkStub::~DistributedInputSinkStub()
{}

int32_t DistributedInputSinkStub::OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply,
    MessageOption &option)
{
    switch (code) {
        case static_cast<uint32_t>(IDistributedSinkInput::MessageCode::INIT): {
            int32_t ret = Init();
            if (!reply.WriteInt32(ret)) {
                return ERR_DH_INPUT_SINK_STUB_ON_REMOTE_REQUEST_FAIL;
            }
            break;
        }

        case static_cast<uint32_t>(IDistributedSinkInput::MessageCode::RELEASE): {
            int32_t ret = Release();
            if (!reply.WriteInt32(ret)) {
                return ERR_DH_INPUT_SINK_STUB_ON_REMOTE_REQUEST_FAIL;
            }
            break;
        }

        case static_cast<uint32_t>(IDistributedSinkInput::MessageCode::ISSTART_REMOTE_INPUT): {
            uint32_t inputType = data.ReadUint32();
            sptr<IStartDInputServerCallback> callback =
                iface_cast<IStartDInputServerCallback>(data.ReadRemoteObject());
            int32_t ret = IsStartDistributedInput(inputType, callback);
            if (!reply.WriteInt32(ret)) {
                return ERR_DH_INPUT_SINK_STUB_ON_REMOTE_REQUEST_FAIL;
            }
            break;
        }
        default:
            return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
    }
    return DH_SUCCESS;
}
} // namespace DistributedInput
} // namespace DistributedHardware
} // namespace OHOS
