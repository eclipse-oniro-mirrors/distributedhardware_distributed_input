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

#include "distributed_input_handler.h"

#include <cstring>
#include <dirent.h>
#include <fcntl.h>
#include <sstream>
#include <sys/stat.h>
#include <unistd.h>

#include <openssl/sha.h>
#include <sys/inotify.h>
#include <linux/input.h>
#include <linux/uinput.h>

#include "anonymous_string.h"
#include "distributed_hardware_log.h"
#include "nlohmann/json.hpp"

#include "constants_dinput.h"
#include "dinput_errcode.h"
#include "softbus_bus_center.h"

namespace OHOS {
namespace DistributedHardware {
namespace DistributedInput {
IMPLEMENT_SINGLE_INSTANCE(DistributedInputHandler);
DistributedInputHandler::DistributedInputHandler()
    : collectThreadID_(-1), isCollectingEvents_(false),
    isStartCollectEventThread(false)
{
    inputHub_ = std::make_unique<InputHub>();
    this->m_listener = nullptr;
}

DistributedInputHandler::~DistributedInputHandler()
{
    StopInputMonitorDeviceThread();
}

void DistributedInputHandler::StructTransJson(const InputDevice& pBuf, std::string& strDescriptor)
{
    DHLOGI("[%s] %d, %d, %d, %d, %s.\n",
        (pBuf.name).c_str(), pBuf.bus, pBuf.vendor, pBuf.product, pBuf.version, (pBuf.descriptor).c_str());
    nlohmann::json tmpJson;
    tmpJson["name"] = pBuf.name;
    tmpJson["location"] = pBuf.location;
    tmpJson["uniqueId"] = pBuf.uniqueId;
    tmpJson["bus"] = pBuf.bus;
    tmpJson["vendor"] = pBuf.vendor;
    tmpJson["product"] = pBuf.product;
    tmpJson["version"] = pBuf.version;
    tmpJson["descriptor"] = pBuf.descriptor;
    tmpJson["nonce"] = pBuf.nonce;
    tmpJson["classes"] = pBuf.classes;

    std::ostringstream stream;
    stream << tmpJson.dump();
    strDescriptor = stream.str();
    return;
}

int32_t DistributedInputHandler::Initialize()
{
    if (!isStartCollectEventThread) {
        InitCollectEventsThread();
        isStartCollectEventThread = true;
    }
    return DH_SUCCESS;
}

std::vector<DHItem> DistributedInputHandler::Query()
{
    std::vector<DHItem> retInfos;
    std::vector<InputDevice> vecInput = inputHub_->GetAllInputDevices();
    for (auto iter : vecInput) {
        DHItem item;
        item.dhId = iter.descriptor;
        StructTransJson(iter, item.attrs);
        retInfos.push_back(item);
    }

    return retInfos;
}

std::map<std::string, std::string> DistributedInputHandler::QueryExtraInfo()
{
    std::map<std::string, std::string> ret;
    return ret;
}

bool DistributedInputHandler::IsSupportPlugin()
{
    return true;
}

void DistributedInputHandler::RegisterPluginListener(std::shared_ptr<PluginListener> listener)
{
    this->m_listener = listener;
}

void DistributedInputHandler::UnRegisterPluginListener()
{
    this->m_listener = nullptr;
}

int32_t DistributedInputHandler::GetDeviceInfo(std::string& deviceId)
{
    std::unique_lock<std::mutex> my_lock(operationMutex_);
    auto localNode = std::make_unique<NodeBasicInfo>();
    int32_t retCode = GetLocalNodeDeviceInfo("ohos.dhardware", localNode.get());
    if (retCode != 0) {
        DHLOGE("Could not get device id.");
        return ERR_DH_INPUT_HANDLER_GET_DEVICE_ID_FAIL;
    }

    deviceId = localNode->networkId;
    DHLOGI("device id is %s", GetAnonyString(deviceId).c_str());
    return DH_SUCCESS;
}

bool DistributedInputHandler::InitCollectEventsThread()
{
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

    isCollectingEvents_ = true;
    collectThreadID_ = -1;
    int32_t ret = pthread_create(&collectThreadID_, &attr, CollectEventsThread, this);
    if (ret != 0) {
        DHLOGE(
            "DistributedInputHandler::InitCollectEventsThread create thread failed:%d \n", ret);
        pthread_attr_destroy(&attr);
        collectThreadID_ = -1;
        isCollectingEvents_ = false;
        return false;
    }
    return true;
}

void *DistributedInputHandler::CollectEventsThread(void *param)
{
    DistributedInputHandler *pThis = reinterpret_cast<DistributedInputHandler *>(param);

    std::string deviceId;
    pThis->GetDeviceInfo(deviceId);
    pThis->StartInputMonitorDeviceThread(deviceId);
    DHLOGI("DistributedInputHandler::CollectEventsThread exist!");
    return nullptr;
}

void DistributedInputHandler::StartInputMonitorDeviceThread(const std::string deviceId)
{
    while (isCollectingEvents_) {
        size_t count = inputHub_->StartCollectInputHandler(mEventBuffer, INPUT_DEVICR_BUFFER_SIZE);
        if (count > 0) {
            DHLOGI("Count: %zu", count);
            for (size_t iCnt = 0; iCnt < count; iCnt++) {
                NotifyHardWare(iCnt);
            }
        } else {
            continue;
        }
    }
    isCollectingEvents_ = false;
    DHLOGI("DistributedInputHandler::StartCollectEventsThread exit!");
}

void DistributedInputHandler::NotifyHardWare(int iCnt)
{
    switch (mEventBuffer[iCnt].type) {
        case DeviceType::DEVICE_ADDED:
            if (this->m_listener != nullptr) {
                std::string hdInfo;
                StructTransJson(mEventBuffer[iCnt].deviceInfo, hdInfo);
                this->m_listener->PluginHardware(mEventBuffer[iCnt].deviceInfo.descriptor, hdInfo);
            }
            break;
        case DeviceType::DEVICE_REMOVED:
            if (this->m_listener != nullptr) {
                this->m_listener->UnPluginHardware(mEventBuffer[iCnt].deviceInfo.descriptor);
            }
            break;
        default:
            break;
    }
}

void DistributedInputHandler::StopInputMonitorDeviceThread()
{
    isCollectingEvents_ = false;
    isStartCollectEventThread = false;
    inputHub_->StopCollectInputHandler();
    if (collectThreadID_ != (pthread_t)(-1)) {
        DHLOGI("DistributedInputHandler::Wait collect thread exit");
        pthread_join(collectThreadID_, NULL);
        collectThreadID_ = (pthread_t)(-1);
    }
    DHLOGI("DistributedInputHandler::StopInputMonitorDeviceThread exit!");
}

IHardwareHandler* GetHardwareHandler()
{
    return &DistributedInputHandler::GetInstance();
}
} // namespace DistributedInput
} // namespace DistributedHardware
} // namespace OHOS
