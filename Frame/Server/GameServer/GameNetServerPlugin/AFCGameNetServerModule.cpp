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

#include "AFCGameNetServerModule.h"
#include "SDK/Interface/AFIModule.h"
#include "SDK/Proto/NFProtocolDefine.hpp"

bool AFCGameNetServerModule::Init()
{
    m_pNetModule = ARK_NEW AFINetServerModule(pPluginManager);
    return true;
}

bool AFCGameNetServerModule::AfterInit()
{
    m_pKernelModule = pPluginManager->FindModule<AFIKernelModule>();
    m_pClassModule = pPluginManager->FindModule<AFIClassModule>();
    m_pSceneProcessModule = pPluginManager->FindModule<AFISceneProcessModule>();
    m_pElementModule = pPluginManager->FindModule<AFIElementModule>();
    m_pLogModule = pPluginManager->FindModule<AFILogModule>();
    m_pUUIDModule = pPluginManager->FindModule<AFIGUIDModule>();
    m_pGameServerToWorldModule = pPluginManager->FindModule<AFIGameServerToWorldModule>();
    m_AccountModule = pPluginManager->FindModule<AFIAccountModule>();

    m_pNetModule->AddReceiveCallBack(AFMsg::EGMI_PTWG_PROXY_REFRESH, this, &AFCGameNetServerModule::OnRefreshProxyServerInfoProcess);
    m_pNetModule->AddReceiveCallBack(AFMsg::EGMI_PTWG_PROXY_REGISTERED, this, &AFCGameNetServerModule::OnProxyServerRegisteredProcess);
    m_pNetModule->AddReceiveCallBack(AFMsg::EGMI_PTWG_PROXY_UNREGISTERED, this, &AFCGameNetServerModule::OnProxyServerUnRegisteredProcess);
    m_pNetModule->AddReceiveCallBack(AFMsg::EGMI_REQ_ENTER_GAME, this, &AFCGameNetServerModule::OnClienEnterGameProcess);
    m_pNetModule->AddReceiveCallBack(AFMsg::EGMI_REQ_LEAVE_GAME, this, &AFCGameNetServerModule::OnClienLeaveGameProcess);
    m_pNetModule->AddReceiveCallBack(AFMsg::EGMI_REQ_ROLE_LIST, this, &AFCGameNetServerModule::OnReqiureRoleListProcess);
    m_pNetModule->AddReceiveCallBack(AFMsg::EGMI_REQ_CREATE_ROLE, this, &AFCGameNetServerModule::OnCreateRoleGameProcess);
    m_pNetModule->AddReceiveCallBack(AFMsg::EGMI_REQ_DELETE_ROLE, this, &AFCGameNetServerModule::OnDeleteRoleGameProcess);
    m_pNetModule->AddReceiveCallBack(AFMsg::EGMI_REQ_RECOVER_ROLE, this, &AFCGameNetServerModule::OnClienSwapSceneProcess);
    m_pNetModule->AddReceiveCallBack(AFMsg::EGMI_REQ_SWAP_SCENE, this, &AFCGameNetServerModule::OnClienSwapSceneProcess);
    m_pNetModule->AddReceiveCallBack(AFMsg::EGMI_REQ_SEARCH_GUILD, this, &AFCGameNetServerModule::OnTransWorld);
    m_pNetModule->AddReceiveCallBack(AFMsg::EGEC_REQ_CREATE_CHATGROUP, this, &AFCGameNetServerModule::OnTransWorld);
    m_pNetModule->AddReceiveCallBack(AFMsg::EGEC_REQ_JOIN_CHATGROUP, this, &AFCGameNetServerModule::OnTransWorld);
    m_pNetModule->AddReceiveCallBack(AFMsg::EGEC_REQ_LEAVE_CHATGROUP, this, &AFCGameNetServerModule::OnTransWorld);
    m_pNetModule->AddReceiveCallBack(AFMsg::EGEC_REQ_SUBSCRIPTION_CHATGROUP, this, &AFCGameNetServerModule::OnTransWorld);

    m_pNetModule->AddEventCallBack(this, &AFCGameNetServerModule::OnSocketPSEvent);

    m_pKernelModule->RegisterCommonClassEvent(this, &AFCGameNetServerModule::OnClassCommonEvent);
    m_pKernelModule->RegisterCommonPropertyEvent(this, &AFCGameNetServerModule::OnPropertyCommonEvent);
    m_pKernelModule->RegisterCommonRecordEvent(this, &AFCGameNetServerModule::OnRecordCommonEvent);

    m_pKernelModule->AddClassCallBack(NFrame::Player::ThisName(), this, &AFCGameNetServerModule::OnObjectClassEvent);

    ARK_SHARE_PTR<AFIClass> xLogicClass = m_pClassModule->GetElement("Server");
    if(nullptr != xLogicClass)
    {
        NFList<std::string>& xNameList = xLogicClass->GetConfigNameList();
        std::string strConfigName;
        for(bool bRet = xNameList.First(strConfigName); bRet; bRet = xNameList.Next(strConfigName))
        {
            const int nServerType = m_pElementModule->GetPropertyInt(strConfigName, "Type");
            const int nServerID = m_pElementModule->GetPropertyInt(strConfigName, "ServerID");
            if(nServerType == NF_SERVER_TYPES::NF_ST_GAME && pPluginManager->AppID() == nServerID)
            {
                const int nPort = m_pElementModule->GetPropertyInt(strConfigName, "Port");
                const int nMaxConnect = m_pElementModule->GetPropertyInt(strConfigName, "MaxOnline");
                const int nCpus = m_pElementModule->GetPropertyInt(strConfigName, "CpuCount");
                const std::string strName(m_pElementModule->GetPropertyString(strConfigName, "Name"));
                const std::string strIP(m_pElementModule->GetPropertyString(strConfigName, "IP"));

                int nRet = m_pNetModule->Initialization(nMaxConnect, strIP, nPort, nServerID, nCpus);
                if(nRet < 0)
                {
                    std::ostringstream strLog;
                    strLog << "Cannot init server net, Port = " << nPort;
                    m_pLogModule->LogError(NULL_GUID, strLog, __FUNCTION__, __LINE__);
                    ARK_ASSERT(nRet, "Cannot init server net", __FILE__, __FUNCTION__);
                    exit(0);
                }
            }
        }
    }

    return true;
}

bool AFCGameNetServerModule::Shut()
{

    return true;
}

bool AFCGameNetServerModule::Execute()
{
    return m_pNetModule->Execute();
}

void AFCGameNetServerModule::OnSocketPSEvent(const NetEventType eEvent, const AFGUID& xClientID, const int nServerID)
{
    if(eEvent == DISCONNECTED)
    {
        m_pLogModule->LogInfo(xClientID, "NF_NET_EVENT_EOF", "Connection closed", __FUNCTION__, __LINE__);
        OnClientDisconnect(xClientID);
    }
    else  if(eEvent == CONNECTED)
    {
        m_pLogModule->LogInfo(xClientID, "NF_NET_EVENT_CONNECTED", "connected success", __FUNCTION__, __LINE__);
        OnClientConnected(xClientID);
    }
}

