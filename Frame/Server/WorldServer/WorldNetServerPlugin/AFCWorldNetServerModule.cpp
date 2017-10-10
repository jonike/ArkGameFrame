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

#include "AFCWorldNetServerModule.h"
#include "AFWorldNetServerPlugin.h"
#include "SDK/Proto/AFMsgDefine.h"

bool AFCWorldNetServerModule::Init()
{
    m_pNetModule = ARK_NEW AFINetServerModule(pPluginManager);
    return true;
}

bool AFCWorldNetServerModule::AfterInit()
{
    m_pKernelModule = pPluginManager->FindModule<AFIKernelModule>();
    m_pWorldLogicModule = pPluginManager->FindModule<AFIWorldLogicModule>();
    m_pLogModule = pPluginManager->FindModule<AFILogModule>();
    m_pElementModule = pPluginManager->FindModule<AFIElementModule>();
    m_pClassModule = pPluginManager->FindModule<AFIClassModule>();

    m_pNetModule->AddReceiveCallBack(AFMsg::EGMI_PTWG_PROXY_REFRESH, this, &AFCWorldNetServerModule::OnRefreshProxyServerInfoProcess);
    m_pNetModule->AddReceiveCallBack(AFMsg::EGMI_PTWG_PROXY_REGISTERED, this, &AFCWorldNetServerModule::OnProxyServerRegisteredProcess);
    m_pNetModule->AddReceiveCallBack(AFMsg::EGMI_PTWG_PROXY_UNREGISTERED, this, &AFCWorldNetServerModule::OnProxyServerUnRegisteredProcess);
    m_pNetModule->AddReceiveCallBack(AFMsg::EGMI_GTW_GAME_REGISTERED, this, &AFCWorldNetServerModule::OnGameServerRegisteredProcess);
    m_pNetModule->AddReceiveCallBack(AFMsg::EGMI_GTW_GAME_UNREGISTERED, this, &AFCWorldNetServerModule::OnGameServerUnRegisteredProcess);
    m_pNetModule->AddReceiveCallBack(AFMsg::EGMI_GTW_GAME_REFRESH, this, &AFCWorldNetServerModule::OnRefreshGameServerInfoProcess);
    m_pNetModule->AddReceiveCallBack(AFMsg::EGMI_ACK_ONLINE_NOTIFY, this, &AFCWorldNetServerModule::OnOnlineProcess);
    m_pNetModule->AddReceiveCallBack(AFMsg::EGMI_ACK_OFFLINE_NOTIFY, this, &AFCWorldNetServerModule::OnOfflineProcess);

    m_pNetModule->AddEventCallBack(this, &AFCWorldNetServerModule::OnSocketEvent);

    ARK_SHARE_PTR<AFIClass> xLogicClass = m_pClassModule->GetElement("Server");
    if(nullptr != xLogicClass)
    {
        NFList<std::string>& xNameList = xLogicClass->GetConfigNameList();
        std::string strConfigName;
        for(bool bRet = xNameList.First(strConfigName); bRet; bRet = xNameList.Next(strConfigName))
        {
            const int nServerType = m_pElementModule->GetPropertyInt(strConfigName, "Type");
            const int nServerID = m_pElementModule->GetPropertyInt(strConfigName, "ServerID");
            if(nServerType == NF_SERVER_TYPES::NF_ST_WORLD && pPluginManager->AppID() == nServerID)
            {
                const int nPort = m_pElementModule->GetPropertyInt(strConfigName, "Port");
                const int nMaxConnect = m_pElementModule->GetPropertyInt(strConfigName, "MaxOnline");
                const int nCpus = m_pElementModule->GetPropertyInt(strConfigName, "CpuCount");
                const std::string strName(m_pElementModule->GetPropertyString(strConfigName, "Name"));
                const std::string strIP(m_pElementModule->GetPropertyString(strConfigName, "IP"));

                int nRet = m_pNetModule->Initialization(nMaxConnect, strIP, nPort, nCpus, nServerID);
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

bool AFCWorldNetServerModule::Shut()
{
    return true;
}

bool AFCWorldNetServerModule::Execute()
{
    LogGameServer();

    return m_pNetModule->Execute();
}

void AFCWorldNetServerModule::OnGameServerRegisteredProcess(const AFIMsgHead& xHead, const int nMsgID, const char* msg, const uint32_t nLen, const AFGUID& xClientID)
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
        ARK_SHARE_PTR<ServerData> pServerData =  mGameMap.GetElement(xData.server_id());
        if(nullptr == pServerData)
        {
            pServerData = ARK_SHARE_PTR<ServerData>(ARK_NEW ServerData());
            mGameMap.AddElement(xData.server_id(), pServerData);
        }

        pServerData->xClient = xClientID;
        *(pServerData->pData) = xData;

        m_pLogModule->LogInfo(AFGUID(0, xData.server_id()), xData.server_name(), "GameServerRegistered");
    }

    SynGameToProxy();
}

void AFCWorldNetServerModule::OnGameServerUnRegisteredProcess(const AFIMsgHead& xHead, const int nMsgID, const char* msg, const uint32_t nLen, const AFGUID& xClientID)
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
        mGameMap.RemoveElement(xData.server_id());

        m_pLogModule->LogInfo(AFGUID(0, xData.server_id()), xData.server_name(), "GameServerRegistered");
    }
}

