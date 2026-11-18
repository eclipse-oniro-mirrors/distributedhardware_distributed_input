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

constexpr DInputFuzzUnsigned SIMULATION_RESULT_CODE =
    static_cast<DInputFuzzUnsigned>(ISimulationEventListener::Message::RESULT_ON);

void WriteEventPayload(MessageParcel &parcel, FuzzedDataProvider &fdp)
{
    parcel.WriteUint32(fdp.ConsumeIntegral<DInputFuzzUnsigned>());
    parcel.WriteUint32(fdp.ConsumeIntegral<DInputFuzzUnsigned>());
    parcel.WriteInt32(fdp.ConsumeIntegral<DInputFuzzSigned>());
}

void RunRequest(SimulationEventListenerStub &stub, DInputFuzzUnsigned code,
    FuzzedDataProvider &fdp, bool validToken)
{
    FuzzUtil::IpcRequestContext context;
    FuzzUtil::WriteInterfaceToken(context.data,
        SimulationEventListenerStub::GetDescriptor(), validToken);
    WriteEventPayload(context.data, fdp);
    stub.OnRemoteRequest(code, context.data, context.reply, context.option);
}
} // namespace

void SimulationEventListenerStubFuzzTest(const DInputFuzzByte *data, size_t size)
{
    if (data == nullptr || size == DINPUT_FUZZ_EMPTY_SIZE) {
        return;
    }
    sptr<TestSimulationEventListenerStub> stub(
        new (std::nothrow) TestSimulationEventListenerStub());
    if (stub == nullptr) {
        return;
    }
    FuzzedDataProvider fdp(data, size);
    RunRequest(*stub, SIMULATION_RESULT_CODE, fdp, true);
    RunRequest(*stub, SIMULATION_RESULT_CODE, fdp, false);
    RunRequest(*stub, fdp.ConsumeIntegral<DInputFuzzUnsigned>(), fdp, fdp.ConsumeBool());
    DInputFuzzUnsigned type = fdp.ConsumeIntegral<DInputFuzzUnsigned>();
    DInputFuzzUnsigned code = fdp.ConsumeIntegral<DInputFuzzUnsigned>();
    DInputFuzzSigned value = fdp.ConsumeIntegral<DInputFuzzSigned>();
    static_cast<void>(stub->OnSimulationEvent(type, code, value));
}
} // namespace DistributedInput
} // namespace DistributedHardware
} // namespace OHOS

extern "C" int LLVMFuzzerTestOneInput(
    const OHOS::DistributedHardware::DistributedInput::DInputFuzzByte *data, size_t size)
{
    OHOS::DistributedHardware::DistributedInput::SimulationEventListenerStubFuzzTest(data, size);
    return DINPUT_FUZZ_SUCCESS;
}