void AFCGameNetServerModule::OnClientDisconnect(const AFGUID& xClientID)
{
    //ֻ���������ض���
    int nServerID = 0;
    ARK_SHARE_PTR<GateServerInfo> pServerData = mProxyMap.First();
    while(nullptr != pServerData)
    {
        if(xClientID == pServerData->xServerData.xClient)
        {
            nServerID = pServerData->xServerData.pData->server_id();
            break;
        }

        pServerData = mProxyMap.Next();
    }

    mProxyMap.RemoveElement(nServerID);
}

void AFCGameNetServerModule::OnClientConnected(const AFGUID& xClientID)
{

}

void AFCGameNetServerModule::OnClienEnterGameProcess(const AFIMsgHead& xHead, const int nMsgID, const char* msg, const uint32_t nLen, const AFGUID& xClientID)
{
    //�ڽ�����Ϸ֮ǰnPlayerIDΪ�������ص�FD
    AFGUID nGateClientID;
    AFMsg::ReqEnterGameServer xMsg;
    if(!m_pNetModule->ReceivePB(xHead, nMsgID, msg, nLen, xMsg, nGateClientID))
    {
        return;
    }

    AFGUID nRoleID = AFINetServerModule::PBToNF(xMsg.id());

    if(m_pKernelModule->GetObject(nRoleID))
    {
        m_pKernelModule->DestroyObject(nRoleID);
    }

    //////////////////////////////////////////////////////////////////////////

    ARK_SHARE_PTR<AFCGameNetServerModule::GateBaseInfo>  pGateInfo = GetPlayerGateInfo(nRoleID);
    if(nullptr != pGateInfo)
    {
        if(RemovePlayerGateInfo(nRoleID))
        {
            m_pLogModule->LogError(nRoleID, "RemovePlayerGateInfo fail", "", __FUNCTION__, __LINE__);
        }
    }

    ARK_SHARE_PTR<AFCGameNetServerModule::GateServerInfo> pGateServereinfo = GetGateServerInfoByClientID(xClientID);
    if(nullptr == pGateServereinfo)
    {
        return;
    }

    int nGateID = -1;
    if(pGateServereinfo->xServerData.pData)
    {
        nGateID = pGateServereinfo->xServerData.pData->server_id();
    }

    if(nGateID < 0)
    {
        return;
    }

    if(!AddPlayerGateInfo(nRoleID, nGateClientID, nGateID))
    {
        return;
    }

    //Ĭ��1�ų���
    int nSceneID = 1;
    AFCDataList var;
    var.AddString("Name");
    var.AddString(xMsg.name().c_str());

    var.AddString("GateID");
    var.AddInt(nGateID);

    var.AddString("ClientID");
    var.AddObject(nGateClientID);

    ARK_SHARE_PTR<AFIObject> pObject = m_pKernelModule->CreateObject(nRoleID, nSceneID, 0, NFrame::Player::ThisName(), "", var);
    if(nullptr == pObject)
    {
        //�ڴ�й©
        //mRoleBaseData
        //mRoleFDData
        return;
    }

    pObject->SetPropertyInt("LoadPropertyFinish", 1);
    pObject->SetPropertyInt("GateID", nGateID);
    pObject->SetPropertyInt("GameID", pPluginManager->AppID());

    m_pKernelModule->DoEvent(pObject->Self(), NFrame::Player::ThisName(), CLASS_OBJECT_EVENT::COE_CREATE_FINISH, AFCDataList());

    AFCDataList varEntry;
    varEntry << pObject->Self();
    varEntry << int32_t(0);
    varEntry << (int32_t)nSceneID;
    varEntry << int32_t (-1);
    m_pKernelModule->DoEvent(pObject->Self(), AFED_ON_CLIENT_ENTER_SCENE, varEntry);
}

void AFCGameNetServerModule::OnClienLeaveGameProcess(const AFIMsgHead& xHead, const int nMsgID, const char* msg, const uint32_t nLen, const AFGUID& xClientID)
{
    AFGUID nPlayerID;
    AFMsg::ReqLeaveGameServer xMsg;
    if(!m_pNetModule->ReceivePB(xHead, nMsgID, msg, nLen, xMsg, nPlayerID))
    {
        return;
    }

    if(nPlayerID.IsNull())
    {
        return;
    }

    if(m_pKernelModule->GetObject(nPlayerID))
    {
        m_pKernelModule->DestroyObject(nPlayerID);
    }

    if(!RemovePlayerGateInfo(nPlayerID))
    {
        m_pLogModule->LogError(nPlayerID, "RemovePlayerGateInfo", "", __FUNCTION__, __LINE__);
    }
}

int AFCGameNetServerModule::OnPropertyEnter(const AFIDataList& argVar, const AFGUID& self)
{
    if(argVar.GetCount() <= 0 || self.IsNull())
    {
        return 0;
    }

    AFMsg::MultiObjectPropertyList xPublicMsg;
    AFMsg::MultiObjectPropertyList xPrivateMsg;

    //��Ϊ�Լ�������
    //1.public���͸�������
    //2.����Լ����б��У��ٴη���private����
    ARK_SHARE_PTR<AFIObject> pObject = m_pKernelModule->GetObject(self);
    if(nullptr != pObject)
    {
        AFMsg::ObjectPropertyList* pPublicData = xPublicMsg.add_multi_player_property();
        AFMsg::ObjectPropertyList* pPrivateData = xPrivateMsg.add_multi_player_property();

        *(pPublicData->mutable_player_id()) = AFINetServerModule::NFToPB(self);
        *(pPrivateData->mutable_player_id()) = AFINetServerModule::NFToPB(self);

        ARK_SHARE_PTR<AFIPropertyMgr> pPropertyManager = pObject->GetPropertyManager();

        for(int i = 0; i < pPropertyManager->GetPropertyCount(); i++)
        {
            AFProperty* pPropertyInfo = pPropertyManager->GetPropertyByIndex(i);
            if(NULL == pPropertyInfo)
            {
                continue;
            }

            if(pPropertyInfo->Changed())
            {
                if(pPropertyInfo->IsPublic())
                {
                    AFMsg::PropertyPBData* pDataInt = pPublicData->add_property_data_list();
                    AFINetServerModule::DataToPBProperty(pPropertyInfo->GetValue(), pPropertyInfo->GetName().c_str(), *pDataInt);
                }

                if(pPropertyInfo->IsPrivate())
                {
                    AFMsg::PropertyPBData* pDataInt = pPrivateData->add_property_data_list();
                    AFINetServerModule::DataToPBProperty(pPropertyInfo->GetValue(), pPropertyInfo->GetName().c_str(), *pDataInt);
                }
            }
        }

        for(int i = 0; i < argVar.GetCount(); i++)
        {
            AFGUID identOther = argVar.Object(i);
            if(self == identOther)
            {
                //�ҵ����������ص�FD
                SendMsgPBToGate(AFMsg::EGMI_ACK_OBJECT_PROPERTY_ENTRY, xPrivateMsg, identOther);
            }
            else
            {
                SendMsgPBToGate(AFMsg::EGMI_ACK_OBJECT_PROPERTY_ENTRY, xPublicMsg, identOther);
            }
        }
    }

    return 0;
}

