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

#ifndef DISTRIBUTED_INPUT_COLLECTOR_H
#define DISTRIBUTED_INPUT_COLLECTOR_H

#include <atomic>
#include <map>
#include <mutex>
#include <string>

#include <sys/epoll.h>
#include <linux/input.h>

#include "event_handler.h"
#include "refbase.h"

#include "constants_dinput.h"
#include "input_hub.h"
#include "i_sharing_dhid_listener.h"

namespace OHOS {
namespace DistributedHardware {
namespace DistributedInput {
class DistributedInputCollector {
public:
    static DistributedInputCollector &GetInstance();
    int32_t Init(std::shared_ptr<AppExecFwk::EventHandler> sinkHandler);
    void Release();
    void SetSharingTypes(bool enabled, const uint32_t &inputType);
    void SetSharingDhIds(bool enabled, std::vector<std::string> dhIds);
    void GetMouseNodePath(std::vector<std::string> dhIds, std::string &mouseNodePath, std::string &dhid);
    // false for sharing device exist, true for all devices stop sharing
    bool IsAllDevicesStoped();
    int32_t RegisterSharingDhIdListener(sptr<ISharingDhIdListener> sharingDhIdListener);

private:
    DistributedInputCollector();
    ~DistributedInputCollector();

    bool InitCollectEventsThread();
    static void *CollectEventsThread(void *param);
    void StartCollectEventsThread();
    void StopCollectEventsThread();
    void ReportDhIdSharingState(const AffectDhIds &dhIds);

    RawEvent mEventBuffer[INPUT_EVENT_BUFFER_SIZE];
    pthread_t collectThreadID_;
    bool isCollectingEvents_;
    bool isStartGetDeviceHandlerThread;
    std::unique_ptr<InputHub> inputHub_;
    std::shared_ptr<AppExecFwk::EventHandler> sinkHandler_;
    uint32_t inputTypes_;

    std::mutex sharingDhIdListenerMtx_;
    sptr<ISharingDhIdListener> sharingDhIdListener_ = nullptr;
};
} // namespace DistributedInput
} // namespace DistributedHardware
} // namespace OHOS

#endif // DISTRIBUTED_INPUT_COLLECTOR_H