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
#include <fcntl.h>
#include <cstring>
#include <unistd.h>
#include "nlohmann/json.hpp"
#include "virtual_keyboard.h"
#include "virtual_mouse.h"
#include "virtual_touchpad.h"
#include "distributed_hardware_log.h"

namespace OHOS {
namespace DistributedHardware {
namespace DistributedInput {
namespace {
static const uint32_t INPUT_DEVICE_CLASS_KEYBOARD = static_cast<uint32_t>(DeviceClasses::INPUT_DEVICE_CLASS_KEYBOARD);
static const uint32_t INPUT_DEVICE_CLASS_CURSOR = static_cast<uint32_t>(DeviceClasses::INPUT_DEVICE_CLASS_CURSOR);
static const uint32_t INPUT_DEVICE_CLASS_TOUCH = static_cast<uint32_t>(DeviceClasses::INPUT_DEVICE_CLASS_TOUCH);
}

Distributed_input_node_manager::Distributed_input_node_manager()
{
}

Distributed_input_node_manager::~Distributed_input_node_manager()
{
    CloseAllDevicesLocked();
}

int32_t Distributed_input_node_manager::openDevicesNode(const std::string& devId, const std::string& dhId,
    const std::string& parameters)
{
    InputDevice event;
    stringTransJsonTransStruct(parameters, event);
    if (CreateHandle(event, devId) < 0) {
        return FAILURE;
    }

    return SUCCESS;
}

void Distributed_input_node_manager::stringTransJsonTransStruct(const std::string& str, InputDevice& pBuf)
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

int32_t Distributed_input_node_manager::CreateHandle(InputDevice event, const std::string& devId)
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

void Distributed_input_node_manager::AddDeviceLocked(const std::string& dhId, std::unique_ptr<VirtualDevice> device)
{
    auto [dev_it, inserted] = devices_.insert_or_assign(dhId, std::move(device));
    if (!inserted) {
        DHLOGI("Device id %s exists, replaced. \n", dhId.c_str());
    }
}

int32_t Distributed_input_node_manager::CloseDeviceLocked(const std::string &dhId)
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

void Distributed_input_node_manager::CloseAllDevicesLocked()
{
    for (const auto& [id, virdevice] : devices_) {
        CloseDeviceLocked(id);
    }
}

int32_t Distributed_input_node_manager::getDevice(const std::string& dhId, VirtualDevice*& device)
{
    for (const auto& [id, virdevice] : devices_) {
        if (id == dhId) {
            device = virdevice.get();
            return SUCCESS;
        }
    }
    return FAILURE;
}

void Distributed_input_node_manager::StartInjectThread()
{
    thread_ = std::thread(&Distributed_input_node_manager::InjectEvent, this);
}

void Distributed_input_node_manager::ReportEvent(const RawEvent rawEvent)
{
    std::lock_guard<std::mutex> lockGuard(mutex_);
    injectQueue_.push_back(rawEvent);
    conditionVariable_.notify_one();
}

void Distributed_input_node_manager::InjectEvent()
{
    std::unique_lock<std::mutex> uniqueLock(mutex_);
    while (true) {
        conditionVariable_.wait(uniqueLock);
        while (injectQueue_.size() > 0) {
            VirtualDevice* device;
            struct input_event event = {};
            std::string dhId = injectQueue_[0].descriptor;
            event.type = injectQueue_[0].type;
            event.code = injectQueue_[0].code;
            event.value = injectQueue_[0].value;
            DHLOGW("InjectEvent (dhId=%s)", dhId.c_str());
            DHLOGI("%d, %d, %d.\n", event.type, event.code, event.value);
            if (getDevice(dhId, device) < 0) {
                DHLOGE("could not find the device\n");
            }
            if (device != nullptr) {
                device->InjectInputEvent(event);
            }
            injectQueue_.erase(injectQueue_.begin());
        }
    }
}
} // namespace DistributedInput
} // namespace DistributedHardware
} // namespace OHOS
