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

#include "distributed_input_collector.h"

#include <cstring>
#include <dirent.h>
#include <fcntl.h>
#include <openssl/sha.h>
#include <sys/inotify.h>
#include <unistd.h>
#include <linux/input.h>
#include "sys/stat.h"
#include "nlohmann/json.hpp"
#include "distributed_hardware_log.h"

namespace OHOS {
namespace DistributedHardware {
namespace DistributedInput {
namespace {
static const uint32_t INPUT_DEVICE_CLASS_KEYBOARD = static_cast<uint32_t>(DeviceClasses::INPUT_DEVICE_CLASS_KEYBOARD);
static const uint32_t INPUT_DEVICE_CLASS_CURSOR   = static_cast<uint32_t>(DeviceClasses::INPUT_DEVICE_CLASS_CURSOR);
}

DistributedInputCollector::DistributedInputCollector()
    : collectThreadID_(-1), isCollectingEvents_(false),
    isStartGetDeviceHandlerThread(false), input_types_(0)
{
    inputHub_ = std::make_unique<InputHub>();
}

DistributedInputCollector::~DistributedInputCollector()
{
    StopCollectEventsThread();
}

DistributedInputCollector &DistributedInputCollector::GetInstance()
{
    static DistributedInputCollector instance;
    return instance;
}

int32_t DistributedInputCollector::Init(std::shared_ptr<AppExecFwk::EventHandler> sinkHandler)
{
    sinkHandler_ = sinkHandler;
    if (sinkHandler_ == nullptr) {
        DHLOGE("DistributedInputCollector::Init sinkHandler_ failed \n");
        return FAILURE;
    }
    if (!isStartGetDeviceHandlerThread) {
        InitCollectEventsThread();
        isStartGetDeviceHandlerThread = true;
    }
    return SUCCESS;
}

bool DistributedInputCollector::InitCollectEventsThread()
{
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

    isCollectingEvents_ = true;
    collectThreadID_ = -1;
    int32_t ret = pthread_create(&collectThreadID_, &attr, CollectEventsThread, this);
    if (ret != 0) {
        DHLOGE(
            "DistributedInputCollector::InitCollectEventsThread create  thread failed:%d \n", ret);
        pthread_attr_destroy(&attr);
        collectThreadID_ = -1;
        isCollectingEvents_ = false;
        return false;
    }
    return true;
}

void *DistributedInputCollector::CollectEventsThread(void *param)
{
    DistributedInputCollector *pThis = reinterpret_cast<DistributedInputCollector *>(param);
    pThis->StartCollectEventsThread();
    DHLOGW("DistributedInputCollector::CollectEventsThread exist!");
    return nullptr;
}

void DistributedInputCollector::StartCollectEventsThread()
{
    while (isCollectingEvents_) {
        size_t count = inputHub_->CollectInputEvents(mEventBuffer, INPUT_EVENT_BUFFER_SIZE);
        if (count > 0) {
            DHLOGI("Count: %zu", count);
        } else {
            continue;
        }

        // The RawEvent obtained by the controlled end calls transport and is
        // sent to the main control end.
        std::shared_ptr<nlohmann::json> jsonArrayMsg = std::make_shared<nlohmann::json>();
        for (int ind = 0; ind < count; ind++) {
            nlohmann::json tmpJson;
            tmpJson[INPUT_KEY_WHEN] = mEventBuffer[ind].when;
            tmpJson[INPUT_KEY_TYPE] = mEventBuffer[ind].type;
            tmpJson[INPUT_KEY_CODE] = mEventBuffer[ind].code;
            tmpJson[INPUT_KEY_VALUE] = mEventBuffer[ind].value;
            tmpJson[INPUT_KEY_DESCRIPTOR] = mEventBuffer[ind].descriptor;
            jsonArrayMsg->push_back(tmpJson);
        }

        AppExecFwk::InnerEvent::Pointer msgEvent = AppExecFwk::InnerEvent::Get(
            static_cast<uint32_t>(EHandlerMsgType::DINPUT_SINK_EVENT_HANDLER_MSG), jsonArrayMsg, 0);
        if (sinkHandler_ != nullptr) {
            sinkHandler_->SendEvent(msgEvent, 0, AppExecFwk::EventQueue::Priority::IMMEDIATE);
        }
    }
    DHLOGW("DistributedInputCollector::StartCollectEventsThread exit!");
}

void DistributedInputCollector::StopCollectEventsThread()
{
    isCollectingEvents_ = false;
    isStartGetDeviceHandlerThread = false;
    if (collectThreadID_ != (pthread_t)(-1)) {
        DHLOGI("DistributedInputCollector::Wait collect thread exit");
        pthread_join(collectThreadID_, NULL);
        collectThreadID_ = (pthread_t)(-1);
    }
    DHLOGW("DistributedInputCollector::StopCollectEventsThread exit!");
}

void DistributedInputCollector::SetInputTypes(const int32_t& inputType)
{
    if (inputType & INPUT_TYPE_MOUSE) {
        input_types_ |= INPUT_DEVICE_CLASS_CURSOR;
    }
    if (inputType & INPUT_TYPE_KEYBOARD) {
        input_types_ |= INPUT_DEVICE_CLASS_KEYBOARD;
    }
    if (inputType & INPUT_TYPE_TOUCH) {
        input_types_ |= INPUT_DEVICE_CLASS_CURSOR;
    }
    inputHub_->SetSupportInputType(input_types_);
}
} // namespace DistributedInput
} // namespace DistributedHardware
} // namespace OHOS
