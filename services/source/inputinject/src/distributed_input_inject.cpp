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

#include "distributed_input_inject.h"

#include <sstream>
#include "nlohmann/json.hpp"
#include "distributed_hardware_log.h"

namespace OHOS {
namespace DistributedHardware {
namespace DistributedInput {
DistributedInputInject::DistributedInputInject()
    : isStartInjectThread_(false)
{
    inputNodeManager_ = std::make_unique<Distributed_input_node_manager>();
}

DistributedInputInject::~DistributedInputInject()
{
    isStartInjectThread_ = false;
}

DistributedInputInject &DistributedInputInject::GetInstance()
{
    static DistributedInputInject instance;
    return instance;
}

int32_t DistributedInputInject::RegisterDistributedHardware(const std::string& devId, const std::string& dhId,
    const std::string& parameters)
{
    DHLOGI("%s called",  __func__);
    DHLOGI("deviveId:%s",  devId.c_str());
    DHLOGI("hardwareId:%s",  dhId.c_str());
    DHLOGI("describe:%s",  parameters.c_str());

    if (inputNodeManager_ == nullptr) {
        DHLOGE("the Distributed_input_node_manager is null\n");
        return FAILURE;
    }
    if (inputNodeManager_->openDevicesNode(devId, dhId, parameters) < 0) {
        DHLOGE("create virtual device error\n");
        return FAILURE;
    }
    return SUCCESS;
}

int32_t DistributedInputInject::UnregisterDistributedHardware(const std::string& devId, const std::string& dhId)
{
    DHLOGI("%s called deviveId:%s hardwareId:%s\n",
        __func__, devId.c_str(), dhId.c_str());
    if (inputNodeManager_ == nullptr) {
        DHLOGE("the Distributed_input_node_manager is null\n");
        return FAILURE;
    }
    if (inputNodeManager_->CloseDeviceLocked(dhId) < 0) {
        DHLOGE("delete virtual device error\n");
        return FAILURE;
    }
    return SUCCESS;
}

int32_t DistributedInputInject::StructTransJson(const InputDevice& pBuf, std::string& strDescriptor)
{
    DHLOGI(
        "[%s] %d, %d, %d, %d, %s.\n",
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
    return SUCCESS;
}

int32_t DistributedInputInject::PrepareRemoteInput()
{
    if (inputNodeManager_ == nullptr) {
        DHLOGE("the Distributed_input_node_manager is null\n");
        return FAILURE;
    }
    if (!isStartInjectThread_) {
        isStartInjectThread_ = true;
        inputNodeManager_->StartInjectThread();
    }
    return SUCCESS;
}

int32_t DistributedInputInject::RegisterDistributedEvent(RawEvent* buffer, size_t bufferSize)
{
    if (inputNodeManager_ == nullptr) {
        DHLOGE("the Distributed_input_node_manager is null\n");
        return FAILURE;
    }
    DHLOGE("RegisterDistributedEvent start %zu\n", bufferSize);
    for (size_t i = 0; i < bufferSize; i++) {
        inputNodeManager_->ReportEvent(buffer[i]);
    }
    return SUCCESS;
}
} // namespace DistributedInput
} // namespace DistributedHardware
} // namespace OHOS