bool OnRecordEnterPack(AFRecord* pRecord, AFMsg::ObjectRecordBase* pObjectRecordBase)
{
    if(!pRecord || !pObjectRecordBase)
    {
        return false;
    }

    for(int i = 0; i < pRecord->GetRowCount(); i++)
    {
        AFMsg::RecordAddRowStruct* pAddRowStruct = pObjectRecordBase->add_row_struct();
        pAddRowStruct->set_row(i);
        for(int j = 0; j < pRecord->GetColCount(); j++)
        {
            AFMsg::RecordPBData* pAddData = pAddRowStruct->add_record_data_list();

            AFCData xRowColData;
            if(!pRecord->GetValue(i, j, xRowColData))
            {
                ARK_ASSERT(0, "Get record value failed, please check", __FILE__, __FUNCTION__);
                continue;
            }

            AFINetServerModule::RecordToPBRecord(xRowColData, i, j, *pAddData);
        }
    }

    return true;
}

int AFCGameNetServerModule::OnRecordEnter(const AFIDataList& argVar, const AFGUID& self)
{
    if(argVar.GetCount() <= 0 || self.IsNull())
    {
        return 0;
    }

    AFMsg::MultiObjectRecordList xPublicMsg;
    AFMsg::MultiObjectRecordList xPrivateMsg;

    ARK_SHARE_PTR<AFIObject> pObject = m_pKernelModule->GetObject(self);
    if(nullptr == pObject)
    {
        return 0;
    }

    AFMsg::ObjectRecordList* pPublicData = NULL;
    AFMsg::ObjectRecordList* pPrivateData = NULL;

    ARK_SHARE_PTR<AFIRecordMgr> pRecordManager = pObject->GetRecordManager();

    size_t nRecordCount = pRecordManager->GetCount();
    for(int i = 0; i < nRecordCount; ++i)
    {
        AFRecord* pRecord = pRecordManager->GetRecordByIndex(i);
        if(NULL == pRecord)
        {
            continue;
        }

        if(!pRecord->IsPublic() && !pRecord->IsPrivate())
        {
            continue;
        }

        AFMsg::ObjectRecordBase* pPrivateRecordBase = NULL;
        AFMsg::ObjectRecordBase* pPublicRecordBase = NULL;
        if(pRecord->IsPublic())
        {
            if(!pPublicData)
            {
                pPublicData = xPublicMsg.add_multi_player_record();
                *(pPublicData->mutable_player_id()) = AFINetServerModule::NFToPB(self);
            }
            pPublicRecordBase = pPublicData->add_record_list();
            pPublicRecordBase->set_record_name(pRecord->GetName());

            if(!OnRecordEnterPack(pRecord, pPublicRecordBase))
            {
                m_pLogModule->LogError(self, "OnRecordEnterPack fail ", "", __FUNCTION__, __LINE__);
                return -1;
            }
        }

        if(pRecord->IsPrivate())
        {
            if(!pPrivateData)
            {
                pPrivateData = xPrivateMsg.add_multi_player_record();
                *(pPrivateData->mutable_player_id()) = AFINetServerModule::NFToPB(self);
            }
            pPrivateRecordBase = pPrivateData->add_record_list();
            pPrivateRecordBase->set_record_name(pRecord->GetName());

            if(OnRecordEnterPack(pRecord, pPrivateRecordBase))
            {
                m_pLogModule->LogError(self, "OnRecordEnterPack fail ", "", __FUNCTION__, __LINE__);
                return -1;
            }
        }
    }

    for(int i = 0; i < argVar.GetCount(); i++)
    {
        AFGUID identOther = argVar.Object(i);
        if(self == identOther)
        {
            if(xPrivateMsg.multi_player_record_size() > 0)
            {
                SendMsgPBToGate(AFMsg::EGMI_ACK_OBJECT_RECORD_ENTRY, xPrivateMsg, identOther);
            }
        }
        else
        {
            if(xPublicMsg.multi_player_record_size() > 0)
            {
                SendMsgPBToGate(AFMsg::EGMI_ACK_OBJECT_RECORD_ENTRY, xPublicMsg, identOther);
            }
        }
    }

    return 0;
}

int AFCGameNetServerModule::OnObjectListEnter(const AFIDataList& self, const AFIDataList& argVar)
{
    if(self.GetCount() <= 0 || argVar.GetCount() <= 0)
    {
        return 0;
    }

    AFMsg::AckPlayerEntryList xPlayerEntryInfoList;
    for(int i = 0; i < argVar.GetCount(); i++)
    {
        AFGUID identOld = argVar.Object(i);
        //�ų��ն���
        if(identOld.IsNull())
        {
            continue;
        }

        AFMsg::PlayerEntryInfo* pEntryInfo = xPlayerEntryInfoList.add_object_list();
        *(pEntryInfo->mutable_object_guid()) = AFINetServerModule::NFToPB(identOld);
        Point3D xPoint;
        xPoint.x = m_pKernelModule->GetPropertyFloat(identOld, "x");
        xPoint.y = m_pKernelModule->GetPropertyFloat(identOld, "y");
        xPoint.z = m_pKernelModule->GetPropertyFloat(identOld, "z");

        *pEntryInfo->mutable_pos() = AFINetServerModule::NFToPB(xPoint);
        pEntryInfo->set_career_type(m_pKernelModule->GetPropertyInt(identOld, "Job"));
        pEntryInfo->set_player_state(m_pKernelModule->GetPropertyInt(identOld, "State"));
        pEntryInfo->set_config_id(m_pKernelModule->GetPropertyString(identOld, "ConfigID"));
        pEntryInfo->set_scene_id(m_pKernelModule->GetPropertyInt(identOld, "SceneID"));
        pEntryInfo->set_class_id(m_pKernelModule->GetPropertyString(identOld, "ClassName"));
    }

    if(xPlayerEntryInfoList.object_list_size() <= 0)
    {
        return 0;
    }

    for(int i = 0; i < self.GetCount(); i++)
    {
        AFGUID ident = self.Object(i);
        if(ident.IsNull())
        {
            continue;
        }

        //�����ڲ�ͬ��������,�õ��������ڵ�����FD
        SendMsgPBToGate(AFMsg::EGMI_ACK_OBJECT_ENTRY, xPlayerEntryInfoList, ident);
    }

    return 1;
}

int AFCGameNetServerModule::OnObjectListLeave(const AFIDataList& self, const AFIDataList& argVar)
{
    if(self.GetCount() <= 0 || argVar.GetCount() <= 0)
    {
        return 0;
    }

    AFMsg::AckPlayerLeaveList xPlayerLeaveInfoList;
    for(int i = 0; i < argVar.GetCount(); i++)
    {
        AFGUID identOld = argVar.Object(i);
        //�ų��ն���
        if(identOld.IsNull())
        {
            continue;
        }

        AFMsg::Ident* pIdent = xPlayerLeaveInfoList.add_object_list();
        *pIdent = AFINetServerModule::NFToPB(argVar.Object(i));
    }

    for(int i = 0; i < self.GetCount(); i++)
    {
        AFGUID ident = self.Object(i);
        if(ident.IsNull())
        {
            continue;
        }
        //�����ڲ�ͬ��������,�õ��������ڵ�����FD
        SendMsgPBToGate(AFMsg::EGMI_ACK_OBJECT_LEAVE, xPlayerLeaveInfoList, ident);
    }

    return 1;
}


