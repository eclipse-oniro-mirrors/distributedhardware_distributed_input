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

#include "ipccallbackbroad_fuzzer.h"

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

#include <fuzzer/FuzzedDataProvider.h>

#include "add_white_list_infos_call_back_stub.h"
#include "constants_dinput.h"
#include "del_white_list_infos_call_back_stub.h"
#include "get_sink_screen_infos_call_back_stub.h"
#include "input_node_listener_stub.h"
#include "ipc_callback_fuzzer_common.h"
#include "prepare_d_input_call_back_stub.h"
#include "register_d_input_call_back_stub.h"
#include "register_session_state_callback_stub.h"
#include "sharing_dhid_listener_stub.h"
#include "simulation_event_listener_stub.h"
#include "start_d_input_call_back_stub.h"
#include "start_stop_d_inputs_call_back_stub.h"
#include "start_stop_result_call_back_stub.h"
#include "stop_d_input_call_back_stub.h"
#include "unprepare_d_input_call_back_stub.h"
#include "unregister_d_input_call_back_stub.h"

namespace OHOS {
namespace DistributedHardware {
namespace DistributedInput {
namespace {
constexpr DInputFuzzUnsigned UNKNOWN_CODE_MASK = 0x80000000U;
constexpr DInputFuzzUnsigned EXTRA_CASE_COUNT = 8U;
constexpr DInputFuzzUnsigned EXTRA_ADD_WHITE_LIST_CASE = 0U;
constexpr DInputFuzzUnsigned EXTRA_NODE_OFFLINE_CASE = 1U;
constexpr DInputFuzzUnsigned EXTRA_NO_SHARING_CASE = 2U;
constexpr DInputFuzzUnsigned EXTRA_SIMULATION_CASE = 3U;
constexpr DInputFuzzUnsigned EXTRA_VECTOR_OVERSIZE_CASE = 4U;
constexpr DInputFuzzUnsigned EXTRA_SESSION_STATE_CASE = 5U;
constexpr DInputFuzzUnsigned EXTRA_START_INPUT_CASE = 6U;
constexpr DInputFuzzUnsigned START_INPUT_RESULT_CODE =
    static_cast<DInputFuzzUnsigned>(IStartDInputCallback::Message::RESULT);
constexpr DInputFuzzUnsigned STOP_INPUT_RESULT_CODE =
    static_cast<DInputFuzzUnsigned>(IStopDInputCallback::Message::RESULT);
constexpr DInputFuzzUnsigned START_STOP_RESULT_START_CODE =
    static_cast<DInputFuzzUnsigned>(IStartStopResultCallback::Message::RESULT_START);

struct IpcRequestSpec {
    std::u16string descriptor;
    DInputFuzzUnsigned code;
    bool validToken;
};

class TestAddWhiteListInfosCallbackStub final : public AddWhiteListInfosCallbackStub {
public:
    void OnResult(const std::string &deviceId, const std::string &strJson) override
    {
        (void)deviceId;
        (void)strJson;
    }
};

class TestDelWhiteListInfosCallbackStub final : public DelWhiteListInfosCallbackStub {
public:
    void OnResult(const std::string &deviceId) override
    {
        (void)deviceId;
    }
};

class TestGetSinkScreenInfosCallbackStub final : public GetSinkScreenInfosCallbackStub {
public:
    void OnResult(const std::string &strJson) override
    {
        (void)strJson;
    }
};

class TestInputNodeListenerStub final : public InputNodeListenerStub {
public:
    void OnNodeOnLine(const std::string &srcDevId, const std::string &sinkDevId,
        const std::string &sinkNodeId, const std::string &sinkNodeDesc) override
    {
        (void)srcDevId;
        (void)sinkDevId;
        (void)sinkNodeId;
        (void)sinkNodeDesc;
    }

