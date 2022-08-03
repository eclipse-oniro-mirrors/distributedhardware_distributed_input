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

#include "virtual_mouse.h"

namespace OHOS {
namespace DistributedHardware {
namespace DistributedInput {
namespace {
const std::vector<uint32_t> EVT_TYPES {
        EV_KEY, EV_REL, EV_MSC, EV_SYN
    };
const std::vector<uint32_t> KEYS {
        BTN_MOUSE, BTN_LEFT, BTN_RIGHT, BTN_MIDDLE, BTN_SIDE, BTN_EXTRA, BTN_FORWARD, BTN_BACK, BTN_TASK
    };
const std::vector<uint32_t> PROPERTIES {};
const std::vector<uint32_t> ABS {};
const std::vector<uint32_t> RELBITS {
    REL_X, REL_Y, REL_WHEEL, REL_WHEEL_HI_RES
};
}

VirtualMouse::VirtualMouse(const InputDevice& event) : VirtualDevice(event) {}

VirtualMouse::~VirtualMouse() {}

const std::vector<uint32_t>& VirtualMouse::GetEventTypes() const
{
    return EVT_TYPES;
}

const std::vector<uint32_t>& VirtualMouse::GetKeys() const
{
    return KEYS;
}

const std::vector<uint32_t>& VirtualMouse::GetProperties() const
{
    return PROPERTIES;
}

const std::vector<uint32_t>& VirtualMouse::GetAbs() const
{
    return ABS;
}

const std::vector<uint32_t>& VirtualMouse::GetRelBits() const
{
    return RELBITS;
}
} // namespace DistributedInput
} // namespace DistributedHardware
} // namespace OHOS

