/*
Tencent is pleased to support the open source community by making 
PhxPaxos available.
Copyright (C) 2016 THL A29 Limited, a Tencent company. 
All rights reserved.

Licensed under the BSD 3-Clause License (the "License"); you may 
not use this file except in compliance with the License. You may 
obtain a copy of the License at

https://opensource.org/licenses/BSD-3-Clause

Unless required by applicable law or agreed to in writing, software 
distributed under the License is distributed on an "AS IS" basis, 
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or 
implied. See the License for the specific language governing 
permissions and limitations under the License.

See the AUTHORS file for names of contributors. 
*/

#include "bytes_buffer.h"
#include <stdio.h>
#include <assert.h>

namespace phxpaxos
{

#define DEFAULT_BUFFER_LEN 1048576

BytesBuffer :: BytesBuffer()
    : m_pcBuffer(nullptr), m_iLen(DEFAULT_BUFFER_LEN)
{
    m_pcBuffer = new char[m_iLen];
    assert(m_pcBuffer != nullptr);
}
    
BytesBuffer :: ~BytesBuffer()
{
    delete []m_pcBuffer;
}

char * BytesBuffer :: GetPtr()
{
    return m_pcBuffer;
}

int BytesBuffer :: GetLen()
{
    return m_iLen;
}

void BytesBuffer :: Ready(const int iBufferLen)
{
    if (m_iLen < iBufferLen)
    {
        delete []m_pcBuffer;
        m_pcBuffer = nullptr;

        while (m_iLen < iBufferLen)
        {
            m_iLen *= 2; 
        }

        m_pcBuffer = new char[m_iLen];
        assert(m_pcBuffer != nullptr);
    }
}

}


