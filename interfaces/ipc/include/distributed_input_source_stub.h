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

#ifndef DISRIBUTED_INPUT_SOURCE_STUB_H
#define DISRIBUTED_INPUT_SOURCE_STUB_H

#include "i_distributed_source_input.h"

#include <iostream>

#include "iremote_stub.h"

namespace OHOS {
namespace DistributedHardware {
namespace DistributedInput {
class DistributedInputSourceStub : public IRemoteStub<IDistributedSourceInput> {
public:
    DistributedInputSourceStub();
    virtual ~DistributedInputSourceStub() override;

    virtual int32_t OnRemoteRequest(
        uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) override;

private:
    int32_t HandleInitDistributedHardware(MessageParcel &reply);
    int32_t HandleReleaseDistributedHardware(MessageParcel &reply);
    int32_t HandleRegisterDistributedHardware(MessageParcel &data, MessageParcel &reply);
    int32_t HandleUnregisterDistributedHardware(MessageParcel &data, MessageParcel &reply);
    int32_t HandlePrepareRemoteInput(MessageParcel &data, MessageParcel &reply);
    int32_t HandleUnprepareRemoteInput(MessageParcel &data, MessageParcel &reply);
    int32_t HandleStartRemoteInput(MessageParcel &data, MessageParcel &reply);
    int32_t HandleStopRemoteInput(MessageParcel &data, MessageParcel &reply);
    int32_t HandleIsStartDistributedInput(MessageParcel &data, MessageParcel &reply);
    DISALLOW_COPY_AND_MOVE(DistributedInputSourceStub);
};
} // namespace DistributedInput
} // namespace DistributedHardware
} // namespace OHOS

#endif // DISRIBUTED_INPUT_STUB_H
