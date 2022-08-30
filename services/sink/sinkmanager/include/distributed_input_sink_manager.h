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

#ifndef DISTRIBUTED_INPUT_SINK_MANAGER_SERVICE_H
#define DISTRIBUTED_INPUT_SINK_MANAGER_SERVICE_H

#include <cstring>
#include <set>
#include <map>
#include <mutex>
#include <sys/types.h>
#include <unistd.h>

#include "event_handler.h"
#include "ipublisher_listener.h"
#include "publisher_listener_stub.h"
#include "screen.h"
#include "singleton.h"
#include "system_ability.h"

#include "constants_dinput.h"
#include "dinput_sink_trans_callback.h"
#include "distributed_input_sink_stub.h"
#include "distributed_input_sink_event_handler.h"

namespace OHOS {
namespace DistributedHardware {
namespace DistributedInput {
enum class ServiceSinkRunningState { STATE_NOT_START, STATE_RUNNING };

class DistributedInputSinkManager : public SystemAbility, public DistributedInputSinkStub {
    DECLARE_SYSTEM_ABILITY(DistributedInputSinkManager)

public:
    DistributedInputSinkManager(int32_t saId, bool runOnCreate);
    ~DistributedInputSinkManager();

    class DInputSinkListener : public DInputSinkTransCallback {
    public:
        DInputSinkListener(DistributedInputSinkManager *manager);
        ~DInputSinkListener();
        void onPrepareRemoteInput(const int32_t& sessionId, const std::string &deviceId);
        void onUnprepareRemoteInput(const int32_t& sessionId);
        void onStartRemoteInput(const int32_t& sessionId, const uint32_t& inputTypes);
        void onStopRemoteInput(const int32_t& sessionId, const uint32_t& inputTypes);
        void onStartRemoteInputDhid(const int32_t &sessionId, const std::string &strDhids);
        void onStopRemoteInputDhid(const int32_t &sessionId, const std::string &strDhids);

    private:
        DistributedInputSinkManager *sinkManagerObj_;
        static inline int bit_is_set(const unsigned long *array, int bit)
        {
            return !!(array[bit / LONG_BITS] & (1LL << (bit % LONG_BITS)));
        }
        void SleepTimeMs();
        void StringSplitToSet(const std::string &str, const char split, std::set<std::string> &vecStr);
        void CheckKeyState(const int32_t &sessionId, const std::string &strDhids);
    };

    class ProjectWindowListener : public PublisherListenerStub {
    public:
        ProjectWindowListener();
        ~ProjectWindowListener();
        void OnMessage(const DHTopic topic, const std::string& message) override;

    private:
        int32_t ParseMessage(const std::string& message, std::string& srcDeviceId, uint64_t& srcWinId,
            SinkScreenInfo& sinkScreenInfo);
        int32_t UpdateSinkScreenInfoCache(const std::string& srcDevId, const uint64_t srcWinId,
            const SinkScreenInfo& sinkScreenInfoTmp);
        uint32_t GetScreenWidth();
        uint32_t GetScreenHeight();

    private:
        sptr<Rosen::Screen> screen_;
        std::mutex handleScreenMutex_;
    };

public:
    void OnStart() override;

    void OnStop() override;

    virtual int32_t Init() override;

    virtual int32_t Release() override;

    DInputServerType GetStartTransFlag();

    void SetStartTransFlag(const DInputServerType flag);

    uint32_t GetInputTypes();

    void SetInputTypes(const uint32_t& inputTypess);

    /*
     * GetEventHandler, get the ui_service manager service's handler.
     *
     * @return Returns EventHandler ptr.
     */
    std::shared_ptr<DistributedInputSinkEventHandler> GetEventHandler();

    int32_t NotifyStartDScreen(const SrcScreenInfo& srcScreenInfo) override;

    int32_t NotifyStopDScreen(const std::string& srcScreenInfoKey) override;

    int32_t RegisterSharingDhIdListener(sptr<ISharingDhIdListener> sharingDhIdListener) override;

    int32_t Dump(int32_t fd, const std::vector<std::u16string>& args) override;

private:
    void CleanExceptionalInfo(const SrcScreenInfo& srcScreenInfo);

private:
    ServiceSinkRunningState serviceRunningState_ = ServiceSinkRunningState::STATE_NOT_START;
    DInputServerType isStartTrans_ = DInputServerType::NULL_SERVER_TYPE;
    std::shared_ptr<DistributedInputSinkManager::DInputSinkListener> statuslistener_;

    std::shared_ptr<AppExecFwk::EventRunner> runner_;
    std::shared_ptr<DistributedInputSinkEventHandler> handler_;
    std::mutex mutex_;
    bool InitAuto();
    DInputDeviceType inputTypes_;
    sptr<ProjectWindowListener> projectWindowListener_ = nullptr;
    std::set<std::string> sharingDhIds_;
    std::map<int32_t, std::set<std::string>> sharingDhIdsMap_;
    void StoreStartDhids(int32_t sessionId, const std::set<std::string> &dhIds);
    void DeleteStopDhids(int32_t sessionId, const std::set<std::string> delDhIds, std::vector<std::string> &stopDhIds);
    bool IsDeleteDhidExist(int32_t sessionId, const std::string &delstr);
};
} // namespace DistributedInput
} // namespace DistributedHardware
} // namespace OHOS

#endif // DISTRIBUTED_INPUT_SINK_MANAGER_SERVICE_H
