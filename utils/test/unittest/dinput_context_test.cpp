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

#include "dinput_context_test.h"

#include "dinput_context.h"
#include "dinput_errcode.h"

using namespace testing::ext;
using namespace OHOS::DistributedHardware::DistributedInput;
using namespace std;

namespace OHOS {
namespace DistributedHardware {
namespace DistributedInput {
constexpr uint32_t SIZE_AFTER_GET = 1;
constexpr uint32_t HEIGHT = 1080;

void DInputContextTest::SetUp()
{
}

void DInputContextTest::TearDown()
{
}

void DInputContextTest::SetUpTestCase()
{
}

void DInputContextTest::TearDownTestCase()
{
}

HWTEST_F(DInputContextTest, GetSourceWindId001, testing::ext::TestSize.Level0)
{
    std::string devId = "hello";
    uint64_t sourceWinId = 1;
    std::string ret = DInputContext::GetInstance().GetScreenInfoKey(devId, sourceWinId);
    EXPECT_EQ("hello###1", ret);
}

HWTEST_F(DInputContextTest, RemoveSinkScreenInfo001, testing::ext::TestSize.Level0)
{
    std::string sourceWinId = "hello";
    int32_t ret = DInputContext::GetInstance().RemoveSinkScreenInfo(sourceWinId);
    EXPECT_EQ(DH_SUCCESS, ret);
}

HWTEST_F(DInputContextTest, UpdateSinkScreenInfo001, testing::ext::TestSize.Level0)
{
    std::string sourceWinId = "hello";
    SinkScreenInfo sinkScreenInfo;
    int32_t ret = DInputContext::GetInstance().UpdateSinkScreenInfo(sourceWinId, sinkScreenInfo);
    EXPECT_EQ(ERR_DH_INPUT_CONTEXT_KEY_NOT_EXIST, ret);
}

HWTEST_F(DInputContextTest, UpdateSinkScreenInfo002, testing::ext::TestSize.Level0)
{
    std::string sourceWinId = "hello";
    SinkScreenInfo sinkScreenInfo = DInputContext::GetInstance().GetSinkScreenInfo(sourceWinId);
    int32_t ret = DInputContext::GetInstance().UpdateSinkScreenInfo(sourceWinId, sinkScreenInfo);
    EXPECT_EQ(DH_SUCCESS, ret);
    DInputContext::GetInstance().RemoveSinkScreenInfo(sourceWinId);
}

HWTEST_F(DInputContextTest, GetSinkScreenInfo001, testing::ext::TestSize.Level0)
{
    std::string sourceWinId = "hello";
    SinkScreenInfo sinkScreenInfo = DInputContext::GetInstance().GetSinkScreenInfo(sourceWinId);
    EXPECT_EQ(SIZE_AFTER_GET, DInputContext::GetInstance().sinkScreenInfoMap_.size());
    DInputContext::GetInstance().RemoveSinkScreenInfo(sourceWinId);
}

HWTEST_F(DInputContextTest, GetSinkScreenInfo002, testing::ext::TestSize.Level0)
{
    std::string sourceWinId = "hello";
    SinkScreenInfo sinkScreenInfo1;
    sinkScreenInfo1.sinkPhyHeight = HEIGHT;
    DInputContext::GetInstance().sinkScreenInfoMap_[sourceWinId] = sinkScreenInfo1;
    SinkScreenInfo sinkScreenInfo = DInputContext::GetInstance().GetSinkScreenInfo(sourceWinId);
    EXPECT_EQ(HEIGHT, sinkScreenInfo.sinkPhyHeight);
    DInputContext::GetInstance().RemoveSinkScreenInfo(sourceWinId);
}

HWTEST_F(DInputContextTest, RemoveSrcScreenInfo001, testing::ext::TestSize.Level0)
{
    std::string sourceWinId = "hello";
    int32_t ret = DInputContext::GetInstance().RemoveSrcScreenInfo(sourceWinId);
    EXPECT_EQ(DH_SUCCESS, ret);
}

HWTEST_F(DInputContextTest, UpdateSrcScreenInfo001, testing::ext::TestSize.Level0)
{
    std::string sourceWinId = "hello";
    SrcScreenInfo srcScreenInfo;
    int32_t ret = DInputContext::GetInstance().UpdateSrcScreenInfo(sourceWinId, srcScreenInfo);
    EXPECT_EQ(ERR_DH_INPUT_CONTEXT_KEY_NOT_EXIST, ret);
}

HWTEST_F(DInputContextTest, UpdateSrcScreenInfo002, testing::ext::TestSize.Level0)
{
    std::string sourceWinId = "hello";
    SrcScreenInfo srcScreenInfo = DInputContext::GetInstance().GetSrcScreenInfo(sourceWinId);
    int32_t ret = DInputContext::GetInstance().UpdateSrcScreenInfo(sourceWinId, srcScreenInfo);
    EXPECT_EQ(DH_SUCCESS, ret);
    DInputContext::GetInstance().RemoveSrcScreenInfo(sourceWinId);
}

HWTEST_F(DInputContextTest, GetSrcScreenInfo001, testing::ext::TestSize.Level0)
{
    std::string sourceWinId = "hello";
    SrcScreenInfo srcScreenInfo = DInputContext::GetInstance().GetSrcScreenInfo(sourceWinId);
    EXPECT_EQ(SIZE_AFTER_GET, DInputContext::GetInstance().srcScreenInfoMap_.size());
    DInputContext::GetInstance().RemoveSrcScreenInfo(sourceWinId);
}

HWTEST_F(DInputContextTest, GetSrcScreenInfo002, testing::ext::TestSize.Level0)
{
    std::string sourceWinId = "hello";
    SrcScreenInfo srcScreenInfo1;
    srcScreenInfo1.sourcePhyHeight = HEIGHT;
    DInputContext::GetInstance().srcScreenInfoMap_[sourceWinId] = srcScreenInfo1;
    SrcScreenInfo srcScreenInfo = DInputContext::GetInstance().GetSrcScreenInfo(sourceWinId);
    EXPECT_EQ(HEIGHT, srcScreenInfo.sourcePhyHeight);
    DInputContext::GetInstance().RemoveSrcScreenInfo(sourceWinId);
}

HWTEST_F(DInputContextTest, SetGetLocalTouchScreenInfo001, testing::ext::TestSize.Level0)
{
    LocalTouchScreenInfo localTouchScreenInfo;
    localTouchScreenInfo.sinkShowWidth = HEIGHT;
    DInputContext::GetInstance().SetLocalTouchScreenInfo(localTouchScreenInfo);
    EXPECT_EQ(HEIGHT, DInputContext::GetInstance().GetLocalTouchScreenInfo().sinkShowWidth);
}

HWTEST_F(DInputContextTest, CalculateTransformInfo001, testing::ext::TestSize.Level0)
{
    SinkScreenInfo sinkScreenInfo;
    int32_t ret = DInputContext::GetInstance().CalculateTransformInfo(sinkScreenInfo);
    EXPECT_EQ(ERR_DH_INPUT_CONTEXT_CALCULATE_FAIL, ret);
}

HWTEST_F(DInputContextTest, CalculateTransformInfo002, testing::ext::TestSize.Level0)
{
    SinkScreenInfo sinkScreenInfo;
    sinkScreenInfo.sinkPhyHeight = 1080;
    sinkScreenInfo.sinkPhyWidth = 960;
    sinkScreenInfo.sinkShowHeight = 1080;
    sinkScreenInfo.sinkShowWidth = 960;
    int32_t ret = DInputContext::GetInstance().CalculateTransformInfo(sinkScreenInfo);
    EXPECT_EQ(ERR_DH_INPUT_CONTEXT_CALCULATE_FAIL, ret);
}

HWTEST_F(DInputContextTest, CalculateTransformInfo003, testing::ext::TestSize.Level0)
{
    SinkScreenInfo sinkScreenInfo;
    sinkScreenInfo.sinkPhyHeight = 1080;
    sinkScreenInfo.sinkPhyWidth = 960;
    sinkScreenInfo.sinkShowHeight = 1080;
    sinkScreenInfo.sinkShowWidth = 960;
    sinkScreenInfo.sinkProjShowHeight = 640;
    sinkScreenInfo.sinkProjShowWidth = 480;
    int32_t ret = DInputContext::GetInstance().CalculateTransformInfo(sinkScreenInfo);
    EXPECT_EQ(DH_SUCCESS, ret);
}
} // namespace DistributedInput
} // namespace DistributedHardware
} // namespace OHOS
