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

#include "AFCPropertyModule.h"
#include "SDK/Base/AFTime.hpp"

bool AFCPropertyModule::Init()
{
    return true;
}

bool AFCPropertyModule::Shut()
{
    return true;
}

bool AFCPropertyModule::Execute()
{
    return true;
}

bool AFCPropertyModule::AfterInit()
{
    m_pKernelModule = pPluginManager->FindModule<AFIKernelModule>();
    m_pElementModule = pPluginManager->FindModule<AFIElementModule>();
    m_pClassModule = pPluginManager->FindModule<AFIClassModule>();
    m_pPropertyConfigModule = pPluginManager->FindModule<AFIPropertyConfigModule>();
    m_pLevelModule = pPluginManager->FindModule<AFILevelModule>();

    m_pKernelModule->AddClassCallBack(NFrame::Player::ThisName(), this, &AFCPropertyModule::OnObjectClassEvent);

    return true;
}

int AFCPropertyModule::GetPropertyValue(const AFGUID& self, const std::string& strPropertyName, const NFPropertyGroup eGroupType)
{
    //TODO:Ҫ��һ�� attr_name->col��ӳ��

    //if(NFPropertyGroup::NPG_ALL != eGroupType)
    //{
    //    return m_pKernelModule->GetRecordInt(self, NFrame::Player::R_CommPropertyValue(), eGroupType, strPropertyName);
    //}

    //return m_pKernelModule->GetPropertyInt(self, strPropertyName);

    return 0;
}

int AFCPropertyModule::SetPropertyValue(const AFGUID& self, const std::string& strPropertyName, const NFPropertyGroup eGroupType, const int nValue)
{
    //TODO:Ҫ��һ�� attr_name->col��ӳ��

    //if(NFPropertyGroup::NPG_ALL != eGroupType)
    //{
    //    ARK_SHARE_PTR<AFIObject> pObject = m_pKernelModule->GetObject(self);
    //    if(nullptr != pObject)
    //    {
    //       AFRecord* pRecord = m_pKernelModule->FindRecord(self, NFrame::Player::R_CommPropertyValue());
    //        if(NULL != pRecord)
    //        {
    //            //pRecord->SetUsed(eGroupType, true);
    //            return pRecord->SetInt(eGroupType, strPropertyName, nValue);
    //        }
    //    }

    //    //return m_pKernelModule->SetRecordInt( self, mstrCommPropertyName, eGroupType, *pTableCol, nValue );
    //}

    ////��̬����û�У������õ�����ֵ
    //m_pKernelModule->SetPropertyInt(self, strPropertyName, nValue);

    return 0;
}


int AFCPropertyModule::AddPropertyValue(const AFGUID& self, const std::string& strPropertyName, const NFPropertyGroup eGroupType, const int nValue)
{
    //if(NFPropertyGroup::NPG_ALL != eGroupType)
    //{
    //    ARK_SHARE_PTR<AFIObject> pObject = m_pKernelModule->GetObject(self);
    //    if(nullptr != pObject)
    //    {
    //        ARK_SHARE_PTR<AFIRecord> pRecord = m_pKernelModule->FindRecord(self, NFrame::Player::R_CommPropertyValue());
    //        if(nullptr != pRecord)
    //        {
    //            pRecord->SetUsed(eGroupType, true);
    //            int nPropertyValue = pRecord->GetInt(eGroupType, strPropertyName);

    //            return pRecord->SetInt(eGroupType, strPropertyName, nPropertyValue + nValue);
    //        }
    //    }
    //}

    return 0;
}

int AFCPropertyModule::SubPropertyValue(const AFGUID& self, const std::string& strPropertyName, const NFPropertyGroup eGroupType, const int nValue)
{
    //if(NFPropertyGroup::NPG_ALL != eGroupType)
    //{
    //    ARK_SHARE_PTR<AFIObject> pObject = m_pKernelModule->GetObject(self);
    //    if(nullptr != pObject)
    //    {
    //        ARK_SHARE_PTR<AFIRecord> pRecord = m_pKernelModule->FindRecord(self, NFrame::Player::R_CommPropertyValue());
    //        if(nullptr != pRecord)
    //        {
    //            pRecord->SetUsed(eGroupType, true);
    //            int nPropertyValue = pRecord->GetInt(eGroupType, strPropertyName);

    //            return pRecord->SetInt(eGroupType, strPropertyName, nPropertyValue - nValue);
    //        }
    //    }
    //}

    return 0;
}

