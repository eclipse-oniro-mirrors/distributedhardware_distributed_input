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

#include "white_list_util.h"

#include <cstring>
#include <fstream>
#include <sstream>

#include "anonymous_string.h"
#include "distributed_hardware_log.h"

#include "dinput_errcode.h"

namespace OHOS {
namespace DistributedHardware {
namespace DistributedInput {
namespace {
    const char* WHITE_LIST_FILE_PATH = "/etc/dinput_business_event_whitelist.cfg";
    const char* SPLIT_LINE = "|";
    const char* SPLIT_COMMA = ",";
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
    DHLOGI("start, deviceId=%s", GetAnonyString(deviceId).c_str());
    ClearWhiteList();

    if (deviceId.empty()) {
        // device id error
        DHLOGE("%s error, deviceId empty", __func__);
        return ERR_DH_INPUT_WHILTELIST_INIT_FAIL;
    }

    std::ifstream inFile(WHITE_LIST_FILE_PATH, std::ios::in | std::ios::binary);
    if (!inFile.is_open()) {
        // file open error
        DHLOGE("%s error, file open fail path=%s", __func__, WHITE_LIST_FILE_PATH);
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

        std::size_t pos1 = line.find(SPLIT_COMMA);
        while (std::string::npos != pos1) {
            std::string column = line.substr(0, pos1);
            line = line.substr(pos1 + 1, line.size());
            pos1 = line.find(SPLIT_COMMA);
            vecKeyCode.clear();
            ReadLineDataStepOne(column, vecKeyCode, vecCombinationKey);
        }

        if (!line.empty()) {
            int32_t keyCode = std::stoi(line);
            if (keyCode != 0) {
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

    DHLOGI("success, deviceId=%s", GetAnonyString(deviceId).c_str());
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
    std::size_t pos2 = column.find(SPLIT_LINE);
    while (pos2 != std::string::npos) {
        std::string single = column.substr(0, pos2);
        column = column.substr(pos2 + 1, column.size());
        pos2 = column.find(SPLIT_LINE);

        if (!single.empty()) {
            int32_t keyCode = std::stoi(single);
            if (keyCode != 0) {
                vecKeyCode.push_back(keyCode);
            }
        }
    }

    if (!column.empty()) {
        int32_t keyCode = std::stoi(column);
        if (keyCode != 0) {
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
    DHLOGI("deviceId=%s", GetAnonyString(deviceId).c_str());

    std::lock_guard<std::mutex> lock(mutex_);
    mapDeviceWhiteList_[deviceId] = vecWhiteList;
    return DH_SUCCESS;
}

int32_t WhiteListUtil::ClearWhiteList(const std::string &deviceId)
{
    DHLOGI("deviceId=%s", GetAnonyString(deviceId).c_str());

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
    DHLOGI("start, deviceId=%s", GetAnonyString(deviceId).c_str());

    std::lock_guard<std::mutex> lock(mutex_);
    TYPE_DEVICE_WHITE_LIST_MAP::const_iterator iter = mapDeviceWhiteList_.find(deviceId);
    if (iter != mapDeviceWhiteList_.end()) {
        vecWhiteList = iter->second;
        DHLOGI("GetWhiteList success, deviceId=%s", GetAnonyString(deviceId).c_str());
        return DH_SUCCESS;
    }

    DHLOGI("GetWhiteList fail, deviceId=%s", GetAnonyString(deviceId).c_str());
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
    DHLOGI("start, deviceId=%s", GetAnonyString(deviceId).c_str());

    std::lock_guard<std::mutex> lock(mutex_);
    if (mapDeviceWhiteList_.empty()) {
        DHLOGE("%s called, whilte list is empty!", __func__);
        return false;
    }

    TYPE_DEVICE_WHITE_LIST_MAP::const_iterator iter = mapDeviceWhiteList_.find(deviceId);
    if (iter == mapDeviceWhiteList_.end()) {
        DHLOGE("%s called, not find by deviceId!", __func__);
        return false;
    }

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
} // namespace DistributedInput
} // namespace DistributedHardware
} // namespace OHOS