void AFCWorldNetServerModule::OnRefreshGameServerInfoProcess(const AFIMsgHead& xHead, const int nMsgID, const char* msg, const uint32_t nLen, const AFGUID& xClientID)
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

        ARK_SHARE_PTR<ServerData> pServerData =  mGameMap.GetElement(xData.server_id());
        if(nullptr == pServerData)
        {
            pServerData = ARK_SHARE_PTR<ServerData>(ARK_NEW ServerData());
            mGameMap.AddElement(xData.server_id(), pServerData);
        }

        pServerData->xClient = xClientID;
        *(pServerData->pData) = xData;

        m_pLogModule->LogInfo(AFGUID(0, xData.server_id()), xData.server_name(), "GameServerRegistered");
    }

    SynGameToProxy();
}

void AFCWorldNetServerModule::OnProxyServerRegisteredProcess(const AFIMsgHead& xHead, const int nMsgID, const char* msg, const uint32_t nLen, const AFGUID& xClientID)
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

        ARK_SHARE_PTR<ServerData> pServerData =  mProxyMap.GetElement(xData.server_id());
        if(!pServerData)
        {
            pServerData = ARK_SHARE_PTR<ServerData>(ARK_NEW ServerData());
            mProxyMap.AddElement(xData.server_id(), pServerData);
        }

        pServerData->xClient = xClientID;
        *(pServerData->pData) = xData;

        m_pLogModule->LogInfo(AFGUID(0, xData.server_id()), xData.server_name(), "Proxy Registered");

        SynGameToProxy(xClientID);
    }
}

void AFCWorldNetServerModule::OnProxyServerUnRegisteredProcess(const AFIMsgHead& xHead, const int nMsgID, const char* msg, const uint32_t nLen, const AFGUID& xClientID)
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

        mGameMap.RemoveElement(xData.server_id());

        m_pLogModule->LogInfo(AFGUID(0, xData.server_id()), xData.server_name(), "Proxy UnRegistered");
    }
}

void AFCWorldNetServerModule::OnRefreshProxyServerInfoProcess(const AFIMsgHead& xHead, const int nMsgID, const char* msg, const uint32_t nLen, const AFGUID& xClientID)
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

        ARK_SHARE_PTR<ServerData> pServerData =  mProxyMap.GetElement(xData.server_id());
        if(nullptr == pServerData)
        {
            pServerData = ARK_SHARE_PTR<ServerData>(ARK_NEW ServerData());
            mGameMap.AddElement(xData.server_id(), pServerData);
        }

        pServerData->xClient = xClientID;
        *(pServerData->pData) = xData;

        m_pLogModule->LogInfo(AFGUID(0, xData.server_id()), xData.server_name(), "Proxy Registered");

        SynGameToProxy(xClientID);
    }
}

int AFCWorldNetServerModule::OnLeaveGameProcess(const AFIMsgHead& xHead, const int nMsgID, const char* msg, const uint32_t nLen, const AFGUID& xClientID)
{

    return 0;
}

