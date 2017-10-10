/*****************************************************************************
// * This source file is part of ArkGameFrame                                *
// * For the latest info, see https://github.com/ArkGame                     *
// *                                                                         *
// * Copyright(c) 2013 - 2017 ArkGame authors.                               *
// *                                                                         *
// * Licensed under the Apache License, Version 2.0 (the "License");         *
// * you may not use this file except in compliance with the License.        *
// * You may obtain a copy of the License at                                 *
// *                                                                         *
// *     http://www.apache.org/licenses/LICENSE-2.0                          *
// *                                                                         *
// * Unless required by applicable law or agreed to in writing, software     *
// * distributed under the License is distributed on an "AS IS" BASIS,       *
// * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.*
// * See the License for the specific language governing permissions and     *
// * limitations under the License.                                          *
// *                                                                         *
// *                                                                         *
// * @file      AFINetServerModule.h                                                *
// * @author    Ark Game Tech                                                *
// * @date      2015-12-15                                                   *
// * @brief     AFINetServerModule                                                  *
*****************************************************************************/
#ifndef AFI_NET_SERVER_MODULE_H
#define AFI_NET_SERVER_MODULE_H

#include "SDK/Base/AFGUID.h"
#include "AFIModule.h"
#include "AFIPluginManager.h"
#include "SDK/Net/AFCNetServer.h"
#include "SDK/Base/AFQueue.h"
#include "SDK/Proto/AFMsgDefine.h"
#include "SDK/Proto/AFDefine.pb.h"
#include "SDK/Interface/AFINetModule.h"

////////////////////////////////////////////////////////////////////////////

#define CLIENT_MSG_PROCESS( nMsgID, msgData, nLen, msg)                 \
    AFGUID nPlayerID;                                \
    msg xMsg;                                           \
    if (!AFINetServerModule::ReceivePB(xHead,nMsgID, msgData, nLen, xMsg, nPlayerID))   \
    {                                                   \
        m_pLogModule->LogError(AFGUID(), "", "Parse msg error", __FUNCTION__, __LINE__); \
        return;                                         \
    }                                                   \
    \
    ARK_SHARE_PTR<AFIObject> pObject = m_pKernelModule->GetObject(nPlayerID); \
    if ( NULL == pObject.get() )                        \
    {                                                   \
        m_pLogModule->LogError(nPlayerID, "FromClient Object do not Exist", "", __FUNCTION__, __LINE__); \
        return;                                         \
    }

#define CLIENT_MSG_PROCESS_NO_OBJECT( nMsgID, msgData, nLen, msg)                 \
    AFGUID nPlayerID;                                \
    msg xMsg;                                           \
    if (!AFINetServerModule::ReceivePB(xHead,nMsgID, msgData, nLen, xMsg, nPlayerID))             \
    {                                                   \
        m_pLogModule->LogError(AFGUID(), "", "Parse msg error", __FUNCTION__, __LINE__); \
        return;                                         \
    }

static AFGUID PBToNF(AFMsg::Ident xID)
{
    AFGUID  xIdent;
    xIdent.nIdent = xID.svrid();
    xIdent.nSerial = xID.index();

    return xIdent;
}

static AFMsg::Ident NFToPB(AFGUID xID)
{
    AFMsg::Ident  xIdent;
    xIdent.set_svrid(xID.nIdent);
    xIdent.set_index(xID.nSerial);

    return xIdent;
}

//////////////////////////////////////////////////////////////////////////
struct ServerData
{
    ServerData()
    {
        pData = ARK_SHARE_PTR<AFMsg::ServerInfoReport>(ARK_NEW AFMsg::ServerInfoReport());
    }
    ~ServerData()
    {
        pData = NULL;
    }

    AFGUID xClient;
    ARK_SHARE_PTR<AFMsg::ServerInfoReport> pData;
};

struct SessionData
{
    SessionData()
    {
        mnLogicState = 0;
        mnGameID = 0;
    }

    int32_t mnLogicState;
    int32_t mnGameID;
    AFGUID mnUserID;
    AFGUID mnClientID;
    AFGUID mnHashIdentID;
    std::string mstrAccout;
};

class AFINetServerModule : public AFINetModule
{
protected:
    AFINetServerModule()
    {
        //important to init the value of pPluginManager
        //pPluginManager
        nLastTime = GetPluginManager()->GetNowTime();
        m_pNet = NULL;
    }
public:
    template<typename BaseType>
    AFINetServerModule(AFIPluginManager* p, BaseType* pBase, void (BaseType::*handleRecieve)(const int, const int, const char*, const uint32_t, const AFGUID))
    {
        pPluginManager = p;
        nLastTime = GetPluginManager()->GetNowTime();
        m_pNet = NULL;
    }

    AFINetServerModule(AFIPluginManager* p)
    {
        pPluginManager = p;
        nLastTime = GetPluginManager()->GetNowTime();
        m_pNet = NULL;
    }

    virtual ~AFINetServerModule()
    {
        if(m_pNet)
        {
            m_pNet->Final();
        }

        delete m_pNet;
        m_pNet = NULL;
    }

