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

#ifndef DISTRIBUTED_INPUT_SOURCE_MANAGER_SERVICE_H
#define DISTRIBUTED_INPUT_SOURCE_MANAGER_SERVICE_H

#include <cstring>
#include <mutex>
#include <set>

#include <unistd.h>
#include <sys/types.h>

#include "event_handler.h"
#include "ipublisher_listener.h"
#include "publisher_listener_stub.h"
#include "singleton.h"
#include "system_ability.h"
#include "system_ability_status_change_stub.h"

#include "constants_dinput.h"
#include "dinput_context.h"
#include "dinput_source_trans_callback.h"
#include "distributed_input_node_manager.h"
#include "distributed_input_source_event_handler.h"
#include "distributed_input_source_sa_cli_mgr.h"
#include "distributed_input_source_stub.h"
#include "idinput_dbg_itf.h"

namespace OHOS {
namespace DistributedHardware {
namespace DistributedInput {
using EventRunner = OHOS::AppExecFwk::EventRunner;
using EventHandler = OHOS::AppExecFwk::EventHandler;
enum class ServiceSourceRunningState { STATE_NOT_START, STATE_RUNNING };
const uint32_t DINPUT_SOURCE_MANAGER_RIGISTER_MSG = 1;
const uint32_t DINPUT_SOURCE_MANAGER_UNRIGISTER_MSG = 2;
const uint32_t DINPUT_SOURCE_MANAGER_PREPARE_MSG = 3;
const uint32_t DINPUT_SOURCE_MANAGER_UNPREPARE_MSG = 4;
const uint32_t DINPUT_SOURCE_MANAGER_START_MSG = 5;
const uint32_t DINPUT_SOURCE_MANAGER_STOP_MSG = 6;
const uint32_t DINPUT_SOURCE_MANAGER_START_DHID_MSG = 7;
const uint32_t DINPUT_SOURCE_MANAGER_STOP_DHID_MSG = 8;
const uint32_t DINPUT_SOURCE_MANAGER_RECEIVE_DATA_MSG = 9;
const uint32_t DINPUT_SOURCE_MANAGER_STARTSERVER_MSG = 10;
const uint32_t DINPUT_SOURCE_MANAGER_KEY_STATE_MSG = 11;
const std::string INPUT_SOURCEMANAGER_KEY_DEVID = "deviceId";
const std::string INPUT_SOURCEMANAGER_KEY_HWID = "hardwareId";
const std::string INPUT_SOURCEMANAGER_KEY_ITP = "inputTypes";
const std::string INPUT_SOURCEMANAGER_KEY_DHID = "dhids";
const std::string INPUT_SOURCEMANAGER_KEY_TYPE = "type";
const std::string INPUT_SOURCEMANAGER_KEY_CODE = "code";
const std::string INPUT_SOURCEMANAGER_KEY_VALUE = "value";
const std::string INPUT_SOURCEMANAGER_KEY_FROM_START_DHID = "fromStartDhid";
const std::string INPUT_SOURCEMANAGER_KEY_RESULT = "result";
const std::string INPUT_SOURCEMANAGER_KEY_WHITELIST = "whitelist";
const uint32_t DINPUT_SOURCE_SWITCH_OFF = 0;
const uint32_t DINPUT_SOURCE_SWITCH_ON = 1;
const uint32_t DINPUT_SOURCE_WRITE_EVENT_SIZE = 1;

// Node Info that registerd by remote node
typedef struct BeRegNodeInfo {
    // source node network id
    std::string srcId;
    // sink node dh id
    std::string dhId;
    // node desc on sink node
    std::string nodeDesc;

    bool operator==(const BeRegNodeInfo &node)
    {
        return (srcId == node.srcId) && (dhId == node.dhId) && (nodeDesc == node.nodeDesc);
    }

    bool operator<(const BeRegNodeInfo &node) const
    {
        return (srcId + dhId + nodeDesc).compare(node.srcId + node.dhId + node.nodeDesc) < 0;
    }
} BeRegNodeInfo;

class DistributedInputSourceManager : public SystemAbility, public DistributedInputSourceStub {
    DECLARE_SYSTEM_ABILITY(DistributedInputSourceManager)

typedef struct InputDeviceId {
    std::string devId;
    std::string dhId;
    std::string nodeDesc;

    bool operator==(const InputDeviceId &inputId)
    {
        return (devId == inputId.devId) && (dhId == inputId.dhId) && (nodeDesc == inputId.nodeDesc);
    }
} InputDeviceId;

public:
    DistributedInputSourceManager(int32_t saId, bool runOnCreate);
    ~DistributedInputSourceManager();

