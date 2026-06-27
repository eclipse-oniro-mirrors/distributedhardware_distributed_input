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

#include "simulation_event_listener_stub_fuzzer.h"

#include <cstddef>
#include <new>

#include <fuzzer/FuzzedDataProvider.h>

#include "i_simulation_event_listener.h"
#include "ipc_callback_fuzzer_common.h"
#include "simulation_event_listener_stub.h"

namespace OHOS {
namespace DistributedHardware {
namespace DistributedInput {
namespace {
class TestSimulationEventListenerStub : public SimulationEventListenerStub {
public:
    DInputFuzzSigned OnSimulationEvent(DInputFuzzUnsigned type, DInputFuzzUnsigned code,
        DInputFuzzSigned value) override
    {
        lastType_ = type;
        lastCode_ = code;
        lastValue_ = value;
        return static_cast<DInputFuzzSigned>((type ^ code) & DINPUT_FUZZ_SIGNED_MASK);
    }

private:
    DInputFuzzUnsigned lastType_ = DINPUT_FUZZ_DEFAULT_VALUE;
    DInputFuzzUnsigned lastCode_ = DINPUT_FUZZ_DEFAULT_VALUE;
    DInputFuzzSigned lastValue_ = DINPUT_FUZZ_DEFAULT_VALUE;
};

void WriteEventPayload(MessageParcel &parcel, FuzzedDataProvider &fdp)
{
    parcel.DINPUT_FUZZ_WRITE_UNSIGNED(fdp.ConsumeIntegral<DInputFuzzUnsigned>());
    parcel.DINPUT_FUZZ_WRITE_UNSIGNED(fdp.ConsumeIntegral<DInputFuzzUnsigned>());
    parcel.DINPUT_FUZZ_WRITE_SIGNED(fdp.ConsumeIntegral<DInputFuzzSigned>());
}

void RunRequest(SimulationEventListenerStub &stub, DInputFuzzUnsigned code, FuzzedDataProvider &fdp, bool validToken)
{
    FuzzUtil::IpcRequestContext context;
    FuzzUtil::WriteInterfaceToken(context.data, SimulationEventListenerStub::GetDescriptor(), validToken);
    WriteEventPayload(context.data, fdp);
    stub.OnRemoteRequest(code, context.data, context.reply, context.option);
}
} // namespace

void SimulationEventListenerStubFuzzTest(const DInputFuzzByte *data, size_t size)
{
    if (data == nullptr || size == DINPUT_FUZZ_EMPTY_SIZE) {
        return;
    }
    sptr<SimulationEventListenerStub> stub(new (std::nothrow) TestSimulationEventListenerStub());
    if (stub == nullptr) {
        return;
    }
    FuzzedDataProvider fdp(data, size);
    RunRequest(*stub, static_cast<DInputFuzzUnsigned>(ISimulationEventListener::Message::RESULT_ON), fdp, true);
    RunRequest(*stub, static_cast<DInputFuzzUnsigned>(ISimulationEventListener::Message::RESULT_ON), fdp, false);
    RunRequest(*stub, fdp.ConsumeIntegral<DInputFuzzUnsigned>(), fdp, fdp.ConsumeBool());
}
} // namespace DistributedInput
} // namespace DistributedHardware
} // namespace OHOS

extern "C" int LLVMFuzzerTestOneInput(const DInputFuzzByte *data, size_t size)
{
    OHOS::DistributedHardware::DistributedInput::SimulationEventListenerStubFuzzTest(data, size);
    return DINPUT_FUZZ_SUCCESS;
}
