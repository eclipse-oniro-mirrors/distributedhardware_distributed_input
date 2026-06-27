/*
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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

#include "input_node_listener_stub_fuzzer.h"

#include <cstddef>
#include <new>
#include <string>

#include <fuzzer/FuzzedDataProvider.h>

#include "i_input_node_listener.h"
#include "input_node_listener_stub.h"
#include "ipc_callback_fuzzer_common.h"

namespace OHOS {
namespace DistributedHardware {
namespace DistributedInput {
namespace {
class TestInputNodeListenerStub : public InputNodeListenerStub {
public:
    void OnNodeOnLine(const std::string &srcDevId, const std::string &sinkDevId,
        const std::string &sinkNodeId, const std::string &sinkNodeDesc) override
    {
        lastSrcDevId_ = srcDevId;
        lastSinkDevId_ = sinkDevId;
        lastSinkNodeId_ = sinkNodeId;
        lastSinkNodeDesc_ = sinkNodeDesc;
    }

    void OnNodeOffLine(const std::string &srcDevId, const std::string &sinkDevId,
        const std::string &sinkNodeId) override
    {
        lastSrcDevId_ = srcDevId;
        lastSinkDevId_ = sinkDevId;
        lastSinkNodeId_ = sinkNodeId;
    }

private:
    std::string lastSrcDevId_;
    std::string lastSinkDevId_;
    std::string lastSinkNodeId_;
    std::string lastSinkNodeDesc_;
};

void WriteOnlinePayload(MessageParcel &parcel, FuzzedDataProvider &fdp)
{
    parcel.WriteString(fdp.ConsumeRandomLengthString(DINPUT_FUZZ_MAX_STRING_LENGTH));
    parcel.WriteString(fdp.ConsumeRandomLengthString(DINPUT_FUZZ_MAX_STRING_LENGTH));
    parcel.WriteString(fdp.ConsumeRandomLengthString(DINPUT_FUZZ_MAX_STRING_LENGTH));
    parcel.WriteString(fdp.ConsumeRandomLengthString(DINPUT_FUZZ_MAX_STRING_LENGTH));
}

void WriteOfflinePayload(MessageParcel &parcel, FuzzedDataProvider &fdp)
{
    parcel.WriteString(fdp.ConsumeRandomLengthString(DINPUT_FUZZ_MAX_STRING_LENGTH));
    parcel.WriteString(fdp.ConsumeRandomLengthString(DINPUT_FUZZ_MAX_STRING_LENGTH));
    parcel.WriteString(fdp.ConsumeRandomLengthString(DINPUT_FUZZ_MAX_STRING_LENGTH));
}

void RunRequest(InputNodeListenerStub &stub, DInputFuzzUnsigned code, FuzzedDataProvider &fdp, bool validToken)
{
    FuzzUtil::IpcRequestContext context;
    FuzzUtil::WriteInterfaceToken(context.data, InputNodeListenerStub::GetDescriptor(), validToken);
    if (code == static_cast<DInputFuzzUnsigned>(InputNodeListener::Message::RESULT_ON)) {
        WriteOnlinePayload(context.data, fdp);
    } else {
        WriteOfflinePayload(context.data, fdp);
    }
    stub.OnRemoteRequest(code, context.data, context.reply, context.option);
}
} // namespace

void InputNodeListenerStubFuzzTest(const DInputFuzzByte *data, size_t size)
{
    if (data == nullptr || size == DINPUT_FUZZ_EMPTY_SIZE) {
        return;
    }
    sptr<TestInputNodeListenerStub> stub(new (std::nothrow) TestInputNodeListenerStub());
    if (stub == nullptr) {
        return;
    }
    FuzzedDataProvider fdp(data, size);
    RunRequest(*stub, static_cast<DInputFuzzUnsigned>(InputNodeListener::Message::RESULT_ON), fdp, true);
    RunRequest(*stub, static_cast<DInputFuzzUnsigned>(InputNodeListener::Message::RESULT_OFF), fdp, true);
    RunRequest(*stub, static_cast<DInputFuzzUnsigned>(InputNodeListener::Message::RESULT_ON), fdp, false);
    RunRequest(*stub, fdp.ConsumeIntegral<DInputFuzzUnsigned>(), fdp, fdp.ConsumeBool());
    stub->OnNodeOnLine(fdp.ConsumeRandomLengthString(DINPUT_FUZZ_MAX_STRING_LENGTH),
        fdp.ConsumeRandomLengthString(DINPUT_FUZZ_MAX_STRING_LENGTH),
        fdp.ConsumeRandomLengthString(DINPUT_FUZZ_MAX_STRING_LENGTH),
        fdp.ConsumeRandomLengthString(DINPUT_FUZZ_MAX_STRING_LENGTH));
    stub->OnNodeOffLine(fdp.ConsumeRandomLengthString(DINPUT_FUZZ_MAX_STRING_LENGTH),
        fdp.ConsumeRandomLengthString(DINPUT_FUZZ_MAX_STRING_LENGTH),
        fdp.ConsumeRandomLengthString(DINPUT_FUZZ_MAX_STRING_LENGTH));
}
} // namespace DistributedInput
} // namespace DistributedHardware
} // namespace OHOS

extern "C" int LLVMFuzzerTestOneInput(
    const OHOS::DistributedHardware::DistributedInput::DInputFuzzByte *data, size_t size)
{
    OHOS::DistributedHardware::DistributedInput::InputNodeListenerStubFuzzTest(data, size);
    return DINPUT_FUZZ_SUCCESS;
}