    void OnNodeOffLine(const std::string &srcDevId, const std::string &sinkDevId,
        const std::string &sinkNodeId) override
    {
        (void)srcDevId;
        (void)sinkDevId;
        (void)sinkNodeId;
    }
};

class TestPrepareDInputCallbackStub final : public PrepareDInputCallbackStub {
public:
    void OnResult(const std::string &devId, const DInputFuzzSigned &status) override
    {
        (void)devId;
        (void)status;
    }
};

class TestUnprepareDInputCallbackStub final : public UnprepareDInputCallbackStub {
public:
    void OnResult(const std::string &devId, const DInputFuzzSigned &status) override
    {
        (void)devId;
        (void)status;
    }
};

class TestRegisterDInputCallbackStub final : public RegisterDInputCallbackStub {
public:
    void OnResult(const std::string &devId, const std::string &dhId,
        const DInputFuzzSigned &status) override
    {
        (void)devId;
        (void)dhId;
        (void)status;
    }
};

class TestUnregisterDInputCallbackStub final : public UnregisterDInputCallbackStub {
public:
    void OnResult(const std::string &devId, const std::string &dhId,
        const DInputFuzzSigned &status) override
    {
        (void)devId;
        (void)dhId;
        (void)status;
    }
};

class TestStartDInputCallbackStub final : public StartDInputCallbackStub {
public:
    void OnResult(const std::string &devId, const DInputFuzzUnsigned &inputTypes,
        const DInputFuzzSigned &status) override
    {
        (void)devId;
        (void)inputTypes;
        (void)status;
    }
};

class TestStopDInputCallbackStub final : public StopDInputCallbackStub {
public:
    void OnResult(const std::string &devId, const DInputFuzzUnsigned &inputTypes,
        const DInputFuzzSigned &status) override
    {
        (void)devId;
        (void)inputTypes;
        (void)status;
    }
};

class TestStartStopDInputsCallbackStub final : public StartStopDInputsCallbackStub {
public:
    void OnResultDhids(const std::string &devId, const DInputFuzzSigned &status) override
    {
        (void)devId;
        (void)status;
    }
};

class TestStartStopResultCallbackStub final : public StartStopResultCallbackStub {
public:
    void OnStart(const std::string &srcId, const std::string &sinkId,
        std::vector<std::string> &dhIds) override
    {
        (void)srcId;
        (void)sinkId;
        (void)dhIds;
    }

    void OnStop(const std::string &srcId, const std::string &sinkId,
        std::vector<std::string> &dhIds) override
    {
        (void)srcId;
        (void)sinkId;
        (void)dhIds;
    }
};

class TestSharingDhIdListenerStub final : public SharingDhIdListenerStub {
public:
    DInputFuzzSigned OnSharing(const std::string &dhId) override
    {
        (void)dhId;
        return DINPUT_FUZZ_SUCCESS;
    }

