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
// * @file  	AFCGUIDModule.cpp                                              *
// * @author    Ark Game Tech                                                *
// * @date      2015-12-15                                                   *
// * @brief     AFCGUIDModule                                                  *
*****************************************************************************/
#include "AFCGUIDModule.h"
#include <mutex>

namespace GUIDModule
{
    //https://github.com/nebula-im/snowflake4cxx
    /**
    * Twitter_Snowflake
    * SnowFlake�Ľṹ����(ÿ������-�ֿ�):
    * 0 - 0000000000 0000000000 0000000000 0000000000 0 - 00000 - 00000 - 000000000000
    * 1λ��ʶ������long����������Java���Ǵ����ŵģ����λ�Ƿ���λ��������0��������1������idһ�������������λ��0
    * 41λʱ���(���뼶)��ע�⣬41λʱ��ز��Ǵ洢��ǰʱ���ʱ��أ����Ǵ洢ʱ��صĲ�ֵ����ǰʱ��� - ��ʼʱ���)
    * �õ���ֵ��������ĵĿ�ʼʱ��أ�һ�������ǵ�id��������ʼʹ�õ�ʱ�䣬�����ǳ�����ָ���ģ������������IdWorker���startTime���ԣ���41λ��ʱ��أ�����ʹ��69�꣬��T = (1L << 41) / (1000L * 60 * 60 * 24 * 365) = 69
    * 10λ�����ݻ���λ�����Բ�����1024���ڵ㣬����5λdatacenterId��5λworkerId
    * 12λ���У������ڵļ�����12λ�ļ���˳���֧��ÿ���ڵ�ÿ����(ͬһ������ͬһʱ���)����4096��ID���
    * �������պ�64λ��
    * SnowFlake���ŵ��ǣ������ϰ���ʱ���������򣬲��������ֲ�ʽϵͳ�ڲ������ID��ײ(����������ID�ͻ���ID������)������Ч�ʽϸߡ�
    */

#ifndef _MSC_VER
# include <sys/time.h>
#else
# include <windows.h>
# include <winsock2.h>
# include <time.h>
#endif

#ifdef _MSC_VER
    int gettimeofday(struct timeval* tp, void *tzp)
    {
        time_t clock;
        struct tm tm;
        SYSTEMTIME wtm;
        GetLocalTime(&wtm);
        tm.tm_year = wtm.wYear - 1900;
        tm.tm_mon = wtm.wMonth - 1;
        tm.tm_mday = wtm.wDay;
        tm.tm_hour = wtm.wHour;
        tm.tm_min = wtm.wMinute;
        tm.tm_sec = wtm.wSecond;
        tm.tm_isdst = -1;
        clock = mktime(&tm);
        tp->tv_sec = clock;
        tp->tv_usec = wtm.wMilliseconds * 1000;
        return (0);
    }
#endif

    uint64_t GetNowInMsec()
    {
        struct timeval tv;
        gettimeofday(&tv, 0);
        return uint64_t(tv.tv_sec) * 1000 + tv.tv_usec / 1000;
    }

    uint64_t WaitUntilNextMillis(uint64_t last_timestamp)
    {
        uint64_t timestamp = GetNowInMsec();
        while (timestamp <= last_timestamp)
        {
            timestamp = GetNowInMsec();
        }
        return timestamp;
    }

    class IdWorkerUnThreadSafe
    {
    public:
        IdWorkerUnThreadSafe(uint16_t worker_id, uint16_t data_center_id)
            : worker_id_(worker_id)
            , data_center_id_(data_center_id)
        {}

        uint64_t GetNextID()
        {
            uint64_t timestamp = GetNowInMsec();

            // 在当前秒�?
            if (last_timestamp_ == timestamp)
            {
                sequence_ = (sequence_ + 1) & 0xFFF;
                if (sequence_ == 0)
                {
                    timestamp = WaitUntilNextMillis(last_timestamp_);
                }
            }
            else
            {
                sequence_ = 0;
            }

            last_timestamp_ = timestamp;
            return ((timestamp & 0x1FFFFFF) << 22 |
                (data_center_id_ & 0x1F) << 17 |
                (worker_id_ & 0x1F) << 12 |
                (sequence_ & 0xFFF));
        }

    protected:
        uint16_t worker_id_{ 0 };
        uint16_t data_center_id_{ 0 };
        uint64_t last_timestamp_{ 0 };
        uint32_t sequence_{ 0 };
    };

    class IdWorkerThreadSafe
    {
    public:
        IdWorkerThreadSafe(uint16_t worker_id, uint16_t data_center_id)
            : id_worker_(worker_id, data_center_id)
        {}

        uint64_t GetNextID()
        {
            std::lock_guard<std::mutex> g(lock_);
            return id_worker_.GetNextID();
        }

    protected:
        IdWorkerUnThreadSafe id_worker_;
        std::mutex lock_;
    };
}

//////////////////////////////////////////////////////////////////////////

AFCGUIDModule::AFCGUIDModule(AFIPluginManager* p)
{
    pPluginManager = p;
}

bool AFCGUIDModule::Init()
{
    return true;
}

bool AFCGUIDModule::AfterInit()
{
    return true;
}

bool AFCGUIDModule::Execute()
{
    return true;
}

bool AFCGUIDModule::BeforeShut()
{
    if (NULL != m_pIDWoker)
    {
        delete m_pIDWoker;
        m_pIDWoker = NULL;
    }

    return true;
}

bool AFCGUIDModule::Shut()
{
    return true;
}

void AFCGUIDModule::SetWorkerAndDatacenter(uint16_t worker_id, uint16_t data_center_id)
{
#ifdef AF_THREAD_SAFE
    m_pIDWoker = ARK_NEW GUIDModule::IdWorkerThreadSafe(worker_id, data_center_id);
#else
    m_pIDWoker = ARK_NEW GUIDModule::IdWorkerUnThreadSafe(worker_id, data_center_id);
#endif
}

uint64_t AFCGUIDModule::CreateGUID()
{
    assert(NULL != m_pIDWoker);

    return m_pIDWoker->GetNextID();
}
