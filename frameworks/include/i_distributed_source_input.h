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

#ifndef I_DISTRIBUTED_SOURCE_INPUT_H
#define I_DISTRIBUTED_SOURCE_INPUT_H

#include <string>
#include <vector>

#include "iremote_broker.h"
#include "iremote_object.h"

#include "constants_dinput.h"
#include "i_add_white_list_infos_call_back.h"
#include "i_del_white_list_infos_call_back.h"
#include "i_prepare_d_input_call_back.h"
#include "i_register_d_input_call_back.h"
#include "i_start_d_input_call_back.h"
#include "i_start_d_input_server_call_back.h"
#include "i_stop_d_input_call_back.h"
#include "i_start_stop_d_inputs_call_back.h"
#include "i_start_stop_result_call_back.h"
#include "i_unprepare_d_input_call_back.h"
#include "i_unregister_d_input_call_back.h"
#include "i_input_node_listener.h"
#include "i_simulation_event_listener.h"

namespace OHOS {
namespace DistributedHardware {
namespace DistributedInput {
class IDistributedSourceInput : public IRemoteBroker {
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"ohos.DistributedHardware.DistributedInput.IDistributedSourceInput");

    virtual int32_t Init() = 0;

    virtual int32_t Release() = 0;

    virtual int32_t RegisterDistributedHardware(
        const std::string& devId, const std::string& dhId, const std::string& parameters,
        sptr<IRegisterDInputCallback> callback) = 0;

    virtual int32_t UnregisterDistributedHardware(
        const std::string& devId, const std::string& dhId,
        sptr<IUnregisterDInputCallback> callback) = 0;

    virtual int32_t PrepareRemoteInput(const std::string &deviceId, sptr<IPrepareDInputCallback> callback) = 0;

    virtual int32_t UnprepareRemoteInput(const std::string &deviceId, sptr<IUnprepareDInputCallback> callback) = 0;

    virtual int32_t StartRemoteInput(
        const std::string& deviceId, const uint32_t& inputTypes, sptr<IStartDInputCallback> callback) = 0;

    virtual int32_t StopRemoteInput(
        const std::string& deviceId, const uint32_t& inputTypes, sptr<IStopDInputCallback> callback) = 0;

    virtual int32_t StartRemoteInput(const std::string &srcId, const std::string &sinkId, const uint32_t &inputTypes,
        sptr<IStartDInputCallback> callback) = 0;

    virtual int32_t StopRemoteInput(const std::string &srcId, const std::string &sinkId, const uint32_t &inputTypes,
        sptr<IStopDInputCallback> callback) = 0;

    virtual int32_t PrepareRemoteInput(const std::string &srcId, const std::string &sinkId,
        sptr<IPrepareDInputCallback> callback) = 0;

    virtual int32_t UnprepareRemoteInput(const std::string &srcId, const std::string &sinkId,
        sptr<IUnprepareDInputCallback> callback) = 0;

    virtual int32_t StartRemoteInput(const std::string &sinkId, const std::vector<std::string> &dhIds,
        sptr<IStartStopDInputsCallback> callback) = 0;

    virtual int32_t StopRemoteInput(const std::string &sinkId, const std::vector<std::string> &dhIds,
        sptr<IStartStopDInputsCallback> callback) = 0;

    virtual int32_t StartRemoteInput(const std::string &srcId, const std::string &sinkId,
        const std::vector<std::string> &dhIds, sptr<IStartStopDInputsCallback> callback) = 0;

    virtual int32_t StopRemoteInput(const std::string &srcId, const std::string &sinkId,
        const std::vector<std::string> &dhIds, sptr<IStartStopDInputsCallback> callback) = 0;

    virtual int32_t IsStartDistributedInput(
        const uint32_t& inputType, sptr<IStartDInputServerCallback> callback) = 0;

    virtual int32_t RegisterAddWhiteListCallback(sptr<IAddWhiteListInfosCallback> addWhiteListCallback) = 0;
    virtual int32_t RegisterDelWhiteListCallback(sptr<IDelWhiteListInfosCallback> delWhiteListCallback) = 0;
    virtual int32_t RegisterInputNodeListener(sptr<InputNodeListener> listener) = 0;
    virtual int32_t UnregisterInputNodeListener(sptr<InputNodeListener> listener) = 0;

    virtual int32_t SyncNodeInfoRemoteInput(const std::string &userDevId, const std::string &dhid,
        const std::string &nodeDesc) = 0;
    virtual int32_t RegisterSimulationEventListener(sptr<ISimulationEventListener> listener) = 0;
    virtual int32_t UnregisterSimulationEventListener(sptr<ISimulationEventListener> listener) = 0;

    enum class MessageCode {
        INIT = 0xf001,
        RELEASE = 0xf002,
        REGISTER_REMOTE_INPUT = 0xf003,
        UNREGISTER_REMOTE_INPUT = 0xf004,
        PREPARE_REMOTE_INPUT = 0xf005,
        UNPREPARE_REMOTE_INPUT = 0xf006,
        START_REMOTE_INPUT = 0xf007,
        STOP_REMOTE_INPUT = 0xf008,
        ISSTART_REMOTE_INPUT = 0xf009,
        PREPARE_RELAY_REMOTE_INPUT = 0xf00a,
        UNPREPARE_RELAY_REMOTE_INPUT = 0xf00b,
        START_RELAY_TYPE_REMOTE_INPUT = 0xf00c,
        STOP_RELAY_TYPE_REMOTE_INPUT = 0xf00d,
        START_DHID_REMOTE_INPUT = 0xf00e,
        STOP_DHID_REMOTE_INPUT = 0xf00f,
        START_RELAY_DHID_REMOTE_INPUT = 0xf010,
        STOP_RELAY_DHID_REMOTE_INPUT = 0xf011,
        REGISTER_ADD_WHITE_LIST_CB_REMOTE_INPUT = 0xf013,
        REGISTER_DEL_WHITE_LIST_CB_REMOTE_INPUT = 0xf014,
        REGISTER_NODE_LISTENER = 0xf015,
        UNREGISTER_NODE_LISTENER = 0xf016,
        REGISTER_SIMULATION_EVENT_LISTENER = 0xf017,
        UNREGISTER_SIMULATION_EVENT_LISTENER = 0xf018,
        SYNC_NODE_INFO_REMOTE_INPUT = 0xf019,
    };
};
} // namespace DistributedInput
} // namespace DistributedHardware
} // namespace OHOS

#endif // I_DISTRIBUTED_SOURCE_INPUT_H
