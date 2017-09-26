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

#include "SDK/Base/AFMacros.hpp"
#include "AFIModule.h"
#include "SDK/Base/AFMapEx.h"
#include "SDK/Core/AFIPropertyMgr.h"
#include "SDK/Core/AFIRecordMgr.h"

class AFIClass : public NFList<std::string>
{
public:
    virtual ~AFIClass() {}

    virtual ARK_SHARE_PTR<AFIPropertyMgr> GetPropertyManager() = 0;
    virtual ARK_SHARE_PTR<AFIRecordMgr> GetRecordManager() = 0;

    virtual void SetParent(ARK_SHARE_PTR<AFIClass> pClass) = 0;
    virtual ARK_SHARE_PTR<AFIClass> GetParent() = 0;

    virtual void SetTypeName(const char* strType) = 0;
    virtual const std::string& GetTypeName() = 0;
    virtual const std::string& GetClassName() = 0;

    virtual const bool AddConfigName(std::string& strConfigName) = 0;
    virtual NFList<std::string>& GetConfigNameList() = 0;

    virtual void SetInstancePath(const std::string& strPath) = 0;
    virtual const std::string& GetInstancePath() = 0;

    virtual bool AddClassCallBack(const CLASS_EVENT_FUNCTOR_PTR& cb) = 0;
    virtual bool DoEvent(const AFGUID& objectID, const CLASS_OBJECT_EVENT eClassEvent, const AFIDataList& valueList) = 0;
};

class AFIClassModule
    : public AFIModule,
      public AFMapEx<std::string, AFIClass>
{
public:
    virtual ~AFIClassModule() {}

    virtual bool Load() = 0;
    virtual bool Save() = 0;
    virtual bool Clear() = 0;

    template<typename BaseType>
    bool AddClassCallBack(const std::string& strClassName, BaseType* pBase, int (BaseType::*handler)(const AFGUID&, const std::string&, const CLASS_OBJECT_EVENT, const AFIDataList&))
    {
        CLASS_EVENT_FUNCTOR functor = std::bind(handler, pBase, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4);
        CLASS_EVENT_FUNCTOR_PTR functorPtr(new CLASS_EVENT_FUNCTOR(functor));
        return AddClassCallBack(strClassName, functorPtr);
    }

    virtual bool DoEvent(const AFGUID& objectID, const std::string& strClassName, const CLASS_OBJECT_EVENT eClassEvent, const AFIDataList& valueList) = 0;

    virtual bool AddClassCallBack(const std::string& strClassName, const CLASS_EVENT_FUNCTOR_PTR& cb) = 0;
    virtual ARK_SHARE_PTR<AFIPropertyMgr> GetClassPropertyManager(const std::string& strClassName) = 0;
    virtual ARK_SHARE_PTR<AFIRecordMgr> GetClassRecordManager(const std::string& strClassName) = 0;
};
