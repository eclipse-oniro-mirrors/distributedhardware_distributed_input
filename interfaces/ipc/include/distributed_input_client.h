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

#ifndef DISTRIBUTED_INPUT_CLIENT_H
#define DISTRIBUTED_INPUT_CLIENT_H

#include <atomic>
#include <iostream>
#include <mutex>
#include <string>

#include "add_white_list_infos_call_back_stub.h"
#include "del_white_list_infos_call_back_stub.h"
#include "i_distributed_source_input.h"
#include "i_distributed_sink_input.h"
#include "register_d_input_call_back_stub.h"
#include "start_d_input_server_call_back_stub.h"
#include "unregister_d_input_call_back_stub.h"

#include "dinput_sa_manager.h"
#include "idistributed_hardware_source.h"
#include "idistributed_hardware_sink.h"

namespace OHOS {
namespace DistributedHardware {
namespace DistributedInput {
class DistributedInputClient {
public:
    ~DistributedInputClient(){};

    static DistributedInputClient &GetInstance();

    int32_t InitSource();

    int32_t ReleaseSource();

    int32_t InitSink();

    int32_t ReleaseSink();

    int32_t RegisterDistributedHardware(const std::string& devId, const std::string& dhId,
        const std::string& parameters, const std::shared_ptr<RegisterCallback>& callback);

    int32_t UnregisterDistributedHardware(const std::string& devId, const std::string& dhId,
        const std::shared_ptr<UnregisterCallback>& callback);

    int32_t PrepareRemoteInput(const std::string& deviceId, sptr<IPrepareDInputCallback> callback);

    int32_t UnprepareRemoteInput(const std::string& deviceId, sptr<IUnprepareDInputCallback> callback);

    int32_t StartRemoteInput(
        const std::string& deviceId, const uint32_t& inputTypes, sptr<IStartDInputCallback> callback);

    int32_t StopRemoteInput(
        const std::string& deviceId, const uint32_t& inputTypes, sptr<IStopDInputCallback> callback);

    bool IsNeedFilterOut(const std::string &deviceId, const BusinessEvent &event);

    DInputServerType IsStartDistributedInput(const uint32_t& inputType);

public:
    class RegisterDInputCb : public OHOS::DistributedHardware::DistributedInput::RegisterDInputCallbackStub {
    public:
        RegisterDInputCb() = default;
        virtual ~RegisterDInputCb() = default;
        void OnResult(const std::string& devId, const std::string& dhId, const int32_t& status);
    };

    class UnregisterDInputCb : public OHOS::DistributedHardware::DistributedInput::UnregisterDInputCallbackStub {
    public:
        UnregisterDInputCb() = default;
        virtual ~UnregisterDInputCb() = default;
        void OnResult(const std::string& devId, const std::string& dhId, const int32_t& status);
    };

    class StartDInputServerCb : public OHOS::DistributedHardware::DistributedInput::StartDInputServerCallbackStub {
    public:
        StartDInputServerCb() = default;
        virtual ~StartDInputServerCb() = default;
        void OnResult(const int32_t& status, const uint32_t& inputTypes);
    };

    class AddWhiteListInfosCb : public OHOS::DistributedHardware::DistributedInput::AddWhiteListInfosCallbackStub {
    public:
        AddWhiteListInfosCb() = default;
        virtual ~AddWhiteListInfosCb() = default;
        void OnResult(const std::string &deviceId, const std::string &strJson);
    };

    class DelWhiteListInfosCb : public OHOS::DistributedHardware::DistributedInput::DelWhiteListInfosCallbackStub {
    public:
        DelWhiteListInfosCb() = default;
        virtual ~DelWhiteListInfosCb() = default;
        void OnResult(const std::string &deviceId);
    };

private:
    DistributedInputClient();
    bool IsJsonData(std::string strData) const;
    void AddWhiteListInfos(const std::string &deviceId, const std::string &strJson) const;
    void DelWhiteListInfos(const std::string &deviceId) const;

private:
    static std::shared_ptr<DistributedInputClient> instance;

    bool m_bIsAlreadyInitWhiteList = false;
    const std::string localDevId_ = "localNodeDevice";

    DInputServerType serverType = DInputServerType::NULL_SERVER_TYPE;
    DInputDeviceType inputTypes_ = DInputDeviceType::NONE;

    sptr<StartDInputServerCb> sinkTypeCallback = nullptr;
    sptr<StartDInputServerCb> sourceTypeCallback = nullptr;
    sptr<AddWhiteListInfosCb> addWhiteListCallback = nullptr;
    sptr<DelWhiteListInfosCb> delWhiteListCallback = nullptr;

    struct DHardWareFwkRegistInfo {
        std::string devId;
        std::string dhId;
        std::shared_ptr<RegisterCallback> callback = nullptr;
    };

    struct DHardWareFwkUnRegistInfo {
        std::string devId;
        std::string dhId;
        std::shared_ptr<UnregisterCallback> callback = nullptr;
    };

    std::vector<DHardWareFwkRegistInfo> dHardWareFwkRstInfos;
    std::vector<DHardWareFwkUnRegistInfo> dHardWareFwkUnRstInfos;
    std::mutex operationMutex_;
};
}  // namespace DistributedInput
}  // namespace DistributedHardware
}  // namespace OHOS

#endif  // DISTRIBUTED_INPUT_CLIENT_H