int AFCGameNetServerModule::OnPropertyCommonEvent(const AFGUID& self, const std::string& strPropertyName, const AFIData& oldVar, const AFIData& newVar)
{
    //if ( NFrame::Player::ThisName() == m_pKernelModule->GetPropertyString( self, "ClassName" ) )
    {
        if("GroupID" == strPropertyName)
        {
            //�Լ�����Ҫ֪���Լ���������Ա仯��,���Ǳ��˾Ͳ���Ҫ֪����
            int nRet = OnGroupEvent(self, strPropertyName, oldVar, newVar);
        }

        if("SceneID" == strPropertyName)
        {
            //�Լ�����Ҫ֪���Լ���������Ա仯��,���Ǳ��˾Ͳ���Ҫ֪����
            int nRet =  OnContainerEvent(self, strPropertyName, oldVar, newVar);
        }

        if(NFrame::Player::ThisName() == std::string(m_pKernelModule->GetPropertyString(self, "ClassName")))
        {
            if(m_pKernelModule->GetPropertyInt(self, "LoadPropertyFinish") <= 0)
            {
                return 0;
            }
        }
    }

    AFCDataList valueBroadCaseList;
    int nCount = 0;//argVar.GetCount() ;
    if(nCount <= 0)
    {
        nCount = GetBroadCastObject(self, strPropertyName, false, valueBroadCaseList);
    }
    else
    {
        //����Ĳ�����Ҫ�㲥�Ķ����б�
        //valueBroadCaseList = argVar;
    }

    if(valueBroadCaseList.GetCount() <= 0)
    {
        return 0;
    }

    AFMsg::ObjectPropertyPBData xPropertyData;
    AFMsg::Ident* pIdent = xPropertyData.mutable_player_id();
    *pIdent = AFINetServerModule::NFToPB(self);
    AFMsg::PropertyPBData* pData = xPropertyData.add_property_list();
    AFINetServerModule::DataToPBProperty(oldVar, strPropertyName.c_str(), *pData);

    for(int i = 0; i < valueBroadCaseList.GetCount(); i++)
    {
        AFGUID identOld = valueBroadCaseList.Object(i);

        SendMsgPBToGate(AFMsg::EGMI_ACK_PROPERTY_DATA, xPropertyData, identOld);
    }

    return 0;
}

int AFCGameNetServerModule::OnRecordCommonEvent(const AFGUID& self, const RECORD_EVENT_DATA& xEventData, const AFIData& oldVar, const AFIData& newVar)
{
    const std::string& strRecordName = xEventData.strRecordName;
    const int nOpType = xEventData.nOpType;
    const int nRow = xEventData.nRow;
    const int nCol = xEventData.nCol;

    int nObjectContainerID = m_pKernelModule->GetPropertyInt(self, "SceneID");
    int nObjectGroupID = m_pKernelModule->GetPropertyInt(self, "GroupID");

    if(nObjectGroupID < 0)
    {
        //����
        return 0;
    }

    if(NFrame::Player::ThisName() == std::string(m_pKernelModule->GetPropertyString(self, "ClassName")))
    {
        if(m_pKernelModule->GetPropertyInt(self, "LoadPropertyFinish") <= 0)
        {
            return 0;
        }
    }

    AFCDataList valueBroadCaseList;
    GetBroadCastObject(self, strRecordName, true, valueBroadCaseList);

    switch(nOpType)
    {
    case AFRecord::RecordOptype::Add:
        {
            AFMsg::ObjectRecordAddRow xAddRecordRow;
            AFMsg::Ident* pIdent = xAddRecordRow.mutable_player_id();
            *pIdent = AFINetServerModule::NFToPB(self);

            xAddRecordRow.set_record_name(strRecordName);

            AFMsg::RecordAddRowStruct* pAddRowData = xAddRecordRow.add_row_data();
            pAddRowData->set_row(nRow);

            //add row ��Ҫ������row
            AFRecord* xRecord = m_pKernelModule->FindRecord(self, strRecordName);
            if(xRecord)
            {
                AFCDataList xRowDataList;
                if(xRecord->QueryRow(nRow, xRowDataList))
                {
                    for(int i = 0; i < xRowDataList.GetCount(); i++)
                    {
                        AFMsg::RecordPBData* pAddData = pAddRowData->add_record_data_list();
                        AFINetServerModule::RecordToPBRecord(xRowDataList, nRow, nCol, *pAddData);
                    }

                    for(int i = 0; i < valueBroadCaseList.GetCount(); i++)
                    {
                        AFGUID identOther = valueBroadCaseList.Object(i);

                        SendMsgPBToGate(AFMsg::EGMI_ACK_ADD_ROW, xAddRecordRow, identOther);
                    }
                }
            }
        }
        break;
    case AFRecord::RecordOptype::Del:
        {
            AFMsg::ObjectRecordRemove xReoveRecordRow;

            AFMsg::Ident* pIdent = xReoveRecordRow.mutable_player_id();
            *pIdent = AFINetServerModule::NFToPB(self);

            xReoveRecordRow.set_record_name(strRecordName);
            xReoveRecordRow.add_remove_row(nRow);

            for(int i = 0; i < valueBroadCaseList.GetCount(); i++)
            {
                AFGUID identOther = valueBroadCaseList.Object(i);

                SendMsgPBToGate(AFMsg::EGMI_ACK_REMOVE_ROW, xReoveRecordRow, identOther);
            }
        }
        break;
    case AFRecord::RecordOptype::Swap:
        {
            //��ʵ��2��row����
            AFMsg::ObjectRecordSwap xSwapRecord;
            *xSwapRecord.mutable_player_id() = AFINetServerModule::NFToPB(self);

            xSwapRecord.set_origin_record_name(strRecordName);
            xSwapRecord.set_target_record_name(strRecordName);   // ��ʱû��
            xSwapRecord.set_row_origin(nRow);
            xSwapRecord.set_row_target(nCol);

            for(int i = 0; i < valueBroadCaseList.GetCount(); i++)
            {
                AFGUID identOther = valueBroadCaseList.Object(i);

                SendMsgPBToGate(AFMsg::EGMI_ACK_SWAP_ROW, xSwapRecord, identOther);
            }
        }
        break;
    case AFRecord::RecordOptype::Update:
        {
            AFMsg::ObjectRecordPBData xRecordChanged;
            *xRecordChanged.mutable_player_id() = AFINetServerModule::NFToPB(self);
            xRecordChanged.set_record_name(strRecordName);
            AFMsg::RecordPBData* recordProperty = xRecordChanged.add_record_list();
            AFINetServerModule::RecordToPBRecord(newVar, nRow, nCol, *recordProperty);

            for(int i = 0; i < valueBroadCaseList.GetCount(); i++)
            {
                AFGUID identOther = valueBroadCaseList.Object(i);

                SendMsgPBToGate(AFMsg::EGMI_ACK_RECORD_DATA, xRecordChanged, identOther);
            }
        }
        break;
    case AFRecord::RecordOptype::Create:
        break;
    case AFRecord::RecordOptype::Cleared:
        break;
    default:
        break;
    }

    return 0;
}