    void OnStart() override;

    void OnStop() override;

    virtual int32_t Init() override;

    virtual int32_t Release() override;

    virtual int32_t RegisterDistributedHardware(const std::string& devId, const std::string& dhId,
        const std::string& parameters, sptr<IRegisterDInputCallback> callback) override;

    virtual int32_t UnregisterDistributedHardware(const std::string& devId, const std::string& dhId,
        sptr<IUnregisterDInputCallback> callback) override;

    virtual int32_t PrepareRemoteInput(const std::string &deviceId, sptr<IPrepareDInputCallback> callback) override;

    virtual int32_t UnprepareRemoteInput(const std::string &deviceId, sptr<IUnprepareDInputCallback> callback) override;

    virtual int32_t StartRemoteInput(
        const std::string& deviceId, const uint32_t& inputTypes, sptr<IStartDInputCallback> callback) override;

    virtual int32_t StopRemoteInput(
        const std::string& deviceId, const uint32_t& inputTypes, sptr<IStopDInputCallback> callback) override;

    virtual int32_t StartRemoteInput(const std::string &srcId, const std::string &sinkId, const uint32_t &inputTypes,
        sptr<IStartDInputCallback> callback) override;

    virtual int32_t StopRemoteInput(const std::string &srcId, const std::string &sinkId, const uint32_t &inputTypes,
        sptr<IStopDInputCallback> callback) override;

    virtual int32_t PrepareRemoteInput(const std::string &srcId, const std::string &sinkId,
        sptr<IPrepareDInputCallback> callback) override;

    virtual int32_t UnprepareRemoteInput(const std::string &srcId, const std::string &sinkId,
        sptr<IUnprepareDInputCallback> callback) override;

    virtual int32_t StartRemoteInput(const std::string &sinkId, const std::vector<std::string> &dhIds,
        sptr<IStartStopDInputsCallback> callback) override;

    virtual int32_t StopRemoteInput(const std::string &sinkId, const std::vector<std::string> &dhIds,
        sptr<IStartStopDInputsCallback> callback) override;

    virtual int32_t StartRemoteInput(const std::string &srcId, const std::string &sinkId,
        const std::vector<std::string> &dhIds, sptr<IStartStopDInputsCallback> callback) override;

    virtual int32_t StopRemoteInput(const std::string &srcId, const std::string &sinkId,
        const std::vector<std::string> &dhIds, sptr<IStartStopDInputsCallback> callback) override;

    virtual int32_t RegisterAddWhiteListCallback(sptr<IAddWhiteListInfosCallback> addWhiteListCallback) override;
    virtual int32_t RegisterDelWhiteListCallback(sptr<IDelWhiteListInfosCallback> delWhiteListCallback) override;
    virtual int32_t RegisterInputNodeListener(sptr<InputNodeListener> listener) override;
    virtual int32_t UnregisterInputNodeListener(sptr<InputNodeListener> listener) override;

    virtual int32_t SyncNodeInfoRemoteInput(const std::string &userDevId, const std::string &dhId,
        const std::string &nodeDesc) override;
    virtual int32_t RegisterSimulationEventListener(sptr<ISimulationEventListener> listener) override;
    virtual int32_t UnregisterSimulationEventListener(sptr<ISimulationEventListener> listener) override;

    int32_t Dump(int32_t fd, const std::vector<std::u16string>& args) override;

