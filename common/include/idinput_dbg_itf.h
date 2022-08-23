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

#ifndef OHOS_DINPUT_DBG_ITF_H
#define OHOS_DINPUT_DBG_ITF_H

#include <string>
#include <cstdint>

namespace OHOS {
namespace DistributedHardware {
namespace DistributedInput {
const std::string GET_DBG_ITF_FUNC = "GetDBGItf";
class IDInputDBGItf {
public:
    virtual int32_t Init() = 0;
};
extern "C" __attribute__((visibility("default"))) IDInputDBGItf* GetDBGItf();
}
}
}

#endif