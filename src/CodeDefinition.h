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
#include <vector>
#include <string>

class Deserialize;

class CodeDefinition 
{
public:
    static ObjRef parsePyc(istream& iss, PyVM* vm, bool hasHeader);

    void parseCode(Deserialize& s, PyVM* vm);
public:
    string co_name;
    uint co_argcount;
    uint co_nlocals;
    vector<string> co_varnames;
    vector<string> co_cellvars;
    vector<string> co_freevars;
    string co_code;
    vector<ObjRef> co_consts;
    vector<string> co_names;
    string co_filename;
    uint co_firstlineno;
    string co_lnotab;
    uint co_stacksize;
    uint co_flags;
};