int AFCPropertyModule::OnObjectLevelEvent(const AFGUID& self, const std::string& strPropertyName, const AFIData& oldVar, const AFIData& newVar)
{
    int nRet = RefreshBaseProperty(self);

    bool bRet = FullHPMP(self);
    bRet = FullSP(self);

    return 0;
}

int AFCPropertyModule::OnRecordPropertyEvent(const AFGUID& self, const RECORD_EVENT_DATA& xEventData, const AFIData& oldVar, const AFIData& newVar)
{
    ////������ֵ
    //const std::string& strRecordName = xEventData.strRecordName;
    //const int nOpType = xEventData.nOpType;
    //const int nRow = xEventData.nRow;
    //const int nCol = xEventData.nCol;

    //int nAllValue = 0;
    //ARK_SHARE_PTR<AFIRecord> pRecord = m_pKernelModule->FindRecord(self, NFrame::Player::R_CommPropertyValue());
    //for(int i = 0; i < (int)(NFPropertyGroup::NPG_ALL); i++)
    //{
    //    if(i < pRecord->GetRows())
    //    {
    //        int nValue = pRecord->GetInt(i, nCol);
    //        nAllValue += nValue;
    //    }
    //}

    //m_pKernelModule->SetPropertyInt(self, pRecord->GetColTag(nCol), nAllValue);

    return 0;
}

int AFCPropertyModule::OnObjectClassEvent(const AFGUID& self, const std::string& strClassName, const CLASS_OBJECT_EVENT eClassEvent, const AFIDataList& var)
{
    //if(strClassName == NFrame::Player::ThisName())
    //{
    //    if(CLASS_OBJECT_EVENT::COE_CREATE_NODATA == eClassEvent)
    //    {
    //        ARK_SHARE_PTR<AFIRecord> pRecord = m_pKernelModule->FindRecord(self, NFrame::Player::R_CommPropertyValue());
    //        if(nullptr != pRecord)
    //        {
    //            for(int i = 0; i < NPG_ALL; i++)
    //            {
    //                pRecord->AddRow(-1);
    //            }
    //        }

    //        m_pKernelModule->AddPropertyCallBack(self, NFrame::Player::Level(), this, &AFCPropertyModule::OnObjectLevelEvent);

    //        // TODO:һ�����Իص�
    //        m_pKernelModule->AddRecordCallBack(self, NFrame::Player::R_CommPropertyValue(), this, &AFCPropertyModule::OnRecordPropertyEvent);
    //    }
    //    else if(CLASS_OBJECT_EVENT::COE_CREATE_EFFECTDATA == eClassEvent)
    //    {
    //        int nOnlineCount = m_pKernelModule->GetPropertyInt(self, NFrame::Player::OnlineCount());
    //        if(nOnlineCount <= 0 && m_pKernelModule->GetPropertyInt(self, NFrame::Player::SceneID()) > 0)
    //        {
    //            //��һ�γ��������û�������
    //            m_pKernelModule->SetPropertyInt(self, NFrame::Player::Level(), 1);
    //        }
    //    }
    //    else if(CLASS_OBJECT_EVENT::COE_CREATE_FINISH == eClassEvent)
    //    {
    //        int nOnlineCount = m_pKernelModule->GetPropertyInt(self, NFrame::Player::OnlineCount());
    //        m_pKernelModule->SetPropertyInt(self, NFrame::Player::OnlineCount(), (nOnlineCount + 1));

    //    }
    //}

    return 0;
}

