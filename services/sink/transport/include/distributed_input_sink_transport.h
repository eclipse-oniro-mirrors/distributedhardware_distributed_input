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

#ifndef DISTRIBUTED_INPUT_SINK_TRANSPORT_H
#define DISTRIBUTED_INPUT_SINK_TRANSPORT_H

#include <mutex>
#include <set>
#include <string>
#include <vector>

#include "constants.h"
#include "event_handler.h"
#include "nlohmann/json.hpp"

#include "dinput_sink_trans_callback.h"
#include "dinput_softbus_define.h"
#include "distributed_input_sink_switch.h"

namespace OHOS {
namespace DistributedHardware {
namespace DistributedInput {
class DistributedInputSinkTransport {
public:
    static DistributedInputSinkTransport &GetInstance();
    DistributedInputSinkTransport();
    ~DistributedInputSinkTransport();

    int32_t Init();

    void RegistSinkRespCallback(std::shared_ptr<DInputSinkTransCallback> callback);
    int32_t RespPrepareRemoteInput(const int32_t sessionId, std::string &smsg);
    int32_t RespUnprepareRemoteInput(const int32_t sessionId, std::string &smsg);
    int32_t RespStartRemoteInput(const int32_t sessionId, std::string &smsg);
    int32_t RespStopRemoteInput(const int32_t sessionId, std::string &smsg);
    int32_t RespLatency(const int32_t sessionId, std::string &smsg);

    int32_t OnSessionOpened(int32_t sessionId, int32_t result);
    void OnSessionClosed(int32_t sessionId);
    void OnBytesReceived(int32_t sessionId, const void *data, uint32_t dataLen);

    class DInputSinkEventHandler : public AppExecFwk::EventHandler {
    public:
        DInputSinkEventHandler(const std::shared_ptr<AppExecFwk::EventRunner> &runner);
        ~DInputSinkEventHandler() {}

        void ProcessEvent(const AppExecFwk::InnerEvent::Pointer &event) override;
        void RecordEventLog(const std::shared_ptr<nlohmann::json> &events);
    };

    std::shared_ptr<DistributedInputSinkTransport::DInputSinkEventHandler> GetEventHandler();
    void CloseAllSession();

private:
    int32_t SendMessage(int32_t sessionId, std::string &message);
    void HandleSessionData(int32_t sessionId, const std::string& messageData);
    void NotifyPrepareRemoteInput(int32_t sessionId, const nlohmann::json &recMsg);
    void NotifyUnprepareRemoteInput(int32_t sessionId, const nlohmann::json &recMsg);
    void NotifyStartRemoteInput(int32_t sessionId, const nlohmann::json &recMsg);
    void NotifyStopRemoteInput(int32_t sessionId, const nlohmann::json &recMsg);
    void NotifyLatency(int32_t sessionId, const nlohmann::json &recMsg);

private:
    std::string deviceId_;
    std::string mySessionName_;

    std::shared_ptr<DistributedInputSinkTransport::DInputSinkEventHandler> eventHandler_;
    std::shared_ptr<DInputSinkTransCallback> callback_;
};
}  // namespace DistributedInput
}  // namespace DistributedHardware
}  // namespace OHOS

#endif  // DISTRIBUTED_INPUT_SINK_TRANSPORT_H
