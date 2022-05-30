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

#ifndef WHITE_LIST_UTIL_H
#define WHITE_LIST_UTIL_H

#include <mutex>

#include "constants_dinput.h"

namespace OHOS {
namespace DistributedHardware {
namespace DistributedInput {
using TYPE_KEY_CODE_VEC = std::vector<int32_t>;
using TYPE_COMBINATION_KEY_VEC = std::vector<TYPE_KEY_CODE_VEC>;
using TYPE_WHITE_LIST_VEC = std::vector<TYPE_COMBINATION_KEY_VEC>;
using TYPE_DEVICE_WHITE_LIST_MAP = std::map<std::string, TYPE_WHITE_LIST_VEC>;
class WhiteListUtil {
public:
    static WhiteListUtil &GetInstance(void);
    int32_t Init(const std::string &deviceId);
    int32_t UnInit(void);
    int32_t SyncWhiteList(const std::string &deviceId, const TYPE_WHITE_LIST_VEC &vecWhiteList);
    int32_t ClearWhiteList(const std::string &deviceId);
    int32_t ClearWhiteList(void);
    int32_t GetWhiteList(const std::string &deviceId, TYPE_WHITE_LIST_VEC &vecWhiteList);
    bool IsNeedFilterOut(const std::string &deviceId, const BusinessEvent &event);
private:
    WhiteListUtil();
    ~WhiteListUtil();
    WhiteListUtil(const WhiteListUtil &other) = delete;
    const WhiteListUtil &operator=(const WhiteListUtil &other) = delete;
    void ReadLineDataStepOne(std::string &column, TYPE_KEY_CODE_VEC &vecKeyCode,
                             TYPE_COMBINATION_KEY_VEC &vecCombinationKey) const;
    bool CheckSubVecData(const TYPE_COMBINATION_KEY_VEC::iterator &iter2,
                         const TYPE_KEY_CODE_VEC::iterator &iter3) const;
private:
    TYPE_DEVICE_WHITE_LIST_MAP mapDeviceWhiteList_;
    std::mutex mutex_;
};
} // namespace DistributedInput
} // namespace DistributedHardware
} // namespace OHOS

#endif  // WHITE_LIST_UTIL_H
