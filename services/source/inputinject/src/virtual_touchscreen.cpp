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

#include "virtual_touchscreen.h"

namespace OHOS {
namespace DistributedHardware {
namespace DistributedInput {
namespace {
const std::vector<uint32_t> EVT_TYPES {
        EV_KEY, EV_ABS
    };
const std::vector<uint32_t> KEYS {
        KEY_F1, KEY_VOLUMEDOWN, KEY_VOLUMEUP, KEY_POWER, BTN_TOOL_FINGER, BTN_TOUCH, BTN_TRIGGER_HAPPY2,
        BTN_TRIGGER_HAPPY3
    };
const std::vector<uint32_t> PROPERTIES {
        INPUT_PROP_DIRECT
    };
const std::vector<uint32_t> ABS {
        ABS_X, ABS_Y, ABS_PRESSURE, ABS_MT_TOUCH_MAJOR, ABS_MT_TOUCH_MINOR, ABS_MT_ORIENTATION, ABS_MT_POSITION_X,
        ABS_MT_POSITION_Y, ABS_MT_BLOB_ID, ABS_MT_TRACKING_ID, ABS_MT_PRESSURE
    };
const std::vector<uint32_t> RELBITS {};
}

VirtualTouchScreen::VirtualTouchScreen(const InputDevice& event, LocalAbsInfo& absInfo, uint32_t phyWidth,
    uint32_t phyHeight) : VirtualDevice(event)
{
    dev_.absmin[ABS_X] = 0;
    dev_.absmax[ABS_X] = phyWidth;
    dev_.absfuzz[ABS_X] = 0;
    dev_.absflat[ABS_X] = 0;

    dev_.absmin[ABS_Y] = 0;
    dev_.absmax[ABS_Y] = phyHeight;
    dev_.absfuzz[ABS_Y] = 0;
    dev_.absflat[ABS_Y] = 0;

    dev_.absmin[ABS_PRESSURE] = absInfo.absPressureMin;
    dev_.absmax[ABS_PRESSURE] = absInfo.absPressureMax;
    dev_.absfuzz[ABS_PRESSURE] = 0;
    dev_.absflat[ABS_PRESSURE] = 0;

    dev_.absmin[ABS_MT_TOUCH_MAJOR] = absInfo.absMtTouchMajorMin;
    dev_.absmax[ABS_MT_TOUCH_MAJOR] = absInfo.absMtTouchMajorMax;
    dev_.absfuzz[ABS_MT_TOUCH_MAJOR] = 0;
    dev_.absflat[ABS_MT_TOUCH_MAJOR] = 0;

    dev_.absmin[ABS_MT_TOUCH_MINOR] = absInfo.absMtTouchMinorMin;
    dev_.absmax[ABS_MT_TOUCH_MINOR] = absInfo.absMtTouchMinorMax;
    dev_.absfuzz[ABS_MT_TOUCH_MINOR] = 0;
    dev_.absflat[ABS_MT_TOUCH_MINOR] = 0;

    dev_.absmin[ABS_MT_ORIENTATION] = absInfo.absMtOrientationMin;
    dev_.absmax[ABS_MT_ORIENTATION] = absInfo.absMtOrientationMax;
    dev_.absfuzz[ABS_MT_ORIENTATION] = 0;
    dev_.absflat[ABS_MT_ORIENTATION] = 0;

    dev_.absmin[ABS_MT_POSITION_X] = 0;
    dev_.absmax[ABS_MT_POSITION_X] = phyWidth;
    dev_.absfuzz[ABS_MT_POSITION_X] = 0;
    dev_.absflat[ABS_MT_POSITION_X] = 0;

    dev_.absmin[ABS_MT_POSITION_Y] = 0;
    dev_.absmax[ABS_MT_POSITION_Y] = phyHeight;
    dev_.absfuzz[ABS_MT_POSITION_Y] = 0;
    dev_.absflat[ABS_MT_POSITION_Y] = 0;

    dev_.absmin[ABS_MT_BLOB_ID] = absInfo.absMtBlobIdMin;
    dev_.absmax[ABS_MT_BLOB_ID] = absInfo.absMtBlobIdMax;
    dev_.absfuzz[ABS_MT_BLOB_ID] = 0;
    dev_.absflat[ABS_MT_BLOB_ID] = 0;

    dev_.absmin[ABS_MT_TRACKING_ID] = absInfo.absMtTrackingIdMin;
    dev_.absmax[ABS_MT_TRACKING_ID] = absInfo.absMtTrackingIdMax;
    dev_.absfuzz[ABS_MT_TRACKING_ID] = 0;
    dev_.absflat[ABS_MT_TRACKING_ID] = 0;

    dev_.absmin[ABS_MT_PRESSURE] = absInfo.absMtPressureMin;
    dev_.absmax[ABS_MT_PRESSURE] = absInfo.absMtPressureMax;
    dev_.absfuzz[ABS_MT_PRESSURE] = 0;
    dev_.absflat[ABS_MT_PRESSURE] = 0;
}

VirtualTouchScreen::~VirtualTouchScreen() {}

const std::vector<uint32_t>& VirtualTouchScreen::GetEventTypes() const
{
    return EVT_TYPES;
}

const std::vector<uint32_t>& VirtualTouchScreen::GetKeys() const
{
    return KEYS;
}

const std::vector<uint32_t>& VirtualTouchScreen::GetProperties() const
{
    return PROPERTIES;
}

const std::vector<uint32_t>& VirtualTouchScreen::GetAbs() const
{
    return ABS;
}

const std::vector<uint32_t>& VirtualTouchScreen::GetRelBits() const
{
    return RELBITS;
}
} // namespace DistributedInput
} // namespace DistributedHardware
} // namespace OHOS

