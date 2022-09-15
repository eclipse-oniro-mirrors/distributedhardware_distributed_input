/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <iostream>
#include <cstring>
#include <thread>

#include "securec.h"
#include "session_mock.h"

constexpr int32_t DH_SUCCESS = 0;
constexpr int32_t DH_ERROR = -1;
constexpr int32_t MOCK_SESSION_ID = 1;
static ISessionListener g_listener;
static char g_peerDeviceId[CHAR_ARRAY_SIZE];
static char g_peerSessionName[CHAR_ARRAY_SIZE];
static char g_mySessionName[CHAR_ARRAY_SIZE];
int CreateSessionServer(const char *pkgName, const char *sessionName, const ISessionListener *listener)
{
    std::cout << "CreateSessionServer start sessionName:" << sessionName << std::endl;
    std::string tmpstr = sessionName;
    if (tmpstr.size() <= 0) {
        std::cout << "CreateSessionServer sessionName is empty." << std::endl;
        return DH_ERROR;
    }
    if (listener == nullptr) {
        std::cout << "CreateSessionServer listener is null." << std::endl;
        return DH_ERROR;
    }
    if (strcpy_s(g_mySessionName, tmpstr.size(), tmpstr.c_str()) != DH_SUCCESS) {
        std::cout << "strcpy_s faild" << std::endl;
        return DH_ERROR;
    }
    g_listener.onBytesReceived = listener->onBytesReceived;
    g_listener.onMessageReceived = listener->onMessageReceived;
    g_listener.onSessionClosed = listener->onSessionClosed;
    g_listener.onSessionOpened = listener->onSessionOpened;
    g_listener.onStreamReceived = listener->onStreamReceived;
    return DH_SUCCESS;
}

int RemoveSessionServer(const char *pkgName, const char *sessionName)
{
    return DH_SUCCESS;
}

int OpenSession(const char *mySessionName, const char *peerSessionName, const char *peerDeviceId, const char *groupId,
    const SessionAttribute *attr)
{
    if (strlen(peerSessionName) <= 0) {
        return DH_ERROR;
    }
    if (strlen(peerDeviceId) <= 0) {
        return DH_ERROR;
    }
    if (strncpy_s(g_peerSessionName, sizeof(g_peerSessionName), peerSessionName, CHAR_ARRAY_SIZE) != DH_SUCCESS) {
        std::cout << "strncpy_s faild" << std::endl;
        return DH_ERROR;
    }
    if (strncpy_s(g_peerDeviceId, sizeof(g_peerDeviceId), peerDeviceId, DEVICE_ID_SIZE_MAX) != DH_SUCCESS) {
        std::cout << "strncpy_s faild" << std::endl;
        return DH_ERROR;
    }
    std::thread thd(OpenSessionResult);
    thd.detach();
    return MOCK_SESSION_ID;
}

void OpenSessionResult()
{
    g_listener.onSessionOpened(MOCK_SESSION_ID, DH_SUCCESS);
}

void CloseSession(int sessionId) {}

int SendBytes(int sessionId, const void *data, unsigned int len)
{
    return DH_SUCCESS;
}

int SendMessage(int sessionId, const void *data, unsigned int len)
{
    return DH_SUCCESS;
}

int SendStream(int sessionId, const StreamData *data, const StreamData *ext, const FrameInfo *param)
{
    return DH_SUCCESS;
}

int GetMySessionName(int sessionId, char *sessionName, unsigned int len)
{
    if (strncpy_s(sessionName, sizeof(sessionName), g_mySessionName, CHAR_ARRAY_SIZE) != DH_SUCCESS) {
        std::cout << "strncpy_s faild" << std::endl;
        return DH_ERROR;
    }
    return DH_SUCCESS;
}

int GetPeerSessionName(int sessionId, char *sessionName, unsigned int len)
{
    if (strncpy_s(sessionName, sizeof(sessionName), g_peerSessionName, CHAR_ARRAY_SIZE) != DH_SUCCESS) {
        std::cout << "strncpy_s faild" << std::endl;
        return DH_ERROR;
    }
    return DH_SUCCESS;
}

int GetPeerDeviceId(int sessionId, char *devId, unsigned int len)
{
    if (strncpy_s(devId, sizeof(devId), g_peerDeviceId, DEVICE_ID_SIZE_MAX) != DH_SUCCESS) {
        std::cout << "strncpy_s faild" << std::endl;
        return DH_ERROR;
    }
    return DH_SUCCESS;
}

int GetSessionSide(int sessionId)
{
    return DH_SUCCESS;
}

int SetFileReceiveListener(const char *pkgName, const char *sessionName, const IFileReceiveListener *recvListener,
    const char *rootDir)
{
    return DH_SUCCESS;
}

int SetFileSendListener(const char *pkgName, const char *sessionName, const IFileSendListener *sendListener)
{
    return DH_SUCCESS;
}

int SendFile(int sessionId, const char *sFileList[], const char *dFileList[], uint32_t fileCnt)
{
    return DH_SUCCESS;
}