    DInputFuzzSigned OnNoSharing(const std::string &dhId) override
    {
        (void)dhId;
        return DINPUT_FUZZ_SUCCESS;
    }
};

class TestSimulationEventListenerStub final : public SimulationEventListenerStub {
public:
    DInputFuzzSigned OnSimulationEvent(DInputFuzzUnsigned type, DInputFuzzUnsigned code,
        DInputFuzzSigned value) override
    {
        (void)type;
        (void)code;
        (void)value;
        return DINPUT_FUZZ_SUCCESS;
    }
};

class TestRegisterSessionStateCallbackStub final : public RegisterSessionStateCallbackStub {
public:
    void OnResult(const std::string &devId, const DInputFuzzUnsigned status) override
    {
        (void)devId;
        (void)status;
    }
};

std::string ConsumeString(FuzzedDataProvider &fdp)
{
    return fdp.ConsumeRandomLengthString(DINPUT_FUZZ_MAX_STRING_LENGTH);
}

DInputFuzzUnsigned ConsumeUnknownCode(FuzzedDataProvider &fdp)
{
    return fdp.ConsumeIntegral<DInputFuzzUnsigned>() | UNKNOWN_CODE_MASK;
}

void WriteOneStringPayload(MessageParcel &parcel, FuzzedDataProvider &fdp)
{
    parcel.WriteString(ConsumeString(fdp));
}

void WriteTwoStringPayload(MessageParcel &parcel, FuzzedDataProvider &fdp)
{
    parcel.WriteString(ConsumeString(fdp));
    parcel.WriteString(ConsumeString(fdp));
}

void WriteStringStatusPayload(MessageParcel &parcel, FuzzedDataProvider &fdp)
{
    parcel.WriteString(ConsumeString(fdp));
    parcel.WriteInt32(fdp.ConsumeIntegral<DInputFuzzSigned>());
}

void WriteStringUintStatusPayload(MessageParcel &parcel, FuzzedDataProvider &fdp)
{
    parcel.WriteString(ConsumeString(fdp));
    parcel.WriteUint32(fdp.ConsumeIntegral<DInputFuzzUnsigned>());
}

void WriteTwoStringStatusPayload(MessageParcel &parcel, FuzzedDataProvider &fdp)
{
    parcel.WriteString(ConsumeString(fdp));
    parcel.WriteString(ConsumeString(fdp));
    parcel.WriteInt32(fdp.ConsumeIntegral<DInputFuzzSigned>());
}

void WriteStartStopPayload(MessageParcel &parcel, FuzzedDataProvider &fdp)
{
    parcel.WriteString(ConsumeString(fdp));
    parcel.WriteUint32(fdp.ConsumeIntegral<DInputFuzzUnsigned>());
    parcel.WriteInt32(fdp.ConsumeIntegral<DInputFuzzSigned>());
}

void WriteNodeOnlinePayload(MessageParcel &parcel, FuzzedDataProvider &fdp)
{
    parcel.WriteString(ConsumeString(fdp));
    parcel.WriteString(ConsumeString(fdp));
    parcel.WriteString(ConsumeString(fdp));
    parcel.WriteString(ConsumeString(fdp));
}

void WriteNodeOfflinePayload(MessageParcel &parcel, FuzzedDataProvider &fdp)
{
    parcel.WriteString(ConsumeString(fdp));
    parcel.WriteString(ConsumeString(fdp));
    parcel.WriteString(ConsumeString(fdp));
}

void WriteSimulationPayload(MessageParcel &parcel, FuzzedDataProvider &fdp)
{
    parcel.WriteUint32(fdp.ConsumeIntegral<DInputFuzzUnsigned>());
    parcel.WriteUint32(fdp.ConsumeIntegral<DInputFuzzUnsigned>());
    parcel.WriteInt32(fdp.ConsumeIntegral<DInputFuzzSigned>());
}

void WriteVectorPayload(MessageParcel &parcel, FuzzedDataProvider &fdp, bool oversize)
{
    parcel.WriteString(ConsumeString(fdp));
    parcel.WriteString(ConsumeString(fdp));
    DInputFuzzUnsigned vectorSize = oversize ?
        IPC_VECTOR_MAX_SIZE + DINPUT_FUZZ_VECTOR_OVERSIZE_STEP :
        fdp.ConsumeIntegralInRange<DInputFuzzUnsigned>(
            DINPUT_FUZZ_MIN_VECTOR_SIZE, DINPUT_FUZZ_MAX_VECTOR_SIZE);
    parcel.WriteUint32(vectorSize);
    DInputFuzzUnsigned writeCount = oversize ? DINPUT_FUZZ_NO_VECTOR_WRITE : vectorSize;
    for (DInputFuzzUnsigned i = DINPUT_FUZZ_DEFAULT_VALUE; i < writeCount; ++i) {
        parcel.WriteString(ConsumeString(fdp));
    }
}

template<typename Stub, typename PayloadWriter>
void RunIpcRequest(Stub &stub, const IpcRequestSpec &request,
    FuzzedDataProvider &fdp, PayloadWriter payloadWriter)
{
    FuzzUtil::IpcRequestContext context;
    FuzzUtil::WriteInterfaceToken(context.data, request.descriptor, request.validToken);
    payloadWriter(context.data, fdp);
    stub.OnRemoteRequest(request.code, context.data, context.reply, context.option);
}

template<typename Stub, typename PayloadWriter>
void RunKnownAndNegative(Stub &stub, const std::u16string &descriptor, DInputFuzzUnsigned code,
    FuzzedDataProvider &fdp, PayloadWriter payloadWriter)
{
    RunIpcRequest(stub, { descriptor, code, true }, fdp, payloadWriter);
    RunIpcRequest(stub, { descriptor, code, false }, fdp, payloadWriter);
    RunIpcRequest(stub, { descriptor, ConsumeUnknownCode(fdp), true }, fdp, payloadWriter);
}

void WriteNormalVectorPayload(MessageParcel &parcel, FuzzedDataProvider &provider)
{
    WriteVectorPayload(parcel, provider, false);
}

void WriteOversizeVectorPayload(MessageParcel &parcel, FuzzedDataProvider &provider)
{
    WriteVectorPayload(parcel, provider, true);
}

void ExerciseBasicCallbacks(FuzzedDataProvider &fdp)
{
    TestAddWhiteListInfosCallbackStub addWhiteListStub;
    RunKnownAndNegative(addWhiteListStub, AddWhiteListInfosCallbackStub::GetDescriptor(),
        static_cast<DInputFuzzUnsigned>(IAddWhiteListInfosCallback::Message::RESULT),
        fdp, WriteTwoStringPayload);

    TestDelWhiteListInfosCallbackStub delWhiteListStub;
    RunKnownAndNegative(delWhiteListStub, DelWhiteListInfosCallbackStub::GetDescriptor(),
        static_cast<DInputFuzzUnsigned>(IDelWhiteListInfosCallback::Message::RESULT),
        fdp, WriteOneStringPayload);

    TestGetSinkScreenInfosCallbackStub getScreenInfosStub;
    RunKnownAndNegative(getScreenInfosStub, GetSinkScreenInfosCallbackStub::GetDescriptor(),
        static_cast<DInputFuzzUnsigned>(IGetSinkScreenInfosCallback::Message::RESULT),
        fdp, WriteOneStringPayload);
}

void ExerciseLifecycleCallbacks(FuzzedDataProvider &fdp)
{
    TestPrepareDInputCallbackStub prepareStub;
    RunKnownAndNegative(prepareStub, PrepareDInputCallbackStub::GetDescriptor(),
        static_cast<DInputFuzzUnsigned>(IPrepareDInputCallback::Message::RESULT),
        fdp, WriteStringStatusPayload);

    TestUnprepareDInputCallbackStub unprepareStub;
    RunKnownAndNegative(unprepareStub, UnprepareDInputCallbackStub::GetDescriptor(),
        static_cast<DInputFuzzUnsigned>(IUnprepareDInputCallback::Message::RESULT),
        fdp, WriteStringStatusPayload);

    TestStartDInputCallbackStub startStub;
    RunKnownAndNegative(startStub, StartDInputCallbackStub::GetDescriptor(),
        START_INPUT_RESULT_CODE, fdp, WriteStartStopPayload);

    TestStopDInputCallbackStub stopStub;
    RunKnownAndNegative(stopStub, StopDInputCallbackStub::GetDescriptor(),
        STOP_INPUT_RESULT_CODE, fdp, WriteStartStopPayload);
}

void ExerciseRegisterCallbacks(FuzzedDataProvider &fdp)
{
    TestRegisterDInputCallbackStub registerStub;
    RunKnownAndNegative(registerStub, RegisterDInputCallbackStub::GetDescriptor(),
        static_cast<DInputFuzzUnsigned>(IRegisterDInputCallback::Message::RESULT), fdp,
        WriteTwoStringStatusPayload);

    TestUnregisterDInputCallbackStub unregisterStub;
    RunKnownAndNegative(unregisterStub, UnregisterDInputCallbackStub::GetDescriptor(),
        static_cast<DInputFuzzUnsigned>(IUnregisterDInputCallback::Message::RESULT), fdp,
        WriteTwoStringStatusPayload);

    TestRegisterSessionStateCallbackStub sessionStateStub;
    RunKnownAndNegative(sessionStateStub, RegisterSessionStateCallbackStub::GetDescriptor(),
        static_cast<DInputFuzzUnsigned>(ISessionStateCallback::Message::RESULT), fdp,
        WriteStringUintStatusPayload);
}

void ExerciseListenerCallbacks(FuzzedDataProvider &fdp)
{
    TestInputNodeListenerStub nodeStub;
    RunKnownAndNegative(nodeStub, InputNodeListenerStub::GetDescriptor(),
        static_cast<DInputFuzzUnsigned>(InputNodeListener::Message::RESULT_ON),
        fdp, WriteNodeOnlinePayload);
    RunIpcRequest(nodeStub,
        { InputNodeListenerStub::GetDescriptor(),
            static_cast<DInputFuzzUnsigned>(InputNodeListener::Message::RESULT_OFF), true },
        fdp, WriteNodeOfflinePayload);
    nodeStub.OnNodeOffLine(ConsumeString(fdp), ConsumeString(fdp), ConsumeString(fdp));

    TestSharingDhIdListenerStub sharingStub;
    RunKnownAndNegative(sharingStub, SharingDhIdListenerStub::GetDescriptor(),
        static_cast<DInputFuzzUnsigned>(ISharingDhIdListener::Message::SHARING),
        fdp, WriteOneStringPayload);
    RunIpcRequest(sharingStub,
        { SharingDhIdListenerStub::GetDescriptor(),
            static_cast<DInputFuzzUnsigned>(ISharingDhIdListener::Message::NO_SHARING), true },
        fdp, WriteOneStringPayload);
    sharingStub.OnNoSharing(ConsumeString(fdp));

    TestSimulationEventListenerStub simulationStub;
    RunKnownAndNegative(simulationStub, SimulationEventListenerStub::GetDescriptor(),
        static_cast<DInputFuzzUnsigned>(ISimulationEventListener::Message::RESULT_ON),
        fdp, WriteSimulationPayload);
}

void ExerciseVectorCallbacks(FuzzedDataProvider &fdp)
{
    TestStartStopDInputsCallbackStub startStopDInputsStub;
    RunKnownAndNegative(startStopDInputsStub, StartStopDInputsCallbackStub::GetDescriptor(),
        static_cast<DInputFuzzUnsigned>(IStartStopDInputsCallback::Message::RESULT_STRING), fdp,
        WriteStringStatusPayload);

    TestStartStopResultCallbackStub resultStub;
    RunKnownAndNegative(resultStub, StartStopResultCallbackStub::GetDescriptor(),
        START_STOP_RESULT_START_CODE, fdp, WriteNormalVectorPayload);
    RunIpcRequest(resultStub,
        { StartStopResultCallbackStub::GetDescriptor(),
            static_cast<DInputFuzzUnsigned>(IStartStopResultCallback::Message::RESULT_STOP), true },
        fdp, WriteNormalVectorPayload);
    RunIpcRequest(resultStub,
        { StartStopResultCallbackStub::GetDescriptor(),
            START_STOP_RESULT_START_CODE, true },
        fdp, WriteOversizeVectorPayload);
    RunIpcRequest(resultStub,
        { StartStopResultCallbackStub::GetDescriptor(),
            static_cast<DInputFuzzUnsigned>(IStartStopResultCallback::Message::RESULT_STOP), true },
        fdp, WriteOversizeVectorPayload);
    std::vector<std::string> emptyDhIds;
    resultStub.OnStop(ConsumeString(fdp), ConsumeString(fdp), emptyDhIds);
}

void ExerciseExtraAddWhiteList(FuzzedDataProvider &fdp)
{
    TestAddWhiteListInfosCallbackStub stub;
    IpcRequestSpec request = {
        AddWhiteListInfosCallbackStub::GetDescriptor(),
        ConsumeUnknownCode(fdp),
        fdp.ConsumeBool()
    };
    RunIpcRequest(stub, request, fdp, WriteTwoStringPayload);
}

void ExerciseExtraNodeOffline(FuzzedDataProvider &fdp)
{
    TestInputNodeListenerStub stub;
    IpcRequestSpec request = {
        InputNodeListenerStub::GetDescriptor(),
        static_cast<DInputFuzzUnsigned>(InputNodeListener::Message::RESULT_OFF),
        fdp.ConsumeBool()
    };
    RunIpcRequest(stub, request, fdp, WriteNodeOfflinePayload);
}

void ExerciseExtraNoSharing(FuzzedDataProvider &fdp)
{
    TestSharingDhIdListenerStub stub;
    IpcRequestSpec request = {
        SharingDhIdListenerStub::GetDescriptor(),
        static_cast<DInputFuzzUnsigned>(ISharingDhIdListener::Message::NO_SHARING),
        fdp.ConsumeBool()
    };
    RunIpcRequest(stub, request, fdp, WriteOneStringPayload);
}

void ExerciseExtraSimulation(FuzzedDataProvider &fdp)
{
    TestSimulationEventListenerStub stub;
    IpcRequestSpec request = {
        SimulationEventListenerStub::GetDescriptor(),
        static_cast<DInputFuzzUnsigned>(ISimulationEventListener::Message::RESULT_ON),
        fdp.ConsumeBool()
    };
    RunIpcRequest(stub, request, fdp, WriteSimulationPayload);
}

void ExerciseExtraVectorOversize(FuzzedDataProvider &fdp)
{
    TestStartStopResultCallbackStub stub;
    IpcRequestSpec request = {
        StartStopResultCallbackStub::GetDescriptor(),
        START_STOP_RESULT_START_CODE,
        fdp.ConsumeBool()
    };
    RunIpcRequest(stub, request, fdp, WriteOversizeVectorPayload);
}

void ExerciseExtraCallbackPath(DInputFuzzUnsigned selector, FuzzedDataProvider &fdp)
{
    switch (selector) {
        case EXTRA_ADD_WHITE_LIST_CASE: {
            ExerciseExtraAddWhiteList(fdp);
            break;
        }
        case EXTRA_NODE_OFFLINE_CASE: {
            ExerciseExtraNodeOffline(fdp);
            break;
        }
        case EXTRA_NO_SHARING_CASE: {
            ExerciseExtraNoSharing(fdp);
            break;
        }
        case EXTRA_SIMULATION_CASE: {
            ExerciseExtraSimulation(fdp);
            break;
        }
        case EXTRA_VECTOR_OVERSIZE_CASE: {
            ExerciseExtraVectorOversize(fdp);
            break;
        }
        default:
            break;
    }
}

void ExerciseExtraStatePath(DInputFuzzUnsigned selector, FuzzedDataProvider &fdp)
{
    switch (selector) {
        case EXTRA_SESSION_STATE_CASE: {
            TestRegisterSessionStateCallbackStub stub;
            IpcRequestSpec request = {
                RegisterSessionStateCallbackStub::GetDescriptor(),
                static_cast<DInputFuzzUnsigned>(ISessionStateCallback::Message::RESULT),
                fdp.ConsumeBool()
            };
            RunIpcRequest(stub, request, fdp, WriteStringUintStatusPayload);
            break;
        }
        case EXTRA_START_INPUT_CASE: {
            TestStartDInputCallbackStub stub;
            IpcRequestSpec request = {
                StartDInputCallbackStub::GetDescriptor(),
                START_INPUT_RESULT_CODE,
                fdp.ConsumeBool()
            };
            RunIpcRequest(stub, request, fdp, WriteStartStopPayload);
            break;
        }
        default: {
            TestUnregisterDInputCallbackStub stub;
            IpcRequestSpec request = {
                UnregisterDInputCallbackStub::GetDescriptor(),
                static_cast<DInputFuzzUnsigned>(IUnregisterDInputCallback::Message::RESULT),
                fdp.ConsumeBool()
            };
            RunIpcRequest(stub, request, fdp, WriteTwoStringStatusPayload);
            break;
        }
    }
}

void ExerciseSelectedExtraPath(FuzzedDataProvider &fdp)
{
    DInputFuzzUnsigned selector = fdp.ConsumeIntegralInRange<DInputFuzzUnsigned>(
        DINPUT_FUZZ_DEFAULT_VALUE, EXTRA_CASE_COUNT - 1U);
    ExerciseExtraCallbackPath(selector, fdp);
    ExerciseExtraStatePath(selector, fdp);
}
} // namespace

void IpcCallbackBroadFuzzTest(const uint8_t *data, size_t size)
{
    if (data == nullptr || size == DINPUT_FUZZ_EMPTY_SIZE) {
        return;
    }

    FuzzedDataProvider fdp(data, size);
    ExerciseBasicCallbacks(fdp);
    ExerciseLifecycleCallbacks(fdp);
    ExerciseRegisterCallbacks(fdp);
    ExerciseListenerCallbacks(fdp);
    ExerciseVectorCallbacks(fdp);
    ExerciseSelectedExtraPath(fdp);
}
} // namespace DistributedInput
} // namespace DistributedHardware
} // namespace OHOS

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    OHOS::DistributedHardware::DistributedInput::IpcCallbackBroadFuzzTest(data, size);
    return DINPUT_FUZZ_SUCCESS;
}