void AFCWorldNetServerModule::OnSocketEvent(const NetEventType eEvent, const AFGUID& xClientID, const int nServerID)
{
    if(eEvent == DISCONNECTED)
    {
        m_pLogModule->LogInfo(xClientID, "NF_NET_EVENT_EOF", "Connection closed", __FUNCTION__, __LINE__);
        OnClientDisconnect(xClientID);
    }
    else  if(eEvent == CONNECTED)
    {
        m_pLogModule->LogInfo(xClientID, "NF_NET_EVENT_CONNECTED", "connectioned success", __FUNCTION__, __LINE__);
        OnClientConnected(xClientID);
    }
}

void AFCWorldNetServerModule::SynGameToProxy()
{
    AFMsg::ServerInfoReportList xData;

    ARK_SHARE_PTR<ServerData> pServerData =  mProxyMap.First();
    while(nullptr != pServerData)
    {
        SynGameToProxy(pServerData->xClient);

        pServerData = mProxyMap.Next();
    }
}

void AFCWorldNetServerModule::SynGameToProxy(const AFGUID& xClientID)
{
    AFMsg::ServerInfoReportList xData;

    ARK_SHARE_PTR<ServerData> pServerData =  mGameMap.First();
    while(nullptr != pServerData)
    {
        AFMsg::ServerInfoReport* pData = xData.add_server_list();
        *pData = *(pServerData->pData);

        pServerData = mGameMap.Next();
    }

    m_pNetModule->SendMsgPB(AFMsg::EGameMsgID::EGMI_STS_NET_INFO, xData, xClientID, AFGUID(0));
}

void AFCWorldNetServerModule::OnClientDisconnect(const AFGUID& xClientID)
{
    //������game����proxy��Ҫ�ҳ���,������ע��
    ARK_SHARE_PTR<ServerData> pServerData =  mGameMap.First();
    while(nullptr != pServerData)
    {
        if(xClientID == pServerData->xClient)
        {
            pServerData->pData->set_server_state(AFMsg::EST_CRASH);
            pServerData->xClient = 0;

            SynGameToProxy();
            return;
        }

        pServerData = mGameMap.Next();
    }

    //////////////////////////////////////////////////////////////////////////

    int nServerID = 0;
    pServerData =  mProxyMap.First();
    while(pServerData)
    {
        if(xClientID == pServerData->xClient)
        {
            nServerID = pServerData->pData->server_id();
            break;
        }

        pServerData = mProxyMap.Next();
    }

    mProxyMap.RemoveElement(nServerID);
}

void AFCWorldNetServerModule::OnClientConnected(const AFGUID& xClientID)
{


}

void AFCWorldNetServerModule::LogGameServer()
{
    if(mnLastCheckTime + 10 > GetPluginManager()->GetNowTime())
    {
        return;
    }

    mnLastCheckTime = GetPluginManager()->GetNowTime();

    m_pLogModule->LogInfo(AFGUID(), "Begin Log GameServer Info", "");

    ARK_SHARE_PTR<ServerData> pGameData = mGameMap.First();
    while(pGameData)
    {
        std::ostringstream stream;
        stream << "Type: " << pGameData->pData->server_type() << " ID: " << pGameData->pData->server_id() << " State: " <<  AFMsg::EServerState_Name(pGameData->pData->server_state()) << " IP: " << pGameData->pData->server_ip() << " xClient: " << pGameData->xClient.n64Value;

        m_pLogModule->LogInfo(AFGUID(), stream);

        pGameData = mGameMap.Next();
    }

    m_pLogModule->LogInfo(AFGUID(), "End Log GameServer Info", "");

    m_pLogModule->LogInfo(AFGUID(), "Begin Log ProxyServer Info", "");

    pGameData = mProxyMap.First();
    while(pGameData)
    {
        std::ostringstream stream;
        stream << "Type: " << pGameData->pData->server_type() << " ID: " << pGameData->pData->server_id() << " State: " <<  AFMsg::EServerState_Name(pGameData->pData->server_state()) << " IP: " << pGameData->pData->server_ip() << " xClient: " << pGameData->xClient.n64Value;

        m_pLogModule->LogInfo(AFGUID(), stream);

        pGameData = mProxyMap.Next();
    }

    m_pLogModule->LogInfo(AFGUID(), "End Log ProxyServer Info", "");
}