    //as server
    int Initialization(const unsigned int nMaxClient, const std::string strIP, const unsigned short nPort, const int nServerID, const int nCpuCount)
    {
        std::string strIPAndPort;
        std::string strPort;
        Ark_to_str(strPort, nPort);
        strIPAndPort = strIP + ":" + strPort;
        m_pNet = ARK_NEW AFCNetServer(this, &AFINetServerModule::OnReceiveNetPack, &AFINetServerModule::OnSocketNetEvent);
        return m_pNet->Initialization(nMaxClient, strIPAndPort, nServerID, nCpuCount);
    }

    virtual bool Execute()
    {
        if(!m_pNet)
        {
            return false;
        }

        //���ϴε����ݴ�����
        KeepAlive();

        return m_pNet->Execute();
    }

    bool SendMsgToAllClientWithOutHead(const int nMsgID, const std::string& msg, const AFGUID& nPlayerID)
    {
        return m_pNet->SendMsgToAllClient(msg.c_str(), msg.length(), nPlayerID);
    }

    bool SendMsgPBToAllClient(const uint16_t nMsgID, const google::protobuf::Message& xData, const AFGUID& nPlayerID)
    {
        std::string strMsg;
        if(!xData.SerializeToString(&strMsg))
        {
            char szData[MAX_PATH] = { 0 };
            ARK_SPRINTF(szData, MAX_PATH, "Send Message to all Failed For Serialize of MsgData, MessageID: %d\n", nMsgID);

            return false;
        }

        return SendMsgToAllClientWithOutHead(nMsgID, strMsg, nPlayerID);
    }

    bool SendMsgPB(const uint16_t nMsgID, const google::protobuf::Message& xData, const AFGUID& xClientID, const AFGUID nPlayer, const std::vector<AFGUID>* pClientIDList = NULL)
    {
        std::string xMsgData;
        if(!xData.SerializeToString(&xMsgData))
        {
            char szData[MAX_PATH] = { 0 };
            ARK_SPRINTF(szData, MAX_PATH, "Send Message to %d Failed For Serialize of MsgData, MessageID: %d\n", xClientID.ToString(), nMsgID);

            return false;
        }

        return SendMsgPB(nMsgID, xMsgData, xClientID, nPlayer, pClientIDList);
    }

    bool SendMsgPB(const uint16_t nMsgID, const std::string& strData, const AFGUID& xClientID, const AFGUID nPlayer, const std::vector<AFGUID>* pClientIDList = NULL)
    {
        if(!m_pNet)
        {
            char szData[MAX_PATH] = { 0 };
            ARK_SPRINTF(szData, MAX_PATH, "Send Message to %d Failed For NULL Of Net, MessageID: %d\n", xClientID.ToString(), nMsgID);
            //LogSend(szData);

            return false;
        }

        if(pClientIDList)
        {
            //playerid��Ҫ������ת����Ϣ��ʱ����ʶ��ʹ�ã�����ʹ�ò�ʹ��
            AFMsg::BrocastMsg xMsg;
            AFMsg::Ident* pPlayerID = xMsg.mutable_player_id();
            *pPlayerID = NFToPB(nPlayer);
            xMsg.set_msg_data(strData.data(), strData.length());
            xMsg.set_nmsgid(nMsgID);

            for(int i = 0; i < pClientIDList->size(); ++i)
            {
                const AFGUID& ClientID = (*pClientIDList)[i];

                AFMsg::Ident* pData = xMsg.add_player_client_list();
                if(pData)
                {
                    *pData = NFToPB(ClientID);
                }
            }

            std::string strMsg;
            if(!xMsg.SerializeToString(&strMsg))
            {
                return false;
            }

            return m_pNet->SendMsgWithOutHead(AFMsg::EGMI_GTG_BROCASTMSG, strMsg.data(), strMsg.size(), xClientID, nPlayer);
        }
        else
        {
            return m_pNet->SendMsgWithOutHead(nMsgID, strData.data(), strData.size(), xClientID, nPlayer);
        }
    }

    AFINet* GetNet()
    {
        return m_pNet;
    }

protected:
    void OnReceiveNetPack(const AFIMsgHead& xHead, const int nMsgID, const char* msg, const uint32_t nLen, const AFGUID& xClientID)
    {
        OnReceiveBaseNetPack(xHead, nMsgID, msg, nLen, xClientID);
    }

    void OnSocketNetEvent(const NetEventType eEvent, const AFGUID& xClientID, int nServerID)
    {
        OnSocketBaseNetEvent(eEvent, xClientID, nServerID);
    }

    void KeepAlive()
    {
        if(!m_pNet)
        {
            return;
        }

        if(m_pNet->IsServer())
        {
            return;
        }

        if(nLastTime + 10 > GetPluginManager()->GetNowTime())
        {
            return;
        }

        nLastTime = GetPluginManager()->GetNowTime();

        AFMsg::ServerHeartBeat xMsg;
        xMsg.set_count(0);

        SendMsgPB(AFMsg::EGameMsgID::EGMI_STS_HEART_BEAT, xMsg, AFGUID(0), AFGUID(0));
    }

private:

    AFINet* m_pNet;
    int64_t nLastTime;
};

#endif


