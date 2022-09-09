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

#include "distributed_input_sourceinject_test.h"

#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <thread>
#include <unistd.h>

#include <linux/input.h>

#include "event_handler.h"
#include "nlohmann/json.hpp"

#include "dinput_errcode.h"
#include "softbus_bus_center.h"

using namespace testing::ext;
using namespace OHOS::DistributedHardware::DistributedInput;
using namespace std;
namespace OHOS {
namespace DistributedHardware {
namespace DistributedInput {
void DistributedInputSourceInjectTest::SetUp()
{
}

void DistributedInputSourceInjectTest::TearDown()
{
}

void DistributedInputSourceInjectTest::SetUpTestCase()
{
}

void DistributedInputSourceInjectTest::TearDownTestCase()
{
}

HWTEST_F(DistributedInputSourceInjectTest, RegisterDistributedHardware01, testing::ext::TestSize.Level1)
{
    InputDevice pBuffer;
    pBuffer.name = "uinput_name_keyboard";
    pBuffer.bus = 0x03;
    pBuffer.vendor = 0x1234;
    pBuffer.product = 0xfedc;
    pBuffer.version = 1;
    pBuffer.physicalPath = "usb-hiusb-ehci-2.1/input1";
    pBuffer.uniqueId = "1";
    pBuffer.classes = INPUT_DEVICE_CLASS_KEYBOARD;
    pBuffer.descriptor = "afv4s8b1dr1b8er1bd65fb16redb1dfb18d1b56df1b68d";

    std::string devId = "y4umjym16tgn21m896f1nt2y1894ty61nty651m89t1m";
    std::string dhId = pBuffer.descriptor;
    std::string parameters;
    DistributedInputInject::GetInstance().StructTransJson(pBuffer, parameters);
    int32_t ret = DistributedInputInject::GetInstance().RegisterDistributedHardware(devId, dhId, parameters);
    EXPECT_EQ(DH_SUCCESS, ret);
}

HWTEST_F(DistributedInputSourceInjectTest, RegisterDistributedHardware02, testing::ext::TestSize.Level1)
{
    InputDevice pBuffer;
    pBuffer.name = "uinput_name_mouse";
    pBuffer.bus = 0x03;
    pBuffer.vendor = 0x1222;
    pBuffer.product = 0xfeda;
    pBuffer.version = 2;
    pBuffer.physicalPath = "usb-hiusb-ehci-2.1/input1";
    pBuffer.uniqueId = "2";
    pBuffer.classes = INPUT_DEVICE_CLASS_CURSOR;
    pBuffer.descriptor = "rt12r1nr81n521be8rb1erbe1w8bg1erb18";

    std::string devId = "1sdvsd1v5w1v2d1v8d1v562sd11v5sd1";
    std::string dhId = pBuffer.descriptor;
    std::string parameters;
    DistributedInputInject::GetInstance().StructTransJson(pBuffer, parameters);
    int32_t ret = DistributedInputInject::GetInstance().RegisterDistributedHardware(devId, dhId, parameters);
    EXPECT_EQ(DH_SUCCESS, ret);
}

HWTEST_F(DistributedInputSourceInjectTest, RegisterDistributedHardware03, testing::ext::TestSize.Level1)
{
    InputDevice pBuffer;
    pBuffer.name = "uinput_name_touch";
    pBuffer.bus = 0x03;
    pBuffer.vendor = 0x1233;
    pBuffer.product = 0xfedb;
    pBuffer.version = 3;
    pBuffer.physicalPath = "usb-hiusb-ehci-2.1/input1";
    pBuffer.uniqueId = "3";
    pBuffer.classes = INPUT_DEVICE_CLASS_TOUCH;
    pBuffer.descriptor = "1ds56v18e1v21v8v1erv15r1v8r1j1ty8";

    std::string devId = "umkyu1b165e1be98151891erbe8r91ev";
    std::string dhId = pBuffer.descriptor;
    std::string parameters;
    DistributedInputInject::GetInstance().StructTransJson(pBuffer, parameters);
    int32_t ret = DistributedInputInject::GetInstance().RegisterDistributedHardware(devId, dhId, parameters);
    EXPECT_EQ(DH_SUCCESS, ret);
}

HWTEST_F(DistributedInputSourceInjectTest, RegisterDistributedEvent01, testing::ext::TestSize.Level1)
{
    struct RawEvent writeBuffer[4];
    RawEvent* event = writeBuffer;

    event->when = 0;
    event->type = EV_KEY;
    event->code = KEY_D;
    event->value = 1;
    event->descriptor = "afv4s8b1dr1b8er1bd65fb16redb1dfb18d1b56df1b68d";
    event += 1;

    event->when = 1;
    event->type = EV_KEY;
    event->code = KEY_D;
    event->value = 0;
    event->descriptor = "afv4s8b1dr1b8er1bd65fb16redb1dfb18d1b56df1b68d";
    event += 1;

    event->when = 2;
    event->type = EV_KEY;
    event->code = KEY_D;
    event->value = 1;
    event->descriptor = "afv4s8b1dr1b8er1bd65fb16redb1dfb18d1b56df1b68d";
    event += 1;

    event->when = 3;
    event->type = EV_KEY;
    event->code = KEY_D;
    event->value = 0;
    event->descriptor = "afv4s8b1dr1b8er1bd65fb16redb1dfb18d1b56df1b68d";

    size_t count = (size_t)(sizeof(writeBuffer) / sizeof(RawEvent));
    int32_t ret = DistributedInputInject::GetInstance().RegisterDistributedEvent(writeBuffer, count);
    EXPECT_EQ(DH_SUCCESS, ret);
}

HWTEST_F(DistributedInputSourceInjectTest, RegisterDistributedEvent02, testing::ext::TestSize.Level1)
{
    struct RawEvent writeBuffer[4];
    RawEvent* event = writeBuffer;

    event->when = 0;
    event->type = EV_REL;
    event->code = REL_X;
    event->value = 2;
    event->descriptor = "rt12r1nr81n521be8rb1erbe1w8bg1erb18";
    event += 1;

    event->when = 1;
    event->type = EV_REL;
    event->code = REL_Y;
    event->value = 2;
    event->descriptor = "rt12r1nr81n521be8rb1erbe1w8bg1erb18";
    event += 1;

    event->when = 2;
    event->type = EV_REL;
    event->code = REL_X;
    event->value = 2;
    event->descriptor = "rt12r1nr81n521be8rb1erbe1w8bg1erb18";
    event += 1;

    event->when = 3;
    event->type = EV_REL;
    event->code = REL_Y;
    event->value = 2;
    event->descriptor = "rt12r1nr81n521be8rb1erbe1w8bg1erb18";

    event->when = 4;
    event->type = EV_SYN;
    event->code = SYN_REPORT;
    event->value = 0;
    event->descriptor = "rt12r1nr81n521be8rb1erbe1w8bg1erb18";

    size_t count = (size_t)(sizeof(writeBuffer) / sizeof(RawEvent));
    int32_t ret = DistributedInputInject::GetInstance().RegisterDistributedEvent(writeBuffer, count);
    EXPECT_EQ(DH_SUCCESS, ret);
}

HWTEST_F(DistributedInputSourceInjectTest, RegisterDistributedEvent03, testing::ext::TestSize.Level1)
{
    struct RawEvent writeBuffer[4];
    RawEvent* event = writeBuffer;

    event->when = 0;
    event->type = EV_ABS;
    event->code = ABS_X;
    event->value = 1;
    event->descriptor = "1ds56v18e1v21v8v1erv15r1v8r1j1ty8";
    event += 1;

    event->when = 1;
    event->type = EV_ABS;
    event->code = ABS_X;
    event->value = 2;
    event->descriptor = "1ds56v18e1v21v8v1erv15r1v8r1j1ty8";
    event += 1;

    event->when = 2;
    event->type = EV_ABS;
    event->code = ABS_X;
    event->value = 3;
    event->descriptor = "1ds56v18e1v21v8v1erv15r1v8r1j1ty8";
    event += 1;

    event->when = 3;
    event->type = EV_ABS;
    event->code = ABS_X;
    event->value = 4;
    event->descriptor = "1ds56v18e1v21v8v1erv15r1v8r1j1ty8";

    size_t count = (size_t)(sizeof(writeBuffer) / sizeof(RawEvent));
    int32_t ret = DistributedInputInject::GetInstance().RegisterDistributedEvent(writeBuffer, count);
    EXPECT_EQ(DH_SUCCESS, ret);
}
} // namespace DistributedInput
} // namespace DistributedHardware
} // namespace OHOS
