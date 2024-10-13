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

#include "distributed_input_source_transport_fuzzer.h"

#include <cstddef>
#include <cstdint>
#include <functional>
#include <iostream>
#include <thread>
#include <unistd.h>

#include <refbase.h>

#include "constants_dinput.h"
#include "distributed_input_inject.h"
#include "distributed_input_source_transport.h"

namespace OHOS {
namespace DistributedHardware {
void OpenInputSoftbusFuzzTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size == 0)) {
        return;
    }

    std::string remoteDevId(reinterpret_cast<const char*>(data), size);

    const uint32_t sleepTimeUs = 100 * 1000;
    usleep(sleepTimeUs);
    DistributedInput::DistributedInputSourceTransport::GetInstance().OpenInputSoftbus(remoteDevId, true);
    DistributedInput::DistributedInputSourceTransport::GetInstance().CloseInputSoftbus(remoteDevId, true);
}
} // namespace DistributedHardware
} // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    OHOS::DistributedHardware::DistributedInput::DistributedInputInject::GetInstance();
    /* Run your code on data */
    OHOS::DistributedHardware::OpenInputSoftbusFuzzTest(data, size);
    return 0;
}