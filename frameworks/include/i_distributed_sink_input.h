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
 * See the License for the specific language governing DistributedInputs and
 * limitations under the License.
 */

#ifndef I_DISTRIBUTED_SINK_INPUT_H
#define I_DISTRIBUTED_SINK_INPUT_H

#include <string>

#include "iremote_broker.h"
#include "iremote_object.h"

#include "i_start_d_input_server_call_back.h"
#include "constants_dinput.h"

namespace OHOS {
namespace DistributedHardware {
namespace DistributedInput {
class IDistributedSinkInput : public IRemoteBroker {
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"ohos.DistributedHardware.DistributedInput.IDistributedSinkInput");

    virtual int32_t Init() = 0;

    virtual int32_t Release() = 0;

    virtual int32_t IsStartDistributedInput(
        const uint32_t& inputType, sptr<IStartDInputServerCallback> callback) = 0;

    enum class MessageCode {
        INIT = 0xf011,
        RELEASE = 0xf012,
        ISSTART_REMOTE_INPUT = 0xf013,
    };
};
} // namespace DistributedInput
} // namespace DistributedHardware
} // namespace OHOS

#endif // I_DISTRIBUTED_SINK_INPUT_H
