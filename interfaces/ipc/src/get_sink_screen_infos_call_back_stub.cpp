/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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

#include "get_sink_screen_infos_call_back_stub.h"

#include "string_ex.h"

#include "constants_dinput.h"
#include "dinput_errcode.h"
#include "dinput_ipc_interface_code.h"
#include "dinput_log.h"

namespace OHOS {
namespace DistributedHardware {
namespace DistributedInput {
GetSinkScreenInfosCallbackStub::GetSinkScreenInfosCallbackStub()
{
}

GetSinkScreenInfosCallbackStub::~GetSinkScreenInfosCallbackStub()
{
}

int32_t GetSinkScreenInfosCallbackStub::OnRemoteRequest(
    uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    if (data.ReadInterfaceToken() != GetDescriptor()) {
        DHLOGE("GetSinkScreenInfosCallbackStub read token valid failed");
        return ERR_DH_INPUT_IPC_READ_TOKEN_VALID_FAIL;
    }
    IGetSinkScreenInfosCBInterfaceCode msgCode = static_cast<IGetSinkScreenInfosCBInterfaceCode>(code);
    switch (msgCode) {
        case IGetSinkScreenInfosCBInterfaceCode::RESULT: {
            std::string strJson = data.ReadString();
            OnResult(strJson);
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
