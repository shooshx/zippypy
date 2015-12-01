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
#include "PyVM.h"


class OpImp {
public:
    OpImp(PyVM *_vm) :vm(_vm) {}

    template<typename OT>
    ObjRef addType(Object *lhs, Object *rhs);
    ObjRef add(const ObjRef& lhsref, const ObjRef& rhsref);

    template<typename OT>
    ObjRef multType(Object *lhs, Object *rhs);
    template<typename TC>
    ObjRef multStr(Object *str, Object *num);
    ObjRef mult(const ObjRef& lhsref, const ObjRef& rhsref);

    template<typename OT>
    ObjRef subType(Object *lhs, Object *rhs);
    ObjRef sub(const ObjRef& lhsref, const ObjRef& rhsref);

    template<typename OT>
    ObjRef divType(Object *lhs, Object *rhs);
    ObjRef div(const ObjRef& lhsref, const ObjRef& rhsref);

    template<typename OT>
    ObjRef minusType(Object *arg);
    ObjRef uminus(const ObjRef& argref);

    ObjRef uplus(const ObjRef& argref);
    ObjRef unot(const ObjRef& argref);

    template<typename LT>
    ObjRef makeListFromStack(Frame& frame, int count);
    void print(const ObjRef& vref, ostream& out);

    ObjRef len(const ObjRef& arg);

    ObjRef hash(const ObjRef& arg) {
        return vm->alloc(new IntObject(hashNum(arg)));
    }
    ObjRef str(const ObjRef& arg) {
        return vm->alloc(new StrObject(stdstr(arg, false)));
    }
    ObjRef repr(const ObjRef& arg) {
        return vm->alloc(new StrObject(stdstr(arg, true)));
    }
    ObjRef hex(const ObjRef& n) {
        int64 num = checked_cast<IntObject>(n)->v;
        stringstream ss; ss << "0x" << std::hex << num;
        return vm->alloc(new StrObject(ss.str()));
    }
    ObjRef int_(const ObjRef& arg);
    ObjRef bool_(const ObjRef& arg);

    bool compare(const ObjRef& lhsref, const ObjRef& rhsref, int op);
    bool compareList(const ListObject* lhs, const ListObject* rhs, int op);
    bool operIn(const ObjRef& lhs, const ObjRef& rhs, bool isPositive);

    ObjRef runtime_import(const ObjRef& filenameObj);

    ObjRef apply_slice(const ObjRef& o, int* startp, int* endp);

private:
    PyVM* vm;
};

