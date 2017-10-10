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

#include <thread>
#include <atomic>
#include "AFINet.h"
#include "SDK/Base/AFQueue.h"
#include "SDK/Base/AFLockFreeQueue.h"
#include "SDK/Base/AFRWLock.hpp"
#include <evpp/libevent.h>
#include <evpp/event_watcher.h>
#include <evpp/event_loop.h>
#include <evpp/event_loop_thread.h>
#include <evpp/tcp_server.h>
#include <evpp/buffer.h>
#include <evpp/tcp_conn.h>
#include <evpp/tcp_client.h>

#pragma pack(push, 1)

class AFCNetServer : public AFINet
{
public:
    AFCNetServer()
        : mnMaxConnect(0)
        , mnCpuCount(0)
        , mnServerID(0)
    {
        bWorking = false;
    }

    template<typename BaseType>
    AFCNetServer(BaseType* pBaseType, void (BaseType::*handleRecieve)(const AFIMsgHead& xHead, const int, const char*, const uint32_t, const AFGUID&), void (BaseType::*handleEvent)(const NetEventType, const AFGUID&, const int))
        : mnMaxConnect(0)
        , mnCpuCount(0)
        , mnServerID(0)
    {
        mRecvCB = std::bind(handleRecieve, pBaseType, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5);
        mEventCB = std::bind(handleEvent, pBaseType, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
        bWorking = false;
    }

    virtual ~AFCNetServer()
    {
        Final();
    };

public:
    virtual bool Execute();

    virtual int Initialization(const unsigned int nMaxClient, const std::string& strAddrPort, const int nServerID, const int nCpuCount);
    virtual bool Final();
    virtual bool IsServer()
    {
        return true;
    }

    virtual bool SendMsgWithOutHead(const int16_t nMsgID, const char* msg, const uint32_t nLen, const AFGUID& xClientID, const AFGUID& xPlayerID);
    virtual bool SendMsgToAllClientWithOutHead(const int16_t nMsgID, const char* msg, const uint32_t nLen, const AFGUID& xPlayerID);

    virtual bool CloseNetObject(const AFGUID& xClientID);
    virtual bool Log(int severity, const char* msg)
    {
        return true;
    };

public:
    //From Worker Thread
    static void OnMessage(const evpp::TCPConnPtr& conn,
                          evpp::Buffer* msg, void* pData);
    void OnMessageInner(const evpp::TCPConnPtr& conn,
                        evpp::Buffer* msg);

    //From ListenThread
    static void OnClientConnection(const evpp::TCPConnPtr& conn, void* pData);
    void OnClientConnectionInner(const evpp::TCPConnPtr& conn);

private:
    bool SendMsgToAllClient(const char* msg, const uint32_t nLen);
    bool SendMsg(const char* msg, const uint32_t nLen, const AFGUID& xClient);
    bool AddNetObject(const AFGUID& xClientID, NetObject* pObject);
    bool RemoveNetObject(const AFGUID& xClientID);
    NetObject* GetNetObject(const AFGUID& xClientID);

private:
    void ProcessMsgLogicThread();
    void ProcessMsgLogicThread(NetObject* pObject);
    bool CloseSocketAll();
    bool DismantleNet(NetObject* pObject);

protected:
    int DeCode(const char* strData, const uint32_t unLen, AFCMsgHead& xHead);
    int EnCode(const AFCMsgHead& xHead, const char* strData, const uint32_t unDataLen, std::string& strOutData);

private:
    std::unique_ptr<evpp::TCPServer> m_pTcpSrv;
    std::unique_ptr<evpp::EventLoopThread> m_pListenThread;
    std::map<AFGUID, NetObject*> mmObject;
    AFCReaderWriterLock mRWLock;
    int mnMaxConnect;
    std::string mstrIPPort;
    int mnCpuCount;
    int mnServerID;

    NET_RECEIVE_FUNCTOR mRecvCB;
    NET_EVENT_FUNCTOR mEventCB;
};

#pragma pack(pop)


