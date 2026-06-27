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

#include "start_stop_result_callback_stub_fuzzer.h"

#include <cstddef>
#include <new>
#include <string>
#include <vector>

#include <fuzzer/FuzzedDataProvider.h>

#include "constants_dinput.h"
#include "i_start_stop_result_call_back.h"
#include "ipc_callback_fuzzer_common.h"
#include "start_stop_result_call_back_stub.h"

namespace OHOS {
namespace DistributedHardware {
namespace DistributedInput {
namespace {
class TestStartStopResultCallbackStub : public StartStopResultCallbackStub {
public:
    void OnStart(const std::string &srcId, const std::string &sinkId, std::vector<std::string> &dhIds) override
    {
        lastSrcId_ = srcId;
        lastSinkId_ = sinkId;
        lastDhIds_ = dhIds;
    }

    void OnStop(const std::string &srcId, const std::string &sinkId, std::vector<std::string> &dhIds) override
    {
        lastSrcId_ = srcId;
        lastSinkId_ = sinkId;
        lastDhIds_ = dhIds;
    }

private:
    std::string lastSrcId_;
    std::string lastSinkId_;
    std::vector<std::string> lastDhIds_;
};

void WriteVectorPayload(MessageParcel &parcel, FuzzedDataProvider &fdp, bool oversize)
{
    parcel.WriteString(fdp.ConsumeRandomLengthString(DINPUT_FUZZ_MAX_STRING_LENGTH));
    parcel.WriteString(fdp.ConsumeRandomLengthString(DINPUT_FUZZ_MAX_STRING_LENGTH));
    DInputFuzzUnsigned vectorSize = oversize ? IPC_VECTOR_MAX_SIZE + DINPUT_FUZZ_VECTOR_OVERSIZE_STEP :
        fdp.ConsumeIntegralInRange<DInputFuzzUnsigned>(DINPUT_FUZZ_MIN_VECTOR_SIZE, DINPUT_FUZZ_MAX_VECTOR_SIZE);
    parcel.DINPUT_FUZZ_WRITE_UNSIGNED(vectorSize);
    DInputFuzzUnsigned writeCount = oversize ? DINPUT_FUZZ_NO_VECTOR_WRITE : vectorSize;
    for (DInputFuzzUnsigned i = DINPUT_FUZZ_DEFAULT_VALUE; i < writeCount; ++i) {
        parcel.WriteString(fdp.ConsumeRandomLengthString(DINPUT_FUZZ_MAX_STRING_LENGTH));
    }
}

void RunRequest(StartStopResultCallbackStub &stub, DInputFuzzUnsigned code, FuzzedDataProvider &fdp, bool validToken,
    bool oversize)
{
    FuzzUtil::IpcRequestContext context;
    FuzzUtil::WriteInterfaceToken(context.data, StartStopResultCallbackStub::GetDescriptor(), validToken);
    WriteVectorPayload(context.data, fdp, oversize);
    stub.OnRemoteRequest(code, context.data, context.reply, context.option);
}
} // namespace

void StartStopResultCallbackStubFuzzTest(const DInputFuzzByte *data, size_t size)
{
    if (data == nullptr || size == DINPUT_FUZZ_EMPTY_SIZE) {
        return;
    }
    sptr<TestStartStopResultCallbackStub> stub(new (std::nothrow) TestStartStopResultCallbackStub());
    if (stub == nullptr) {
        return;
    }
    FuzzedDataProvider fdp(data, size);
    RunRequest(*stub, static_cast<DInputFuzzUnsigned>(IStartStopResultCallback::Message::RESULT_START), fdp, true,
        false);
    RunRequest(*stub, static_cast<DInputFuzzUnsigned>(IStartStopResultCallback::Message::RESULT_STOP), fdp, true,
        false);
    RunRequest(*stub, static_cast<DInputFuzzUnsigned>(IStartStopResultCallback::Message::RESULT_START), fdp, true,
        true);
    RunRequest(*stub, static_cast<DInputFuzzUnsigned>(IStartStopResultCallback::Message::RESULT_STOP), fdp, false,
        false);
    RunRequest(*stub, fdp.ConsumeIntegral<DInputFuzzUnsigned>(), fdp, fdp.ConsumeBool(), false);
    std::vector<std::string> dhIds = { fdp.ConsumeRandomLengthString(DINPUT_FUZZ_MAX_STRING_LENGTH) };
    stub->OnStart(fdp.ConsumeRandomLengthString(DINPUT_FUZZ_MAX_STRING_LENGTH),
        fdp.ConsumeRandomLengthString(DINPUT_FUZZ_MAX_STRING_LENGTH), dhIds);
    stub->OnStop(fdp.ConsumeRandomLengthString(DINPUT_FUZZ_MAX_STRING_LENGTH),
        fdp.ConsumeRandomLengthString(DINPUT_FUZZ_MAX_STRING_LENGTH), dhIds);
}
} // namespace DistributedInput
} // namespace DistributedHardware
} // namespace OHOS

extern "C" int LLVMFuzzerTestOneInput(
    const OHOS::DistributedHardware::DistributedInput::DInputFuzzByte *data, size_t size)
{
    OHOS::DistributedHardware::DistributedInput::StartStopResultCallbackStubFuzzTest(data, size);
    return DINPUT_FUZZ_SUCCESS;
}