int AFCGameNetServerModule::OnClassCommonEvent(const AFGUID& self, const std::string& strClassName, const CLASS_OBJECT_EVENT eClassEvent, const AFIDataList& var)
{
    ////////////1:�㲥���Ѿ����ڵ���//////////////////////////////////////////////////////////////
    if(CLASS_OBJECT_EVENT::COE_DESTROY == eClassEvent)
    {
        //ɾ�����߱�־

        //////////////////////////////////////////////////////////////////////////

        int nObjectContainerID = m_pKernelModule->GetPropertyInt(self, "SceneID");
        int nObjectGroupID = m_pKernelModule->GetPropertyInt(self, "GroupID");

        if(nObjectGroupID < 0)
        {
            //����
            return 0;
        }

        AFCDataList valueAllObjectList;
        AFCDataList valueBroadCaseList;
        AFCDataList valueBroadListNoSelf;
        m_pKernelModule->GetGroupObjectList(nObjectContainerID, nObjectGroupID, valueAllObjectList);

        for(int i = 0; i < valueAllObjectList.GetCount(); i++)
        {
            AFGUID identBC = valueAllObjectList.Object(i);
            const std::string strClassName(m_pKernelModule->GetPropertyString(identBC, "ClassName"));
            if(NFrame::Player::ThisName() == strClassName)
            {
                valueBroadCaseList << identBC;
                if(identBC != self)
                {
                    valueBroadListNoSelf << identBC;
                }
            }
        }

        //����Ǹ����Ĺ֣�����Ҫ���ͣ���Ϊ�����뿪������ʱ��һ����һ����Ϣ����
        OnObjectListLeave(valueBroadListNoSelf, AFCDataList() << self);
    }

    else if(CLASS_OBJECT_EVENT::COE_CREATE_NODATA == eClassEvent)
    {
        //id��fd,gateid��
        ARK_SHARE_PTR<GateBaseInfo> pDataBase = mRoleBaseData.GetElement(self);
        if(nullptr != pDataBase)
        {
            //�ظ��ͻ��˽�ɫ������Ϸ����ɹ���
            AFMsg::AckEventResult xMsg;
            xMsg.set_event_code(AFMsg::EGEC_ENTER_GAME_SUCCESS);

            *xMsg.mutable_event_client() = AFINetServerModule::NFToPB(pDataBase->xClientID);
            *xMsg.mutable_event_object() = AFINetServerModule::NFToPB(self);

            SendMsgPBToGate(AFMsg::EGMI_ACK_ENTER_GAME, xMsg, self);
        }
    }
    else if(CLASS_OBJECT_EVENT::COE_CREATE_LOADDATA == eClassEvent)
    {
    }
    else if(CLASS_OBJECT_EVENT::COE_CREATE_HASDATA == eClassEvent)
    {
        //�Լ��㲥���Լ��͹���
        if(strClassName == NFrame::Player::ThisName())
        {
            OnObjectListEnter(AFCDataList() << self, AFCDataList() << self);

            OnPropertyEnter(AFCDataList() << self, self);
            OnRecordEnter(AFCDataList() << self, self);
        }
    }
    else if(CLASS_OBJECT_EVENT::COE_CREATE_FINISH == eClassEvent)
    {

    }
    return 0;
}

int AFCGameNetServerModule::OnGroupEvent(const AFGUID& self, const std::string& strPropertyName, const AFIData& oldVar, const AFIData& newVar)
{
    //���������仯��ֻ���ܴ�A������0���л���B������0��
    //��Ҫע�����------------�κβ�ı��ʱ�򣬴������ʵ��δ����㣬��ˣ���ı��ʱ���ȡ������б�Ŀ����ǲ������Լ���
    int nSceneID = m_pKernelModule->GetPropertyInt(self, "SceneID");

    //�㲥�������Լ���ȥ(�㽵����Ծ��)
    int nOldGroupID = oldVar.GetInt();
    if(nOldGroupID > 0)
    {
        AFCDataList valueAllOldObjectList;
        AFCDataList valueAllOldPlayerList;
        m_pKernelModule->GetGroupObjectList(nSceneID, nOldGroupID, valueAllOldObjectList);
        if(valueAllOldObjectList.GetCount() > 0)
        {
            //�Լ�ֻ��Ҫ�㲥�������
            for(int i = 0; i < valueAllOldObjectList.GetCount(); i++)
            {
                AFGUID identBC = valueAllOldObjectList.Object(i);

                if(valueAllOldObjectList.Object(i) == self)
                {
                    valueAllOldObjectList.SetObject(i, AFGUID());
                }

                const std::string strClassName(m_pKernelModule->GetPropertyString(identBC, "ClassName"));
                if(NFrame::Player::ThisName() == strClassName)
                {
                    valueAllOldPlayerList << identBC;
                }
            }

            OnObjectListLeave(valueAllOldPlayerList, AFCDataList() << self);

            //�ϵ�ȫ��Ҫ�㲥ɾ��
            OnObjectListLeave(AFCDataList() << self, valueAllOldObjectList);
        }

        m_pKernelModule->DoEvent(self, AFED_ON_CLIENT_LEAVE_SCENE, AFCDataList() << nOldGroupID);
    }

    //�ٹ㲥�������Լ�����(��������Ծ��)
    int nNewGroupID = newVar.GetInt();
    if(nNewGroupID > 0)
    {
        //������Ҫ���Լ��ӹ㲥���ų�
        //////////////////////////////////////////////////////////////////////////
        AFCDataList valueAllObjectList;
        AFCDataList valueAllObjectListNoSelf;
        AFCDataList valuePlayerList;
        AFCDataList valuePlayerListNoSelf;
        m_pKernelModule->GetGroupObjectList(nSceneID, nNewGroupID, valueAllObjectList);
        for(int i = 0; i < valueAllObjectList.GetCount(); i++)
        {
            AFGUID identBC = valueAllObjectList.Object(i);
            const std::string strClassName(m_pKernelModule->GetPropertyString(identBC, "ClassName"));
            if(NFrame::Player::ThisName() == strClassName)
            {
                valuePlayerList << identBC;
                if(identBC != self)
                {
                    valuePlayerListNoSelf << identBC;
                }
            }

            if(identBC != self)
            {
                valueAllObjectListNoSelf << identBC;
            }
        }

        //�㲥������,�Լ�����(���ﱾ��Ӧ�ù㲥���Լ�)
        if(valuePlayerListNoSelf.GetCount() > 0)
        {
            OnObjectListEnter(valuePlayerListNoSelf, AFCDataList() << self);
        }

        const std::string strSelfClassName(m_pKernelModule->GetPropertyString(self, "ClassName"));

        //�㲥���Լ�,���еı��˳���
        if(valueAllObjectListNoSelf.GetCount() > 0)
        {
            if(strSelfClassName == NFrame::Player::ThisName())
            {
                OnObjectListEnter(AFCDataList() << self, valueAllObjectListNoSelf);
            }
        }

        if(strSelfClassName == NFrame::Player::ThisName())
        {
            for(int i = 0; i < valueAllObjectListNoSelf.GetCount(); i++)
            {
                //��ʱ�����ٹ㲥�Լ������Ը��Լ�
                //���Ѿ����ڵ��˵����Թ㲥����������
                AFGUID identOld = valueAllObjectListNoSelf.Object(i);

                OnPropertyEnter(AFCDataList() << self, identOld);
                //���Ѿ����ڵ��˵ı�㲥����������
                OnRecordEnter(AFCDataList() << self, identOld);
            }
        }

        //���������˵����Թ㲥���ܱߵ���
        if(valuePlayerListNoSelf.GetCount() > 0)
        {
            OnPropertyEnter(valuePlayerListNoSelf, self);
            OnRecordEnter(valuePlayerListNoSelf, self);
        }
    }

    return 0;
}