    class DInputSourceListener : public DInputSourceTransCallback {
    public:
        DInputSourceListener(DistributedInputSourceManager *manager);
        virtual ~DInputSourceListener();
        void onResponseRegisterDistributedHardware(const std::string deviceId, const std::string dhId, bool result);
        void onResponsePrepareRemoteInput(const std::string deviceId, bool result, const std::string &object);
        void onResponseUnprepareRemoteInput(const std::string deviceId, bool result);
        void onResponseStartRemoteInput(const std::string deviceId, const uint32_t inputTypes, bool result);
        void onResponseStopRemoteInput(const std::string deviceId, const uint32_t inputTypes, bool result);
        void onResponseStartRemoteInputDhid(const std::string deviceId, const std::string &dhids, bool result);
        void onResponseStopRemoteInputDhid(const std::string deviceId, const std::string &dhids, bool result);
        void onResponseKeyState(const std::string deviceId, const std::string &dhid, const uint32_t type,
            const uint32_t code, const uint32_t value);
        void onReceivedEventRemoteInput(const std::string deviceId, const std::string &event);
        void RecordEventLog(int64_t when, int32_t type, int32_t code, int32_t value, const std::string& path);

    private:
        DistributedInputSourceManager *sourceManagerObj_;
    };

    class DInputSourceManagerEventHandler : public AppExecFwk::EventHandler {
    public:
        DInputSourceManagerEventHandler(const std::shared_ptr<AppExecFwk::EventRunner> &runner,
            DistributedInputSourceManager *manager);
        ~DInputSourceManagerEventHandler() {}

        void ProcessEvent(const AppExecFwk::InnerEvent::Pointer &event) override;
    private:
        void NotifyRegisterCallback(const AppExecFwk::InnerEvent::Pointer &event);
        void NotifyUnregisterCallback(const AppExecFwk::InnerEvent::Pointer &event);
        void NotifyPrepareCallback(const AppExecFwk::InnerEvent::Pointer &event);
        void NotifyUnprepareCallback(const AppExecFwk::InnerEvent::Pointer &event);
        void NotifyStartCallback(const AppExecFwk::InnerEvent::Pointer &event);
        void NotifyStopCallback(const AppExecFwk::InnerEvent::Pointer &event);
        void NotifyStartDhidCallback(const AppExecFwk::InnerEvent::Pointer &event);
        void NotifyStopDhidCallback(const AppExecFwk::InnerEvent::Pointer &event);
        void NotifyKeyStateCallback(const AppExecFwk::InnerEvent::Pointer &event);
        void NotifyStartServerCallback(const AppExecFwk::InnerEvent::Pointer &event);

        DistributedInputSourceManager *sourceManagerObj_;
    };

    class StartDScreenListener : public PublisherListenerStub {
    public:
        StartDScreenListener();
        ~StartDScreenListener();
        void OnMessage(const DHTopic topic, const std::string& message) override;

    private:
        int32_t ParseMessage(const std::string& message, std::string& sinkDevId, SrcScreenInfo& srcScreenInfo);
        int32_t UpdateSrcScreenInfoCache(const SrcScreenInfo& TmpInfo);
    };

    class StopDScreenListener : public PublisherListenerStub {
    public:
        StopDScreenListener();
        ~StopDScreenListener();
        void OnMessage(const DHTopic topic, const std::string& message) override;

    private:
        int32_t ParseMessage(const std::string& message, std::string& sinkDevId, uint64_t& sourceWinId);
    };

    class DeviceOfflineListener : public PublisherListenerStub {
    public:
        DeviceOfflineListener(DistributedInputSourceManager* srcManagerContext);

        ~DeviceOfflineListener();

        void OnMessage(const DHTopic topic, const std::string& message);

    private:
        void DeleteNodeInfoAndNotify(const std::string& offlineDevId);

    private:
        DistributedInputSourceManager* sourceManagerContext_;
    };

    class DScreenSourceSvrRecipient : public IRemoteObject::DeathRecipient {
    public:
        DScreenSourceSvrRecipient(const std::string& srcDevId, const std::string& sinkDevId, const uint64_t srcWinId);
        ~DScreenSourceSvrRecipient();
        void OnRemoteDied(const wptr<IRemoteObject> &remote) override;

