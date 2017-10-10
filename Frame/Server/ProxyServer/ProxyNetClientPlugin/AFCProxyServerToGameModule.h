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

#include <string>
#include "SDK/Proto/AFMsgDefine.h"
#include "SDK/Core/AFCHeartBeatManager.h"
#include "SDK/Interface/AFIProxyServerToGameModule.h"
#include "SDK/Interface/AFIProxyNetServerModule.h"
#include "SDK/Interface/AFIKernelModule.h"
#include "SDK/Interface/AFIProxyLogicModule.h"
#include "SDK/Interface/AFINetServerModule.h"
#include "SDK/Interface/AFIElementModule.h"
#include "SDK/Interface/AFILogModule.h"
#include "SDK/Interface/AFIClassModule.h"
#include "SDK/Interface/AFINetClientModule.hpp"


class AFCProxyServerToGameModule : public AFIProxyServerToGameModule
{
public:

    AFCProxyServerToGameModule(AFIPluginManager* p)
    {
        pPluginManager = p;
    }

    virtual bool Init();
    virtual bool Shut();
    virtual bool Execute();

    virtual bool AfterInit();
    virtual AFINetClientModule* GetClusterModule();

protected:

    void OnSocketGSEvent(const NetEventType eEvent, const AFGUID& xClientID, const int nServerID);

    void Register(const int nServerID);

    void OnAckEnterGame(const AFIMsgHead& xHead, const int nMsgID, const char* msg, const uint32_t nLen, const AFGUID& xClientID);
    void OnBrocastmsg(const AFIMsgHead& xHead, const int nMsgID, const char* msg, const uint32_t nLen, const AFGUID& xClientID);
    void LogServerInfo(const std::string& strServerInfo);

    void Transpond(const AFIMsgHead& xHead, const int nMsgID, const char* msg, const uint32_t nLen, const AFGUID& xClientID);

private:

    AFILogModule* m_pLogModule;
    AFIProxyLogicModule* m_pProxyLogicModule;
    AFIKernelModule* m_pKernelModule;
    AFIProxyNetServerModule* m_pProxyServerNet_ServerModule;
    AFIElementModule* m_pElementModule;
    AFIClassModule* m_pClassModule;
    AFINetClientModule* m_pNetClientModule;
};