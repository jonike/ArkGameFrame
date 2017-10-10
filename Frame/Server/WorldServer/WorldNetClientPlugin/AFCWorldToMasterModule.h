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
#include "SDK/Interface/AFIWorldToMasterModule.h"
#include "SDK/Interface/AFIWorldLogicModule.h"
#include "SDK/Interface/AFINetServerModule.h"
#include "SDK/Interface/AFIClassModule.h"
#include "SDK/Interface/AFIElementModule.h"
#include "SDK/Interface/AFILogModule.h"
#include "SDK/Interface/AFIWorldNetServerModule.h"
#include "SDK/Interface/AFINetClientModule.hpp"

class AFCWorldToMasterModule
    : public AFIWorldToMasterModule
{
public:
    AFCWorldToMasterModule(AFIPluginManager* p)
    {
        pPluginManager = p;
    }

    virtual bool Init();
    virtual bool BeforeShut();
    virtual bool Shut();
    virtual bool Execute();
    virtual bool AfterInit();

protected:

    void OnSocketMSEvent(const NetEventType eEvent, const AFGUID& xClientID, const int nServerID);

    //���Ӷ�ʧ,ɾ2��(���Ӷ����ʺŶ���)
    void OnClientDisconnect(const AFGUID& xClientID);
    //������
    void OnClientConnected(const AFGUID& xClientID);

    virtual void LogServerInfo(const std::string& strServerInfo);


    void Register(const int nServerID);
    void RefreshWorldInfo();

    void OnSelectServerProcess(const AFIMsgHead& xHead, const int nMsgID, const char* msg, const uint32_t nLen, const AFGUID& xClientID);
    void OnKickClientProcess(const AFIMsgHead& xHead, const int nMsgID, const char* msg, const uint32_t nLen, const AFGUID& xClientID);

    void InvalidMessage(const AFIMsgHead& xHead, const int nMsgID, const char* msg, const uint32_t nLen, const AFGUID& xClientID);
private:

    AFILogModule* m_pLogModule;
    AFIElementModule* m_pElementModule;
    AFIClassModule* m_pClassModule;
    AFIWorldLogicModule* m_pWorldLogicModule;
    AFIWorldNetServerModule* m_pWorldNet_ServerModule;
    AFINetClientModule* m_pNetClientModule;
};