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


// http://daeken.com/2010-02-20_Python_Marshal_Format.html
void CodeDefinition::parseCode(Deserialize& s, PyVM* vm)
{
    co_argcount = s.read<uint>();
    co_nlocals = s.read<uint>();
    co_stacksize = s.read<uint>();
    co_flags = s.read<uint>();
    co_code = extract<string>(parseNext(s, vm));
    co_consts = extract<vector<ObjRef>>(parseNext(s, vm));
    
    co_names = extract<vector<string>>(parseNext(s, vm));
    co_varnames = extract<vector<string>>(parseNext(s, vm));
    co_freevars = extract<vector<string>>(parseNext(s, vm));
    co_cellvars = extract<vector<string>>(parseNext(s, vm));
    co_filename = extract<string>(parseNext(s, vm));
    co_name = extract<string>(parseNext(s, vm));

    co_firstlineno = s.read<uint>();
    co_lnotab = extract<string>(parseNext(s, vm));
}

ObjRef parseNext(Deserialize& s, PyVM* vm) 
{
    uchar t = s.read<uchar>();
    int o = s.offset();
    switch (t) {
    case '0': return ObjRef(); // should not happen
    case 'N': return vm->makeNone();
    case 'F': return vm->makeFromT(false);
    case 'T': return vm->makeFromT(true);
    case 'i': return vm->makeFromT(s.read<int>());
    case 'I': return vm->makeFromT(s.read<int64>());
    case 'g': return vm->makeFromT(s.read<double>());
    case 's': { 
        uint sz = s.read<uint>(); 
        return vm->makeFromT(s.readStr(sz)); 
    }
    case '(': 
    case '[': {
        uint sz = s.read<uint>(); 
        ListObject* obj = (t == '(') ? new TupleObject : new ListObject;
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
    case 't': { // interned str
        uint sz = s.read<uint>(); 
        ObjRef obj = vm->makeFromT(s.readStr(sz)); 
        s.m_internedStr.push_back(obj);
        return obj;
    }
    case 'R': return s.m_internedStr[s.read<int>()];
    case 'l': { // long, encoded as digits in base 2^15
        int h = s.read<uint>(); // number of digits, sign is the sign of the number
        int sz = std::abs(h);
        int pos = 0;
        int64 res = 0;
        for(int i = 0; i < sz; ++i) {
            auto b = s.read<ushort>();
            res |= (int64)b << pos;
            pos += 15; 
        }
        return vm->makeFromT(h < 0 ? -res : res);
    }
    case 'u': { // utf8 unicode
        uint sz = s.read<uint>(); 
        wstring us;
        CHECK(wstrFromUtf8(s.readStr(sz), &us), "Failed reading UTF8");
        return vm->makeFromT(us); 
    }
    case '{':
    case '>':
    case 'y': // binary complex
    case 'x': // old complex
    case 'f': // old float
    case 'S': // stop iter
    case '.': // ellipsis
    default:
        THROW("Unsupported marshal type `" << t << "`");
    }

}


ObjRef CodeDefinition::parsePyc(istream& iss, PyVM* vm, bool hasHeadr)
{
    Deserialize d(iss);
    if (hasHeadr) {
        uint magic = d.read<uint>();
        CHECK(magic == 0x0a0df303, "Unexpected magic number in marshal format " << hex << magic);
        uint timestamp = d.read<uint>();
    }
    return parseNext(d, vm);
}