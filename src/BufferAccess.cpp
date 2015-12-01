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
#include "BufferAccess.h"


#define CATCH_ACCESS_VIOLATION(func) catch(const exception&) { throw; }\
     catch(...) { THROW("Exception in " #func " " << (void*)m_p << "+" << m_offset); }
    // in case we didn't have ExceptionsTranslator

template<typename T>
ObjRef AccessBuffer::readNum() {
    try {
        T v = 0;
        memcpy(&v, m_p + m_offset, sizeof(T));
        m_offset += sizeof(T);
        return m_vm->alloc(new IntObject(v));
    } CATCH_ACCESS_VIOLATION(readNum)
}
template<typename T>
void AccessBuffer::writeNum(const ObjRef& o) {
    try {
        T v = extract<T>(o);
        memcpy(m_p + m_offset, &v, sizeof(v));
        m_offset += sizeof(v);
    } CATCH_ACCESS_VIOLATION(writeNum)
}

ObjRef AccessBuffer::readPtr() {
    return readNum<size_t>();
}
ObjRef AccessBuffer::readInt(int numBytes) {
    switch (numBytes) {
    case 1: return readNum<char>();
    case 2: return readNum<short>(); 
    case 4: return readNum<int>();
    case 8: return readNum<int64>();
    default: THROW("can't readInt " << numBytes);
    }
}
ObjRef AccessBuffer::readCStr() {
    try {
        int len = (int)strlen(m_p + m_offset);
        ObjRef r = readBuf(len);
        m_offset += 1; // consume nullptr termination
        return r;
    } CATCH_ACCESS_VIOLATION(readCStr)
}
ObjRef AccessBuffer::readBuf(int len) {
    try {
        string s(m_p + m_offset, len);
        m_offset += len;
        return m_vm->makeFromT(s);
    } CATCH_ACCESS_VIOLATION(readBuf)
}
ObjRef AccessBuffer::readWCStr(){ 
    try {
        int count = (int)wcslen((wchar_t*)(m_p + m_offset));
        wstring s;
        s.resize(count);
        memcpy((char*)s.data(), m_p + m_offset, count * sizeof(wchar_t));
        m_offset += (count + 1) * sizeof(wchar_t); // consume nullptr termination
        return m_vm->makeFromT(s);
    } CATCH_ACCESS_VIOLATION(readWCStr)
}

void AccessBuffer::writePtr(const ObjRef& v) {
    writeNum<size_t>(v);
}
void AccessBuffer::writeInt(int numBytes, const ObjRef v) {
    switch (numBytes) {
    case 1: return writeNum<char>(v);
    //case 2: return writeNum<short>(v); # TODO - Fix, Not compiling
    case 4: return writeNum<int>(v);
    case 8: return writeNum<int64>(v);
    default: THROW("can't writeInt " << numBytes);
    }
}

void AccessBuffer::writeCStr(const string& s) {
    try {
        int count = (int)s.length() + 1;
        memcpy(m_p + m_offset, (char*)s.c_str(), count); // c_str guarantees that it will be nullptr terminated
        m_offset += count;
    } CATCH_ACCESS_VIOLATION(writeCStr)
}

void AccessBuffer::seekBytes(int origin, int offset) {
    int res = 0;
    if (origin == ORIGIN_BEG)
        res = offset;
    else if (origin == ORIGIN_CUR)
        res = m_offset + offset;
    else
        THROW("AccessBuffer::seekBytes: unexpected origin value " << origin);
    CHECK(res >= 0, "AccessBuffer::seekBytes: seek result is negative " << res);
    m_offset = res;
}


ClassObjRef AccessBuffer::addToModule(const ModuleObjRef& mod) {
    auto cls = mod->class_<AccessBuffer>("AccessBuffer", CtorDef<size_t>());

    cls->def(&AccessBuffer::readPtr, "readPtr");
    cls->def(&AccessBuffer::readInt, "readInt");
    cls->def(&AccessBuffer::readCStr, "readCStr");
    cls->def(&AccessBuffer::readWCStr, "readWCStr");
    cls->def(&AccessBuffer::readBuf, "readBuf");

	cls->def(&AccessBuffer::writeCStr, "writeCStr");
    cls->def(&AccessBuffer::writePtr, "writePtr");
    cls->def(&AccessBuffer::writeInt, "writeInt");

    cls->def(&AccessBuffer::offset, "offset");
    cls->def(&AccessBuffer::setOffset, "setOffset");
    cls->def(&AccessBuffer::c_ptr, "c_ptr");

    cls->def(&AccessBuffer::seekBytes, "seekBytes");
    cls->addMember(mod->m_vm->makeFromT((int)ORIGIN_BEG), "ORIGIN_BEG");
    cls->addMember(mod->m_vm->makeFromT((int)ORIGIN_CUR), "ORIGIN_CUR");
    cls->addMember(mod->m_vm->makeFromT(sizeof(void*)), "SIZEOF_PTR");

    return cls;
}


//---------------------------------------------------------------------------------------------

template<typename T>
void BufferBuilder::writeNum(const ObjRef& o) {
    if (m_offset + sizeof(T) > m_buf.size())
        m_buf.resize(m_offset + sizeof(T));
    T v = extract<T>(o);
    memcpy((char*)m_buf.data() + m_offset, &v, sizeof(T));
    m_offset += sizeof(T);
}

void BufferBuilder::writePtr(const ObjRef& v) {
    writeNum<size_t>(v);
}

void BufferBuilder::writeInt(int numBytes, const ObjRef v) {
    switch (numBytes) {
    case 1: return writeNum<char>(v);
    case 2: return writeNum<short>(v);
    case 4: return writeNum<int>(v);
    case 8: return writeNum<int64>(v);
    default: THROW("can't writeInt " << numBytes);
    }
}

void BufferBuilder::writeCStr(const string& s) {
    if (m_offset + s.length() + 1 > m_buf.size())
        m_buf.resize(m_offset + s.length() + 1);
    memcpy((char*)m_buf.data() + m_offset, s.c_str(), s.length() + 1);
    m_offset += (int)s.length() + 1;
}

ClassObjRef BufferBuilder::addToModule(const ModuleObjRef& mod) {
    auto cls = mod->class_<BufferBuilder>("BufferBuilder", CtorDef<NoType>());

    cls->def(&BufferBuilder::writePtr, "writePtr");
    cls->def(&BufferBuilder::writeInt, "writeInt");
    cls->def(&BufferBuilder::writeCStr, "writeCStr");
    
    cls->def(&BufferBuilder::resize, "resize");
    cls->def(&BufferBuilder::size, "size");
    cls->def(&BufferBuilder::c_ptr, "c_ptr");
    cls->def(&BufferBuilder::str, "str");
    return cls;
}



