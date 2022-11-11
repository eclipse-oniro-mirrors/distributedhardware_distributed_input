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

#include "distributed_input_sinkmanager_test.h"

#include "accesstoken_kit.h"
#include "nativetoken_kit.h"
#include "if_system_ability_manager.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"
#include "nativetoken_kit.h"
#include "token_setproc.h"
#include "softbus_common.h"

#include "dinput_errcode.h"

using namespace testing::ext;
using namespace OHOS::Security::AccessToken;
using namespace OHOS::DistributedHardware::DistributedInput;
using namespace std;
namespace OHOS {
namespace DistributedHardware {
namespace DistributedInput {
void DistributedInputSinkManagerTest::SetUp()
{
    sinkManager_ = new DistributedInputSinkManager(DISTRIBUTED_HARDWARE_INPUT_SINK_SA_ID, true);

    uint64_t tokenId;
    const char *perms[2];
    perms[0] = OHOS_PERMISSION_DISTRIBUTED_SOFTBUS_CENTER;
    perms[1] = OHOS_PERMISSION_DISTRIBUTED_DATASYNC;
    NativeTokenInfoParams infoInstance = {
        .dcapsNum = 0,
        .permsNum = 2,
        .aclsNum = 0,
        .dcaps = NULL,
        .perms = perms,
        .acls = NULL,
        .processName = "dsoftbus_service",
        .aplStr = "system_core",
    };
    tokenId = GetAccessTokenId(&infoInstance);
    SetSelfTokenID(tokenId);
    OHOS::Security::AccessToken::AccessTokenKit::ReloadNativeTokenInfo();
}

void DistributedInputSinkManagerTest::TearDown()
{
    if (sinkManager_ != nullptr) {
        delete sinkManager_;
        sinkManager_ = nullptr;
    }
}

void DistributedInputSinkManagerTest::SetUpTestCase()
{
}

void DistributedInputSinkManagerTest::TearDownTestCase()
{
}

HWTEST_F(DistributedInputSinkManagerTest, InitAuto, testing::ext::TestSize.Level0)
{
    bool ret = sinkManager_->InitAuto();
    EXPECT_EQ(true, ret);
}

HWTEST_F(DistributedInputSinkManagerTest, Init, testing::ext::TestSize.Level0)
{
    int32_t ret = sinkManager_->Init();
    EXPECT_EQ(DH_SUCCESS, ret);
}

HWTEST_F(DistributedInputSinkManagerTest, GetStartTransFlag, testing::ext::TestSize.Level0)
{
    DInputServerType flag = DInputServerType::SINK_SERVER_TYPE;
    sinkManager_->SetStartTransFlag(flag);
    DInputServerType retFlag = sinkManager_->GetStartTransFlag();
    EXPECT_EQ(flag, retFlag);
}

HWTEST_F(DistributedInputSinkManagerTest, GetInputTypes, testing::ext::TestSize.Level0)
{
    uint32_t inputTypes = static_cast<uint32_t>(DInputDeviceType::MOUSE);
    sinkManager_->SetInputTypes(inputTypes);
    uint32_t retType = sinkManager_->GetInputTypes();
    EXPECT_EQ(inputTypes, retType);
}

HWTEST_F(DistributedInputSinkManagerTest, IsStopDhidOnCmdStillNeed01, testing::ext::TestSize.Level0)
{
    int32_t sessionId = 1;
    std::vector<std::string> dhIds;
    dhIds.push_back("Input_123123123123");
    dhIds.push_back("Input_456456456456");
    dhIds.push_back("Input_789789789789");
    sinkManager_->StoreStartDhids(sessionId, dhIds);

    std::string stopDhId = "Input_123123123123";
    bool ret = sinkManager_->IsStopDhidOnCmdStillNeed(sessionId, stopDhId);
    EXPECT_EQ(false, ret);
}

HWTEST_F(DistributedInputSinkManagerTest, IsStopDhidOnCmdStillNeed02, testing::ext::TestSize.Level0)
{
    int32_t sessionId = 1;
    std::vector<std::string> dhIds;
    dhIds.push_back("Input_123123123123");
    dhIds.push_back("Input_456456456456");
    dhIds.push_back("Input_789789789789");
    sinkManager_->StoreStartDhids(sessionId, dhIds);

    sessionId = 1000;
    std::string stopDhId = "Input_123123123123";
    bool ret = sinkManager_->IsStopDhidOnCmdStillNeed(sessionId, stopDhId);
    EXPECT_EQ(true, ret);
}

HWTEST_F(DistributedInputSinkManagerTest, DeleteStopDhids01, testing::ext::TestSize.Level0)
{
    int32_t sessionId = 1;
    std::vector<std::string> stopDhIds;
    std::vector<std::string> stopIndeedDhIds;
    stopDhIds.push_back("Input_123123123123");
    for (auto iter : stopDhIds) {
        sinkManager_->sharingDhIds_.insert(iter);
    }
    sinkManager_->sharingDhIdsMap_[sessionId] = sinkManager_->sharingDhIds_;
    sinkManager_->DeleteStopDhids(sessionId, stopDhIds, stopIndeedDhIds);
    EXPECT_EQ(0, sinkManager_->sharingDhIdsMap_.size());
}
} // namespace DistributedInput
} // namespace DistributedHardware
} // namespace OHOS