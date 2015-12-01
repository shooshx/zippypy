// Copyright 2015 by Intigua, Inc.
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
#pragma once
#include "objects.h"

// used for raw memory access from python
// write into and read from an existing buffer of memory
class AccessBuffer 
{
public:
    enum ESeekOrigin { ORIGIN_BEG = 0, ORIGIN_CUR }; // there is not ORIGIN_END since we don't know the size of the buffer
    // add this class to the given module
    static ClassObjRef addToModule(const ModuleObjRef& mod);

    AccessBuffer(PyVM* vm, size_t addr) : m_vm(vm), m_p((char*)addr), m_offset(0) 
    {}
    int offset() {
        return m_offset;
    }
    void setOffset(int v) {
        m_offset = v;
    }

    ObjRef readPtr();
    ObjRef readInt(int numBytes);
    // read a nullptr terminated string from the buffer
    ObjRef readCStr();
    ObjRef readBuf(int len);
    ObjRef readWCStr();
	void writeCStr(const string& s);

    // set the cursor to a position in the buffer
    // origin is one of ORIGIN_BEG, ORIGIN_CUR, offset is positive or negative, can use SIZEOF_PTR
    // seeking before start of the buffer causes an exception.
    void seekBytes(int origin, int offset);

    void writePtr(const ObjRef& v);
    void writeInt(int numBytes, const ObjRef v);
    size_t c_ptr() {
        return (size_t)m_p + m_offset;
    }
private:
    template<typename T>
    ObjRef readNum();
    template<typename T>
    void writeNum(const ObjRef& o);

private:
    char* m_p;
    int m_offset; // in bytes
    PyVM* m_vm;
};

// create a new buffer and write to it
class BufferBuilder
{
public:
    static ClassObjRef addToModule(const ModuleObjRef& mod);

    BufferBuilder(PyVM*) : m_offset(0) {}
    void resize(int sz) {
        m_buf.resize(sz);
    }
    int size() { 
        return (int)m_buf.size();
    }
    size_t c_ptr() {
        return (size_t)m_buf.data();
    }
    string str() {
        return m_buf;
    }

    void writePtr(const ObjRef& v);
    void writeInt(int numBytes, const ObjRef v);
    void writeCStr(const string& s);
private:
    template<typename T>
    void writeNum(const ObjRef& o);

private:
    string m_buf;
    int m_offset;
};

