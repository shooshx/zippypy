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
#include "objects.h"
#include "CodeDefinition.h"

class Deserialize
{
public:
    Deserialize(istream& ins) : m_in(ins) 
    {}
    template<typename T>
    T read() {
        T v;
        m_in.read((char*)&v, sizeof(T));
        return v;
    }
    string readStr(uint count) {
        string s;
        s.resize(count);
        m_in.read((char*)s.data(), count);
        return s;
    }

    vector<ObjRef> m_internedStr;
    int offset() const {
        return (int)m_in.tellg();
    }

private:
    istream& m_in;
};

ObjRef parseNext(Deserialize& s, PyVM* vm);

#define CO_FAST_LOCAL   0x20
#define CO_FAST_CELL    0x40
#define CO_FAST_FREE    0x80

// https://github.com/daeken/Benjen/blob/master/daeken.com/entries/python-marshal-format.md
// https://github.com/python/cpython/blob/main/Python/marshal.c

void CodeDefinition::parseCode(Deserialize& s, PyVM* vm)
{
    co_argcount = s.read<uint>();
    co_posonlyargcount = s.read<uint>();
    co_kwonlyargcount = s.read<uint>();
    co_stacksize = s.read<uint>();
    co_flags = s.read<uint>();
    co_code = extract<string>(parseNext(s, vm));
    co_consts = extract<vector<ObjRef>>(parseNext(s, vm));
    co_names = extract<vector<string>>(parseNext(s, vm));
    co_localsplusnames = extract<vector<string>>(parseNext(s, vm));
    auto localspluskinds = parseNext(s, vm);
    co_filename = extract<string>(parseNext(s, vm));
    co_name = extract<string>(parseNext(s, vm));
    co_qualname = extract<string>(parseNext(s, vm));
    co_firstlineno = s.read<uint>();
    auto linetable = parseNext(s, vm);
    auto exceptiontable = parseNext(s, vm);

    co_nlocalplus = (int)co_localsplusnames.size();
    co_nlocals = 0; // TBD3 // get_localsplus_counts
    co_ncellvars = 0;
    co_nfreevars = 0;

    /*
       for (int i = 0; i < nlocalsplus; i++) {
        _PyLocals_Kind kind = _PyLocals_GetKind(kinds, i);
        if (kind & CO_FAST_LOCAL) {
            nlocals += 1;
            if (kind & CO_FAST_CELL) {
                ncellvars += 1;
            }
        }
        else if (kind & CO_FAST_CELL) {
            ncellvars += 1;
        }
        else if (kind & CO_FAST_FREE) {
            nfreevars += 1;
        }
    }
   */
}

#define FLAG_REF '\x80'

// implemeted in marshal.c
ObjRef parseNext(Deserialize& s, PyVM* vm) 
{
    uchar code = s.read<uchar>();
    uchar flag = code & FLAG_REF;
    uchar type = code & ~FLAG_REF;

    //int o = s.offset();
    switch (type) {
    case '0': return ObjRef(); // should not happen
    case 'N': return vm->makeNone();
    case 'F': return vm->makeFromT(false);
    case 'T': return vm->makeFromT(true);
    case 'i': return vm->makeFromT(s.read<int>());
    case 'I': return vm->makeFromT(s.read<int64>());
    case 'g': return vm->makeFromT(s.read<double>());
    case 'l': { // long, encoded as digits in base 2^15
        int h = s.read<uint>(); // number of digits, sign is the sign of the number
        int sz = std::abs(h);
        int pos = 0;
        int64 res = 0;  // not supporting real long values
        for (int i = 0; i < sz; ++i) {
            auto b = s.read<ushort>();
            res |= (int64)b << pos;
            pos += 15;
        }
        return vm->makeFromT(h < 0 ? -res : res);
    }
    case ')':
    case '(': 
    case '[': {
        uint sz = (type != ')') ? s.read<uint>() : s.read<uchar>();  // small or normal
        ListObject* obj = (type != '[') ? new TupleObject : new ListObject;
        obj->v.resize(sz);
        for(uint i = 0; i < sz; ++i)
            obj->v[i] = parseNext(s, vm);
        return vm->alloc(obj);
    }
    case 'c': {
        auto* co = new CodeObject;
        co->m_co.parseCode(s, vm);
        return vm->alloc(co);
    }
    case 'z': // short ascii
    case 'Z': // interned short ascii
    case 'a': // ascii
    case 'A': // interned ascii
    case 's': // bytes
    case 'u': // utf8 unicode
    case 't': { // interned bytes
        uint sz = (type == 'z' || type == 'Z') ? s.read<uchar>() : s.read<uint>(); 
        ObjRef obj = vm->makeFromT(s.readStr(sz)); 
        if (type == 't' || type == 'A' || type == 'Z')
            s.m_internedStr.push_back(obj);
        return obj;
    }
   // case 'R': return s.m_internedStr[s.read<int>()];

    case '{':
    case '>':
    case 'y': // binary complex
    case 'x': // old complex
    case 'f': // old float
    case 'S': // stop iter
    case '.': // ellipsis
    default:
        THROW("Unsupported marshal type `" << type << "`");
    }

}

#define MAGIC_NUMBER 0x0a0d0da7  // for version 3.11.3

// Lib\importlib\_bootstrap_external.py
ObjRef CodeDefinition::parsePyc(istream& iss, PyVM* vm, bool hasHeadr)
{
    Deserialize d(iss);
    if (hasHeadr) {
        uint magic = d.read<uint>();
        CHECK(magic == MAGIC_NUMBER, "Unexpected magic number in marshal format " << hex << magic);
        uint timestamp = d.read<uint>();
    }
    return parseNext(d, vm);
}