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
#include "SDK/Interface/AFIKernelModule.h"
#include "SDK/Interface/AFILoginNet_ServerModule.h"
#include "SDK/Interface/AFILoginLogicModule.h"
#include "SDK/Interface/AFILogModule.h"
#include "SDK/Interface/AFINetServerModule.h"
#include "SDK/Interface/AFIElementModule.h"
#include "SDK/Interface/AFIClassModule.h"
#include "SDK/Interface/AFILoginToMasterModule.h"
#include "SDK/Interface/AFIGUIDModule.h"

#define NET_MSG_PROCESS(xNFMsg, msg) \
    AFGUID nPlayerID; \
    xNFMsg xMsg; \
    if (!ReceivePB(nMsgID, msg, nLen, xMsg, nPlayerID)) \
    { \
        return 0; \
    } \
    \
    AFIActorMessage xActorMsg; \
    xActorMsg.eType = AFIActorMessage::EACTOR_NET_MSG; \
    xActorMsg.nSubMsgID = nMsgID; \
    xMsg.SerializeToString(&xActorMsg.data); \
    pPluginManager->GetFramework().Send(xActorMsg, pPluginManager->GetAddress(), pPluginManager->GetAddress());

class AFCLoginNet_ServerModule
    : public AFILoginNet_ServerModule

{
public:
    AFCLoginNet_ServerModule(AFIPluginManager* p)
    {
        pPluginManager = p;
    }

    virtual bool Init();
    virtual bool Shut();
    virtual bool Execute();


    virtual bool BeforeShut();
    virtual bool AfterInit();

    virtual void LogReceive(const char* str) {}
    virtual void LogSend(const char* str) {}

    virtual int OnSelectWorldResultsProcess(const int nWorldID, const AFGUID xSenderID, const int nLoginID, const std::string& strAccount, const std::string& strWorldIP, const int nWorldPort, const std::string& strKey);

protected:
    void OnSocketClientEvent(const NetEventType eEvent, const AFGUID& xClientID, const int nServerID);

protected:
    void OnClientDisconnect(const AFGUID& xClientID);
    void OnClientConnected(const AFGUID& xClientID);

    //����
    void OnLoginProcess(const AFIMsgHead& xHead, const int nMsgID, const char* msg, const uint32_t nLen, const AFGUID& xClientID);

    //ѡ�������
    void OnSelectWorldProcess(const AFIMsgHead& xHead, const int nMsgID, const char* msg, const uint32_t nLen, const AFGUID& xClientID);

    //����鿴�����б�
    void OnViewWorldProcess(const AFIMsgHead& xHead, const int nMsgID, const char* msg, const uint32_t nLen, const AFGUID& xClientID);

    void OnHeartBeat(const AFIMsgHead& xHead, const int nMsgID, const char* msg, const uint32_t nLen, const AFGUID& xClientID);
    void OnLogOut(const AFIMsgHead& xHead, const int nMsgID, const char* msg, const uint32_t nLen, const AFGUID& xClientID);
    void InvalidMessage(const AFIMsgHead& xHead, const int nMsgID, const char* msg, const uint32_t nLen, const AFGUID& xClientID);

protected:
    void SynWorldToClient(const AFGUID& xClientID);

    AFMapEx<AFGUID, SessionData> mmClientSessionData;

private:

    AFILoginToMasterModule* m_pLoginToMasterModule;
    AFIClassModule* m_pClassModule;
    AFIElementModule* m_pElementModule;
    AFIKernelModule* m_pKernelModule;
    AFILogModule* m_pLogModule;
    AFILoginLogicModule* m_pLoginLogicModule;
    AFIGUIDModule* m_pUUIDModule;
    AFINetServerModule* m_pNetModule;
};