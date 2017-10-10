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

#include "HelloWorld2.h"
#include "SDK/Core/AFCObject.h"
#include "SDK/Core/AFIComponent.h"

bool HelloWorld2::Init()
{
    std::cout << "Hello, world2, Init" << std::endl;

    return true;
}

int HelloWorld2::OnPropertyCallBackEvent(const AFGUID& self, const std::string& strProperty, const AFIData& oldVar, const AFIData& newVar)
{
    //���Իص��¼���ֻҪ����ֵ�����б仯���ͻᱻ�ص�
    std::cout << "OnPropertyCallBackEvent Property: " << strProperty << " OldValue: " << oldVar.GetInt() << " NewValue: " << newVar.GetInt() << std::endl;

    return 0;
}

bool HelloWorld2::AfterInit()
{
    AFCDataList xData;
    xData.AddInt(111);

    {
        std::cout << "Hello, world2, AfterInit" << std::endl;

        //created a object for this test
        AFIObject* pObject = new AFCObject(AFGUID(0, 1), pPluginManager);

        //add a property name is "Hello" of this object
        pObject->GetPropertyManager()->AddProperty("Hello", AFCData(DT_INT, 1), 0);
        //add a property name is "World" of this object
        pObject->GetPropertyManager()->AddProperty("World", AFCData(DT_INT, 1), 0);

        //set the "world" property value as 1111
        pObject->SetPropertyInt("World", 1111);
        int n1 = pObject->GetPropertyInt("World");
        //get the "world" property value and printf it
        const int nProperty1 = pObject->GetPropertyInt("World");
        std::cout << "Property World:" << nProperty1 << std::endl;

        //add a call back functin for "world" property
        pObject->AddPropertyCallBack("World", this, &HelloWorld2::OnPropertyCallBackEvent);

        ////set the "world" property value as 2222[than the function "HelloWorld2::OnPropertyCallBackEvent" will be called]
        pObject->SetPropertyInt("World", 2222);
        int n2 = pObject->GetPropertyInt("World");

        //get the "world" property value and printf it
        const int nProperty2 = pObject->GetPropertyInt("World");
        std::cout << "Property World:" << nProperty2 << std::endl;

    }

    return true;
}

bool HelloWorld2::Execute()
{
    //ÿִ֡��
    //std::cout << "Hello, world2, Execute" << std::endl;

    return true;
}

bool HelloWorld2::BeforeShut()
{
    //����ʼ��֮ǰ
    std::cout << "Hello, world2, BeforeShut" << std::endl;

    return true;
}

bool HelloWorld2::Shut()
{
    //����ʼ��
    std::cout << "Hello, world2, Shut" << std::endl;

    return true;
}