int AFCGameNetServerModule::OnContainerEvent(const AFGUID& self, const std::string& strPropertyName, const AFIData& oldVar, const AFIData& newVar)
{
    //���������仯��ֻ���ܴ�A������0���л���B������0��
    //��Ҫע�����------------�κ������ı��ʱ����ұ�����0��
    int nOldSceneID = oldVar.GetInt();
    int nNowSceneID = newVar.GetInt();

    m_pLogModule->LogInfo(self, "Enter Scene:", nNowSceneID);

    //�Լ���ʧ,��Ҳ��ù㲥����Ϊ����ʧ֮ǰ����ص�0�㣬���ѹ㲥�����
    AFCDataList valueOldAllObjectList;
    AFCDataList valueNewAllObjectList;
    AFCDataList valueAllObjectListNoSelf;
    AFCDataList valuePlayerList;
    AFCDataList valuePlayerNoSelf;

    m_pKernelModule->GetGroupObjectList(nOldSceneID, 0, valueOldAllObjectList);
    m_pKernelModule->GetGroupObjectList(nNowSceneID, 0, valueNewAllObjectList);

    for(int i = 0; i < valueOldAllObjectList.GetCount(); i++)
    {
        AFGUID identBC = valueOldAllObjectList.Object(i);
        if(identBC == self)
        {
            valueOldAllObjectList.SetObject(i, AFGUID());
        }
    }

    for(int i = 0; i < valueNewAllObjectList.GetCount(); i++)
    {
        AFGUID identBC = valueNewAllObjectList.Object(i);
        const std::string strClassName(m_pKernelModule->GetPropertyString(identBC, "ClassName"));
        if(NFrame::Player::ThisName() == strClassName)
        {
            valuePlayerList << identBC;
            if(identBC != self)
            {
                valuePlayerNoSelf << identBC;
            }
        }

        if(identBC != self)
        {
            valueAllObjectListNoSelf << identBC;
        }
    }

    //////////////////////////////////////////////////////////////////////////

    //���Ǿɳ���0���NPC��Ҫ�㲥
    OnObjectListLeave(AFCDataList() << self, valueOldAllObjectList);

    //�㲥�������˳��ֶ���(�������ң�������㲥���Լ�)
    //����㲥�Ķ���0���
    if(valuePlayerList.GetCount() > 0)
    {
        //��self�㲥��argVar��Щ��
        OnObjectListEnter(valuePlayerNoSelf, AFCDataList() << self);
    }

    //�²��Ȼ��0����0��NPC�㲥���Լ�------------�Լ��㲥���Լ���������㲥����Ϊ����ID�ڿ糡��ʱ�ᾭ���仯

    //��valueAllObjectList�㲥��self
    OnObjectListEnter(AFCDataList() << self, valueAllObjectListNoSelf);

    ////////////////////���Ѿ����ڵ��˵����Թ㲥����������//////////////////////////////////////////////////////
    for(int i = 0; i < valueAllObjectListNoSelf.GetCount(); i++)
    {
        AFGUID identOld = valueAllObjectListNoSelf.Object(i);
        OnPropertyEnter(AFCDataList() << self, identOld);
        ////////////////////���Ѿ����ڵ��˵ı�㲥����������//////////////////////////////////////////////////////
        OnRecordEnter(AFCDataList() << self, identOld);
    }

    //���������˵����Թ㲥���ܱߵ���()
    if(valuePlayerNoSelf.GetCount() > 0)
    {
        OnPropertyEnter(valuePlayerNoSelf, self);
        OnRecordEnter(valuePlayerNoSelf, self);
    }

    return 0;
}

int AFCGameNetServerModule::GetBroadCastObject(const AFGUID& self, const std::string& strPropertyName, const bool bTable, AFIDataList& valueObject)
{
    int nObjectContainerID = m_pKernelModule->GetPropertyInt(self, "SceneID");
    int nObjectGroupID = m_pKernelModule->GetPropertyInt(self, "GroupID");

    //��ͨ�����������жϹ㲥����
    std::string strClassName = m_pKernelModule->GetPropertyString(self, "ClassName");

    ARK_SHARE_PTR<AFIPropertyMgr> pClassPropertyManager = m_pClassModule->GetClassPropertyManager(strClassName);
    ARK_SHARE_PTR<AFIRecordMgr> pClassRecordManager = m_pClassModule->GetClassRecordManager(strClassName);

    AFRecord* pRecord = NULL;
    AFProperty* pProperty(nullptr);
    if(bTable)
    {
        if(nullptr == pClassRecordManager)
        {
            return -1;
        }

        pRecord = pClassRecordManager->GetRecord(strPropertyName.c_str());
        if(nullptr == pRecord)
        {
            return -1;
        }
    }
    else
    {
        if(nullptr == pClassPropertyManager)
        {
            return -1;
        }

        pProperty = pClassPropertyManager->GetProperty(strPropertyName.c_str());
        if(nullptr == pProperty)
        {
            return -1;
        }
    }

    if(NFrame::Player::ThisName() == strClassName)
    {
        if(bTable)
        {
            if(pRecord->IsPublic())
            {
                int nCount = GetBroadCastObject(nObjectContainerID, nObjectGroupID, valueObject);
                if(nCount < 0)
                {
                    //log
                }
            }
            else if(pRecord->IsPrivate())
            {
                valueObject.AddObject(self);
            }
        }
        else
        {
            if(pProperty->IsPublic())
            {
                int nCount = GetBroadCastObject(nObjectContainerID, nObjectGroupID, valueObject);
            }
            else if(pProperty->IsPrivate())
            {
                valueObject.AddObject(self);
            }
        }
        //һ����Ҷ����㲥
        return valueObject.GetCount();
    }

    //�������,NPC�͹������
    if(bTable)
    {
        if(pRecord->IsPublic())
        {
            //�㲥���ͻ����Լ����ܱ���
            int nCount = GetBroadCastObject(nObjectContainerID, nObjectGroupID, valueObject);
        }
    }
    else
    {
        if(pProperty->IsPublic())
        {
            //�㲥���ͻ����Լ����ܱ���
            int nCount = GetBroadCastObject(nObjectContainerID, nObjectGroupID, valueObject);
        }
    }

    return valueObject.GetCount();
}

int AFCGameNetServerModule::GetBroadCastObject(const int nObjectContainerID, const int nGroupID, AFIDataList& valueObject)
{
    AFCDataList valContainerObjectList;
    m_pKernelModule->GetGroupObjectList(nObjectContainerID, nGroupID, valContainerObjectList);
    for(int i = 0; i < valContainerObjectList.GetCount(); i++)
    {
        const std::string& strObjClassName = m_pKernelModule->GetPropertyString(valContainerObjectList.Object(i), "ClassName");
        if(NFrame::Player::ThisName() == strObjClassName)
        {
            valueObject.AddObject(valContainerObjectList.Object(i));
        }
    }

    return valueObject.GetCount();
}