void AFCWorldNetServerModule::OnOnlineProcess(const AFIMsgHead& xHead, const int nMsgID, const char* msg, const uint32_t nLen, const AFGUID& xClientID)
{
    CLIENT_MSG_PROCESS_NO_OBJECT(nMsgID, msg, nLen, AFMsg::RoleOnlineNotify);

}

void AFCWorldNetServerModule::OnOfflineProcess(const AFIMsgHead& xHead, const int nMsgID, const char* msg, const uint32_t nLen, const AFGUID& xClientID)
{
    CLIENT_MSG_PROCESS_NO_OBJECT(nMsgID, msg, nLen, AFMsg::RoleOfflineNotify);
}

bool AFCWorldNetServerModule::SendMsgToGame(const int nGameID, const AFMsg::EGameMsgID eMsgID, google::protobuf::Message& xData, const AFGUID nPlayer)
{
    ARK_SHARE_PTR<ServerData> pData = mGameMap.GetElement(nGameID);
    if(nullptr != pData)
    {
        m_pNetModule->SendMsgPB(eMsgID, xData, pData->xClient, nPlayer);
    }

    return true;
}

bool AFCWorldNetServerModule::SendMsgToGame(const AFIDataList& argObjectVar, const AFIDataList& argGameID, const AFMsg::EGameMsgID eMsgID, google::protobuf::Message& xData)
{
    if(argGameID.GetCount() != argObjectVar.GetCount())
    {
        return false;
    }

    for(int i = 0; i < argObjectVar.GetCount(); i++)
    {
        const AFGUID& identOther = argObjectVar.Object(i);
        const int64_t  nGameID = argGameID.Int(i);

        SendMsgToGame(nGameID, eMsgID, xData, identOther);
    }

    return true;
}

bool AFCWorldNetServerModule::SendMsgToPlayer(const AFMsg::EGameMsgID eMsgID, google::protobuf::Message& xData, const AFGUID nPlayer)
{
    int nGameID = GetPlayerGameID(nPlayer);
    if(nGameID < 0)
    {
        return false;
    }

    return SendMsgToGame(nGameID, eMsgID, xData, nPlayer);
}

int AFCWorldNetServerModule::OnObjectListEnter(const AFIDataList& self, const AFIDataList& argVar)
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
        //*pEntryInfo->mutable_pos() = AFINetServerModule::NFToPB(m_pKernelModule->GetObject(identOld, "Pos")); todo
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
        SendMsgToPlayer(AFMsg::EGMI_ACK_OBJECT_ENTRY, xPlayerEntryInfoList, ident);
    }

    return 1;
}


int AFCWorldNetServerModule::OnObjectListLeave(const AFIDataList& self, const AFIDataList& argVar)
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
        SendMsgToPlayer(AFMsg::EGMI_ACK_OBJECT_LEAVE, xPlayerLeaveInfoList, ident);
    }

    return 1;
}


int AFCWorldNetServerModule::OnRecordEnter(const AFIDataList& argVar, const AFIDataList& argGameID, const AFGUID& self)
{
    if(argVar.GetCount() <= 0 || self.IsNull())
    {
        return 0;
    }

    if(argVar.GetCount() != argGameID.GetCount())
    {
        return 1;
    }

    AFMsg::MultiObjectRecordList xPublicMsg;
    AFMsg::MultiObjectRecordList xPrivateMsg;

    ARK_SHARE_PTR<AFIObject> pObject = m_pKernelModule->GetObject(self);
    if(nullptr == pObject)
    {
        return 1;
    }

    AFMsg::ObjectRecordList* pPublicData = NULL;
    AFMsg::ObjectRecordList* pPrivateData = NULL;

    ARK_SHARE_PTR<AFIRecordMgr> pRecordManager = pObject->GetRecordManager();
    int nRecordCount = pRecordManager->GetCount();
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

            bool bRet = OnRecordEnterPack(pRecord, pPublicRecordBase);
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

            bool bRet = OnRecordEnterPack(pRecord, pPrivateRecordBase);
        }
    }

    for(int i = 0; i < argVar.GetCount(); i++)
    {
        const AFGUID& identOther = argVar.Object(i);
        const int64_t nGameID = argGameID.Int(i);
        if(self == identOther)
        {
            if(xPrivateMsg.multi_player_record_size() > 0)
            {
                SendMsgToGame(nGameID, AFMsg::EGMI_ACK_OBJECT_RECORD_ENTRY, xPrivateMsg, identOther);
            }
        }
        else
        {
            if(xPublicMsg.multi_player_record_size() > 0)
            {
                SendMsgToGame(nGameID, AFMsg::EGMI_ACK_OBJECT_RECORD_ENTRY, xPublicMsg, identOther);
            }
        }
    }

    return 0;
}

