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

#include "virtual_touchpad.h"

namespace OHOS {
namespace DistributedHardware {
namespace DistributedInput {
namespace {
const std::vector<uint32_t> EVT_TYPES {
        EV_KEY, EV_ABS
    };
const std::vector<uint32_t> KEYS {
        BTN_0, BTN_1, BTN_2, BTN_3, BTN_4, BTN_5, BTN_6, BTN_STYLUS, BTN_TOOL_PEN
    };
const std::vector<uint32_t> PROPERTIES {};
const std::vector<uint32_t> ABS {
    ABS_X, ABS_Y, ABS_WHEEL, ABS_MISC
};
const std::vector<uint32_t> RELBITS {};
}

VirtualTouchpad::VirtualTouchpad(const InputDevice& event) : VirtualDevice(event)
{
    const int absMaxWheel = 71;

    dev_.absmin[ABS_X] = 0;
    dev_.absmax[ABS_X] = 1;
    dev_.absfuzz[ABS_X] = 0;
    dev_.absflat[ABS_X] = 0;

    dev_.absmin[ABS_Y] = 0;
    dev_.absmax[ABS_Y] = 1;
    dev_.absfuzz[ABS_Y] = 0;
    dev_.absflat[ABS_Y] = 0;

    dev_.absmin[ABS_WHEEL] = 0;
    dev_.absmax[ABS_WHEEL] = absMaxWheel;
    dev_.absfuzz[ABS_WHEEL] = 0;
    dev_.absflat[ABS_WHEEL] = 0;

    dev_.absmin[ABS_MISC] = 0;
    dev_.absmax[ABS_MISC] = 0;
    dev_.absfuzz[ABS_MISC] = 0;
    dev_.absflat[ABS_MISC] = 0;
}

VirtualTouchpad::~VirtualTouchpad() {}

const std::vector<uint32_t>& VirtualTouchpad::GetEventTypes() const
{
    return EVT_TYPES;
}

const std::vector<uint32_t>& VirtualTouchpad::GetKeys() const
{
    return KEYS;
}

const std::vector<uint32_t>& VirtualTouchpad::GetProperties() const
{
    return PROPERTIES;
}

const std::vector<uint32_t>& VirtualTouchpad::GetAbs() const
{
    return ABS;
}

const std::vector<uint32_t>& VirtualTouchpad::GetRelBits() const
{
    return RELBITS;
}
} // namespace DistributedInput
} // namespace DistributedHardware
} // namespace OHOS

