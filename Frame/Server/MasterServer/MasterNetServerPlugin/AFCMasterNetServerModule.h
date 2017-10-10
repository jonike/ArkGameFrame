/*
* This source file is part of ArkGameFrame
* For the latest info, see https://github.com/ArkGame
*
* Copyright (c) 2013-2017 ArkGame authors.
*
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
*
*/

#pragma once

#include "SDK/Proto/AFMsgDefine.h"
#include "SDK/Interface/AFIMasterNetServerModule.h"
#include "SDK/Interface/AFIKernelModule.h"
#include "SDK/Interface/AFILogModule.h"
#include "SDK/Interface/AFINetServerModule.h"
#include "SDK/Interface/AFIClassModule.h"
#include "SDK/Interface/AFIElementModule.h"

class AFCMasterNetServerModule
    : public AFIMasterNetServerModule
{
public:
    AFCMasterNetServerModule(AFIPluginManager* p)
        : m_pElementModule(nullptr)
        , m_pClassModule(nullptr)
        , m_pKernelModule(nullptr)
        , m_pLogModule(nullptr)
        , m_pNetModule(nullptr)
    {
        pPluginManager = p;
        mnLastLogTime = pPluginManager->GetNowTime();
    }

    virtual bool Init();
    virtual bool Shut();

    virtual bool AfterInit();
    virtual bool Execute();

    virtual void LogReceive(const char* str) {}
    virtual void LogSend(const char* str) {}

protected:

    void OnSocketEvent(const NetEventType eEvent, const AFGUID& xClientID, const int nServerID);

    //���Ӷ�ʧ,ɾ2��(���Ӷ����ʺŶ���)
    void OnClientDisconnect(const AFGUID& xClientID);
    //������
    void OnClientConnected(const AFGUID& xClientID);

protected:
    //���������ע�ᣬˢ����Ϣ
    void OnWorldRegisteredProcess(const AFIMsgHead& xHead, const int nMsgID, const char* msg, const uint32_t nLen, const AFGUID& xClientID);
    void OnWorldUnRegisteredProcess(const AFIMsgHead& xHead, const int nMsgID, const char* msg, const uint32_t nLen, const AFGUID& xClientID);
    void OnRefreshWorldInfoProcess(const AFIMsgHead& xHead, const int nMsgID, const char* msg, const uint32_t nLen, const AFGUID& xClientID);

    //////////////////////////////////////////////////////////////////////////
    //��¼������ע�ᣬˢ����Ϣ
    void OnLoginRegisteredProcess(const AFIMsgHead& xHead, const int nMsgID, const char* msg, const uint32_t nLen, const AFGUID& xClientID);
    void OnLoginUnRegisteredProcess(const AFIMsgHead& xHead, const int nMsgID, const char* msg, const uint32_t nLen, const AFGUID& xClientID);
    void OnRefreshLoginInfoProcess(const AFIMsgHead& xHead, const int nMsgID, const char* msg, const uint32_t nLen, const AFGUID& xClientID);

    //ѡ�������������Ϣ
    void OnSelectWorldProcess(const AFIMsgHead& xHead, const int nMsgID, const char* msg, const uint32_t nLen, const AFGUID& xClientID);
    void OnSelectServerResultProcess(const AFIMsgHead& xHead, const int nMsgID, const char* msg, const uint32_t nLen, const AFGUID& xClientID);

    //////////////////////////////////////////////////////////////////////////

    void SynWorldToLogin();
    void LogGameServer();

    void OnHeartBeat(const AFIMsgHead& xHead, const int nMsgID, const char* msg, const uint32_t nLen, const AFGUID& xClientID);
    void InvalidMessage(const AFIMsgHead& xHead, const int nMsgID, const char* msg, const uint32_t nLen, const AFGUID& xClientID);

private:

    int64_t mnLastLogTime;

    //serverid,data
    AFMapEx<int, ServerData> mWorldMap;
    AFMapEx<int, ServerData> mLoginMap;


    AFIElementModule* m_pElementModule;
    AFIClassModule* m_pClassModule;
    AFIKernelModule* m_pKernelModule;
    AFILogModule* m_pLogModule;
    AFINetServerModule* m_pNetModule;
};