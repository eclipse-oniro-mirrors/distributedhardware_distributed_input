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

#include "register_d_input_callback_stub_fuzzer.h"

#include <cstddef>
#include <new>
#include <string>

#include <fuzzer/FuzzedDataProvider.h>

#include "i_register_d_input_call_back.h"
#include "ipc_callback_fuzzer_common.h"
#include "register_d_input_call_back_stub.h"

namespace OHOS {
namespace DistributedHardware {
namespace DistributedInput {
namespace {
class TestRegisterDInputCallbackStub : public RegisterDInputCallbackStub {
public:
    void OnResult(const std::string &devId, const std::string &dhId,
        const DInputFuzzSigned &status) override
    {
        lastDevId_ = devId;
        lastDhId_ = dhId;
        lastStatus_ = status;
    }

private:
    std::string lastDevId_;
    std::string lastDhId_;
    DInputFuzzSigned lastStatus_ = DINPUT_FUZZ_DEFAULT_VALUE;
};

constexpr DInputFuzzUnsigned REGISTER_RESULT_CODE =
    static_cast<DInputFuzzUnsigned>(IRegisterDInputCallback::Message::RESULT);

void WriteResultPayload(MessageParcel &parcel, FuzzedDataProvider &fdp)
{
    parcel.WriteString(fdp.ConsumeRandomLengthString(DINPUT_FUZZ_MAX_STRING_LENGTH));
    parcel.WriteString(fdp.ConsumeRandomLengthString(DINPUT_FUZZ_MAX_STRING_LENGTH));
    parcel.DINPUT_FUZZ_WRITE_SIGNED(fdp.ConsumeIntegral<DInputFuzzSigned>());
}

void RunRequest(RegisterDInputCallbackStub &stub, DInputFuzzUnsigned code,
    FuzzedDataProvider &fdp, bool validToken)
{
    FuzzUtil::IpcRequestContext context;
    FuzzUtil::WriteInterfaceToken(context.data,
        RegisterDInputCallbackStub::GetDescriptor(), validToken);
    WriteResultPayload(context.data, fdp);
    stub.OnRemoteRequest(code, context.data, context.reply, context.option);
}
} // namespace

void RegisterDInputCallbackStubFuzzTest(const DInputFuzzByte *data, size_t size)
{
    if (data == nullptr || size == DINPUT_FUZZ_EMPTY_SIZE) {
        return;
    }
    sptr<TestRegisterDInputCallbackStub> stub(
        new (std::nothrow) TestRegisterDInputCallbackStub());
    if (stub == nullptr) {
        return;
    }
    FuzzedDataProvider fdp(data, size);
    RunRequest(*stub, REGISTER_RESULT_CODE, fdp, true);
    RunRequest(*stub, REGISTER_RESULT_CODE, fdp, false);
    RunRequest(*stub, fdp.ConsumeIntegral<DInputFuzzUnsigned>(), fdp, fdp.ConsumeBool());
    std::string devId = fdp.ConsumeRandomLengthString(DINPUT_FUZZ_MAX_STRING_LENGTH);
    std::string dhId = fdp.ConsumeRandomLengthString(DINPUT_FUZZ_MAX_STRING_LENGTH);
    DInputFuzzSigned status = fdp.ConsumeIntegral<DInputFuzzSigned>();
    stub->OnResult(devId, dhId, status);
}
} // namespace DistributedInput
} // namespace DistributedHardware
} // namespace OHOS

extern "C" int LLVMFuzzerTestOneInput(
    const OHOS::DistributedHardware::DistributedInput::DInputFuzzByte *data, size_t size)
{
    OHOS::DistributedHardware::DistributedInput::RegisterDInputCallbackStubFuzzTest(data, size);
    return DINPUT_FUZZ_SUCCESS;
}