int AFCPropertyModule::RefreshBaseProperty(const AFGUID& self)
{
    //ARK_SHARE_PTR<AFIRecord> pRecord = m_pKernelModule->FindRecord(self, NFrame::Player::R_CommPropertyValue());
    //if(nullptr == pRecord)
    //{
    //    return 1;
    //}

    ////��ʼ����+�ȼ�����(ְҵ����)
    //int eJobType = m_pKernelModule->GetPropertyInt(self, NFrame::Player::Job());
    //int nLevel = m_pKernelModule->GetPropertyInt(self, NFrame::Player::Level());

    //for(int i = 0; i < pRecord->GetCols(); ++i)
    //{
    //    const std::string& strColTag = pRecord->GetColTag(i);
    //    int nValue = m_pPropertyConfigModule->CalculateBaseValue(eJobType, nLevel, strColTag);
    //    SetPropertyValue(self, strColTag, NFPropertyGroup::NPG_JOBLEVEL, nValue);
    //}

    return 1;
}

bool AFCPropertyModule::FullHPMP(const AFGUID& self)
{
    int64_t nMaxHP = m_pKernelModule->GetPropertyInt(self, NFrame::Player::MAXHP());
    if(nMaxHP > 0)
    {
        m_pKernelModule->SetPropertyInt(self, NFrame::Player::HP(), nMaxHP);
    }

    int64_t nMaxMP = m_pKernelModule->GetPropertyInt(self, NFrame::Player::MAXMP());
    if(nMaxMP > 0)
    {
        m_pKernelModule->SetPropertyInt(self, NFrame::Player::MP(), nMaxMP);
    }

    return true;
}

bool AFCPropertyModule::AddHP(const AFGUID& self, const int64_t& nValue)
{
    if(nValue <= 0)
    {
        return false;
    }

    int64_t nCurValue = m_pKernelModule->GetPropertyInt(self, NFrame::Player::HP());
    int64_t nMaxValue = m_pKernelModule->GetPropertyInt(self, NFrame::Player::MAXHP());

    if(nCurValue > 0)
    {
        nCurValue += nValue;
        if(nCurValue > nMaxValue)
        {
            nCurValue = nMaxValue;
        }

        m_pKernelModule->SetPropertyInt(self, NFrame::Player::HP(), nCurValue);
    }

    return true;
}

bool AFCPropertyModule::EnoughHP(const AFGUID& self, const int64_t& nValue)
{
    int64_t nCurValue = m_pKernelModule->GetPropertyInt(self, NFrame::Player::HP());
    if((nCurValue > 0) && (nCurValue - nValue >= 0))
    {
        return true;
    }

    return false;
}

bool AFCPropertyModule::ConsumeHP(const AFGUID& self, const int64_t& nValue)
{
    int64_t nCurValue = m_pKernelModule->GetPropertyInt(self, NFrame::Player::HP());
    if((nCurValue > 0) && (nCurValue - nValue >= 0))
    {
        nCurValue -= nValue;
        m_pKernelModule->SetPropertyInt(self, NFrame::Player::HP(), nCurValue);

        return true;
    }

    return false;
}

bool AFCPropertyModule::AddMP(const AFGUID& self, const int64_t& nValue)
{
    if(nValue <= 0)
    {
        return false;
    }

    int64_t nCurValue = m_pKernelModule->GetPropertyInt(self, NFrame::Player::MP());
    int64_t nMaxValue = m_pKernelModule->GetPropertyInt(self, NFrame::Player::MAXMP());

    nCurValue += nValue;
    if(nCurValue > nMaxValue)
    {
        nCurValue = nMaxValue;
    }

    m_pKernelModule->SetPropertyInt(self, NFrame::Player::MP(), nCurValue);

    return true;
}

bool AFCPropertyModule::ConsumeMP(const AFGUID& self, const int64_t& nValue)
{
    int64_t nCurValue = m_pKernelModule->GetPropertyInt(self, NFrame::Player::MP());
    if((nCurValue > 0) && (nCurValue - nValue >= 0))
    {
        nCurValue -= nValue;
        m_pKernelModule->SetPropertyInt(self, NFrame::Player::MP(), nCurValue);

        return true;
    }

    return false;
}

bool AFCPropertyModule::EnoughMP(const AFGUID& self, const int64_t& nValue)
{
    int64_t nCurValue = m_pKernelModule->GetPropertyInt(self, NFrame::Player::MP());
    if((nCurValue > 0) && (nCurValue - nValue >= 0))
    {
        return true;
    }

    return false;
}

