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

#include "AFCSceneProcessModule.h"
#include "SDK/Base/AFTime.hpp"
#include "SDK/Proto/NFProtocolDefine.hpp"

bool AFCSceneProcessModule::Init()
{
    return true;
}

bool AFCSceneProcessModule::Shut()
{
    return true;
}

bool AFCSceneProcessModule::Execute()
{
    return true;
}

bool AFCSceneProcessModule::AfterInit()
{
    m_pKernelModule = pPluginManager->FindModule<AFIKernelModule>();
    m_pElementModule = pPluginManager->FindModule<AFIElementModule>();
    m_pClassModule = pPluginManager->FindModule<AFIClassModule>();
    m_pLogModule = pPluginManager->FindModule<AFILogModule>();
    m_pGameServerNet_ServerModule = pPluginManager->FindModule<AFIGameNetServerModule>();

    m_pKernelModule->AddClassCallBack(NFrame::Player::ThisName(), this, &AFCSceneProcessModule::OnObjectClassEvent);
    //////////////////////////////////////////////////////////////////////////

    //��ʼ����������
    // #ifdef NF_USE_ACTOR
    //  int nSelfActorID = pPluginManager->GetActorID();
    // #endif
    ARK_SHARE_PTR<AFIClass> pLogicClass =  m_pClassModule->GetElement("Scene");
    if(nullptr != pLogicClass)
    {
        NFList<std::string>& list = pLogicClass->GetConfigNameList();

        std::string strData;
        bool bRet = list.First(strData);
        while(bRet)
        {
            int nSceneID = ARK_LEXICAL_CAST<int>(strData);

            if(!LoadSceneResource(nSceneID))
            {
                return false;
            }

            m_pKernelModule->CreateScene(nSceneID);

            bRet = list.Next(strData);
        }
    }

    return true;
}

bool AFCSceneProcessModule::CreateSceneObject(const int nSceneID, const int nGroupID)
{
    ARK_SHARE_PTR<AFMapEx<std::string, SceneSeedResource>> pSceneResource = mtSceneResourceConfig.GetElement(nSceneID);
    if(nullptr != pSceneResource)
    {
        ARK_SHARE_PTR<SceneSeedResource> pResource = pSceneResource->First();
        while(nullptr != pResource)
        {
            const std::string strClassName(m_pElementModule->GetPropertyString(pResource->strConfigID, NFrame::NPC::ClassName()));

            AFCDataList arg;
            arg << NFrame::NPC::X() << pResource->fSeedX;
            arg << NFrame::NPC::Y() << pResource->fSeedY;
            arg << NFrame::NPC::Z() << pResource->fSeedZ;
            arg << NFrame::NPC::SeedID() << pResource->strSeedID;

            m_pKernelModule->CreateObject(NULL_GUID, nSceneID, nGroupID, strClassName, pResource->strConfigID, arg);

            pResource = pSceneResource->Next();
        }
    }

    return true;
}

int AFCSceneProcessModule::CreateCloneScene(const int& nSceneID)
{
    const E_SCENE_TYPE eType = GetCloneSceneType(nSceneID);
    int nTargetGroupID = m_pKernelModule->RequestGroupScene(nSceneID);

    if(nTargetGroupID > 0 && eType == SCENE_TYPE_CLONE_SCENE)
    {
        if(!CreateSceneObject(nSceneID, nTargetGroupID))
        {
            return -1;
        }
    }

    return nTargetGroupID;
}