int AFCGameNetServerModule::OnObjectClassEvent(const AFGUID& self, const std::string& strClassName, const CLASS_OBJECT_EVENT eClassEvent, const AFIDataList& var)
{
    if(CLASS_OBJECT_EVENT::COE_DESTROY == eClassEvent)
    {
        //SaveDataToNoSql( self, true );
        m_pLogModule->LogInfo(self, "Player Offline", "");
    }
    else if(CLASS_OBJECT_EVENT::COE_CREATE_LOADDATA == eClassEvent)
    {
        //LoadDataFormNoSql( self );
    }
    else if(CLASS_OBJECT_EVENT::COE_CREATE_FINISH == eClassEvent)
    {
        m_pKernelModule->AddEventCallBack(self, AFED_ON_OBJECT_ENTER_SCENE_BEFORE, this, &AFCGameNetServerModule::OnSwapSceneResultEvent);
    }

    return 0;
}

int AFCGameNetServerModule::OnSwapSceneResultEvent(const AFGUID& self, const int nEventID, const AFIDataList& var)
{
    if(var.GetCount() != 7 ||
            !var.TypeEx(AF_DATA_TYPE::DT_OBJECT, AF_DATA_TYPE::DT_INT, AF_DATA_TYPE::DT_INT,
                        AF_DATA_TYPE::DT_INT, AF_DATA_TYPE::DT_FLOAT, AF_DATA_TYPE::DT_FLOAT, AF_DATA_TYPE::DT_FLOAT, AF_DATA_TYPE::DT_UNKNOWN)
      )
    {
        return 1;
    }

    AFGUID ident = var.Object(0);
    int nType = var.Int(1);
    int nTargetScene = var.Int(2);
    int nTargetGroupID = var.Int(3);
    Point3D xPos;
    xPos.x = var.Float(4);
    xPos.y = var.Float(5);
    xPos.z = var.Float(6);

    AFMsg::ReqAckSwapScene xSwapScene;
    xSwapScene.set_transfer_type(AFMsg::ReqAckSwapScene::EGameSwapType::ReqAckSwapScene_EGameSwapType_EGST_NARMAL);
    xSwapScene.set_scene_id(nTargetScene);
    xSwapScene.set_line_id(nTargetGroupID);
    xSwapScene.set_x(xPos.x);
    xSwapScene.set_y(xPos.y);
    xSwapScene.set_z(xPos.z);

    SendMsgPBToGate(AFMsg::EGMI_ACK_SWAP_SCENE, xSwapScene, self);

    return 0;
}

void AFCGameNetServerModule::OnReqiureRoleListProcess(const AFIMsgHead& xHead, const int nMsgID, const char* msg, const uint32_t nLen, const AFGUID& xClientID)
{
    //fd
    AFGUID nGateClientID;
    AFMsg::ReqRoleList xMsg;
    if(!m_pNetModule->ReceivePB(xHead, nMsgID, msg, nLen, xMsg, nGateClientID))
    {
        return;
    }
    AFMsg::AckRoleLiteInfoList xAckRoleLiteInfoList;
    if(!m_AccountModule->GetRoleList(xMsg.account(), xAckRoleLiteInfoList))
    {
        m_pLogModule->LogError(nGateClientID, "Get Role list failed", "", __FUNCTION__, __LINE__);
    }

    m_pNetModule->SendMsgPB(AFMsg::EGMI_ACK_ROLE_LIST, xAckRoleLiteInfoList, xClientID, nGateClientID);
}

void AFCGameNetServerModule::OnCreateRoleGameProcess(const AFIMsgHead& xHead, const int nMsgID, const char* msg, const uint32_t nLen, const AFGUID& xClientID)
{
    AFGUID nGateClientID;
    AFMsg::ReqCreateRole xMsg;
    if(!m_pNetModule->ReceivePB(xHead, nMsgID, msg, nLen, xMsg, nGateClientID))
    {
        return;
    }

    AFMsg::AckRoleLiteInfoList xAckRoleLiteInfoList;
    AFCDataList varList;
    varList.AddInt(xMsg.career());
    varList.AddInt(xMsg.sex());
    varList.AddInt(xMsg.race());
    varList.AddString(xMsg.noob_name().c_str());
    varList.AddInt(xMsg.game_id());
    if(!m_AccountModule->CreateRole(xMsg.account(), xAckRoleLiteInfoList, varList))
    {
        m_pLogModule->LogError(nGateClientID, "create role failed", "", __FUNCTION__, __LINE__);
    }

    m_pNetModule->SendMsgPB(AFMsg::EGMI_ACK_ROLE_LIST, xAckRoleLiteInfoList, xClientID, nGateClientID);
}

void AFCGameNetServerModule::OnDeleteRoleGameProcess(const AFIMsgHead& xHead, const int nMsgID, const char* msg, const uint32_t nLen, const AFGUID& xClientID)
{
    AFGUID nPlayerID;
    AFMsg::ReqDeleteRole xMsg;
    if(!m_pNetModule->ReceivePB(xHead, nMsgID, msg, nLen, xMsg, nPlayerID))
    {
        return;
    }

    AFMsg::AckRoleLiteInfoList xAckRoleLiteInfoList;
    if(!m_AccountModule->DeleteRole(xMsg.account(), xAckRoleLiteInfoList))
    {
        m_pLogModule->LogError(nPlayerID, "delete role failed", "", __FUNCTION__, __LINE__);
    }

    m_pNetModule->SendMsgPB(AFMsg::EGMI_ACK_ROLE_LIST, xAckRoleLiteInfoList, xClientID, nPlayerID);
}

void AFCGameNetServerModule::OnClienSwapSceneProcess(const AFIMsgHead& xHead, const int nMsgID, const char* msg, const uint32_t nLen, const AFGUID& xClientID)
{
    CLIENT_MSG_PROCESS(nMsgID, msg, nLen, AFMsg::ReqAckSwapScene);

    AFCDataList varEntry;
    varEntry << pObject->Self();
    varEntry << int32_t(0);
    varEntry << xMsg.scene_id();
    varEntry << int32_t(-1) ;
    m_pKernelModule->DoEvent(pObject->Self(), AFED_ON_CLIENT_ENTER_SCENE, varEntry);
}

void AFCGameNetServerModule::OnProxyServerRegisteredProcess(const AFIMsgHead& xHead, const int nMsgID, const char* msg, const uint32_t nLen, const AFGUID& xClientID)
{
    AFGUID nPlayerID;
    AFMsg::ServerInfoReportList xMsg;
    if(!m_pNetModule->ReceivePB(xHead, nMsgID, msg, nLen, xMsg, nPlayerID))
    {
        return;
    }

    for(int i = 0; i < xMsg.server_list_size(); ++i)
    {
        const AFMsg::ServerInfoReport& xData = xMsg.server_list(i);
        ARK_SHARE_PTR<GateServerInfo> pServerData = mProxyMap.GetElement(xData.server_id());
        if(nullptr == pServerData)
        {
            pServerData = ARK_SHARE_PTR<GateServerInfo>(ARK_NEW GateServerInfo());
            mProxyMap.AddElement(xData.server_id(), pServerData);
        }

        pServerData->xServerData.xClient = xClientID;
        *(pServerData->xServerData.pData) = xData;

        m_pLogModule->LogInfo(AFGUID(0, xData.server_id()), xData.server_name(), "Proxy Registered");
    }

    return;
}

