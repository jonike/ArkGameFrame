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

#include "AFCLoginNetServerModule.h"
#include "AFLoginNetServerPlugin.h"

const std::string PROPERTY_ACCOUNT = "Account";
const std::string PROPERTY_VERIFIED = "Verified";

bool AFCLoginNetServerModule::Init()
{
    m_pNetModule = ARK_NEW AFINetServerModule(pPluginManager);
    return true;
}

bool AFCLoginNetServerModule::Shut()
{
    return true;
}

bool AFCLoginNetServerModule::BeforeShut()
{
    return true;
}

bool AFCLoginNetServerModule::AfterInit()
{
    m_pKernelModule = pPluginManager->FindModule<AFIKernelModule>();
    m_pLoginLogicModule = pPluginManager->FindModule<AFILoginLogicModule>();
    m_pLogModule = pPluginManager->FindModule<AFILogModule>();
    m_pClassModule = pPluginManager->FindModule<AFIClassModule>();
    m_pElementModule = pPluginManager->FindModule<AFIElementModule>();
    m_pLoginToMasterModule = pPluginManager->FindModule<AFILoginToMasterModule>();
    m_pUUIDModule = pPluginManager->FindModule<AFIGUIDModule>();


    m_pNetModule->AddReceiveCallBack(AFMsg::EGMI_STS_HEART_BEAT, this, &AFCLoginNetServerModule::OnHeartBeat);
    m_pNetModule->AddReceiveCallBack(AFMsg::EGMI_REQ_LOGIN, this, &AFCLoginNetServerModule::OnLoginProcess);
    m_pNetModule->AddReceiveCallBack(AFMsg::EGMI_REQ_LOGOUT, this, &AFCLoginNetServerModule::OnLogOut);
    m_pNetModule->AddReceiveCallBack(AFMsg::EGMI_REQ_CONNECT_WORLD, this, &AFCLoginNetServerModule::OnSelectWorldProcess);
    m_pNetModule->AddReceiveCallBack(AFMsg::EGMI_REQ_WORLD_LIST, this, &AFCLoginNetServerModule::OnViewWorldProcess);
    m_pNetModule->AddReceiveCallBack(this, &AFCLoginNetServerModule::InvalidMessage);

    m_pNetModule->AddEventCallBack(this, &AFCLoginNetServerModule::OnSocketClientEvent);

    ARK_SHARE_PTR<AFIClass> xLogicClass = m_pClassModule->GetElement("Server");
    if(nullptr != xLogicClass)
    {
        NFList<std::string>& xNameList = xLogicClass->GetConfigNameList();
        std::string strConfigName;
        for(bool bRet = xNameList.First(strConfigName); bRet; bRet = xNameList.Next(strConfigName))
        {
            const int nServerType = m_pElementModule->GetPropertyInt(strConfigName, "Type");
            const int nServerID = m_pElementModule->GetPropertyInt(strConfigName, "ServerID");
            if(nServerType == NF_SERVER_TYPES::NF_ST_LOGIN && pPluginManager->AppID() == nServerID)
            {
                const int nPort = m_pElementModule->GetPropertyInt(strConfigName, "Port");
                const int nMaxConnect = m_pElementModule->GetPropertyInt(strConfigName, "MaxOnline");
                const int nCpus = m_pElementModule->GetPropertyInt(strConfigName, "CpuCount");
                const std::string strIP(m_pElementModule->GetPropertyString(strConfigName, "IP"));

                m_pUUIDModule->SetWorkerAndDatacenter(nServerID, nServerID);

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

int AFCLoginNetServerModule::OnSelectWorldResultsProcess(const int nWorldID, const AFGUID xSenderID, const int nLoginID, const std::string& strAccount, const std::string& strWorldIP, const int nWorldPort, const std::string& strWorldKey)
{
    ARK_SHARE_PTR<SessionData> pSessionData = mmClientSessionData.GetElement(xSenderID);
    if(pSessionData)
    {
        AFMsg::AckConnectWorldResult xMsg;
        xMsg.set_world_id(nWorldID);
        xMsg.mutable_sender()->CopyFrom(AFINetServerModule::NFToPB(xSenderID));
        xMsg.set_login_id(nLoginID);
        xMsg.set_account(strAccount);
        xMsg.set_world_ip(strWorldIP);
        xMsg.set_world_port(nWorldPort);
        xMsg.set_world_key(strWorldKey);

        m_pNetModule->SendMsgPB(AFMsg::EGameMsgID::EGMI_ACK_CONNECT_WORLD, xMsg, pSessionData->mnClientID, xSenderID);
    }

    return 0;
}

bool AFCLoginNetServerModule::Execute()
{
    return m_pNetModule->Execute();
}

void AFCLoginNetServerModule::OnClientConnected(const AFGUID& xClientID)
{
    ARK_SHARE_PTR<SessionData> pSessionData(ARK_NEW SessionData());

    pSessionData->mnClientID = xClientID;
    mmClientSessionData.AddElement(xClientID, pSessionData);
}

void AFCLoginNetServerModule::OnClientDisconnect(const AFGUID& xClientID)
{
    mmClientSessionData.RemoveElement(xClientID);
}

void AFCLoginNetServerModule::OnLoginProcess(const AFIMsgHead& xHead, const int nMsgID, const char* msg, const uint32_t nLen, const AFGUID& xClientID)
{
    AFGUID nPlayerID;
    AFMsg::ReqAccountLogin xMsg;
    if(!m_pNetModule->ReceivePB(xHead, nMsgID, msg, nLen, xMsg, nPlayerID))
    {
        return;
    }

    ARK_SHARE_PTR<SessionData> pSession = mmClientSessionData.GetElement(xClientID);
    if(pSession)
    {
        //还没有登录过
        if(pSession->mnLogicState == 0)
        {
            int nState = m_pLoginLogicModule->OnLoginProcess(pSession->mnClientID, xMsg.account(), xMsg.password());
            if(0 != nState)
            {
                std::ostringstream strLog;
                strLog << "Check password failed, Account = " << xMsg.account() << " Password = " << xMsg.password();
                m_pLogModule->LogError(xClientID, strLog, __FUNCTION__, __LINE__);

                AFMsg::AckEventResult xMsg;
                xMsg.set_event_code(AFMsg::EGEC_ACCOUNTPWD_INVALID);

                m_pNetModule->SendMsgPB(AFMsg::EGameMsgID::EGMI_ACK_LOGIN, xMsg, xClientID, nPlayerID);
                return;
            }

            pSession->mnLogicState = 1;
            pSession->mstrAccout = xMsg.account();

            AFMsg::AckEventResult xData;
            xData.set_event_code(AFMsg::EGEC_ACCOUNT_SUCCESS);

            m_pNetModule->SendMsgPB(AFMsg::EGameMsgID::EGMI_ACK_LOGIN, xData, xClientID, nPlayerID);

            m_pLogModule->LogInfo(xClientID, "Login successed :", xMsg.account().c_str());
        }
    }
}

void AFCLoginNetServerModule::OnSelectWorldProcess(const AFIMsgHead& xHead, const int nMsgID, const char* msg, const uint32_t nLen, const AFGUID& xClientID)
{
    AFGUID nPlayerID;
    AFMsg::ReqConnectWorld xMsg;
    if(!m_pNetModule->ReceivePB(xHead, nMsgID, msg, nLen, xMsg, nPlayerID))
    {
        return;
    }

    ARK_SHARE_PTR<SessionData> pSession = mmClientSessionData.GetElement(xClientID);
    if(!pSession)
    {
        return;
    }

    //没登录过
    if(pSession->mnLogicState <= 0)
    {
        return;
    }

    AFMsg::ReqConnectWorld xData;
    xData.set_world_id(xMsg.world_id());
    xData.set_login_id(pPluginManager->AppID());
    xData.mutable_sender()->CopyFrom(AFINetServerModule::NFToPB(pSession->mnClientID));
    xData.set_account(pSession->mstrAccout);

    m_pLoginToMasterModule->GetClusterModule()->SendSuitByPB(pSession->mstrAccout, AFMsg::EGameMsgID::EGMI_REQ_CONNECT_WORLD, xData, xHead.GetPlayerID());//here has a problem to be solve
}

void AFCLoginNetServerModule::OnSocketClientEvent(const NetEventType eEvent, const AFGUID& xClientID, const int nServerID)
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

void AFCLoginNetServerModule::SynWorldToClient(const AFGUID& xClientID)
{
    AFMsg::AckServerList xData;
    xData.set_type(AFMsg::RSLT_WORLD_SERVER);

    AFMapEx<int, AFMsg::ServerInfoReport>& xWorldMap = m_pLoginToMasterModule->GetWorldMap();
    AFMsg::ServerInfoReport* pWorldData = xWorldMap.FirstNude();
    while(pWorldData)
    {
        AFMsg::ServerInfo* pServerInfo = xData.add_info();

        pServerInfo->set_name(pWorldData->server_name());
        pServerInfo->set_status(pWorldData->server_state());
        pServerInfo->set_server_id(pWorldData->server_id());
        pServerInfo->set_wait_count(0);

        pWorldData = xWorldMap.NextNude();
    }


    m_pNetModule->SendMsgPB(AFMsg::EGameMsgID::EGMI_ACK_WORLD_LIST, xData, xClientID, AFGUID(0));
}

void AFCLoginNetServerModule::OnViewWorldProcess(const AFIMsgHead& xHead, const int nMsgID, const char* msg, const uint32_t nLen, const AFGUID& xClientID)
{
    AFGUID nPlayerID;
    AFMsg::ReqServerList xMsg;
    if(!m_pNetModule->ReceivePB(xHead, nMsgID, msg, nLen, xMsg, nPlayerID))
    {
        return;
    }

    if(xMsg.type() == AFMsg::RSLT_WORLD_SERVER)
    {
        SynWorldToClient(xClientID);
    }
}

void AFCLoginNetServerModule::OnHeartBeat(const AFIMsgHead& xHead, const int nMsgID, const char * msg, const uint32_t nLen, const AFGUID& xClientID)
{
}

void AFCLoginNetServerModule::OnLogOut(const AFIMsgHead& xHead, const int nMsgID, const char * msg, const uint32_t nLen, const AFGUID& xClientID)
{
}

void AFCLoginNetServerModule::InvalidMessage(const AFIMsgHead& xHead, const int nMsgID, const char * msg, const uint32_t nLen, const AFGUID& xClientID)
{
    printf("Net || 非法消息:unMsgID=%d\n", nMsgID);
}



