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

#include "SDK/Interface/AFIKernelModule.h"
#include "SDK/Interface/AFILoginLogicModule.h"
#include "SDK/Interface/AFILogModule.h"
#include "SDK/Interface/AFIClassModule.h"
#include "SDK/Interface/AFIElementModule.h"
#include "SDK/Interface/AFINetServerModule.h"
#include "SDK/Interface/AFILoginNetServerModule.h"
#include "SDK/Interface/AFILoginToMasterModule.h"

class AFCLoginToMasterModule
    : public AFILoginToMasterModule
{
public:
    AFCLoginToMasterModule(AFIPluginManager* p)
    {
        pPluginManager = p;
    }


    virtual bool Init();
    virtual bool Shut();
    virtual bool Execute();

    virtual bool AfterInit();
    virtual bool BeforeShut();

    virtual void LogReceive(const char* str) {}
    virtual void LogSend(const char* str) {}

    virtual AFINetClientModule* GetClusterModule();
    virtual AFMapEx<int, AFMsg::ServerInfoReport>& GetWorldMap();

protected:
    void OnSocketMSEvent(const NetEventType eEvent, const AFGUID& xClientID, const int nServerID);

protected:

    //////////////////////////////////////////////////////////////////////////
    void OnSelectServerResultProcess(const AFIMsgHead& xHead, const int nMsgID, const char* msg, const uint32_t nLen, const AFGUID& xClientID);
    void OnWorldInfoProcess(const AFIMsgHead& xHead, const int nMsgID, const char* msg, const uint32_t nLen, const AFGUID& xClientID);

    //////////////////////////////////////////////////////////////////////////
    void Register(const int nServerID);

private:
    AFMapEx<int, AFMsg::ServerInfoReport> mWorldMap;

    AFILoginLogicModule* m_pLoginLogicModule;
    AFILoginNetServerModule* m_pLoginNet_ServerModule;
    AFIElementModule* m_pElementModule;
    AFIKernelModule* m_pKernelModule;
    AFIClassModule* m_pClassModule;
    AFILogModule* m_pLogModule;
    AFINetClientModule* m_pNetClientModule;
};