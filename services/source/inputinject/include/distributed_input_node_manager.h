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

#ifndef DISTRIBUTED_INPUT_NODE_MANAGER_H
#define DISTRIBUTED_INPUT_NODE_MANAGER_H

#include <string>
#include <condition_variable>
#include <mutex>
#include <map>
#include <thread>
#include "virtual_device.h"
#include "constants_dinput.h"

namespace OHOS {
namespace DistributedHardware {
namespace DistributedInput {
class Distributed_input_node_manager {
public:
    Distributed_input_node_manager();
    ~Distributed_input_node_manager();

    int32_t openDevicesNode(const std::string& devId, const std::string& dhId,
    const std::string& parameters);

    int32_t Init();

    int32_t getDevice(const std::string& dhId, VirtualDevice*& device);
    void ReportEvent(const RawEvent rawEvent);
    int32_t CloseDeviceLocked(const std::string& dhId);
    void StartInjectThread();

private:
    bool Release();
    void AddDeviceLocked(const std::string& dhId, std::unique_ptr<VirtualDevice> device);
    int32_t CreateHandle(InputDevice event, const std::string& devId);
    void CloseAllDevicesLocked();
    void stringTransJsonTransStruct(const std::string& str, InputDevice& pBuf);
    void InjectEvent();

    std::map<std::string, std::unique_ptr<VirtualDevice>> devices_;
    std::mutex operationMutex_;
    std::thread thread_;
    std::mutex mutex_;
    std::condition_variable conditionVariable_;
    std::vector<RawEvent> injectQueue_;
};
}  // namespace DistributedInput
}  // namespace DistributedHardware
}  // namespace OHOS

#endif  // DISTRIBUTED_INPUT_INJECT_H