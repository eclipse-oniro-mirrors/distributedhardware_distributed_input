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

#include "white_list_util.h"

#include <cstring>
#include <fstream>
#include <sstream>

#include "distributed_hardware_log.h"
#include "dinput_errcode.h"

namespace OHOS {
namespace DistributedHardware {
namespace DistributedInput {
namespace {
    const char *g_filepath = "/system/profile/dinput_business_event_whitelist.cfg";
    const char *g_splitSymbol1 = ",";
    const char *g_splitSymbol2 = "|";
}

WhiteListUtil::WhiteListUtil()
{
}

WhiteListUtil::~WhiteListUtil()
{
}

WhiteListUtil &WhiteListUtil::GetInstance(void)
{
    static WhiteListUtil instance;
    return instance;
}

int32_t WhiteListUtil::Init(const std::string &deviceId)
{
    DHLOGI("%s called, deviceId=%s", __func__, deviceId.c_str());
    ClearWhiteList();

    if (deviceId.empty()) {
        // device id error
        DHLOGE("%s error, deviceId empty", __func__);
        return ERR_DH_INPUT_WHILTELIST_INIT_FAIL;
    }

    std::ifstream inFile(g_filepath, std::ios::in | std::ios::binary);
    if (!inFile.is_open()) {
        // file open error
        DHLOGE("%s error, file open fail path=%s", __func__, g_filepath);
        return ERR_DH_INPUT_WHILTELIST_INIT_FAIL;
    }

    TYPE_KEY_CODE_VEC vecKeyCode;
    TYPE_COMBINATION_KEY_VEC vecCombinationKey;
    TYPE_WHITE_LIST_VEC vecWhiteList;

    std::string line;
    while (getline(inFile, line)) {
        DHLOGI("%s called success, line=%s", __func__, line.c_str());

        vecKeyCode.clear();
        vecCombinationKey.clear();

        std::size_t pos1 = line.find(g_splitSymbol1);
        while (std::string::npos != pos1) {
            std::string column = line.substr(0, pos1);
            line = line.substr(pos1 + 1, line.size());
            pos1 = line.find(g_splitSymbol1);
            vecKeyCode.clear();
            ReadLineDataStepOne(column, vecKeyCode, vecCombinationKey);
        }

        if (!line.empty()) {
            int32_t keyCode = std::stoi(line);
            if (keyCode) {
                vecKeyCode.push_back(keyCode);
            }

            if (!vecKeyCode.empty()) {
                vecCombinationKey.push_back(vecKeyCode);
                vecKeyCode.clear();
            }
        }

        if (!vecCombinationKey.empty()) {
            vecWhiteList.push_back(vecCombinationKey);
            vecCombinationKey.clear();
        }
    }

    inFile.close();

    std::lock_guard<std::mutex> lock(mutex_);
    mapDeviceWhiteList_[deviceId] = vecWhiteList;

    DHLOGI("%s called success, deviceId=%s", __func__, deviceId.c_str());
    PrintWhiteList();
    return DH_SUCCESS;
}

int32_t WhiteListUtil::UnInit(void)
{
    DHLOGI("%s called", __func__);
    ClearWhiteList();
    return DH_SUCCESS;
}

void WhiteListUtil::ReadLineDataStepOne(std::string &column, TYPE_KEY_CODE_VEC &vecKeyCode,
                                        TYPE_COMBINATION_KEY_VEC &vecCombinationKey) const
{
    std::size_t pos2 = column.find(g_splitSymbol2);
    while (std::string::npos != pos2) {
        std::string single = column.substr(0, pos2);
        column = column.substr(pos2 + 1, column.size());
        pos2 = column.find(g_splitSymbol2);

        if (!single.empty()) {
            int32_t keyCode = std::stoi(single);
            if (keyCode) {
                vecKeyCode.push_back(keyCode);
            }
        }
    }

    if (!column.empty()) {
        int32_t keyCode = std::stoi(column);
        if (keyCode) {
            vecKeyCode.push_back(keyCode);
        }
    }

    if (!vecKeyCode.empty()) {
        vecCombinationKey.push_back(vecKeyCode);
        vecKeyCode.clear();
    }
}

int32_t WhiteListUtil::SyncWhiteList(const std::string &deviceId, const TYPE_WHITE_LIST_VEC &vecWhiteList)
{
    DHLOGI("%s called, deviceId=%s",
        __func__, deviceId.c_str());

    std::lock_guard<std::mutex> lock(mutex_);
    mapDeviceWhiteList_[deviceId] = vecWhiteList;
    PrintWhiteList();
    return DH_SUCCESS;
}

int32_t WhiteListUtil::ClearWhiteList(const std::string &deviceId)
{
    DHLOGI("%s called, deviceId=%s",
        __func__, deviceId.c_str());

    std::lock_guard<std::mutex> lock(mutex_);
    mapDeviceWhiteList_.erase(deviceId);
    return DH_SUCCESS;
}

int32_t WhiteListUtil::ClearWhiteList(void)
{
    std::lock_guard<std::mutex> lock(mutex_);
    TYPE_DEVICE_WHITE_LIST_MAP().swap(mapDeviceWhiteList_);
    return DH_SUCCESS;
}

int32_t WhiteListUtil::GetWhiteList(const std::string &deviceId, TYPE_WHITE_LIST_VEC &vecWhiteList)
{
    DHLOGI("%s called, deviceId=%s",
        __func__, deviceId.c_str());

    std::lock_guard<std::mutex> lock(mutex_);
    TYPE_DEVICE_WHITE_LIST_MAP::iterator iter = mapDeviceWhiteList_.find(deviceId);
    if (iter != mapDeviceWhiteList_.end()) {
        vecWhiteList = iter->second;
        DHLOGI("%s called success, deviceId=%s",
            __func__, deviceId.c_str());
        return DH_SUCCESS;
    }

    DHLOGE("%s called failure, deviceId=%s",
        __func__, deviceId.c_str());
    return ERR_DH_INPUT_WHILTELIST_GET_WHILTELIST_FAIL;
}

bool WhiteListUtil::CheckSubVecData(const TYPE_COMBINATION_KEY_VEC::iterator &iter2,
                                    const TYPE_KEY_CODE_VEC::iterator &iter3) const
{
    bool bIsMatching = false;
    for (TYPE_KEY_CODE_VEC::iterator iter4 = iter2->begin(); iter4 != iter2->end(); ++iter4) {
        if (*iter4 == *iter3) {
            bIsMatching = true;
            break;
        }
    }
    return bIsMatching;
}

bool WhiteListUtil::IsNeedFilterOut(const std::string &deviceId, const BusinessEvent &event)
{
    DHLOGI("%s called!", __func__);

    PrintWhiteList();

    std::lock_guard<std::mutex> lock(mutex_);
    if (mapDeviceWhiteList_.empty()) {
        DHLOGE("%s called, whilte list is empty!", __func__);
        return false;
    }

    TYPE_DEVICE_WHITE_LIST_MAP::iterator iter = mapDeviceWhiteList_.find(deviceId);
    if (iter == mapDeviceWhiteList_.end()) {
        DHLOGE("%s called, not find by deviceId!", __func__);
        return false;
    }

    DHLOGI(
        "%s called, deviceId=%s, keyCode=%d, keyAction=%d",
        __func__, deviceId.c_str(), event.keyCode, event.keyAction);

    TYPE_KEY_CODE_VEC vecKeyCode = event.pressedKeys;
    vecKeyCode.push_back(event.keyCode);
    vecKeyCode.push_back(event.keyAction);
    if (vecKeyCode.empty()) {
        DHLOGE("%s called, vecKeyCode is empty!", __func__);
        return false;
    }

    bool bIsMatching = false;
    TYPE_WHITE_LIST_VEC vecWhiteList = iter->second;
    for (TYPE_WHITE_LIST_VEC::iterator iter1 = vecWhiteList.begin(); iter1 != vecWhiteList.end(); ++iter1) {
        if (vecKeyCode.size() != iter1->size()) {
            DHLOGI(
                "%s called, vecKeyCodeSize=%d, iter1Size=%d",
                __func__, vecKeyCode.size(), iter1->size());
            continue;
        }

        TYPE_COMBINATION_KEY_VEC::iterator iter2 = iter1->begin();
        TYPE_KEY_CODE_VEC::iterator iter3 = vecKeyCode.begin();
        for (; iter2 != iter1->end() && iter3 != vecKeyCode.end(); ++iter2, ++iter3) {
            bIsMatching = false;
            bIsMatching = CheckSubVecData(iter2, iter3);
            if (!bIsMatching) {
                break;
            }
        }
        if (bIsMatching) {
            break;
        }
    }

    DHLOGI("%s called, bIsMatching=%d", __func__, bIsMatching);
    return bIsMatching;
}

void WhiteListUtil::SubPrintWhiteList(const TYPE_WHITE_LIST_VEC &vecWhiteList) const
{
    for (int32_t iIndex = 0; iIndex < vecWhiteList.size(); ++iIndex) {
        TYPE_COMBINATION_KEY_VEC vecCombinationKey = vecWhiteList[iIndex];
        for (int32_t jIndex = 0; jIndex < vecCombinationKey.size(); ++jIndex) {
            TYPE_KEY_CODE_VEC vecKeyCode = vecCombinationKey[jIndex];
            for (int32_t kIndex = 0; kIndex < vecKeyCode.size(); ++kIndex) {
                DHLOGI(
                    "PrintWhiteList [%d][%d][%d]=[%d]",
                    iIndex, jIndex, kIndex, vecKeyCode[kIndex]);
            }
        }
    }
}

void WhiteListUtil::PrintWhiteList(void)
{
    if (mapDeviceWhiteList_.empty()) {
        DHLOGI("%s called, mapDeviceWhiteList_ is empty!", __func__);
        return;
    }

    DHLOGI("%s begin!", __func__);
    for (TYPE_DEVICE_WHITE_LIST_MAP::iterator iter = mapDeviceWhiteList_.begin();
        iter != mapDeviceWhiteList_.end();
        ++iter) {
        DHLOGI("deviceId=%s", iter->first.c_str());
        TYPE_WHITE_LIST_VEC vecWhiteList = iter->second;
        SubPrintWhiteList(vecWhiteList);
        DHLOGI("%s next!", __func__);
    }
    DHLOGI("%s end!", __func__);
    return;
}
} // namespace DistributedInput
} // namespace DistributedHardware
} // namespace OHOS
