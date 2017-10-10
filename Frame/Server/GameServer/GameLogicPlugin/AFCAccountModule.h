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

#include "SDK/Interface/AFIPluginManager.h"
#include "SDK/Interface/AFIKernelModule.h"
#include "SDK/Interface/AFIGameLogicModule.h"
#include "SDK/Interface/AFIElementModule.h"
#include "SDK/Interface/AFIAccountModule.h"
#include "SDK/Proto/AFMsgDefine.h"
#include "SDK/Interface/AFIGUIDModule.h"

class AFCAccountModule
    : public AFIAccountModule
{
public:
    AFCAccountModule(AFIPluginManager* p)
    {
        pPluginManager = p;
    }
    virtual ~AFCAccountModule() {};

    virtual bool Init();
    virtual bool Shut();
    virtual bool Execute(const float fLasFrametime, const float fStartedTime);
    virtual bool AfterInit();

    virtual bool GetRoleList(const std::string& strAccount, AFMsg::AckRoleLiteInfoList& xAckRoleLiteInfoList);
    virtual bool CreateRole(const std::string& strAccount, AFMsg::AckRoleLiteInfoList& xAckRoleLiteInfoList, const AFIDataList& varList);
    virtual bool DeleteRole(const std::string& strAccount, AFMsg::AckRoleLiteInfoList& xAckRoleLiteInfoList);

protected:
    int OnLoadRoleBeginEvent(const AFGUID& object, const int nEventID, const AFIDataList& var);

    int OnLoadRoleFinalEvent(const AFGUID& object, const int nEventID, const AFIDataList& var);

    int OnCreateRoleEvent(const AFGUID& object, const int nEventID, const AFIDataList& var);

    int OnDeleteRoleEvent(const AFGUID& object, const int nEventID, const AFIDataList& var);

    int OnAcountDisConnectEvent(const AFGUID& object, const int nEventID, const AFIDataList& var);

private:

    //�½��������Ӷ��󣬵ȴ������Լ�����֤KEY��KEY��֤��ɾ��
    //-1
    int mnConnectContainer;

    //ѡ�˴�������
    //-3
    int mnRoleHallContainer;

    //AFIEventProcessModule* m_pEventProcessModule;
    //static AFIDataBaseModule* m_pDataBaseModule;
    //AFIDataNoSqlModule* m_pNoSqlModule;
    AFIKernelModule* m_pKernelModule;
    AFIElementModule* m_pElementInfoModule;
    AFIGUIDModule* m_pUUIDModule;
};