void AFCGameNetServerModule::OnProxyServerUnRegisteredProcess(const AFIMsgHead& xHead, const int nMsgID, const char* msg, const uint32_t nLen, const AFGUID& xClientID)
{
    AFGUID nPlayerID;
    AFMsg::ServerInfoReportList xMsg;
    if(!m_pNetModule->ReceivePB(xHead, nMsgID, msg, nLen, xMsg, nPlayerID))
    {
        return;
    }

    for(int i = 0; i < xMsg.server_list_size(); ++i)
    {
        const AFMsg::ServerInfoReport& xData = xMsg.server_list(i);
        mProxyMap.RemoveElement(xData.server_id());


        m_pLogModule->LogInfo(AFGUID(0, xData.server_id()), xData.server_name(), "Proxy UnRegistered");
    }

    return;
}

void AFCGameNetServerModule::OnRefreshProxyServerInfoProcess(const AFIMsgHead& xHead, const int nMsgID, const char* msg, const uint32_t nLen, const AFGUID& xClientID)
{
    AFGUID nPlayerID;
    AFMsg::ServerInfoReportList xMsg;
    if(!m_pNetModule->ReceivePB(xHead, nMsgID, msg, nLen, xMsg, nPlayerID))
    {
        return;
    }

    for(int i = 0; i < xMsg.server_list_size(); ++i)
    {
        const AFMsg::ServerInfoReport& xData = xMsg.server_list(i);
        ARK_SHARE_PTR<GateServerInfo> pServerData = mProxyMap.GetElement(xData.server_id());
        if(nullptr == pServerData)
        {
            pServerData = ARK_SHARE_PTR<GateServerInfo>(ARK_NEW GateServerInfo());
            mProxyMap.AddElement(xData.server_id(), pServerData);
        }

        pServerData->xServerData.xClient = xClientID;
        *(pServerData->xServerData.pData) = xData;

        m_pLogModule->LogInfo(AFGUID(0, xData.server_id()), xData.server_name(), "Proxy Registered");
    }

    return;
}

void AFCGameNetServerModule::SendMsgPBToGate(const uint16_t nMsgID, google::protobuf::Message& xMsg, const AFGUID& self)
{
    ARK_SHARE_PTR<GateBaseInfo> pData = mRoleBaseData.GetElement(self);
    if(nullptr != pData)
    {
        ARK_SHARE_PTR<GateServerInfo> pProxyData = mProxyMap.GetElement(pData->nGateID);
        if(nullptr != pProxyData)
        {
            m_pNetModule->SendMsgPB(nMsgID, xMsg, pProxyData->xServerData.xClient, pData->xClientID);
        }
    }
}

void AFCGameNetServerModule::SendMsgPBToGate(const uint16_t nMsgID, const std::string& strMsg, const AFGUID& self)
{
    ARK_SHARE_PTR<GateBaseInfo> pData = mRoleBaseData.GetElement(self);
    if(nullptr != pData)
    {
        ARK_SHARE_PTR<GateServerInfo> pProxyData = mProxyMap.GetElement(pData->nGateID);
        if(nullptr != pProxyData)
        {
            m_pNetModule->SendMsgPB(nMsgID, strMsg, pProxyData->xServerData.xClient, pData->xClientID);
        }
    }
}

AFINetServerModule* AFCGameNetServerModule::GetNetModule()
{
    return m_pNetModule;
}

bool AFCGameNetServerModule::AddPlayerGateInfo(const AFGUID& nRoleID, const AFGUID& nClientID, const int nGateID)
{
    if(nGateID <= 0)
    {
        //�Ƿ�gate
        return false;
    }

    if(nClientID.IsNull())
    {
        return false;
    }

    ARK_SHARE_PTR<AFCGameNetServerModule::GateBaseInfo> pBaseData = mRoleBaseData.GetElement(nRoleID);
    if(nullptr != pBaseData)
    {
        // �Ѿ�����
        m_pLogModule->LogError(nClientID, "player is exist, cannot enter game", "", __FUNCTION__, __LINE__);
        return false;
    }

    ARK_SHARE_PTR<GateServerInfo> pServerData = mProxyMap.GetElement(nGateID);
    if(nullptr == pServerData)
    {
        return false;
    }

    if(!pServerData->xRoleInfo.insert(std::make_pair(nRoleID, pServerData->xServerData.xClient)).second)
    {
        return false;
    }

    if(!mRoleBaseData.AddElement(nRoleID, ARK_SHARE_PTR<GateBaseInfo>(ARK_NEW GateBaseInfo(nGateID, nClientID))))
    {
        pServerData->xRoleInfo.erase(nRoleID);
        return false;
    }

    return true;
}

bool AFCGameNetServerModule::RemovePlayerGateInfo(const AFGUID& nRoleID)
{
    ARK_SHARE_PTR<GateBaseInfo> pBaseData = mRoleBaseData.GetElement(nRoleID);
    if(nullptr == pBaseData)
    {
        return false;
    }

    mRoleBaseData.RemoveElement(nRoleID);

    ARK_SHARE_PTR<GateServerInfo> pServerData = mProxyMap.GetElement(pBaseData->nGateID);
    if(nullptr == pServerData)
    {
        return false;
    }

    pServerData->xRoleInfo.erase(nRoleID);
    return true;
}

ARK_SHARE_PTR<AFIGameNetServerModule::GateBaseInfo> AFCGameNetServerModule::GetPlayerGateInfo(const AFGUID& nRoleID)
{
    return mRoleBaseData.GetElement(nRoleID);
}

ARK_SHARE_PTR<AFIGameNetServerModule::GateServerInfo> AFCGameNetServerModule::GetGateServerInfo(const int nGateID)
{
    return mProxyMap.GetElement(nGateID);
}

ARK_SHARE_PTR<AFIGameNetServerModule::GateServerInfo> AFCGameNetServerModule::GetGateServerInfoByClientID(const AFGUID& nClientID)
{
    int nGateID = -1;
    ARK_SHARE_PTR<GateServerInfo> pServerData = mProxyMap.First();
    while(nullptr != pServerData)
    {
        if(nClientID == pServerData->xServerData.xClient)
        {
            nGateID = pServerData->xServerData.pData->server_id();
            break;
        }

        pServerData = mProxyMap.Next();
    }

    if(nGateID == -1)
    {
        return nullptr;
    }

    return pServerData;
}

void AFCGameNetServerModule::OnTransWorld(const AFIMsgHead& xHead, const int nMsgID, const char* msg, const uint32_t nLen, const AFGUID& xClientID)
{
    std::string strMsg;
    AFGUID nPlayer;
    int nHasKey = 0;
    if(AFINetServerModule::ReceivePB(xHead, nMsgID, msg, nLen, strMsg, nPlayer))
    {
        nHasKey = nPlayer.nIdent;
    }

    m_pGameServerToWorldModule->SendBySuit(nHasKey, nMsgID, msg, nLen);
}

