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

#include "distributed_input_node_manager.h"

#include <cinttypes>
#include <cstring>
#include <fcntl.h>
#include <unistd.h>

#include "distributed_hardware_log.h"
#include "nlohmann/json.hpp"
#include "virtual_keyboard.h"
#include "virtual_mouse.h"
#include "virtual_touchpad.h"

namespace OHOS {
namespace DistributedHardware {
namespace DistributedInput {
namespace {
static const uint32_t INPUT_DEVICE_CLASS_KEYBOARD = static_cast<uint32_t>(DeviceClasses::INPUT_DEVICE_CLASS_KEYBOARD);
static const uint32_t INPUT_DEVICE_CLASS_CURSOR = static_cast<uint32_t>(DeviceClasses::INPUT_DEVICE_CLASS_CURSOR);
static const uint32_t INPUT_DEVICE_CLASS_TOUCH = static_cast<uint32_t>(DeviceClasses::INPUT_DEVICE_CLASS_TOUCH);
}

DistributedInputNodeManager::DistributedInputNodeManager() : isInjectThreadRunning_(false)
{
}

DistributedInputNodeManager::~DistributedInputNodeManager()
{
    CloseAllDevicesLocked();
    isInjectThreadRunning_.store(false);
    if (eventInjectThread_.joinable()) {
        eventInjectThread_.join();
    }
}

int32_t DistributedInputNodeManager::openDevicesNode(const std::string& devId, const std::string& dhId,
    const std::string& parameters)
{
    InputDevice event;
    stringTransJsonTransStruct(parameters, event);
    if (CreateHandle(event, devId) < 0) {
        return FAILURE;
    }

    return SUCCESS;
}

void DistributedInputNodeManager::stringTransJsonTransStruct(const std::string& str, InputDevice& pBuf)
{
    nlohmann::json recMsg = nlohmann::json::parse(str);
    recMsg.at("name").get_to(pBuf.name);
    recMsg.at("location").get_to(pBuf.location);
    recMsg.at("uniqueId").get_to(pBuf.uniqueId);
    recMsg.at("bus").get_to(pBuf.bus);
    recMsg.at("vendor").get_to(pBuf.vendor);
    recMsg.at("product").get_to(pBuf.product);
    recMsg.at("version").get_to(pBuf.version);
    recMsg.at("descriptor").get_to(pBuf.descriptor);
    recMsg.at("nonce").get_to(pBuf.nonce);
    recMsg.at("classes").get_to(pBuf.classes);
}

int32_t DistributedInputNodeManager::CreateHandle(InputDevice event, const std::string& devId)
{
    std::unique_lock<std::mutex> my_lock(operationMutex_);
    std::unique_ptr<VirtualDevice> device;
    if (event.classes & INPUT_DEVICE_CLASS_KEYBOARD) {
        device = std::make_unique<VirtualKeyboard>(event.name, event.bus, event.vendor, event.product, event.version);
    } else if (event.classes & INPUT_DEVICE_CLASS_CURSOR) {
        device = std::make_unique<VirtualMouse>(event.name, event.bus, event.vendor, event.product, event.version);
    } else if (event.classes & INPUT_DEVICE_CLASS_TOUCH) {
        device = std::make_unique<VirtualTouchpad>(event.name, event.bus, event.vendor, event.product, event.version);
    } else {
        DHLOGW("could not find the deviceType\n");
        return FAILURE;
    }

    if (device == nullptr) {
        DHLOGE("could not create new virtual device == null\n");
        return FAILURE;
    }

    device->SetNetWorkId(devId);

    if (!device->SetUp()) {
        DHLOGE("could not create new virtual device\n");
        return FAILURE;
    }
    AddDeviceLocked(event.descriptor, std::move(device));
    return SUCCESS;
}

void DistributedInputNodeManager::AddDeviceLocked(const std::string& dhId, std::unique_ptr<VirtualDevice> device)
{
    auto [dev_it, inserted] = devices_.insert_or_assign(dhId, std::move(device));
    if (!inserted) {
        DHLOGI("Device id %s exists, replaced. \n", dhId.c_str());
    }
}

int32_t DistributedInputNodeManager::CloseDeviceLocked(const std::string &dhId)
{
    DHLOGI("%s called, dhId=%s", __func__, dhId.c_str());
    std::map<std::string, std::unique_ptr<VirtualDevice>>::iterator iter = devices_.find(dhId);
    if (iter != devices_.end()) {
        devices_.erase(iter);
        return SUCCESS;
    }
    DHLOGE("%s called failure, dhId=%s", __func__, dhId.c_str());
    return FAILURE;
}

void DistributedInputNodeManager::CloseAllDevicesLocked()
{
    for (const auto & [id, virDevice] : devices_) {
        CloseDeviceLocked(id);
    }
}

int32_t DistributedInputNodeManager::getDevice(const std::string& dhId, VirtualDevice*& device)
{
    for (const auto & [id, virDevice] : devices_) {
        if (id == dhId) {
            device = virDevice.get();
            return SUCCESS;
        }
    }
    return FAILURE;
}

void DistributedInputNodeManager::StartInjectThread()
{
    DHLOGI("start");
    isInjectThreadRunning_.store(true);
    eventInjectThread_ = std::thread(&DistributedInputNodeManager::InjectEvent, this);
}

void DistributedInputNodeManager::StopInjectThread()
{
    DHLOGI("start");
    isInjectThreadRunning_.store(false);
    if (eventInjectThread_.joinable()) {
        eventInjectThread_.join();
    }
}

void DistributedInputNodeManager::ReportEvent(const RawEvent rawEvent)
{
    std::lock_guard<std::mutex> lockGuard(mutex_);
    injectQueue_.push(std::make_shared<RawEvent>(rawEvent));
    conditionVariable_.notify_all();
}

void DistributedInputNodeManager::InjectEvent()
{
    DHLOGI("start");
    while (isInjectThreadRunning_.load()) {
        std::shared_ptr<RawEvent> event = nullptr;
        {
            std::unique_lock<std::mutex> waitEventLock(mutex_);
            conditionVariable_.wait(waitEventLock, [this] () { return !injectQueue_.empty(); });
            if (injectQueue_.empty()) {
                continue;
            }
            event = injectQueue_.front();
            injectQueue_.pop();
        }
        if (event == nullptr) {
            DHLOGD("event is null!");
            continue;
        }

        DHLOGD("process event, inject queue size: %zu", injectQueue_.size());
        ProcessInjectEvent(event);
    }
}

void DistributedInputNodeManager::ProcessInjectEvent(const std::shared_ptr<RawEvent> &rawEvent)
{
    std::string dhId = rawEvent->descriptor;
    struct input_event event = {
        .type = rawEvent->type,
        .code = rawEvent->code,
        .value = rawEvent->value
    };
    DHLOGI("InjectEvent dhId: %s, eventType: %d, eventCode: %d, eventValue: %d, when: " PRId64"",
        dhId.c_str(), event.type, event.code, event.value, rawEvent->when);
    VirtualDevice* device = nullptr;
    if (getDevice(dhId, device) < 0) {
        DHLOGE("could not find the device");
        return;
    }
    if (device != nullptr) {
        device->InjectInputEvent(event);
    }
}
} // namespace DistributedInput
} // namespace DistributedHardware
} // namespace OHOS
