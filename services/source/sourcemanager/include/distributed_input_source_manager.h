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

#ifndef DISRIBUTED_INPUT_SOURCE_MANAGER_SERVICE_H
#define DISRIBUTED_INPUT_SOURCE_MANAGER_SERVICE_H

#include <cstring>
#include <mutex>
#include <set>

#include <unistd.h>
#include <sys/types.h>

#include "event_handler.h"
#include "singleton.h"
#include "system_ability.h"

#include "constants_dinput.h"
#include "dinput_source_trans_callback.h"
#include "distributed_input_source_stub.h"
#include "distributed_input_source_event_handler.h"

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
const uint32_t DINPUT_SOURCE_MANAGER_RECEIVE_DATA_MSG = 7;
const uint32_t DINPUT_SOURCE_MANAGER_STARTSERVER_MSG = 8;
const std::string INPUT_SOURCEMANAGER_KEY_DEVID = "deviceId";
const std::string INPUT_SOURCEMANAGER_KEY_HWID = "hardwareId";
const std::string INPUT_SOURCEMANAGER_KEY_ITP = "inputTypes";
const std::string INPUT_SOURCEMANAGER_KEY_RESULT = "result";
const std::string INPUT_SOURCEMANAGER_KEY_WHITELIST = "whitelist";
const uint32_t DINPUT_SOURCE_SWITCH_OFF = 0;
const uint32_t DINPUT_SOURCE_SWITCH_ON = 1;

class DistributedInputSourceManager : public SystemAbility, public DistributedInputSourceStub {
    DECLARE_SYSTEM_ABILITY(DistributedInputSourceManager)

typedef struct InputDeviceId {
    std::string devId;
    std::string dhId;

    bool operator==(const InputDeviceId &inputId)
    {
        return (devId == inputId.devId) && (dhId == inputId.dhId);
    }
} InputDeviceId;

public:
    DistributedInputSourceManager(int32_t saId, bool runOnCreate);
    ~DistributedInputSourceManager() = default;

    void OnStart() override;

    void OnStop() override;

    virtual int32_t Init() override;

    virtual int32_t Release() override;

    virtual int32_t RegisterDistributedHardware(const std::string& devId, const std::string& dhId,
        const std::string& parameters, sptr<IRegisterDInputCallback> callback) override;

    virtual int32_t UnregisterDistributedHardware(const std::string& devId, const std::string& dhId,
        sptr<IUnregisterDInputCallback> callback) override;

    virtual int32_t PrepareRemoteInput(const std::string& deviceId,
        sptr<IPrepareDInputCallback> callback, sptr<IAddWhiteListInfosCallback> addWhiteListCallback)  override;

    virtual int32_t UnprepareRemoteInput(const std::string& deviceId,
        sptr<IUnprepareDInputCallback> callback, sptr<IDelWhiteListInfosCallback> delWhiteListCallback) override;

    virtual int32_t StartRemoteInput(
        const std::string& deviceId, const uint32_t& inputTypes, sptr<IStartDInputCallback> callback) override;

    virtual int32_t StopRemoteInput(
        const std::string& deviceId, const uint32_t& inputTypes, sptr<IStopDInputCallback> callback) override;

    virtual int32_t IsStartDistributedInput(
        const uint32_t& inputType, sptr<IStartDInputServerCallback> callback) override;

    class DInputSourceListener : public DInputSourceTransCallback {
    public:
        DInputSourceListener(DistributedInputSourceManager *manager);
        virtual ~DInputSourceListener();
        void onResponseRegisterDistributedHardware(const std::string deviceId, const std::string dhId, bool result);
        void onResponsePrepareRemoteInput(const std::string deviceId, bool result, const std::string &object);
        void onResponseUnprepareRemoteInput(const std::string deviceId, bool result);
        void onResponseStartRemoteInput(const std::string deviceId, const uint32_t inputTypes, bool result);
        void onResponseStopRemoteInput(const std::string deviceId, const uint32_t inputTypes, bool result);
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
        void NotifyStartServerCallback(const AppExecFwk::InnerEvent::Pointer &event);

        DistributedInputSourceManager *sourceManagerObj_;
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

    IStartDInputServerCallback* GetStartDInputServerCback();
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
        sptr<IAddWhiteListInfosCallback> addWhiteListCallback = nullptr;

        DInputClientPrepareInfo(std::string deviceId, sptr<IPrepareDInputCallback> prepareCallback,
            sptr<IAddWhiteListInfosCallback> addWhiteListCallback) : devId(deviceId), preCallback(prepareCallback),
            addWhiteListCallback(addWhiteListCallback) {}
    };

    struct DInputClientUnprepareInfo {
        std::string devId;
        sptr<IUnprepareDInputCallback> unpreCallback = nullptr;
        sptr<IDelWhiteListInfosCallback>  delWhiteListCallback = nullptr;
    };

    struct DInputClientStartInfo {
        std::string devId;
        uint32_t inputTypes;
        sptr<IStartDInputCallback> callback = nullptr;
    };

    struct DInputClientStopInfo {
        std::string devId;
        uint32_t inputTypes;
        sptr<IStopDInputCallback> callback = nullptr;
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
    sptr<IStartDInputServerCallback>  startServerCallback_ = nullptr;

    std::map<std::string, int32_t> DeviceMap_;
    std::map<std::string, uint32_t> InputTypesMap_;
    std::shared_ptr<AppExecFwk::EventRunner> runner_;
    std::shared_ptr<DistributedInputSourceEventHandler> handler_;
    std::shared_ptr<DInputSourceManagerEventHandler> callBackHandler_;
    std::vector<InputDeviceId> inputDevice_;
    bool InitAuto();
    void handleStartServerCallback(const std::string& devId);
};
} // namespace DistributedInput
} // namespace DistributedHardware
} // namespace OHOS

#endif // DISRIBUTED_INPUT_SOURCE_MANAGER_SERVICE_H
