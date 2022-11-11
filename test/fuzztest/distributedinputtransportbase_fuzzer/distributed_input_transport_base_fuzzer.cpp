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

#include "distributed_input_transport_base_fuzzer.h"

#include <cstddef>
#include <cstdint>
#include <functional>
#include <iostream>
#include <thread>
#include <unistd.h>

#include <refbase.h>

#include "constants_dinput.h"
#include "distributed_input_transport_base.h"

namespace OHOS {
namespace DistributedHardware {
void StartSessionFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size < sizeof(int32_t))) {
        return;
    }

    std::string remoteDevId(reinterpret_cast<const char*>(data), size);

    const uint32_t sleepTimeUs = 100 * 1000;
    usleep(sleepTimeUs);
    DistributedInput::DistributedInputTransportBase::GetInstance().StartSession(remoteDevId);
    DistributedInput::DistributedInputTransportBase::GetInstance().StopSession(remoteDevId);
}

void OnSessionOpenedFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size < sizeof(int32_t))) {
        return;
    }

    int32_t sessionId = *(reinterpret_cast<const int32_t*>(data));
    int32_t result = *(reinterpret_cast<const int32_t*>(data));

    DistributedInput::DistributedInputTransportBase::GetInstance().OnSessionOpened(sessionId, result);
    DistributedInput::DistributedInputTransportBase::GetInstance().OnSessionClosed(sessionId);
}
} // namespace DistributedHardware
} // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    OHOS::DistributedHardware::StartSessionFuzzTest(data, size);
    OHOS::DistributedHardware::OnSessionOpenedFuzzTest(data, size);
    return 0;
}