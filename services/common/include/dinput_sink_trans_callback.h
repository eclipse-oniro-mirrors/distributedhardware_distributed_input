/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#ifndef OHOS_DISTRIBUTED_INPUT_SINK_TANTS_H
#define OHOS_DISTRIBUTED_INPUT_SINK_TANTS_H

#include <string>

namespace OHOS {
namespace DistributedHardware {
namespace DistributedInput {
class DInputSinkTransCallback {
public:
    virtual void onPrepareRemoteInput(const int32_t& sessionId, const std::string &deviceId) = 0;
    virtual void onUnprepareRemoteInput(const int32_t& sessionId) = 0;
    virtual void onStartRemoteInput(const int32_t& sessionId, const uint32_t& inputTypes) = 0;
    virtual void onStopRemoteInput(const int32_t& sessionId, const uint32_t& inputTypes) = 0;
    virtual void onStartRemoteInputDhid(const int32_t &sessionId, const std::string &strDhids) = 0;
    virtual void onStopRemoteInputDhid(const int32_t &sessionId, const std::string &strDhids) = 0;

    virtual void onRelayPrepareRemoteInput(const int32_t &toSrcSessionId, const int32_t &toSinkSessionId,
        const std::string &deviceId) = 0;
    virtual void onRelayUnprepareRemoteInput(const int32_t &toSrcSessionId, const int32_t &toSinkSessionId,
        const std::string &deviceId) = 0;
    virtual void onRelayStartDhidRemoteInput(const int32_t &toSrcSessionId, const int32_t &toSinkSessionId,
        const std::string &deviceId, const std::string &strDhids) = 0;
    virtual void onRelayStopDhidRemoteInput(const int32_t &toSrcSessionId, const int32_t &toSinkSessionId,
        const std::string &deviceId, const std::string &strDhids) = 0;
    virtual void onRelayStartTypeRemoteInput(const int32_t &toSrcSessionId, const int32_t &toSinkSessionId,
        const std::string &deviceId, uint32_t inputTypes) = 0;
    virtual void onRelayStopTypeRemoteInput(const int32_t &toSrcSessionId, const int32_t &toSinkSessionId,
        const std::string &deviceId, uint32_t inputTypes) = 0;
    virtual ~DInputSinkTransCallback() {}
};
} // namespace DistributedInput
} // namespace DistributedHardware
} // namespace OHOS

#endif // OHOS_DISTRIBUTED_INPUT_SINK_TANTS_H