bool AFCWorldNetServerModule::OnRecordEnterPack(AFRecord* pRecord, AFMsg::ObjectRecordBase* pObjectRecordBase)
{
    if(!pRecord || !pObjectRecordBase)
    {
        return false;
    }

    for(int i = 0; i < pRecord->GetRowCount(); i ++)
    {
        //����public����private��Ҫ���ϣ���Ȼpublic�㲥���ǲ���private�͹㲥������
        AFMsg::RecordAddRowStruct* pAddRowStruct = pObjectRecordBase->add_row_struct();
        pAddRowStruct->set_row(i);

        AFCDataList valueList;
        pRecord->QueryRow(i, valueList);

        for(int j = 0; j < valueList.GetCount(); j++)
        {
            AFMsg::RecordPBData* pAddData = pAddRowStruct->add_record_data_list();
            AFINetServerModule::RecordToPBRecord(valueList, i, j, *pAddData);
        }
    }

    return true;
}

int AFCWorldNetServerModule::OnPropertyEnter(const AFIDataList& argVar, const AFIDataList& argGameID, const AFGUID& self)
{
    if(argVar.GetCount() <= 0 || self.IsNull())
    {
        return 0;
    }

    if(argVar.GetCount() != argGameID.GetCount())
    {
        return 1;
    }



    //��Ϊ�Լ�������
    //1.public���͸�������
    //2.����Լ����б��У��ٴη���private����
    ARK_SHARE_PTR<AFIObject> pObject = m_pKernelModule->GetObject(self);
    if(nullptr == pObject)
    {
        return 0;
    }

    AFMsg::MultiObjectPropertyList xPublicMsg;
    AFMsg::MultiObjectPropertyList xPrivateMsg;
    AFMsg::ObjectPropertyList* pPublicData = xPublicMsg.add_multi_player_property();
    AFMsg::ObjectPropertyList* pPrivateData = xPrivateMsg.add_multi_player_property();

    *(pPublicData->mutable_player_id()) = AFINetServerModule::NFToPB(self);
    *(pPrivateData->mutable_player_id()) = AFINetServerModule::NFToPB(self);

    ARK_SHARE_PTR<AFIPropertyMgr> pPropertyManager = pObject->GetPropertyManager();

    for(int i = 0; i < pPropertyManager->GetPropertyCount(); i++)
    {
        AFProperty* pPropertyInfo = pPropertyManager->GetPropertyByIndex(i);
        if(nullptr == pPropertyInfo)
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
        const AFGUID& identOther = argVar.Object(i);
        const int64_t nGameID = argGameID.Int(i);
        if(self == identOther)
        {
            //�ҵ����������ص�FD
            SendMsgToGame(nGameID, AFMsg::EGMI_ACK_OBJECT_PROPERTY_ENTRY, xPrivateMsg, identOther);
        }
        else
        {
            SendMsgToGame(nGameID, AFMsg::EGMI_ACK_OBJECT_PROPERTY_ENTRY, xPublicMsg, identOther);
        }

    }

    return 0;
}

ARK_SHARE_PTR<ServerData> AFCWorldNetServerModule::GetSuitProxyForEnter()
{
    return mProxyMap.First();
}

AFINetServerModule* AFCWorldNetServerModule::GetNetModule()
{
    return m_pNetModule;
}

int AFCWorldNetServerModule::GetPlayerGameID(const AFGUID self)
{
    //to do
    return -1;
}