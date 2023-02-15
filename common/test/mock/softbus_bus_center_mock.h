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

#ifndef SOFTBUS_BUS_CENTER_MOCK_H
#define SOFTBUS_BUS_CENTER_MOCK_H

#include <cstdint>

#ifdef __cplusplus
extern "C" {
#endif
constexpr uint32_t MOCK_NETWORK_ID_BUF_LEN = 65;
constexpr uint32_t MOCK_DEVICE_NAME_BUF_LEN = 65;
constexpr uint32_t UUID_BUF_LEN = 65;
/**
 * @brief Defines the basic information about a device.
 * @since 1.0
 * @version 1.0
 */
typedef struct {
    char networkId[MOCK_NETWORK_ID_BUF_LEN];
    char deviceName[MOCK_DEVICE_NAME_BUF_LEN];
    uint16_t deviceTypeId;
} NodeBasicInfo;

typedef enum {
    NODE_KEY_UDID = 0,    /**< UDID in string format*/
    NODE_KEY_UUID,        /**< UUID in string format */
    NODE_KEY_MASTER_UDID, /**< UDID of master node in string format */
    NODE_KEY_BR_MAC,      /**< BR MAC in string format */
    NODE_KEY_IP_ADDRESS,  /**< IP address in string format */
    NODE_KEY_DEV_NAME,    /**< Device name in string format */
    NODE_KEY_NETWORK_CAPABILITY, /**< Network capability in number format */
    NODE_KEY_NETWORK_TYPE,       /**< Network type in number format */
    NODE_KEY_BLE_OFFLINE_CODE,   /**< Ble offlinecode in string format */
    NODE_KEY_DATA_CHANGE_FLAG,
} NodeDeviceInfoKey;

int32_t GetLocalNodeDeviceInfo(const char *pkgName, NodeBasicInfo *info);
int32_t GetNodeKeyInfo(const char *pkgName, const char *networkId,
    NodeDeviceInfoKey key, uint8_t *info, int32_t infoLen);
#ifdef __cplusplus
}
#endif
#endif // SOFTBUS_BUS_CENTER_MOCK_H