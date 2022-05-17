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

#include "distributed_input_test.h"
#include "idistributed_hardware_source.h"
#include "ipc_skeleton.h"
#include "softbus_bus_center.h"
#include "nlohmann/json.hpp"

#include <cstring>
#include <unistd.h>
#include <thread>

using namespace testing::ext;
using namespace OHOS::DistributedHardware::DistributedInput;
using namespace std;

namespace OHOS {
namespace DistributedHardware {
namespace DistributedInput {
namespace {
    const int32_t TEST_START_SINK = 1;
    const int32_t TEST_START_SOURCE = 2;
    const int32_t TEST_GET_DEVICEINFO = 3;
    const int32_t TEST_GET_HARDWAREINFO = 4;
    const int32_t TEST_IS_START_INPUT = 5;
    const int32_t TEST_IS_NEED_FILTEROUT = 6;
    const int32_t TEST_STOP_SINK = 7;
    const int32_t TEST_STOP_SOURCE = 8;
    const int32_t TEST_HELP = 9;
    const int32_t TEST_REGISTERHARDWARE = 11;
    const int32_t TEST_PREPAREREMOTE = 12;
    const int32_t TEST_STARTREMOTE = 13;
    const int32_t TEST_STOPREMOTE = 14;
    const int32_t TEST_UN_PREPAREREMOTE = 15;
    const int32_t TEST_UN_REGISTERHARDWARE = 16;
}
void DistributedInputTest::SetUp()
{
}

void DistributedInputTest::TearDown()
{
}

void DistributedInputTest::SetUpTestCase()
{
}

void DistributedInputTest::TearDownTestCase()
{
}

void DistributedInputTest::TestPluginListener::PluginHardware(
    const std::string &dhId, const std::string &attrs)
{
    std::cout << std::endl;
    std::cout << "TestPluginListener::PluginHardware " << std::endl;
    std::cout << "dhId::" << dhId <<std::endl;
    std::cout << "attrs::" << attrs <<std::endl;
    std::cout << "Enter the case No : " <<std::endl;
}

void DistributedInputTest::TestPluginListener::UnPluginHardware(
    const std::string &dhId)
{
    std::cout << std::endl;
    std::cout << "TestPluginListener::UnPluginHardware " << std::endl;
    std::cout << "dhId::" << dhId <<std::endl;
    std::cout << "Enter the case No : " <<std::endl;
}

int32_t DistributedInputTest::TestRegisterInputCallback::OnRegisterResult(
    const std::string &devId, const std::string &dhId, int32_t status, const std::string &data)
{
    std::cout << std::endl;
    std::cout << "RegisterInputCallback::OnRegisterResult " << std::endl;
    std::cout << "devId::" << devId <<std::endl;
    std::cout << "dhId::" << dhId <<std::endl;
    std::cout << "status::" << status <<std::endl;
    std::cout << "Enter the case No : " <<std::endl;
    return SUCCESS;
}

int32_t DistributedInputTest::TestUnregisterInputCallback::OnUnregisterResult(
    const std::string &devId, const std::string &dhId, int32_t status, const std::string &data)
{
    std::cout << std::endl;
    std::cout << "UnregisterInputCallback::OnUnregisterResult " << std::endl;
    std::cout << "devId::" << devId <<std::endl;
    std::cout << "dhId::" << dhId <<std::endl;
    std::cout << "status::" << status <<std::endl;
    std::cout << "Enter the case No : " <<std::endl;
    return SUCCESS;
}

void DistributedInputTest::TestPrepareDInputCallback::OnResult(
    const std::string& deviceId, const int32_t& status)
{
    std::cout << std::endl;
    std::cout << "IPrepareDInputCallback::OnResult " << std::endl;
    std::cout << "deviceId::" << deviceId <<std::endl;
    std::cout << "status::" << status <<std::endl;
    std::cout << "Enter the case No : " <<std::endl;
    return;
}

void DistributedInputTest::TestUnprepareDInputCallback::OnResult(
    const std::string& deviceId, const int32_t& status)
{
    std::cout << std::endl;
    std::cout << "IUnprepareDInputCallback::OnResult " << std::endl;
    std::cout << "deviceId::" << deviceId <<std::endl;
    std::cout << "status::" << status <<std::endl;
    std::cout << "Enter the case No : " <<std::endl;
    return;
}

void DistributedInputTest::TestStartDInputCallback::OnResult(
    const std::string& deviceId, const uint32_t& inputTypes, const int32_t& status)
{
    std::cout << std::endl;
    std::cout << "IStartDInputCallback::OnResult " << std::endl;
    std::cout << "deviceId::" << deviceId <<std::endl;
    std::cout << "inputTypes::" << inputTypes <<std::endl;
    std::cout << "status::" << status <<std::endl;
    std::cout << "Enter the case No : " <<std::endl;
    return;
}

void DistributedInputTest::TestStopDInputCallback::OnResult(
    const std::string& deviceId, const uint32_t& inputTypes, const int32_t& status)
{
    std::cout << std::endl;
    std::cout << "TestStopDInputCallback::OnResult " << std::endl;
    std::cout << "deviceId::" << deviceId <<std::endl;
    std::cout << "inputTypes::" << inputTypes <<std::endl;
    std::cout << "status::" << status <<std::endl;
    std::cout << "Enter the case No : " <<std::endl;
    return;
}

void DistributedInputTest::StartSourceSa()
{
    std::cout << "StartSourceSa::start " << std::endl;
    int ret = DistributedInputSourceHandler::GetInstance().InitSource("");
    if (ret == SUCCESS) {
        std::cout << "StartSourceSa is succese! " << std::endl;
    } else {
        std::cout << "StartSourceSa is fall! " << std::endl;
    }

    std::cout << "StartSourceSa::end " << std::endl;
}

void DistributedInputTest::StartSinkSa()
{
    std::cout << "StartSinkSa::start " << std::endl;
    int ret = DistributedInputSinkHandler::GetInstance().InitSink("");
    if (ret == SUCCESS) {
        std::cout << "StartSinkSa is succese! " << std::endl;
    } else {
        std::cout << "StartSinkSa is fall! " << std::endl;
    }
    std::cout << "StartSinkSa::end " << std::endl;
}

void DistributedInputTest::GetHardWhereInfo()
{
    std::cout << "GetHardWhereInfo::start " << std::endl;
    std::cout << "HarewareInfo::" << std::endl;
    DistributedInputHandler::GetInstance().Initialize();
    std::vector<OHOS::DistributedHardware::DHItem> items = DistributedInputHandler::GetInstance().Query();
    int index = 0;
    for (auto item : items) {
        std::cout << "dhId :: " << std::endl;
        std::cout << item.dhId << std::endl;
        std::cout << "attrs:: " << std::endl;
        index = 0;
        while ((index = item.attrs.find(' ', index)) != string::npos) {
            item.attrs.erase(index, 1);
        }
        std::cout << item.attrs << std::endl;
    }
    std::cout << "GetHardWhereInfo::end " << std::endl;
}

void DistributedInputTest::GetDeviceInfo()
{
    std::cout << "GetDeviceInfo::start " << std::endl;

    NodeBasicInfo *info = NULL;
    int32_t infoNum = 0;
    GetAllNodeDeviceInfo("ohos.distributedhardware.devicemanager", &info, &infoNum);
    std::cout << "DeviceInfo::" << std::endl;
    for (int32_t i = 0; i < infoNum; i++) {
        std::cout << "networkId::" << std::endl;
        std::cout << info->networkId << std::endl;
        std::cout << "deviceName::" << std::endl;
        std::cout << info->deviceName << std::endl;
        std::cout << std::endl;
        info++;
    }
    std::cout << "GetDeviceInfo::end " << std::endl;
}

void DistributedInputTest::IsStartDistributedInput()
{
    std::cout << "IsStartDistributedInput::start " << std::endl;
    
    uint32_t inputType = 0;
    std::cout << "inputType::";
    std::cin >> inputType;

    DInputServerType type = DistributedInputKit::IsStartDistributedInput(inputType);
    if (type == DInputServerType::SOURCE_SERVER_TYPE) {
        std::cout << "The inputed type is using in source sa! " << std::endl;
    } else if (type == DInputServerType::SINK_SERVER_TYPE) {
        std::cout << "The inputed type is using in sink sa! " << std::endl;
    } else {
        std::cout << "No sa is using! " << std::endl;
    }

    std::cout << "IsStartDistributedInput::end " << std::endl;
}

void DistributedInputTest::IsNeedFilterOut()
{
    std::cout << "IsNeedFilterOut::start " << std::endl;

    string deviceId = "";
    int32_t pressedKey1 = 0;
    int32_t pressedKey2 = 0;
    int32_t keyCode = 0;
    int32_t keyAction = 0;
    BusinessEvent event;

    std::cout << "deviceId::";
    std::cin >> deviceId;
    std::cout << "BusinessEvent::" << std::endl;
    std::cout << "pressedKey1::";
    std::cin >> pressedKey1;
    std::cout << "pressedKey2::";
    std::cin >> pressedKey2;
    std::cout << "keyCode::";
    std::cin >> keyCode;
    std::cout << "keyAction::";
    std::cin >> keyAction;

    event.pressedKeys.push_back(pressedKey1);
    event.pressedKeys.push_back(pressedKey2);
    event.keyCode = keyCode;
    event.keyAction = keyAction;

    bool ret = DistributedInputKit::IsNeedFilterOut(deviceId, event);
    if (ret == true) {
        std::cout << " Result::true ! " << std::endl;
        std::cout << " The businessEvent is not in the white list ! " << std::endl;
        std::cout << " The businessEvent take effect at the source! " << std::endl;
    } else {
        std::cout << " Result::false ! " << std::endl;
        std::cout << " The businessEvent is in the white list ! " << std::endl;
        std::cout << " The businessEvent don`t take effect at the source! " << std::endl;
    }

    std::cout << "IsNeedFilterOut::end " << std::endl;
}

void DistributedInputTest::RegisterDistributedHardware()
{
    std::cout << "RegisterDistributedHardware::start " << std::endl;

    string rDevId = "";
    string rDhId = "";
    string attrs = "";

    std::cout << "devId::";
    std::cin >> rDevId;
    std::cout << "dhId::";
    std::cin >> rDhId;
    std::cout << "params::";
    std::cin >> attrs;

    enableParam.version = "1.0";
    enableParam.attrs = attrs;

    registerCb = std::make_shared<TestRegisterInputCallback>();
    int32_t ret = DistributedInputSourceHandler::GetInstance().RegisterDistributedHardware(
        rDevId, rDhId, enableParam, registerCb);
    if (ret == SUCCESS) {
        std::cout << "RegisterDistributedHardware is succese! " << std::endl;
    } else {
        std::cout << "RegisterDistributedHardware is fall! " << std::endl;
    }

    std::cout << "RegisterDistributedHardware::end " << std::endl;
}

void DistributedInputTest::PrepareRemoteInput()
{
    std::cout << "PrepareRemoteInput::start " << std::endl;

    string rDevId = "";

    std::cout << "devId::";
    std::cin >> rDevId;

    prepareCb = new(std::nothrow) TestPrepareDInputCallback();
    int32_t ret = DistributedInputKit::PrepareRemoteInput(rDevId, prepareCb);
    if (ret == SUCCESS) {
        std::cout << "PrepareRemoteInput is succese! " << std::endl;
    } else {
        std::cout << "PrepareRemoteInput is fall! " << std::endl;
    }

    std::cout << "PrepareRemoteInput::end " << std::endl;
}

void DistributedInputTest::UnprepareRemoteInput()
{
    std::cout << "UnprepareRemoteInput::start " << std::endl;

    string rDevId = "";

    std::cout << "devId::";
    std::cin >> rDevId;

    unprepareCb = new(std::nothrow) TestUnprepareDInputCallback();
    int32_t ret = DistributedInputKit::UnprepareRemoteInput(rDevId, unprepareCb);
    if (ret == SUCCESS) {
        std::cout << "UnprepareRemoteInput is succese! " << std::endl;
    } else {
        std::cout << "UnprepareRemoteInput is fall! " << std::endl;
    }
    std::cout << "UnprepareRemoteInput::end " << std::endl;
}

void DistributedInputTest::StartRemoteInput()
{
    std::cout << "StartRemoteInput::start " << std::endl;
    std::cout << "You can input 1 ~ 7" << std::endl;
    std::cout << "1:mouse " << std::endl;
    std::cout << "2:keyboard " << std::endl;
    std::cout << "4:touch " << std::endl;
    std::cout << "7:all " << std::endl;

    string rDevId = "";
    uint32_t rInputType = 0;
    std::cout << "inputType::";
    std::cin >> rInputType;
    std::cout << "devId::";
    std::cin >> rDevId;

    startCb = new(std::nothrow) TestStartDInputCallback();
    int32_t ret = DistributedInputKit::StartRemoteInput(rDevId, rInputType, startCb);
    if (ret == SUCCESS) {
        std::cout << "StartRemoteInput is succese! " << std::endl;
    } else {
        std::cout << "StartRemoteInput is fall! " << std::endl;
    }

    std::cout << "StartRemoteInput::end " << std::endl;
}

void DistributedInputTest::StopRemoteInput()
{
    std::cout << "StopRemoteInput::start " << std::endl;
    std::cout << "You can input 1 ~ 7" << std::endl;
    std::cout << "1:mouse " << std::endl;
    std::cout << "2:keyboard " << std::endl;
    std::cout << "4:touch " << std::endl;
    std::cout << "7:all " << std::endl;

    string rDevId = "";
    uint32_t rInputType = 0;

    std::cout << "devId::";
    std::cin >> rDevId;

    std::cout << "inputType::";
    std::cin >> rInputType;

    stopCb = new(std::nothrow) TestStopDInputCallback();
    int32_t ret = DistributedInputKit::StopRemoteInput(rDevId, rInputType, stopCb);
    if (ret == SUCCESS) {
        std::cout << "StopRemoteInput is succese! " << std::endl;
    } else {
        std::cout << "StopRemoteInput is fall! " << std::endl;
    }

    std::cout << "StopRemoteInput::end " << std::endl;
}

void DistributedInputTest::UnregisterDistributedHardware()
{
    std::cout << "UnregisterDistributedHardware::start " << std::endl;

    string rDevId = "";
    string rDhId = "";
    string attrs = "";

    std::cout << "devId::";
    std::cin >> rDevId;
    std::cout << "dhId::";
    std::cin >> rDhId;

    unregisterCb = std::make_shared<TestUnregisterInputCallback>();
    int32_t ret = DistributedInputSourceHandler::GetInstance().UnregisterDistributedHardware(
        rDevId, rDhId, unregisterCb);
    if (ret == SUCCESS) {
        std::cout << "UnregisterDistributedHardware is succese! " << std::endl;
    } else {
        std::cout << "UnregisterDistributedHardware is fall! " << std::endl;
    }

    std::cout << "UnregisterDistributedHardware::end " << std::endl;
}

void DistributedInputTest::StopSourceSa()
{
    std::cout << "StopSourceSa::start " << std::endl;
    int ret = DistributedInputSourceHandler::GetInstance().ReleaseSource();
    if (ret == SUCCESS) {
        std::cout << "StopSourceSa is succese! " << std::endl;
    } else {
        std::cout << "StopSourceSa is fall! " << std::endl;
    }

    std::cout << "StopSourceSa::end " << std::endl;
}

void DistributedInputTest::StopSinkSa()
{
    std::cout << "StopSinkSa::start " << std::endl;
    int ret = DistributedInputSinkHandler::GetInstance().ReleaseSink();
    if (ret == SUCCESS) {
        std::cout << "StopSinkSa is succese! " << std::endl;
    } else {
        std::cout << "StopSinkSa is fall! " << std::endl;
    }
    std::cout << "StopSinkSa::end " << std::endl;
}

void DistributedInputTest::Help()
{
    std::cout << "************************************************************" << std::endl;
    std::cout << "Welcome to the dinput source test demo!" << std::endl;
    std::cout << "1 ::StartSinkSa " << std::endl;
    std::cout << "2 ::StartSourceSa " << std::endl;
    std::cout << "3 ::GetDeviceInfo " << std::endl;
    std::cout << "4 ::GetHardWhereInfo " << std::endl;
    std::cout << "5 ::IsStartDistributedInput " << std::endl;
    std::cout << "6 ::IsNeedFilterOut " << std::endl;
    std::cout << "7 ::StopSinkSa " << std::endl;
    std::cout << "8 ::StopSourceSa " << std::endl;
    std::cout << "9 ::Help " << std::endl;
    std::cout << "0 ::Exit " << std::endl;
    std::cout << "11::RegisterDistributedHardware " << std::endl;
    std::cout << "12::PrepareRemoteInput " << std::endl;
    std::cout << "13::StartRemoteInput " << std::endl;
    std::cout << "14::StopRemoteInput " << std::endl;
    std::cout << "15::UnprepareRemoteInput " << std::endl;
    std::cout << "16::UnregisterDistributedHardware " << std::endl;
    std::cout << "************************************************************" << std::endl;
}

void DistributedInputTest::SwitchCase(int32_t in)
{
    switch (in) {
        case TEST_START_SINK:
            StartSinkSa();
            break;
        case TEST_START_SOURCE:
            StartSourceSa();
            break;
        case TEST_GET_DEVICEINFO:
            GetDeviceInfo();
            break;
        case TEST_GET_HARDWAREINFO:
            GetHardWhereInfo();
            break;
        case TEST_IS_START_INPUT:
            IsStartDistributedInput();
            break;
        case TEST_IS_NEED_FILTEROUT:
            IsNeedFilterOut();
            break;
        case TEST_STOP_SINK:
            StopSinkSa();
            break;
        case TEST_STOP_SOURCE:
            StopSourceSa();
            break;
        case TEST_HELP:
            Help();
            break;
        case TEST_REGISTERHARDWARE:
            RegisterDistributedHardware();
            break;
        case TEST_PREPAREREMOTE:
            PrepareRemoteInput();
            break;
        case TEST_STARTREMOTE:
            StartRemoteInput();
            break;
        case TEST_STOPREMOTE:
            StopRemoteInput();
            break;
        case TEST_UN_PREPAREREMOTE:
            UnprepareRemoteInput();
            break;
        case TEST_UN_REGISTERHARDWARE:
            UnregisterDistributedHardware();
            break;
        default:
            break;
    }
}

HWTEST_F(DistributedInputTest, SystemTest01, testing::ext::TestSize.Level0)
{
    Help();
    pluginListener = std::make_shared<TestPluginListener>();
    DistributedInputHandler::GetInstance().RegisterPluginListener(pluginListener);

    int32_t in;
    while (true) {
        std::cout << std::endl;
        std::cout << "Enter the case No : " << std::endl;
        std::cin >> in;
        if (in == 0) {
            registerCb = nullptr;
            unregisterCb = nullptr;
            prepareCb = nullptr;
            unprepareCb = nullptr;
            startCb = nullptr;
            stopCb = nullptr;
            break;
        }
        SwitchCase(in);
    }
}
} // namespace DistributedInput
} // namespace DistributedHardware
} // namespace OHOS