int AFCSceneProcessModule::OnEnterSceneEvent(const AFGUID& self, const int nEventID, const AFIDataList& var)
{
    if(var.GetCount() != 4
            || !var.TypeEx(AF_DATA_TYPE::DT_OBJECT, AF_DATA_TYPE::DT_INT,
                           AF_DATA_TYPE::DT_INT, AF_DATA_TYPE::DT_INT, AF_DATA_TYPE::DT_UNKNOWN))
    {
        return 0;
    }

    const AFGUID ident = var.Object(0);
    const int nType = var.Int(1);
    const int nTargetScene = var.Int(2);
    const int nTargetGroupID = var.Int(3);
    const int nNowSceneID = m_pKernelModule->GetPropertyInt(self, NFrame::Player::SceneID());
    const int nNowGroupID = m_pKernelModule->GetPropertyInt(self, NFrame::Player::GroupID());

    if(self != ident)
    {
        m_pLogModule->LogError(ident, "you are not you self, but you want to entry this scene", nTargetScene);
        return 1;
    }

    if(nNowSceneID == nTargetScene
            && nTargetGroupID == nNowGroupID)
    {
        //���������������������ͱ��л���
        m_pLogModule->LogInfo(ident, "in same scene and group but it not a clone scene", nTargetScene);

        return 1;
    }

    //ÿ����ң�һ������
    int64_t nNewGroupID = 0;
    if(nTargetGroupID <= 0)
    {
        nNewGroupID = CreateCloneScene(nTargetScene);
    }
    else
    {
        nNewGroupID = nTargetGroupID;
    }

    if(nNewGroupID <= 0)
    {
        m_pLogModule->LogInfo(ident, "CreateCloneScene failed", nTargetScene);
        return 0;
    }

    //�õ�����
    Point3D xRelivePos;
    const std::string strSceneID = ARK_LEXICAL_CAST<std::string>(nTargetScene);
    const std::string& strRelivePosList = m_pElementModule->GetPropertyString(strSceneID, NFrame::Scene::RelivePos());

    AFCDataList valueRelivePosList(strRelivePosList.c_str(), strRelivePosList.length(), ';');
    if(valueRelivePosList.GetCount() >= 1)
    {
        xRelivePos.FromString(valueRelivePosList.String(0));
    }

    AFCDataList xSceneResult(var);
    xSceneResult.AddFloat(xRelivePos.x);
    xSceneResult.AddFloat(xRelivePos.y);
    xSceneResult.AddFloat(xRelivePos.z);

    m_pKernelModule->DoEvent(self, AFED_ON_OBJECT_ENTER_SCENE_BEFORE, xSceneResult);

    if(!m_pKernelModule->SwitchScene(self, nTargetScene, nNewGroupID, xRelivePos.x, xRelivePos.y, xRelivePos.z, 0.0f, var))
    {
        m_pLogModule->LogInfo(ident, "SwitchScene failed", nTargetScene);

        return 0;
    }

    xSceneResult.AddInt(nNewGroupID);
    m_pKernelModule->DoEvent(self, AFED_ON_OBJECT_ENTER_SCENE_RESULT, xSceneResult);

    return 0;
}

int AFCSceneProcessModule::OnLeaveSceneEvent(const AFGUID& object, const int nEventID, const AFIDataList& var)
{
    if(1 != var.GetCount()
            || !var.TypeEx(AF_DATA_TYPE::DT_INT, AF_DATA_TYPE::DT_UNKNOWN))
    {
        return -1;
    }

    int32_t nOldGroupID = var.Int(0);
    if(nOldGroupID > 0)
    {
        int nSceneID = m_pKernelModule->GetPropertyInt(object, NFrame::Player::SceneID());
        if(GetCloneSceneType(nSceneID) == SCENE_TYPE_CLONE_SCENE)
        {
            m_pKernelModule->ReleaseGroupScene(nSceneID, nOldGroupID);

            m_pLogModule->LogInfo(object, "DestroyCloneSceneGroup", nOldGroupID);
        }
    }

    return 0;
}

int AFCSceneProcessModule::OnObjectClassEvent(const AFGUID& self, const std::string& strClassName, const CLASS_OBJECT_EVENT eClassEvent, const AFIDataList& var)
{
    if(strClassName == NFrame::Player::ThisName())
    {
        if(CLASS_OBJECT_EVENT::COE_DESTROY == eClassEvent)
        {
            //����ڸ�����,��ɾ�������Ǹ�����
            int nSceneID = m_pKernelModule->GetPropertyInt(self, NFrame::Player::SceneID());
            if(GetCloneSceneType(nSceneID) == SCENE_TYPE_CLONE_SCENE)
            {
                int nGroupID = m_pKernelModule->GetPropertyInt(self, NFrame::Player::GroupID());

                m_pKernelModule->ReleaseGroupScene(nSceneID, nGroupID);

                m_pLogModule->LogInfo(self, "DestroyCloneSceneGroup", nGroupID);

            }
        }
        else if(CLASS_OBJECT_EVENT::COE_CREATE_HASDATA == eClassEvent)
        {
            m_pKernelModule->AddEventCallBack(self, AFED_ON_CLIENT_ENTER_SCENE, this, &AFCSceneProcessModule::OnEnterSceneEvent);
            m_pKernelModule->AddEventCallBack(self, AFED_ON_CLIENT_LEAVE_SCENE, this, &AFCSceneProcessModule::OnLeaveSceneEvent);
        }
    }

    return 0;
}

