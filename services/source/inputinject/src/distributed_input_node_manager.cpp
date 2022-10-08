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

#include "distributed_input_node_manager.h"

#include <cinttypes>
#include <cstring>

#include <fcntl.h>
#include <unistd.h>

#include "anonymous_string.h"
#include "distributed_hardware_log.h"
#include "nlohmann/json.hpp"

#include "softbus_bus_center.h"

#include "dinput_context.h"
#include "dinput_errcode.h"
#include "dinput_softbus_define.h"
#include "virtual_keyboard.h"
#include "virtual_mouse.h"
#include "virtual_touchpad.h"
#include "virtual_touchscreen.h"

namespace OHOS {
namespace DistributedHardware {
namespace DistributedInput {
DistributedInputNodeManager::DistributedInputNodeManager() : isInjectThreadRunning_(false),
    inputHub_(std::make_unique<InputHub>()), virtualTouchScreenFd_(UN_INIT_FD_VALUE)
{
}

DistributedInputNodeManager::~DistributedInputNodeManager()
{
    DHLOGI("destructor start");
    isInjectThreadRunning_.store(false);
    if (eventInjectThread_.joinable()) {
        eventInjectThread_.join();
    }
    {
        std::lock_guard<std::mutex> lock(virtualDeviceMapMutex_);
        virtualDeviceMap_.clear();
    }
    DHLOGI("destructor end");
}

int32_t DistributedInputNodeManager::openDevicesNode(const std::string& devId, const std::string& dhId,
    const std::string& parameters)
{
    InputDevice event;
    stringTransJsonTransStruct(parameters, event);
    if (CreateHandle(event, devId, dhId) < 0) {
        return ERR_DH_INPUT_SERVER_SOURCE_OPEN_DEVICE_NODE_FAIL;
    }

    return DH_SUCCESS;
}

void DistributedInputNodeManager::stringTransJsonTransStruct(const std::string& str, InputDevice& pBuf)
{
    nlohmann::json recMsg = nlohmann::json::parse(str, nullptr, false);
    if (recMsg.is_discarded()) {
        DHLOGE("recMsg parse failed!");
        return;
    }
    recMsg.at("name").get_to(pBuf.name);
    recMsg.at("physicalPath").get_to(pBuf.physicalPath);
    recMsg.at("uniqueId").get_to(pBuf.uniqueId);
    recMsg.at("bus").get_to(pBuf.bus);
    recMsg.at("vendor").get_to(pBuf.vendor);
    recMsg.at("product").get_to(pBuf.product);
    recMsg.at("version").get_to(pBuf.version);
    recMsg.at("descriptor").get_to(pBuf.descriptor);
    recMsg.at("classes").get_to(pBuf.classes);
}

int32_t DistributedInputNodeManager::CreateHandle(InputDevice event, const std::string& devId, const std::string& dhId)
{
    std::unique_lock<std::mutex> my_lock(operationMutex_);
    std::unique_ptr<VirtualDevice> device;
    if (event.classes & INPUT_DEVICE_CLASS_KEYBOARD) {
        device = std::make_unique<VirtualKeyboard>(event);
    } else if (event.classes & INPUT_DEVICE_CLASS_CURSOR) {
        device = std::make_unique<VirtualMouse>(event);
    } else if (event.classes & INPUT_DEVICE_CLASS_TOUCH_MT) {
        inputHub_->ScanInputDevices(DEVICE_PATH);
        LocalAbsInfo info = DInputContext::GetInstance().GetLocalTouchScreenInfo().localAbsInfo;
        device = std::make_unique<VirtualTouchScreen>(event, info, info.absMtPositionXMax, info.absMtPositionYMax);
    } else if (event.classes & INPUT_DEVICE_CLASS_TOUCH) {
        device = std::make_unique<VirtualTouchpad>(event);
    } else {
        DHLOGW("could not find the deviceType\n");
        return ERR_DH_INPUT_SERVER_SOURCE_CREATE_HANDLE_FAIL;
    }

    if (device == nullptr) {
        DHLOGE("could not create new virtual device == null\n");
        return ERR_DH_INPUT_SERVER_SOURCE_CREATE_HANDLE_FAIL;
    }

    device->SetNetWorkId(devId);

    if (!device->SetUp(devId, dhId)) {
        DHLOGE("could not create new virtual device\n");
        return ERR_DH_INPUT_SERVER_SOURCE_CREATE_HANDLE_FAIL;
    }
    AddDeviceLocked(event.descriptor, std::move(device));
    return DH_SUCCESS;
}

int32_t DistributedInputNodeManager::CreateVirtualTouchScreenNode(const std::string& devId, const std::string& dhId,
    const uint64_t srcWinId, const uint32_t sourcePhyWidth, const uint32_t sourcePhyHeight)
{
    std::unique_lock<std::mutex> my_lock(operationMutex_);
    std::unique_ptr<VirtualDevice> device;
    LocalAbsInfo info = DInputContext::GetInstance().GetLocalTouchScreenInfo().localAbsInfo;
    DHLOGI("CreateVirtualTouchScreenNode start, dhId: %s, sourcePhyWidth: %d, sourcePhyHeight: %d",
        GetAnonyString(dhId).c_str(), sourcePhyWidth, sourcePhyHeight);
    device = std::make_unique<VirtualTouchScreen>(info.deviceInfo, info, sourcePhyWidth - 1, sourcePhyHeight - 1);
    if (device == nullptr) {
        DHLOGE("could not create new virtual touch Screen");
        return ERR_DH_INPUT_SERVER_SOURCE_CREATE_HANDLE_FAIL;
    }
    if (!device->SetUp(devId, dhId)) {
        DHLOGE("Virtual touch Screen setUp fail, devId: %s, dhId: %s", GetAnonyString(devId).c_str(),
            GetAnonyString(dhId).c_str());
        return ERR_DH_INPUT_SERVER_SOURCE_CREATE_HANDLE_FAIL;
    }
    virtualTouchScreenFd_ = device->GetDeviceFd();
    AddDeviceLocked(dhId, std::move(device));
    DHLOGI("CreateVirtualTouchScreenNode end, dhId: %s", GetAnonyString(dhId).c_str());
    return DH_SUCCESS;
}

int32_t DistributedInputNodeManager::RemoveVirtualTouchScreenNode(const std::string& dhId)
{
    return CloseDeviceLocked(dhId);
}

int32_t DistributedInputNodeManager::GetVirtualTouchScreenFd()
{
    return virtualTouchScreenFd_;
}

void DistributedInputNodeManager::AddDeviceLocked(const std::string& dhId, std::unique_ptr<VirtualDevice> device)
{
    DHLOGI("dhId=%s", GetAnonyString(dhId).c_str());
    std::lock_guard<std::mutex> lock(virtualDeviceMapMutex_);
    auto [dev_it, inserted] = virtualDeviceMap_.insert_or_assign(dhId, std::move(device));
    if (!inserted) {
        DHLOGI("Device id %s exists, replaced. \n", GetAnonyString(dhId).c_str());
    }
}

int32_t DistributedInputNodeManager::CloseDeviceLocked(const std::string &dhId)
{
    DHLOGI("CloseDeviceLocked called, dhId=%s", GetAnonyString(dhId).c_str());
    std::lock_guard<std::mutex> lock(virtualDeviceMapMutex_);
    std::map<std::string, std::unique_ptr<VirtualDevice>>::iterator iter = virtualDeviceMap_.find(dhId);
    if (iter != virtualDeviceMap_.end()) {
        DHLOGI("CloseDeviceLocked called success, dhId=%s", GetAnonyString(dhId).c_str());
        virtualDeviceMap_.erase(iter);
        return DH_SUCCESS;
    }
    DHLOGE("CloseDeviceLocked called failure, dhId=%s", GetAnonyString(dhId).c_str());
    return ERR_DH_INPUT_SERVER_SOURCE_CLOSE_DEVICE_FAIL;
}

int32_t DistributedInputNodeManager::getDevice(const std::string& dhId, VirtualDevice*& device)
{
    std::lock_guard<std::mutex> lock(virtualDeviceMapMutex_);
    for (const auto& [id, virDevice] : virtualDeviceMap_) {
        if (id.compare(dhId) == 0) {
            device = virDevice.get();
            return DH_SUCCESS;
        }
    }
    return ERR_DH_INPUT_SERVER_SOURCE_GET_DEVICE_FAIL;
}

void DistributedInputNodeManager::StartInjectThread()
{
    DHLOGI("start");
    isInjectThreadRunning_.store(true);
    eventInjectThread_ = std::thread(&DistributedInputNodeManager::InjectEvent, this);
    DHLOGI("end");
}

void DistributedInputNodeManager::StopInjectThread()
{
    DHLOGI("start");
    isInjectThreadRunning_.store(false);
    conditionVariable_.notify_all();
    if (eventInjectThread_.joinable()) {
        eventInjectThread_.join();
    }
    DHLOGI("end");
}

void DistributedInputNodeManager::ReportEvent(const RawEvent rawEvent)
{
    std::lock_guard<std::mutex> lockGuard(injectThreadMutex_);
    injectQueue_.push(std::make_shared<RawEvent>(rawEvent));
    conditionVariable_.notify_all();
}

void DistributedInputNodeManager::InjectEvent()
{
    DHLOGI("start");
    while (isInjectThreadRunning_.load()) {
        std::shared_ptr<RawEvent> event = nullptr;
        {
            std::unique_lock<std::mutex> waitEventLock(injectThreadMutex_);
            conditionVariable_.wait(waitEventLock,
                [this]() { return !isInjectThreadRunning_.load() || !injectQueue_.empty(); });
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
    DHLOGI("end");
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
        GetAnonyString(dhId).c_str(), event.type, event.code, event.value, rawEvent->when);
    VirtualDevice* device = nullptr;
    if (getDevice(dhId, device) < 0) {
        DHLOGE("could not find the device");
        return;
    }
    if (device != nullptr) {
        device->InjectInputEvent(event);
    }
}

int32_t DistributedInputNodeManager::GetDeviceInfo(std::string &deviceId)
{
    std::unique_lock<std::mutex> my_lock(operationMutex_);
    auto localNode = std::make_unique<NodeBasicInfo>();
    int32_t retCode = GetLocalNodeDeviceInfo(DINPUT_PKG_NAME.c_str(), localNode.get());
    if (retCode != 0) {
        DHLOGE("Could not get device id.");
        return ERR_DH_INPUT_HANDLER_GET_DEVICE_ID_FAIL;
    }

    deviceId = localNode->networkId;
    DHLOGI("device id is %s", GetAnonyString(deviceId).c_str());
    return DH_SUCCESS;
}

void DistributedInputNodeManager::GetDevicesInfoByType(const std::string &networkId, uint32_t inputTypes,
    std::map<int32_t, std::string> &datas)
{
    uint32_t input_types_ = 0;

    if ((inputTypes & static_cast<uint32_t>(DInputDeviceType::MOUSE)) != 0) {
        input_types_ |= INPUT_DEVICE_CLASS_CURSOR;
    }

    if ((inputTypes & static_cast<uint32_t>(DInputDeviceType::KEYBOARD)) != 0) {
        input_types_ |= INPUT_DEVICE_CLASS_KEYBOARD;
    }

    if ((inputTypes & static_cast<uint32_t>(DInputDeviceType::MOUSE)) != 0) {
        input_types_ |= INPUT_DEVICE_CLASS_TOUCH;
    }

    std::lock_guard<std::mutex> lock(virtualDeviceMapMutex_);
    for (const auto &[id, virdevice] : virtualDeviceMap_) {
        if ((virdevice->GetDeviceType() & input_types_) && (virdevice->GetNetWorkId() == networkId)) {
            datas.insert(std::pair<int32_t, std::string>(virdevice->GetDeviceFd(), id));
        }
    }
}

void DistributedInputNodeManager::GetDevicesInfoByDhId(
    std::vector<std::string> dhidsVec, std::map<int32_t, std::string> &datas)
{
    for (auto dhId : dhidsVec) {
        std::lock_guard<std::mutex> lock(virtualDeviceMapMutex_);
        for (const auto &[id, virdevice] : virtualDeviceMap_) {
            if (id == dhId) {
                datas.insert(std::pair<int32_t, std::string>(virdevice->GetDeviceFd(), id));
                break;
            }
        }
    }
}
} // namespace DistributedInput
} // namespace DistributedHardware
} // namespace OHOS
