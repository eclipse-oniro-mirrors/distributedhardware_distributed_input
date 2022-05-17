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

#ifndef DISTRIBUTED_INPUT_KIT_H
#define DISTRIBUTED_INPUT_KIT_H

#include <string>
#include "constants_dinput.h"
#include "distributed_input_client.h"
#include "i_register_d_input_call_back.h"
#include "i_unregister_d_input_call_back.h"
#include "i_prepare_d_input_call_back.h"
#include "i_unprepare_d_input_call_back.h"
#include "i_start_d_input_call_back.h"
#include "i_stop_d_input_call_back.h"
#include "distributed_hardware_log.h"

namespace OHOS {
namespace DistributedHardware {
namespace DistributedInput {
class DistributedInputKit {
public:

    static int32_t PrepareRemoteInput(const std::string& deviceId, sptr<IPrepareDInputCallback> callback);

    static int32_t UnprepareRemoteInput(const std::string& deviceId, sptr<IUnprepareDInputCallback> callback);

    static int32_t StartRemoteInput(
        const std::string& deviceId, const uint32_t& inputTypes, sptr<IStartDInputCallback> callback);

    static int32_t StopRemoteInput(
        const std::string& deviceId, const uint32_t& inputTypes, sptr<IStopDInputCallback> callback);

    static bool IsNeedFilterOut(const std::string &deviceId, const BusinessEvent &event);

    static DInputServerType IsStartDistributedInput(const uint32_t& inputType);
};
} // namespace DistributedInput
} // namespace DistributedHardware
} // namespace OHOS
#endif // DISTRIBUTED_INPUT_KIT_H
