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

#include "AFIModule.h"
#include "AFINetServerModule.h"

class AFIGameNetServerModule
    : public AFIModule
{
public:
    //Ҫ����ǰ���еĶ������ڵ�actor,gateid,fd��
    struct GateBaseInfo
    {
        GateBaseInfo()
        {
            nActorID = 0;
            nGateID = 0;
        }

        GateBaseInfo(const int gateID, const AFGUID xIdent)
        {
            nActorID = 0;
            nGateID = gateID;
            xClientID = xIdent;
        }

        int nActorID;
        int nGateID;
        AFGUID xClientID;
    };

    struct GateServerInfo
    {
        ServerData xServerData;
        //�����������еĶ���<��ɫID,gate_FD>
        std::map<AFGUID, AFGUID> xRoleInfo;
    };

public:
    virtual AFINetServerModule* GetNetModule() = 0;
    virtual void SendMsgPBToGate(const uint16_t nMsgID, google::protobuf::Message& xMsg, const AFGUID& self) = 0;
    virtual void SendMsgPBToGate(const uint16_t nMsgID, const std::string& strMsg, const AFGUID& self) = 0;
    virtual bool AddPlayerGateInfo(const AFGUID& nRoleID, const AFGUID& nClientID, const int nGateID) = 0;
    virtual bool RemovePlayerGateInfo(const AFGUID& nRoleID) = 0;
    virtual ARK_SHARE_PTR<GateBaseInfo> GetPlayerGateInfo(const AFGUID& nRoleID) = 0;

    virtual int OnPropertyEnter(const AFIDataList& argVar, const AFGUID& self) = 0;
    virtual int OnRecordEnter(const AFIDataList& argVar, const AFGUID& self) = 0;

    virtual int OnObjectListEnter(const AFIDataList& self, const AFIDataList& argVar) = 0;
    virtual int OnObjectListLeave(const AFIDataList& self, const AFIDataList& argVar) = 0;
};