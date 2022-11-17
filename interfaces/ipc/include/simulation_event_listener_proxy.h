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

#ifndef SIMULATION_EVENT_LISTENER_PROXY_H
#define SIMULATION_EVENT_LISTENER_PROXY_H

#include "i_simulation_event_listener.h"

#include <string>

#include "iremote_proxy.h"

namespace OHOS {
namespace DistributedHardware {
namespace DistributedInput {
class SimulationEventListenerProxy : public IRemoteProxy<ISimulationEventListener> {
public:
    explicit SimulationEventListenerProxy(const sptr<IRemoteObject> &object);
    ~SimulationEventListenerProxy() override;

    int32_t OnSimulationEvent(uint32_t type, uint32_t code, int32_t value) override;

private:
    static inline BrokerDelegator<SimulationEventListenerProxy> delegator_;
};
} // namespace DistributedInput
} // namespace DistributedHardware
} // namespace OHOS

#endif // SIMULATION_EVENT_LISTENER_PROXY_H
