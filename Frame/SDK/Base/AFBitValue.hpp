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

//���������͵�λ���� ����ֵֻ�������õ�����(int, short, char..��)
template<class T>
class BitValue
{
public:
    enum BitValueDefine
    {
        ErrorValue = -1,        // ����ֵ
        NoneValue = 0,          // û��λֵ
        HaveValue = 1,          // ����λֵ
    };

    BitValue() : m_nValue(0) { }
    BitValue(T nValue) : m_nValue(nValue) { }

    void SetValue(const T& nValue)
    {
        m_nValue = nValue;
    }

    T GetValue() const
    {
        return m_nValue;
    }

    static int GetBitLength()
    {
        return sizeof(T) * 8;
    }

    // ��õ�һ��λֵ��0��λ����
    int GetFirstNoValueIndex()
    {
        for(int i = 0; i < GetBitLength(); ++i)
        {
            if((m_nValue & (T(1) << i)) == NoneValue)
            {
                return i;
            }
        }

        return -1;  // 32λ����ֵ
    }

    T GetBitValue(const int nIndex)         // �õ�ĳλ�ϵ�ֵ
    {
        if(nIndex < 0 || nIndex >= GetBitLength())
        {
            return ErrorValue;
        }

        return (m_nValue & (T(1) << nIndex));
    }

    void SetBitValue(const int nIndex)        // ����ĳλ�ϵ�ֵ
    {
        if(nIndex < 0 || nIndex >= GetBitLength())
        {
            return;
        }

        m_nValue |= (T(1) << nIndex);
    }

    void ClearBitValue(const int nIndex)      // ���ĳλ�ϵ�ֵ
    {
        if(nIndex < 0 || nIndex >= GetBitLength())
        {
            return;
        }

        m_nValue &= ~(T(1) << nIndex);
    }

    bool HaveBitValue(const int nIndex)
    {
        return GetBitValue(nIndex) != NoneValue;
    }

    ////////////////////////////////////////////////////////////////////////////////////////
    static int GetFirstNoValueIndex(const T& nValue)
    {
        for(int i = 0; i < GetBitLength(); ++i)
        {
            if((nValue & (T(1) << i)) == NoneValue)
            {
                return i;
            }
        }

        return -1;  // 32λ����ֵ
    }

    static T GetBitValue(const T & nValue, const int nIndex)
    {
        if(nIndex < 0 || nIndex >= GetBitLength())
        {
            return ErrorValue;
        }

        return (nValue & (T(1) << nIndex));
    }

    static void SetBitValue(T& nValue, const int nIndex)
    {
        if(nIndex < 0 || nIndex >= GetBitLength())
        {
            return;
        }

        nValue |= (T(1) << nIndex);
    }

    static void ClearBitValue(T& nValue, const int nIndex)
    {
        if(nIndex < 0 || nIndex >= GetBitLength())
        {
            return;
        }

        nValue &= ~(T(1) << nIndex);
    }

    static bool HaveBitValue(const T& nValue, const int nIndex)
    {
        return GetBitValue(nValue, nIndex) != NoneValue;
    }

private:
    T m_nValue;
};