    private:
        std::string srcDevId_;
        std::string sinkDevId_;
        uint64_t srcWinId_;
    };

    std::shared_ptr<DInputSourceManagerEventHandler> GetCallbackEventHandler()
    {
        return callBackHandler_;
    };

public:
    void RunRegisterCallback(
        const std::string& devId, const std::string& dhId, const int32_t& status
    );
    void RunUnregisterCallback(
        const std::string& devId, const std::string& dhId, const int32_t& status
    );
    void RunPrepareCallback(
        const std::string& devId, const int32_t& status, const std::string& object
    );
    void RunUnprepareCallback(
        const std::string& devId, const int32_t& status
    );
    void RunStartCallback(
        const std::string& devId, const uint32_t& inputTypes, const int32_t& status
    );
    void RunStopCallback(
        const std::string& devId, const uint32_t& inputTypes, const int32_t& status
    );

    void RunStartDhidCallback(const std::string &sinkId, const std::string &dhIds, const int32_t &status);
    void RunStopDhidCallback(const std::string &sinkId, const std::string &dhIds, const int32_t &status);
    void RunKeyStateCallback(const std::string &sinkId, const std::string &dhId, const uint32_t type,
        const uint32_t code, const uint32_t value);

    DInputServerType GetStartTransFlag();
    void SetStartTransFlag(const DInputServerType flag);
    std::vector<InputDeviceId> GetInputDeviceId();
    void RemoveInputDeviceId(const std::string deviceId, const std::string dhId);
    void SetDeviceMapValue(const std::string deviceId, int32_t value);
    bool GetDeviceMapAllDevSwitchOff();
    int32_t RemoveInputNode(const std::string& devId, const std::string& dhId);
    int32_t DeleteDevice(const std::string& devId, const std::string& dhId);
    void SetInputTypesMap(const std::string deviceId, uint32_t value);
    uint32_t GetInputTypesMap(const std::string deviceId);
    uint32_t GetAllInputTypesMap();

private:
    struct DInputClientRegistInfo {
        std::string devId;
        std::string dhId;
        sptr<IRegisterDInputCallback> callback = nullptr;
    };

    struct DInputClientUnregistInfo {
        std::string devId;
        std::string dhId;
        sptr<IUnregisterDInputCallback> callback = nullptr;
    };

    struct DInputClientPrepareInfo {
        std::string devId;
        sptr<IPrepareDInputCallback> preCallback = nullptr;

        DInputClientPrepareInfo(std::string deviceId, sptr<IPrepareDInputCallback> prepareCallback)
            : devId(deviceId), preCallback(prepareCallback) {}
    };

    struct DInputClientUnprepareInfo {
        std::string devId;
        sptr<IUnprepareDInputCallback> unpreCallback = nullptr;
    };

    struct DInputClientStartInfo {
        std::string devId;
        uint32_t inputTypes;
        sptr<IStartDInputCallback> callback = nullptr;
        DInputClientStartInfo(std::string deviceId, uint32_t types, sptr<IStartDInputCallback> cb)
            : devId(deviceId), inputTypes(types), callback(cb) {}
    };

    struct DInputClientStopInfo {
        std::string devId;
        uint32_t inputTypes;
        sptr<IStopDInputCallback> callback = nullptr;
        DInputClientStopInfo(std::string deviceId, uint32_t types, sptr<IStopDInputCallback> cb)
            : devId(deviceId), inputTypes(types), callback(cb) {}
    };
    // add new prepare/start function
    struct DInputClientStartDhidInfo {
        std::string srcId;
        std::string sinkId;
        std::vector<std::string> dhIds;
        sptr<IStartStopDInputsCallback> callback = nullptr;
    };
    struct DInputClientStopDhidInfo {
        std::string srcId;
        std::string sinkId;
        std::vector<std::string> dhIds;
        sptr<IStartStopDInputsCallback> callback = nullptr;
    };

