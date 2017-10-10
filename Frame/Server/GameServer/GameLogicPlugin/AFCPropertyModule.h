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
#include "SDK/Interface/AFIGameLogicModule.h"
#include "SDK/Interface/AFIPropertyModule.h"
#include "SDK/Interface/AFIElementModule.h"
#include "SDK/Interface/AFIClassModule.h"
#include "SDK/Interface/AFIPropertyConfigModule.h"
#include "SDK/Interface/AFIPluginManager.h"
#include "SDK/Proto/NFProtocolDefine.hpp"
#include "SDK/Interface/AFILevelModule.h"

class AFCPropertyModule : public AFIPropertyModule
{
public:
    AFCPropertyModule(AFIPluginManager* p)
    {
        pPluginManager = p;
    }
    virtual ~AFCPropertyModule() {};

    virtual bool Init();
    virtual bool Shut();
    virtual bool Execute();
    virtual bool AfterInit();

    virtual int RefreshBaseProperty(const AFGUID& self);

    virtual int GetPropertyValue(const AFGUID& self, const std::string& strPropertyName, const NFPropertyGroup eGroupType);
    virtual int SetPropertyValue(const AFGUID& self, const std::string& strPropertyName, const NFPropertyGroup eGroupType, const int nValue);

    virtual int AddPropertyValue(const AFGUID& self, const std::string& strPropertyName, const NFPropertyGroup eGroupType, const int nValue);
    virtual int SubPropertyValue(const AFGUID& self, const std::string& strPropertyName, const NFPropertyGroup eGroupType, const int nValue);

    virtual bool FullHPMP(const AFGUID& self);
    virtual bool AddHP(const AFGUID& self, const int64_t& nValue);
    virtual bool ConsumeHP(const AFGUID& self, const int64_t& nValue);
    virtual bool EnoughHP(const AFGUID& self, const int64_t& nValue);

    virtual bool AddMP(const AFGUID& self, const int64_t& nValue);
    virtual bool ConsumeMP(const AFGUID& self, const int64_t& nValue);
    virtual bool EnoughMP(const AFGUID& self, const int64_t& nValue);

    virtual bool FullSP(const AFGUID& self);
    virtual bool AddSP(const AFGUID& self, const int64_t& nValue);
    virtual bool ConsumeSP(const AFGUID& self, const int64_t& nValue);
    virtual bool EnoughSP(const AFGUID& self, const int64_t& nValue);

    virtual bool AddMoney(const AFGUID& self, const int64_t& nValue);
    virtual bool ConsumeMoney(const AFGUID& self, const int64_t& nValue);
    virtual bool EnoughMoney(const AFGUID& self, const int64_t& nValue);

    virtual bool AddDiamond(const AFGUID& self, const int64_t& nValue);
    virtual bool ConsumeDiamond(const AFGUID& self, const int64_t& nValue);
    virtual bool EnoughDiamond(const AFGUID& self, const int64_t& nValue);

protected:
    int OnObjectClassEvent(const AFGUID& self, const std::string& strClassName, const CLASS_OBJECT_EVENT eClassEvent, const AFIDataList& var);

    int OnObjectLevelEvent(const AFGUID& self, const std::string& strPropertyName, const AFIData& oldVar, const AFIData& newVar);

    int OnRecordPropertyEvent(const AFGUID& self, const RECORD_EVENT_DATA& xEventData, const AFIData& oldVar, const AFIData& newVar);

private:
    AFIKernelModule* m_pKernelModule;
    AFIPropertyConfigModule* m_pPropertyConfigModule;
    AFIElementModule* m_pElementModule;
    AFIClassModule* m_pClassModule;
    AFILevelModule* m_pLevelModule;
};