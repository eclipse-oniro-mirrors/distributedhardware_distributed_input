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

#include "unprepare_d_input_callback_stub_fuzzer.h"

#include <cstddef>
#include <new>
#include <string>

#include <fuzzer/FuzzedDataProvider.h>

#include "i_unprepare_d_input_call_back.h"
#include "ipc_callback_fuzzer_common.h"
#include "unprepare_d_input_call_back_stub.h"

namespace OHOS {
namespace DistributedHardware {
namespace DistributedInput {
namespace {
class TestUnprepareDInputCallbackStub : public UnprepareDInputCallbackStub {
public:
    void OnResult(const std::string &devId, const DInputFuzzSigned &status) override
    {
        lastDevId_ = devId;
        lastStatus_ = status;
    }

private:
    std::string lastDevId_;
    DInputFuzzSigned lastStatus_ = DINPUT_FUZZ_DEFAULT_VALUE;
};

void WriteResultPayload(MessageParcel &parcel, FuzzedDataProvider &fdp)
{
    parcel.WriteString(fdp.ConsumeRandomLengthString(DINPUT_FUZZ_MAX_STRING_LENGTH));
    parcel.DINPUT_FUZZ_WRITE_SIGNED(fdp.ConsumeIntegral<DInputFuzzSigned>());
}

void RunRequest(UnprepareDInputCallbackStub &stub, DInputFuzzUnsigned code, FuzzedDataProvider &fdp, bool validToken)
{
    FuzzUtil::IpcRequestContext context;
    FuzzUtil::WriteInterfaceToken(context.data, UnprepareDInputCallbackStub::GetDescriptor(), validToken);
    WriteResultPayload(context.data, fdp);
    stub.OnRemoteRequest(code, context.data, context.reply, context.option);
}
} // namespace

void UnprepareDInputCallbackStubFuzzTest(const DInputFuzzByte *data, size_t size)
{
    if (data == nullptr || size == DINPUT_FUZZ_EMPTY_SIZE) {
        return;
    }
    sptr<TestUnprepareDInputCallbackStub> stub(new (std::nothrow) TestUnprepareDInputCallbackStub());
    if (stub == nullptr) {
        return;
    }
    FuzzedDataProvider fdp(data, size);
    RunRequest(*stub, static_cast<DInputFuzzUnsigned>(IUnprepareDInputCallback::Message::RESULT), fdp, true);
    RunRequest(*stub, static_cast<DInputFuzzUnsigned>(IUnprepareDInputCallback::Message::RESULT), fdp, false);
    RunRequest(*stub, fdp.ConsumeIntegral<DInputFuzzUnsigned>(), fdp, fdp.ConsumeBool());
    stub->OnResult(fdp.ConsumeRandomLengthString(DINPUT_FUZZ_MAX_STRING_LENGTH),
        fdp.ConsumeIntegral<DInputFuzzSigned>());
}
} // namespace DistributedInput
} // namespace DistributedHardware
} // namespace OHOS

extern "C" int LLVMFuzzerTestOneInput(
    const OHOS::DistributedHardware::DistributedInput::DInputFuzzByte *data, size_t size)
{
    OHOS::DistributedHardware::DistributedInput::UnprepareDInputCallbackStubFuzzTest(data, size);
    return DINPUT_FUZZ_SUCCESS;
}
