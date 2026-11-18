/*
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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

#ifndef IDISTRIBUTED_HARDWARE_SOURCE_H
#define IDISTRIBUTED_HARDWARE_SOURCE_H

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace OHOS {
namespace DistributedHardware {
class RegisterCallback {
public:
    virtual ~RegisterCallback() = default;
    virtual int32_t OnRegisterResult(const std::string &devId, const std::string &dhId,
        int32_t status, const std::string &data) = 0;
};

class UnregisterCallback {
public:
    virtual ~UnregisterCallback() = default;
    virtual int32_t OnUnregisterResult(const std::string &devId, const std::string &dhId,
        int32_t status, const std::string &data) = 0;
};
} // namespace DistributedHardware
} // namespace OHOS

#endif // IDISTRIBUTED_HARDWARE_SOURCE_H
