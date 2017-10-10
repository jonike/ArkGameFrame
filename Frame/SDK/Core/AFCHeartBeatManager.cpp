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

#include "SDK/Base/AFTime.hpp"
#include "AFCHeartBeatManager.h"


AFCHeartBeatManager::~AFCHeartBeatManager()
{
    mHeartBeatElementMapEx.Clear();
}

void AFCHeartBeatElement::DoHeartBeatEvent(int64_t nNowTime)
{
    nCount--;
    HEART_BEAT_FUNCTOR_PTR cb;
    bool bRet = First(cb);
    while(bRet)
    {
        (*cb)(self, strBeatName, nBeatTime, nCount);

        bRet = Next(cb);
    }

    if(nCount <= 0 && !bForever)
    {
        bStop = true;
    }
    else
    {
        nNextTriggerTime = nNowTime + nBeatTime;
    }
}

bool AFCHeartBeatElement::CheckTime(int64_t nNowTime)
{
    if(IsStop())
    {
        return false;
    }

    if(nNowTime < nNextTriggerTime)
    {
        return false;
    }

    if(nCount <= 0 && !bForever)
    {
        bStop = true;
        return false;
    }

    return true;
}

//////////////////////////////////////////////////////////////////////////
bool AFCHeartBeatManager::Execute()
{
    //millisecond
    int64_t nTime = AFCTimeBase::GetInstance().GetNowMillisecond();
    for(std::multimap<int64_t, AFCHeartBeatElement*>::iterator iter = mTimeList.begin(); iter != mTimeList.end();)
    {
        if(iter->second->IsStop())
        {
            AFCHeartBeatElement* pElement = mHeartBeatElementMapEx.GetElement(iter->second->strBeatName);
            if(NULL == pElement)
            {
                continue;
            }

            if(pElement->id == iter->second->id)
            {
                mHeartBeatElementMapEx.RemoveElement(iter->second->strBeatName);
            }

            iter = mTimeList.erase(iter);
            continue;
        }

        if(iter->second->CheckTime(nTime))
        {
            iter->second->DoHeartBeatEvent(nTime);

            if(iter->second->IsStop())
            {
                AFCHeartBeatElement* pElement = mHeartBeatElementMapEx.GetElement(iter->second->strBeatName);
                if(NULL == pElement)
                {
                    continue;
                }

                if(pElement->id == iter->second->id)
                {
                    mHeartBeatElementMapEx.RemoveElement(iter->second->strBeatName);
                }
            }
            else
            {
                mTimeList.insert(std::make_pair(iter->second->nNextTriggerTime, iter->second));
            }

            iter = mTimeList.erase(iter);
            continue;
        }

        break;
    }

    //ɾ�����й�ʱ����
    std::string strHeartBeatName;
    bool bRet = mRemoveListEx.First(strHeartBeatName);
    while(bRet)
    {
        AFCHeartBeatElement* pHeartBeatEx = mHeartBeatElementMapEx.GetElement(strHeartBeatName);
        if(NULL == pHeartBeatEx)
        {
            continue;
        }

        typedef std::pair<std::multimap<int64_t, AFCHeartBeatElement*>::iterator, std::multimap<int64_t, AFCHeartBeatElement*>::iterator > Range;
        Range xRange = mTimeList.equal_range(pHeartBeatEx->nNextTriggerTime);

        for(std::multimap<int64_t, AFCHeartBeatElement*>::iterator iter = xRange.first; iter != xRange.second; ++iter)
        {
            if(iter->second->id == pHeartBeatEx->id)
            {
                iter = mTimeList.erase(iter);
            }
            else
            {
                ++iter;
            }
        }

        if(!RemoveHeartBeat(strHeartBeatName))
        {
            // add log
        }

        bRet = mRemoveListEx.Next(strHeartBeatName);
    }

    mRemoveListEx.ClearAll();

    //////////////////////////////////////////////////////////////////////////
    //���������Ҳ����ʱ��ӵ�
    for(std::list<AFCHeartBeatElement>::iterator iter = mAddListEx.begin(); iter != mAddListEx.end(); ++iter)
    {
        if(NULL == mHeartBeatElementMapEx.GetElement(iter->strBeatName))
        {
            AFCHeartBeatElement* pHeartBeatEx = ARK_NEW AFCHeartBeatElement();
            *pHeartBeatEx = *iter;
            mHeartBeatElementMapEx.AddElement(pHeartBeatEx->strBeatName, pHeartBeatEx);
            mTimeList.insert(std::make_pair(pHeartBeatEx->nNextTriggerTime, pHeartBeatEx));
        }
    }

    mAddListEx.clear();

    return true;
}

bool AFCHeartBeatManager::RemoveHeartBeat(const std::string& strHeartBeatName)
{
    return mRemoveListEx.Add(strHeartBeatName);
}

AFGUID AFCHeartBeatManager::Self()
{
    return mSelf;
}

//////////////////////////////////////////////////////////////////////////
bool AFCHeartBeatManager::AddHeartBeat(const AFGUID self, const std::string& strHeartBeatName, const HEART_BEAT_FUNCTOR_PTR& cb, const int64_t nTime, const int nCount, const bool bForever /*= false*/)
{
    AFCHeartBeatElement xHeartBeat;
    xHeartBeat.nNextTriggerTime = AFCTimeBase::GetInstance().GetNowMillisecond() + nTime;
    xHeartBeat.nBeatTime = nTime;
    xHeartBeat.nCount = nCount;
    xHeartBeat.self = self;
    xHeartBeat.bForever = bForever;
    xHeartBeat.strBeatName = strHeartBeatName;
    xHeartBeat.id = ++mTimerIDIndex.n64Value;
    xHeartBeat.Add(cb);
    mAddListEx.push_back(xHeartBeat);

    return true;
}

bool AFCHeartBeatManager::Exist(const std::string& strHeartBeatName)
{
    if(mHeartBeatElementMapEx.GetElement(strHeartBeatName))
    {
        return true;
    }

    return false;
}