bool AFCPropertyModule::FullSP(const AFGUID& self)
{
    int64_t nMAXCSP = m_pKernelModule->GetPropertyInt(self, NFrame::Player::MAXSP());
    if(nMAXCSP > 0)
    {
        m_pKernelModule->SetPropertyInt(self, NFrame::Player::SP(), nMAXCSP);

        return true;
    }

    return false;
}

bool AFCPropertyModule::AddSP(const AFGUID& self, const int64_t& nValue)
{
    if(nValue <= 0)
    {
        return false;
    }

    int64_t nCurValue = m_pKernelModule->GetPropertyInt(self, NFrame::Player::SP());
    int64_t nMaxValue = m_pKernelModule->GetPropertyInt(self, NFrame::Player::MAXSP());

    nCurValue += nValue;
    if(nCurValue > nMaxValue)
    {
        nCurValue = nMaxValue;
    }

    m_pKernelModule->SetPropertyInt(self, NFrame::Player::SP(), nCurValue);

    return true;
}

bool AFCPropertyModule::ConsumeSP(const AFGUID& self, const int64_t& nValue)
{
    int64_t nCSP = m_pKernelModule->GetPropertyInt(self, NFrame::Player::SP());
    if((nCSP > 0) && (nCSP - nValue >= 0))
    {
        nCSP -= nValue;
        m_pKernelModule->SetPropertyInt(self, NFrame::Player::SP(), nCSP);

        return true;
    }

    return false;
}

bool AFCPropertyModule::EnoughSP(const AFGUID& self, const int64_t& nValue)
{
    int64_t nCurValue = m_pKernelModule->GetPropertyInt(self, NFrame::Player::SP());
    if((nCurValue > 0) && (nCurValue - nValue >= 0))
    {
        return true;
    }

    return false;
}

bool AFCPropertyModule::AddMoney(const AFGUID& self, const int64_t& nValue)
{
    if(nValue <= 0)
    {
        return false;
    }

    int64_t nCurValue = m_pKernelModule->GetPropertyInt(self, NFrame::Player::Gold());
    nCurValue += nValue;
    m_pKernelModule->SetPropertyInt(self, NFrame::Player::Gold(), nCurValue);

    return false;
}

bool AFCPropertyModule::ConsumeMoney(const AFGUID& self, const int64_t& nValue)
{
    if(nValue <= 0)
    {
        return false;
    }

    int64_t nCurValue = m_pKernelModule->GetPropertyInt(self, NFrame::Player::Gold());
    nCurValue -= nValue;
    if(nCurValue >= 0)
    {
        m_pKernelModule->SetPropertyInt(self, NFrame::Player::Gold(), nCurValue);

        return true;
    }

    return false;
}

bool AFCPropertyModule::EnoughMoney(const AFGUID& self, const int64_t& nValue)
{
    int64_t nCurValue = m_pKernelModule->GetPropertyInt(self, NFrame::Player::Gold());
    if((nCurValue > 0) && (nCurValue - nValue >= 0))
    {
        return true;
    }

    return false;
}

bool AFCPropertyModule::AddDiamond(const AFGUID& self, const int64_t& nValue)
{
    if(nValue <= 0)
    {
        return false;
    }

    int64_t nCurValue = m_pKernelModule->GetPropertyInt(self, NFrame::Player::Money());
    nCurValue += nValue;
    m_pKernelModule->SetPropertyInt(self, NFrame::Player::Money(), nCurValue);

    return false;
}

bool AFCPropertyModule::ConsumeDiamond(const AFGUID& self, const int64_t& nValue)
{
    if(nValue <= 0)
    {
        return false;
    }

    int64_t nCurValue = m_pKernelModule->GetPropertyInt(self, NFrame::Player::Money());
    nCurValue -= nValue;
    if(nCurValue >= 0)
    {
        m_pKernelModule->SetPropertyInt(self, NFrame::Player::Money(), nCurValue);

        return true;
    }

    return false;
}

bool AFCPropertyModule::EnoughDiamond(const AFGUID& self, const int64_t& nValue)
{
    int64_t nCurValue = m_pKernelModule->GetPropertyInt(self, NFrame::Player::Money());
    if((nCurValue > 0) && (nCurValue - nValue >= 0))
    {
        return true;
    }

    return false;
}