    ServiceSourceRunningState serviceRunningState_ = ServiceSourceRunningState::STATE_NOT_START;
    DInputServerType isStartTrans_ = DInputServerType::NULL_SERVER_TYPE;
    std::shared_ptr<DistributedInputSourceManager::DInputSourceListener> statuslistener_;

    std::vector<DInputClientRegistInfo> regCallbacks_;
    std::vector<DInputClientUnregistInfo> unregCallbacks_;
    std::vector<DInputClientPrepareInfo> preCallbacks_;
    std::vector<DInputClientUnprepareInfo> unpreCallbacks_;
    std::vector<DInputClientStartInfo> staCallbacks_;
    std::vector<DInputClientStopInfo> stpCallbacks_;

    std::vector<DInputClientStartDhidInfo> staStringCallbacks_;
    std::vector<DInputClientStopDhidInfo> stpStringCallbacks_;

    std::set<sptr<IAddWhiteListInfosCallback>> addWhiteListCallbacks_;
    std::set<sptr<IDelWhiteListInfosCallback>> delWhiteListCallbacks_;
    std::set<sptr<ISimulationEventListener>> simulationEventCallbacks_;

    std::map<std::string, int32_t> DeviceMap_;
    std::map<std::string, uint32_t> InputTypesMap_;
    std::shared_ptr<AppExecFwk::EventRunner> runner_;
    std::shared_ptr<DistributedInputSourceEventHandler> handler_;
    std::shared_ptr<DInputSourceManagerEventHandler> callBackHandler_;
    std::vector<InputDeviceId> inputDevice_;
    bool InitAuto();
    void handleStartServerCallback(const std::string& devId);
    std::mutex mutex_;
    std::mutex operationMutex_;
    sptr<StartDScreenListener> startDScreenListener_ = nullptr;
    sptr<StopDScreenListener> stopDScreenListener_ = nullptr;
    sptr<DeviceOfflineListener> deviceOfflineListener_ = nullptr;

    std::mutex valMutex_;
    std::mutex syncNodeInfoMutex_;
    std::map<std::string, std::set<BeRegNodeInfo>> syncNodeInfoMap_;
    int32_t RelayStartRemoteInputByType(const std::string &srcId, const std::string &sinkId, const uint32_t &inputTypes,
        sptr<IStartDInputCallback> callback);
    int32_t RelayStopRemoteInputByType(const std::string &srcId, const std::string &sinkId, const uint32_t &inputTypes,
        sptr<IStopDInputCallback> callback);
    int32_t RelayPrepareRemoteInput(const std::string &srcId, const std::string &sinkId,
        sptr<IPrepareDInputCallback> callback);
    int32_t RelayUnprepareRemoteInput(const std::string &srcId, const std::string &sinkId,
        sptr<IUnprepareDInputCallback> callback);
    int32_t RelayStartRemoteInputByDhid(const std::string &srcId, const std::string &sinkId,
        const std::vector<std::string> &dhIds, sptr<IStartStopDInputsCallback> callback);
    int32_t RelayStopRemoteInputByDhid(const std::string &srcId, const std::string &sinkId,
        const std::vector<std::string> &dhIds, sptr<IStartStopDInputsCallback> callback);
    bool IsStringDataSame(const std::vector<std::string> &oldDhIds, std::vector<std::string> newDhIds);
    void StringSplitToVector(const std::string &str, const char split, std::vector<std::string> &vecStr);
    void DeleteNodeInfoAndNotify(const std::string& offlineDevId);
    void SendExistVirNodeInfos(sptr<InputNodeListener> listener);
    std::set<BeRegNodeInfo> GetSyncNodeInfo(const std::string& devId);
    void UpdateSyncNodeInfo(const std::string& devId, const std::string& dhId, const std::string &nodeDesc);
    void DeleteSyncNodeInfo(const std::string& devId);

private:
    IDInputDBGItf* dinputDbgItfPtr_ = nullptr;
    void InitDinputDBG();
};
} // namespace DistributedInput
} // namespace DistributedHardware
} // namespace OHOS

#endif // DISTRIBUTED_INPUT_SOURCE_MANAGER_SERVICE_H
