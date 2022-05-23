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

#ifndef OHOS_DINPUT_ERRCODE_H
#define OHOS_DINPUT_ERRCODE_H

#include <cstdint>

namespace OHOS {
namespace DistributedHardware {
namespace DistributedInput {
    constexpr int32_t DH_SUCCESS = 0;

    constexpr int32_t ERR_DH_INPUT_HUB_EPOLL_INIT_FAIL = -60000;
    constexpr int32_t ERR_DH_INPUT_HUB_EPOLL_WAIT_TIMEOUT = -60001;
    constexpr int32_t ERR_DH_INPUT_HUB_OPEN_DEVICEPATH_FAIL = -60002;
    constexpr int32_t ERR_DH_INPUT_HUB_MAKE_INPUT_DEVICE_FAIL = -60003;
    constexpr int32_t ERR_DH_INPUT_HUB_MAKE_DEVICE_FAIL = -60004;
    constexpr int32_t ERR_DH_INPUT_HUB_UNREGISTER_FD_FAIL = -60005;
    constexpr int32_t ERR_DH_INPUT_HUB_GET_EVENT_FAIL = -60006;
    constexpr int32_t ERR_DH_INPUT_HUB_DEVICE_ENABLE_FAIL = -60007;
    // whilte list error code
    constexpr int32_t ERR_DH_INPUT_WHILTELIST_INIT_FAIL = -61001;
    constexpr int32_t ERR_DH_INPUT_WHILTELIST_GET_WHILTELIST_FAIL = -61002;
    // handler error code
    constexpr int32_t ERR_DH_INPUT_HANDLER_GET_DEVICE_ID_FAIL = -63000;
    // service sink error code
    constexpr int32_t ERR_DH_INPUT_SERVER_SINK_COLLECTOR_INIT_FAIL = -64000;
    constexpr int32_t ERR_DH_INPUT_SERVER_SINK_MANAGER_INIT_FAIL = -64001;
    constexpr int32_t ERR_DH_INPUT_SERVER_SINK_START_SWITCH_FAIL = -64002;
    constexpr int32_t ERR_DH_INPUT_SERVER_SINK_GET_OPEN_SESSION_FAIL = -64003;
    constexpr int32_t ERR_DH_INPUT_SERVER_SINK_TRANSPORT_INIT_FAIL = -64004;
    constexpr int32_t ERR_DH_INPUT_SERVER_SINK_TRANSPORT_RESPPREPARE_FAIL = 64005;
    constexpr int32_t ERR_DH_INPUT_SERVER_SINK_TRANSPORT_RESPUNPREPARE_FAIL = -64006;
    constexpr int32_t ERR_DH_INPUT_SERVER_SINK_TRANSPORT_RESPSTART_FAIL = -64007;
    constexpr int32_t ERR_DH_INPUT_SERVER_SINK_TRANSPORT_RESPSTOP_FAIL = -64008;
    constexpr int32_t ERR_DH_INPUT_SERVER_SINK_TRANSPORT_SENDMESSAGE_FAIL = -64009;
    // service source error code
    constexpr int32_t ERR_DH_INPUT_SERVER_SOURCE_INJECT_REGISTER_FAIL = -65000;
    constexpr int32_t ERR_DH_INPUT_SERVER_SOURCE_INJECT_UNREGISTER_FAIL = -65001;
    constexpr int32_t ERR_DH_INPUT_SERVER_SOURCE_INJECT_PREPARE_FAIL = -65002;
    constexpr int32_t ERR_DH_INPUT_SERVER_SOURCE_INJECT_NODE_MANAGER_IS_NULL = -65003;//
    constexpr int32_t ERR_DH_INPUT_SERVER_SOURCE_OPEN_DEVICE_NODE_FAIL = -65004;
    constexpr int32_t ERR_DH_INPUT_SERVER_SOURCE_CREATE_HANDLE_FAIL = -65005;
    constexpr int32_t ERR_DH_INPUT_SERVER_SOURCE_CLOSE_DEVICE_FAIL = -65006;
    constexpr int32_t ERR_DH_INPUT_SERVER_SOURCE_GET_DEVICE_FAIL = -65007;
    constexpr int32_t ERR_DH_INPUT_SERVER_SOURCE_MANAGER_INIT_FAIL = -65008;
    constexpr int32_t ERR_DH_INPUT_SERVER_SOURCE_MANSGER_RELEASE_FAIL = -65009;
    constexpr int32_t ERR_DH_INPUT_SERVER_SOURCE_MANAGER_REGISTER_FAIL = -65010;
    constexpr int32_t ERR_DH_INPUT_SERVER_SOURCE_MANAGER_UNREGISTER_FAIL = -65011;
    constexpr int32_t ERR_DH_INPUT_SERVER_SOURCE_MANAGER_REMOVE_INPUT_NODE_FAIL = -65012;
    constexpr int32_t ERR_DH_INPUT_SERVER_SOURCE_MANAGER_DELETE_DEVICE_FAIL = -65013;
    constexpr int32_t ERR_DH_INPUT_SERVER_SOURCE_MANAGER_PREPARE_FAIL = -65014;
    constexpr int32_t ERR_DH_INPUT_SERVER_SOURCE_MANAGER_UNPREPARE_FAIL = -65015;
    constexpr int32_t ERR_DH_INPUT_SERVER_SOURCE_MANAGER_START_FAIL = -65016;
    constexpr int32_t ERR_DH_INPUT_SERVER_SOURCE_MANAGER_STOP_FAIL = -65017;
    constexpr int32_t ERR_DH_INPUT_SERVER_SOURCE_TRANSPORT_INIT_FAIL = -65018;
    constexpr int32_t ERR_DH_INPUT_SERVER_SOURCE_TRANSPORT_DEVICE_SESSION_STATE = -65019;
    constexpr int32_t ERR_DH_INPUT_SERVER_SOURCE_TRANSPORT_OPEN_SESSION_FAIL = -65020;
    constexpr int32_t ERR_DH_INPUT_SERVER_SOURCE_TRANSPORT_OPEN_SESSION_TIMEOUT = -65021;
    constexpr int32_t ERR_DH_INPUT_SERVER_SOURCE_TRANSPORT_PREPARE_FAIL = -65022;
    constexpr int32_t ERR_DH_INPUT_SERVER_SOURCE_TRANSPORT_UNPREPARE_FAIL = -65023;
    constexpr int32_t ERR_DH_INPUT_SERVER_SOURCE_TRANSPORT_START_FAIL = -65024;
    constexpr int32_t ERR_DH_INPUT_SERVER_SOURCE_TRANSPORT_STOP_FAIL = -65025;
    constexpr int32_t ERR_DH_INPUT_SERVER_SOURCE_TRANSPORT_SENDMESSSAGE = -65026;
    constexpr int32_t ERR_DH_INPUT_SERVER_SOURCE_MANAGERGET_CALLBACK_HANDLER_FAIL = -65027;
    constexpr int32_t ERR_DH_INPUT_SERVER_SOURCE_MANAGER_REGISTER_MSG_IS_BAD = -65028;
    constexpr int32_t ERR_DH_INPUT_SERVER_SOURCE_MANAGER_UNREGISTER_MSG_IS_BAD = -65029;
    constexpr int32_t ERR_DH_INPUT_SERVER_SOURCE_MANAGER_PREPARE_MSG_IS_BAD = -65030;
    constexpr int32_t ERR_DH_INPUT_SERVER_SOURCE_MANAGER_UNPREPARE_MSG_IS_BAD = -65031;
    constexpr int32_t ERR_DH_INPUT_SERVER_SOURCE_MANAGER_START_MSG_IS_BAD = -65032;
    constexpr int32_t ERR_DH_INPUT_SERVER_SOURCE_MANAGER_STOP_MSG_IS_BAD = -65033;
    // handler error code
    constexpr int32_t ERR_DH_INPUT_SINK_HANDLER_INIT_SINK_SA_FAIL = -66000;
    constexpr int32_t ERR_DH_INPUT_SINK_HANDLER_INIT_SOURCE_SA_FAIL = -66001;
    // interface error code
    constexpr int32_t ERR_DH_INPUT_IPC_INVALID_DESCRIPTOR = -67000;
    constexpr int32_t ERR_DH_INPUT_CLIENT_GET_SOURCE_PROXY_FAIL = -67001;
    constexpr int32_t ERR_DH_INPUT_CLIENT_GET_SINK_PROXY_FAIL = -67002;
    constexpr int32_t ERR_DH_INPUT_CLIENT_REGISTER_FAIL = -67003;
    constexpr int32_t ERR_DH_INPUT_CLIENT_UNREGISTER_FAIL = -67004;
    constexpr int32_t ERR_DH_INPUT_CLIENT_PREPARE_FAIL = -67005;
    constexpr int32_t ERR_DH_INPUT_CLIENT_UNPREPARE_FAIL = -67006;
    constexpr int32_t ERR_DH_INPUT_CLIENT_START_FAIL = -67007;
    constexpr int32_t ERR_DH_INPUT_CLIENT_STOP_FAIL = -67008;
    constexpr int32_t ERR_DH_INPUT_SINK_PROXY_INIT_FAIL = -67009;
    constexpr int32_t ERR_DH_INPUT_SINK_PROXY_RELEASE_FAIL = -67010;
    constexpr int32_t ERR_DH_INPUT_SINK_PROXY_IS_START_INPUT_FAIL = -67011;
    constexpr int32_t ERR_DH_INPUT_SINK_STUB_ON_REMOTE_REQUEST_FAIL = -67012;
    constexpr int32_t ERR_DH_INPUT_SOURCE_PROXY_INIT_FAIL = -67013;
    constexpr int32_t ERR_DH_INPUT_SOURCE_PROXY_RELEASE_FAIL = -67014;
    constexpr int32_t ERR_DH_INPUT_SOURCE_PROXY_REGISTER_WRITE_MSG_FAIL = -67015;
    constexpr int32_t ERR_DH_INPUT_SOURCE_PROXY_REGISTER_FAIL = -67016;
    constexpr int32_t ERR_DH_INPUT_SOURCE_PROXY_UNREGISTER_WRITE_MSG_FAIL = -67017;
    constexpr int32_t ERR_DH_INPUT_SOURCE_PROXY_UNREGISTER_FAIL = -67018;
    constexpr int32_t ERR_DH_INPUT_SOURCE_PROXY_PREPARE_WRITE_MSG_FAIL = -67019;
    constexpr int32_t ERR_DH_INPUT_SOURCE_PROXY_PREPARE_FAIL = -67020;
    constexpr int32_t ERR_DH_INPUT_SOURCE_PROXY_UNPREPARE_WRITE_MSG_FAIL = -67021;
    constexpr int32_t ERR_DH_INPUT_SOURCE_PROXY_UNPREPARE_FAIL = -67022;
    constexpr int32_t ERR_DH_INPUT_SOURCE_PROXY_START_WRITE_MSG_FAIL = -67023;
    constexpr int32_t ERR_DH_INPUT_SOURCE_PROXY_START_FAIL = -67024;
    constexpr int32_t ERR_DH_INPUT_SOURCE_PROXY_STOP_WRITE_MSG_FAIL = -67025;
    constexpr int32_t ERR_DH_INPUT_SOURCE_PROXY_STOP_FAIL = -67026;
    constexpr int32_t ERR_DH_INPUT_SOURCE_PROXY_IS_START_INPUT_FAIL = -67027;
    constexpr int32_t ERR_DH_INPUT_SOURCE_STUB_ON_REMOTE_REQUEST_FAIL = -67028;
}
}
}
#endif