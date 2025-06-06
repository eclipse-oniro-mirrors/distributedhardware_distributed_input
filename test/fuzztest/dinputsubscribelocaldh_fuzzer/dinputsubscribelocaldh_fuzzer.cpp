/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "dinputsubscribelocaldh_fuzzer.h"

#include <cstddef>
#include <cstdint>
#include <functional>
#include <iostream>
#include <thread>

#include <refbase.h>

#include "constants_dinput.h"
#include "distributed_input_handler.h"
#include "distributed_input_kit.h"
#include "distributed_input_sink_handler.h"
#include "i_distributed_sink_input.h"

namespace OHOS {
namespace DistributedHardware {
void SubscribeLocalHardwareFuzzTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size == 0)) {
        return;
    }
    std::string dhId(reinterpret_cast<const char*>(data), size);
    std::string params(reinterpret_cast<const char*>(data), size);
    DistributedInput::DistributedInputSinkHandler::GetInstance().SubscribeLocalHardware(dhId, params);

    DistributedInput::DistributedInputSinkHandler::GetInstance().UnsubscribeLocalHardware(dhId);
}
} // namespace DistributedHardware
} // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    /* Run your code on data */
    OHOS::DistributedHardware::SubscribeLocalHardwareFuzzTest(data, size);
    return 0;
}