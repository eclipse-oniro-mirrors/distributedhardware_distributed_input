/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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
#include <pthread.h>
#include <unistd.h>

#include "anonymous_string.h"
#include "softbus_bus_center.h"

#include "dinput_context.h"
#include "dinput_errcode.h"
#include "dinput_log.h"
#include "dinput_softbus_define.h"
#include "dinput_utils_tool.h"

namespace OHOS {
namespace DistributedHardware {
namespace DistributedInput {
DistributedInputNodeManager::DistributedInputNodeManager() : isInjectThreadCreated_(false),
    isInjectThreadRunning_(false), inputHub_(std::make_unique<InputHub>()), virtualTouchScreenFd_(UN_INIT_FD_VALUE)
{
}

DistributedInputNodeManager::~DistributedInputNodeManager()
{
    DHLOGI("destructor start");
    isInjectThreadCreated_.store(false);
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
    if (devId.size() > DEV_ID_LENGTH_MAX || devId.empty() || dhId.size() > DH_ID_LENGTH_MAX || dhId.empty() ||
        parameters.size() > STRING_MAX_SIZE || parameters.empty()) {
        DHLOGE("Params is invalid!");
        return ERR_DH_INPUT_SERVER_SOURCE_OPEN_DEVICE_NODE_FAIL;
    }
    InputDevice event;
    ParseInputDeviceJson(parameters, event);
    if (CreateHandle(event, devId, dhId) < 0) {
        DHLOGE("Can not create virtual node!");
        return ERR_DH_INPUT_SERVER_SOURCE_OPEN_DEVICE_NODE_FAIL;
    }

    return DH_SUCCESS;
}

void DistributedInputNodeManager::ParseInputDeviceJson(const std::string& str, InputDevice& pBuf)
{
    nlohmann::json inputDeviceJson = nlohmann::json::parse(str, nullptr, false);
    if (inputDeviceJson.is_discarded()) {
        DHLOGE("recMsg parse failed!");
        return;
    }
    VerifyInputDevice(inputDeviceJson, pBuf);
}

void DistributedInputNodeManager::VerifyInputDevice(const nlohmann::json& inputDeviceJson, InputDevice& pBuf)
{
    if (IsString(inputDeviceJson, DEVICE_NAME)) {
        pBuf.name = inputDeviceJson[DEVICE_NAME].get<std::string>();
    }
    if (IsString(inputDeviceJson, PHYSICAL_PATH)) {
        pBuf.physicalPath = inputDeviceJson[PHYSICAL_PATH].get<std::string>();
    }
    if (IsString(inputDeviceJson, UNIQUE_ID)) {
        pBuf.uniqueId = inputDeviceJson[UNIQUE_ID].get<std::string>();
    }
    if (IsUInt16(inputDeviceJson, BUS)) {
        pBuf.bus = inputDeviceJson[BUS].get<uint16_t>();
    }
    if (IsUInt16(inputDeviceJson, VENDOR)) {
        pBuf.vendor = inputDeviceJson[VENDOR].get<uint16_t>();
    }
    if (IsUInt16(inputDeviceJson, PRODUCT)) {
        pBuf.product = inputDeviceJson[PRODUCT].get<uint16_t>();
    }
    if (IsUInt16(inputDeviceJson, VERSION)) {
        pBuf.version = inputDeviceJson[VERSION].get<uint16_t>();
    }
    if (IsString(inputDeviceJson, DESCRIPTOR)) {
        pBuf.descriptor = inputDeviceJson[DESCRIPTOR].get<std::string>();
    }
    if (IsUInt32(inputDeviceJson, CLASSES)) {
        pBuf.classes = inputDeviceJson[CLASSES].get<uint32_t>();
    }
    if (IsArray(inputDeviceJson, EVENT_TYPES)) {
        pBuf.eventTypes = inputDeviceJson[EVENT_TYPES].get<std::vector<uint32_t>>();
    }
    if (IsArray(inputDeviceJson, EVENT_KEYS)) {
        pBuf.eventKeys = inputDeviceJson[EVENT_KEYS].get<std::vector<uint32_t>>();
    }
    if (IsArray(inputDeviceJson, ABS_TYPES)) {
        pBuf.absTypes = inputDeviceJson[ABS_TYPES].get<std::vector<uint32_t>>();
    }
    if (IsArray(inputDeviceJson, ABS_INFOS)) {
        pBuf.absInfos = inputDeviceJson[ABS_INFOS].get<std::map<uint32_t, std::vector<int32_t>>>();
    }
    if (IsArray(inputDeviceJson, REL_TYPES)) {
        pBuf.relTypes = inputDeviceJson[REL_TYPES].get<std::vector<uint32_t>>();
    }
    if (IsArray(inputDeviceJson, PROPERTIES)) {
        pBuf.properties = inputDeviceJson[PROPERTIES].get<std::vector<uint32_t>>();
    }
}

int32_t DistributedInputNodeManager::CreateHandle(const InputDevice& inputDevice, const std::string& devId,
    const std::string& dhId)
{
    std::unique_lock<std::mutex> my_lock(operationMutex_);
    std::call_once(callOnceFlag_, [this]() { inputHub_->ScanInputDevices(DEVICE_PATH); });
    std::unique_ptr<VirtualDevice> virtualDevice = std::make_unique<VirtualDevice>(inputDevice);

    virtualDevice->SetNetWorkId(devId);

    if (!virtualDevice->SetUp(inputDevice, devId, dhId)) {
        DHLOGE("could not create new virtual device\n");
        return ERR_DH_INPUT_SERVER_SOURCE_CREATE_HANDLE_FAIL;
    }
    AddDeviceLocked(inputDevice.descriptor, std::move(virtualDevice));
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
    device = std::make_unique<VirtualDevice>(info.deviceInfo);
    if (!device->SetUp(info.deviceInfo, devId, dhId)) {
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
    auto iter = virtualDeviceMap_.find(dhId);
    if (iter != virtualDeviceMap_.end()) {
        device = iter->second.get();
        return DH_SUCCESS;
    }
    return ERR_DH_INPUT_SERVER_SOURCE_GET_DEVICE_FAIL;
}

void DistributedInputNodeManager::StartInjectThread()
{
    if (isInjectThreadCreated_.load()) {
        DHLOGI("InjectThread has been created.");
        return;
    }
    DHLOGI("InjectThread does not created");
    isInjectThreadCreated_.store(true);
    isInjectThreadRunning_.store(true);
    eventInjectThread_ = std::thread(&DistributedInputNodeManager::InjectEvent, this);
}

void DistributedInputNodeManager::StopInjectThread()
{
    if (!isInjectThreadCreated_.load()) {
        DHLOGI("InjectThread does not created, and not need to stop.");
    }
    DHLOGI("InjectThread has been created, and soon will be stopped.");
    isInjectThreadRunning_.store(false);
    isInjectThreadCreated_.store(false);
    conditionVariable_.notify_all();
    if (eventInjectThread_.joinable()) {
        eventInjectThread_.join();
    }
}

void DistributedInputNodeManager::ReportEvent(const RawEvent rawEvent)
{
    std::lock_guard<std::mutex> lockGuard(injectThreadMutex_);
    injectQueue_.push(std::make_shared<RawEvent>(rawEvent));
    conditionVariable_.notify_all();
}

void DistributedInputNodeManager::InjectEvent()
{
    int32_t ret = pthread_setname_np(pthread_self(), EVENT_INJECT_THREAD_NAME);
    if (ret != 0) {
        DHLOGE("InjectEvent setname failed.");
    }
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
    uint32_t inputType = 0;

    if ((inputTypes & static_cast<uint32_t>(DInputDeviceType::MOUSE)) != 0) {
        inputType |= INPUT_DEVICE_CLASS_CURSOR;
    }

    if ((inputTypes & static_cast<uint32_t>(DInputDeviceType::KEYBOARD)) != 0) {
        inputType |= INPUT_DEVICE_CLASS_KEYBOARD;
    }

    if ((inputTypes & static_cast<uint32_t>(DInputDeviceType::MOUSE)) != 0) {
        inputType |= INPUT_DEVICE_CLASS_TOUCH;
    }

    std::lock_guard<std::mutex> lock(virtualDeviceMapMutex_);
    for (const auto &[id, virdevice] : virtualDeviceMap_) {
        if ((virdevice->GetDeviceType() & inputType) && (virdevice->GetNetWorkId() == networkId)) {
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
