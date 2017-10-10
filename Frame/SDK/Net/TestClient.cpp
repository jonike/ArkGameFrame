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
// * @file      TestClient.cpp                                              *
// * @author    Ark Game Tech                                                *
// * @date      2015-12-15                                                   *
// * @brief     TestClient                                                  *
*****************************************************************************/
#include "AFCNetClient.h"
#include <thread>
#include <string>
#include "SDK/Base/AFTime.hpp"
#include "SDK/Base/AFPlatform.hpp"
#pragma comment(lib,"ws2_32.lib")

#if ARK_RUN_MODE == ARK_RUN_MODE_DEBUG
#pragma comment(lib,"AFNet_d.lib")
#pragma comment(lib,"AFCore_d.lib")
#else
#pragma comment(lib,"AFNet.lib")
#pragma comment(lib,"AFCore.lib")
#endif
class TestClientClass
{
public:
    TestClientClass(const int nID)
    {
        //pNet = new AFCNet(this, &TestClientClass::ReciveHandler, &TestClientClass::EventHandler);
        pNet = new AFCNetClient(this, &TestClientClass::ReciveHandler, &TestClientClass::EventHandler);
        pNet->Initialization("192.168.1.143:8088", 1);
        pNet->StopAfter(60);
        bConnected = false;
        nSendMsgCount = 0;
        nReciveMsgCount = 0;
        mnID = nID;
        mnStarTime = AFCTimeBase::GetInstance().GetUTCTime();
        mbTestSendMsg = true;
    }

    void ReciveHandler(const AFIMsgHead& xHead, const int nMsgID, const char* msg, const uint32_t nLen, const AFGUID& xClientID)
    {
        nReciveMsgCount++;
        pNet->SendMsgWithOutHead(nMsgID, msg, nLen, xClientID, 0);
    };

    void EventHandler(const NetEventType e, const AFGUID& xClientID, const int nServerID)
    {
        std::cout << " event_id: " << e << " thread_id: " << std::this_thread::get_id() << std::endl;
        if(e == CONNECTED)
        {
            bConnected = true;

            TestSend();
        }
    }

    void Execute()
    {
        pNet->Execute();
    }

    bool TestSend()
    {
        if(!bConnected)
        {
            return false;
        }

        std::string strData = "1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890";

        const int nTestSize = 100 ;
        const int nBodySize = nTestSize - AFIMsgHead::AF_Head::NF_HEAD_LENGTH;
        char data[nBodySize] = {};

        memset(data, 22, nBodySize);
        pNet->SendMsgWithOutHead(1, data, nBodySize, 0, 0);
        nSendMsgCount++;

        return true;
    }

    bool IsStop()
    {
        return pNet->IsStop();
    }

public:
    bool mbTestSendMsg;

public:
    AFINet* pNet;
    bool bConnected;
    int nSendMsgCount;
    int nReciveMsgCount;
    int mnID;
    int mnStarTime;
private:
};

int main(int argc, char** argv)
{
    std::list<TestClientClass*> list;

    /*   for(int i = 0; i < 1; ++i)
       {
           TestClientClass* x = new TestClientClass(i);;
           list.push_back(x);
       }*/
    TestClientClass* pNet = new TestClientClass(0);;

    int nTime = AFCTimeBase::GetInstance().GetUTCTime();
    int nLastTime = AFCTimeBase::GetInstance().GetUTCTime();
    int nCount = 0;
    while(!pNet->IsStop())
    {
        pNet->Execute();
        ////int nNowTime = AFCTimeBase::GetInstance().GetUTCTime();
        //std::list<TestClientClass*>::iterator it = list.begin();
        //for(it; it != list.end(); ++it)
        //{
        //    //if ((nNowTime - nTime) < 180 * 1000)
        //    //{
        //    //    /*     {
        //    //             nCount++;
        //    //         }
        //    //         else
        //    //         {

        //    //         }*/
        //    //    //std::cout << "Frame Time :" << (nNowTime - nLastTime) << std::endl;
        //    //}

        //    (*it)->Execute();
        //}

        //nLastTime = nNowTime;
        //NFSLEEP(1);
    }

    std::cout << "nSendMsgCount " << pNet->nSendMsgCount << "nReciveMsgSize:" << pNet->pNet->nReceiverSize << "Count" << pNet->nReciveMsgCount << std::endl;
    system("pause");


    return 0;
}