E_SCENE_TYPE AFCSceneProcessModule::GetCloneSceneType(const int nSceneID)
{
    char szSceneIDName[MAX_PATH] = { 0 };
    sprintf(szSceneIDName, "%d", nSceneID);
    if(m_pElementModule->ExistElement(szSceneIDName))
    {
        return (E_SCENE_TYPE)m_pElementModule->GetPropertyInt(szSceneIDName, NFrame::Scene::CanClone());
    }

    return SCENE_TYPE_ERROR;
}

bool AFCSceneProcessModule::IsCloneScene(const int nSceneID)
{
    return GetCloneSceneType(nSceneID) == SCENE_TYPE_CLONE_SCENE;
}

bool AFCSceneProcessModule::ApplyCloneGroup(const int nSceneID, int& nGroupID)
{
    nGroupID = CreateCloneScene(nSceneID);

    return true;
}

bool AFCSceneProcessModule::ExitCloneGroup(const int nSceneID, const int& nGroupID)
{
    return m_pKernelModule->ExitGroupScene(nSceneID, nGroupID);
}

bool AFCSceneProcessModule::LoadSceneResource(const int nSceneID)
{
    char szSceneIDName[MAX_PATH] = { 0 };
    sprintf(szSceneIDName, "%d", nSceneID);

    const std::string strSceneFilePath(m_pElementModule->GetPropertyString(szSceneIDName, NFrame::Scene::FilePath()));
    const int nCanClone = m_pElementModule->GetPropertyInt(szSceneIDName, NFrame::Scene::CanClone());

    //������Ӧ��Դ
    ARK_SHARE_PTR<AFMapEx<std::string, SceneSeedResource>> pSceneResourceMap = mtSceneResourceConfig.GetElement(nSceneID);
    if(nullptr == pSceneResourceMap)
    {
        pSceneResourceMap = ARK_SHARE_PTR<AFMapEx<std::string, SceneSeedResource>>(ARK_NEW AFMapEx<std::string, SceneSeedResource>());
        mtSceneResourceConfig.AddElement(nSceneID, pSceneResourceMap);
    }

    if(strSceneFilePath.empty())
    {
        return false;
    }

    rapidxml::file<> xFileSource(strSceneFilePath.c_str());
    rapidxml::xml_document<>  xFileDoc;
    xFileDoc.parse<0>(xFileSource.data());

    //��Դ�ļ��б�
    rapidxml::xml_node<>* pSeedFileRoot = xFileDoc.first_node();
    for(rapidxml::xml_node<>* pSeedFileNode = pSeedFileRoot->first_node(); pSeedFileNode; pSeedFileNode = pSeedFileNode->next_sibling())
    {
        //���Ӿ�����Ϣ
        std::string strSeedID = pSeedFileNode->first_attribute("ID")->value();
        std::string strConfigID = pSeedFileNode->first_attribute("NPCConfigID")->value();
        float fSeedX = ARK_LEXICAL_CAST<float>(pSeedFileNode->first_attribute("SeedX")->value());
        float fSeedY = ARK_LEXICAL_CAST<float>(pSeedFileNode->first_attribute("SeedY")->value());
        float fSeedZ = ARK_LEXICAL_CAST<float>(pSeedFileNode->first_attribute("SeedZ")->value());

        if(!m_pElementModule->ExistElement(strConfigID))
        {
            assert(0);
        }

        ARK_SHARE_PTR<SceneSeedResource> pSeedResource = pSceneResourceMap->GetElement(strSeedID);
        if(nullptr == pSeedResource)
        {
            pSeedResource = ARK_SHARE_PTR<SceneSeedResource>(ARK_NEW SceneSeedResource());
            pSceneResourceMap->AddElement(strSeedID, pSeedResource);
        }

        pSeedResource->strSeedID = strSeedID;
        pSeedResource->strConfigID = strConfigID;
        pSeedResource->fSeedX = fSeedX;
        pSeedResource->fSeedY = fSeedY;
        pSeedResource->fSeedZ = fSeedZ;

    }

    return true;
}

void AFCSceneProcessModule::OnClienSwapSceneProcess(const AFIMsgHead& xHead, const int nMsgID, const char* msg, const uint32_t nLen, const AFGUID& xClientID)
{
    //CLIENT_MSG_PROCESS(nMsgID, msg, nLen, AFMsg::ReqAckSwapScene);
    //AFIDataList varEntry;
    //varEntry << pObject->Self();
    //varEntry << 0;
    //varEntry << xMsg.scene_id();
    //varEntry << -1;

    //const AFGUID self = AFINetServerModule::PBToNF((xMsg.selfid()));
}
