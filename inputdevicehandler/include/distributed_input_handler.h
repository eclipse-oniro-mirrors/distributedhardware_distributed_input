/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#ifndef DISTRIBUTED_INPUT_HANDLER_H
#define DISTRIBUTED_INPUT_HANDLER_H

#include <string>
#include <mutex>
#include <map>
#include <sys/epoll.h>
#include <functional>
#include <linux/input.h>
#include "ihardware_handler.h"
#include "single_instance.h"
#include "input_hub.h"
#include "constants_dinput.h"

namespace OHOS {
namespace DistributedHardware {
namespace DistributedInput {
class DistributedInputHandler : public IHardwareHandler {
DECLARE_SINGLE_INSTANCE_BASE(DistributedInputHandler);
public:
    virtual int32_t Initialize() override;
    virtual std::vector<DHItem> Query() override;
    virtual std::map<std::string, std::string> QueryExtraInfo() override;
    virtual bool IsSupportPlugin() override;
    virtual void RegisterPluginListener(std::shared_ptr<PluginListener> listener) override;
    virtual void UnRegisterPluginListener() override;

private:
    DistributedInputHandler();
    ~DistributedInputHandler();
    void StructTransJson(const InputDevice& pBuf, std::string& strDescriptor);
    int32_t GetDeviceInfo(std::string& deviceId);
    std::shared_ptr<PluginListener> m_listener;
    bool InitCollectEventsThread();
    void NotifyHardWare(int iCnt);

    pthread_t collectThreadID_;
    bool isCollectingEvents_;
    bool isStartCollectEventThread;
    static void *CollectEventsThread(void *param);
    void StartInputMonitorDeviceThread(const std::string deviceId);
    void StopInputMonitorDeviceThread();

    // The event queue.
    static const int INPUT_DEVICR_BUFFER_SIZE = 32;
    InputDeviceEvent mEventBuffer[INPUT_DEVICR_BUFFER_SIZE];
    std::mutex operationMutex_;
    std::unique_ptr<InputHub> inputHub_;
};

#ifdef __cplusplus
extern "C" {
#endif
__attribute__((visibility("default"))) IHardwareHandler* GetHardwareHandler();
#ifdef __cplusplus
}
#endif
}  // namespace DistributedInput
}  // namespace DistributedHardware
}  // namespace OHOS

#endif  // DISTRIBUTED_INPUT_HANDLER_H
