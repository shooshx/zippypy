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
#include "defs.h"


enum EPyOpcdes {
#define def_op(name, num, flags) name = num,
#include "opcodes_def.h"
#undef def_op
};

uchar opFlags(uchar op) {
    static bool inited = false;
    static uchar opsflags[256] = {0};
    if (!inited) {
#define def_op(name, num, flags) opsflags[num] = flags;
#include "opcodes_def.h"
#undef def_op
        inited = true;
    }
    return opsflags[op];
}

const char* opName(uchar op) {
    static bool inited = false;
    static const char* names[256] = {0};
    if (!inited) {
#define def_op(name, num, flags) names[num] = #name;
#include "opcodes_def.h"
#undef def_op
    }
    return names[op];
}


// cmp_op = ('<', '<=', '==', '!=', '>', '>=', 'in', 'not in', 'is', 'is not', 'exception match', 'BAD')

enum EPyOperators {
    OPER_LESS       = 0,
    OPER_LESS_EQ    = 1,
    OPER_EQ         = 2,
    OPER_NOT_EQ     = 3,
    OPER_GREATER    = 4,
    OPER_GREATER_EQ = 5,
    OPER_IN         = 6,
    OPER_NOT_IN     = 7,
    OPER_IS         = 8,
    OPER_IS_NOT     = 9,
    OPER_EXP_MATCH  = 10
};

