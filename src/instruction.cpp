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
#include "defs.h"
#include "opcodes.h"
#include "objects.h"
#include "PyVM.h"
#include "OpImp.h"
#include "log.h"
#include "utils.h"

#include <sstream>
/*#include <boost/foreach.hpp>
#include <boost/math/special_functions/round.hpp>
#include <boost/property_tree/ptree.hpp> // for json
#include <boost/property_tree/json_parser.hpp>
*/



// NOTICE: this number should be incremented whenever there are additions to the api supplied by PyVM and related classes
// 1: initial release
// 2: added runtime_import() builtin, int(), bool() builtins
#define PYVM_VERSION (2)



class Instruction {
public:
    Instruction(uchar _opcode) :opcode(_opcode), param(0) {}
    uchar opcode;
    ushort param;
};



template<typename LT> // ListObject or TupleObject
ObjRef OpImp::makeListFromStack(Frame& frame, int count) {
    auto ret = vm->alloct<LT>(new LT());
    ret->v.resize(count);
    for(int i = count - 1; i >= 0; --i)
        ret->v[i] = frame.m_stack.pop();
    return ObjRef(ret);
}



template<typename OT>
static bool compareType(Object *lhs, Object *rhs, int op) {
    switch (op) {
    case OPER_LESS:    return ((OT*)lhs)->v < ((OT*)rhs)->v;
    case OPER_LESS_EQ: return ((OT*)lhs)->v <= ((OT*)rhs)->v;
    case OPER_EQ:      return ((OT*)lhs)->v == ((OT*)rhs)->v;
    case OPER_NOT_EQ:  return ((OT*)lhs)->v != ((OT*)rhs)->v;
    case OPER_GREATER: return ((OT*)lhs)->v > ((OT*)rhs)->v;
    case OPER_GREATER_EQ: return ((OT*)lhs)->v >= ((OT*)rhs)->v;
    case OPER_IN: // not relevant for the types that end up here
    default:
        THROW("Unknown op " << op);
    }

}

template<typename OT>
static bool compareStrType(Object *lhs, Object *rhs, int op) {
    if (op == OPER_IN || op == OPER_NOT_IN) {
        auto &rhsv = ((OT*)rhs)->v;
        return (rhsv.find( ((OT*)lhs)->v ) != rhsv.npos) ^ (op == OPER_NOT_IN);
    }
    return compareType<OT>(lhs, rhs, op);
}


static bool operIs(const Object* lhsref, const Object* rhsref) {
    if (lhsref->type == Object::NONE && rhsref->type == Object::NONE)
        return true;
    // CPython also has int(x) == int(x) for x<=256, not implemented here
    return lhsref == rhsref;
}

bool OpImp::operIn(const ObjRef& lhs, const ObjRef& rhs, bool isPositive) {
    switch (rhs->type) {    
    case Object::TUPLE: 
    case Object::LIST: {
        auto* l = checked_dynamic_pcast<ListObject>(rhs.get());
        for(auto it = l->v.begin(); it != l->v.end(); ++it) {
            if (compare(lhs, *it, OPER_EQ))
                return isPositive; // found
        }
        return !isPositive;
    }
    case Object::STRDICT: {
        auto* d = checked_dynamic_pcast<StrDictObject>(rhs.get());
        if (lhs->type != Object::STR)
            return !isPositive;
        const string& key = checked_dynamic_pcast<StrObject>(lhs)->v;
        auto it = d->v.find(key);
        return (it != d->v.end()) == isPositive;
    }
    }
    THROW("Can't do operator in for given objects " << lhs->typeName() << " " << rhs->typeName());
}

bool OpImp::compareList(const ListObject* lhs, const ListObject* rhs, int op) {
    if (op == OPER_IN || op == OPER_NOT_IN)
        return operIn(ObjRef(const_cast<ListObject*>(lhs)), ObjRef(const_cast<ListObject*>(rhs)), op == OPER_IN);
    if (op != OPER_EQ && op != OPER_NOT_EQ)
        THROW("Can't compare lists with " << op);
    if (lhs->size() != rhs->size())
        return op != OPER_EQ;
    for(int i = 0; i < lhs->size(); ++i) {
        if (!compare(lhs->v[i], rhs->v[i], OPER_EQ))
            return op != OPER_EQ;
    }
    return op == OPER_EQ;
}

static bool opHasEq(int op) {
    return op == OPER_EQ || op == OPER_LESS_EQ || op == OPER_GREATER_EQ;
}

static void checkNoCmp(const ObjRef& o) {
    IAttrable* ao = o->tryAs<IAttrable>();
    if (ao != nullptr)
        CHECK( ao->attr("__nocmp__").isNull(), "An object of this type should not be compared (forgot to call '.get()'?");
}

// OpImp needed for creating temp conversion strings
// TBD: detect recursion y = [] y.append(y)
bool OpImp::compare(const ObjRef& lhsref, const ObjRef& rhsref, int op) 
{
    Object *lhs = lhsref.get(), *rhs = rhsref.get();
    // TBD in, not in for lists, tuples, dicts
    switch (op) {
    case OPER_IS:     return operIs(lhs, rhs);
    case OPER_IS_NOT: return !operIs(lhs, rhs);
    }

    if ( (lhs->type == Object::NONE || rhs->type == Object::NONE) && op != OPER_IN && op != OPER_NOT_IN) {
        if (lhs->type == Object::NONE && rhs->type == Object::NONE)
            return opHasEq(op);
        return !opHasEq(op); // None is equal only to None
    }

    if (lhs->type == rhs->type) {
        switch (lhs->type) {
        case Object::INT:  return compareType<IntObject>(lhs, rhs, op);
        case Object::BOOL: return compareType<BoolObject>(lhs, rhs, op);
        case Object::STR:  return compareStrType<StrObject>(lhs, rhs, op);
        case Object::USTR: return compareStrType<UnicodeObject>(lhs, rhs, op);
        case Object::TUPLE:
        case Object::LIST: return compareList((ListObject*)lhs, (ListObject*)rhs, op);
        case Object::FLOAT:return compareType<FloatObject>(lhs, rhs, op);
        case Object::DICT: // TBD
        default: 
            checkNoCmp(lhsref); checkNoCmp(rhsref);
            return (lhs == rhs) == opHasEq(op);  // for all other types, reference compare
        }
    }
    if (lhs->type == Object::USTR && rhs->type == Object::STR) {
        ObjRef urhs = UnicodeObject::fromStr(rhsref, vm);
        return compareStrType<UnicodeObject>(lhs, urhs.get(), op);
    }
    if (lhs->type == Object::STR && rhs->type == Object::USTR) {
        ObjRef ulhs = UnicodeObject::fromStr(lhsref, vm);
        return compareStrType<UnicodeObject>(ulhs.get(), rhs, op);
    }
    if ((lhs->type == Object::FLOAT && rhs->type == Object::INT) ) {
        FloatObject floatRhs((float)((IntObject*)rhs)->v);
        return compareType<FloatObject>(lhs, &floatRhs, op);
    }
    if ((lhs->type == Object::INT && rhs->type == Object::FLOAT) ) {
        FloatObject floatLhs((float)((IntObject*)lhs)->v);
        return compareType<FloatObject>(&floatLhs, rhs, op);
    }
    if (op == OPER_IN || op == OPER_NOT_IN) {
        return operIn(lhsref, rhsref, op == OPER_IN);
    }

    checkNoCmp(lhsref); checkNoCmp(rhsref);	
    return false; // unmatching types. we don't want to throw exception here because we want to support situations like this :  TRUE('aa' in [1,2,3,'aa'])

};

// for hash
bool objEquals(const ObjRef& lhsref, const ObjRef& rhsref, PyVM* vm) {
    OpImp oi(vm);
    return oi.compare(lhsref, rhsref, OPER_EQ);
}


// this is opposed to bool_()
bool asBool(const ObjRef& vref) {
    Object *v = vref.get();
    CHECK(v->type == Object::BOOL, "expected boolean");
    return ((BoolObject*)v)->v;
}

class RecurionTracker {
public:
    struct Guard {
        Guard() : m_tracker(nullptr), m_obj(nullptr) {}
        Guard(RecurionTracker* tracker, Object* o) :m_tracker(tracker), m_obj(o) {
            m_tracker->m_rec.push_back(m_obj);
        }
        bool wasHere() {
            return m_tracker == nullptr;
        }
        ~Guard() {
            if (m_tracker == nullptr)
                return;
            if (m_tracker->m_rec.back() != m_obj)
                LOG_ERROR("RecurionTracker::Guard: Bad recursion?");
            m_tracker->m_rec.pop_back();
        }
    private:
        RecurionTracker *m_tracker;
        Object* m_obj;
    };
    Guard enter(const ObjRef& ref) {
        if (find(m_rec.begin(), m_rec.end(), ref.get()) != m_rec.end())
            return Guard();
        return Guard(this, ref.get());
    }
private:
    vector<Object*> m_rec;
    friend struct RecurionTracker::Guard;
};

template<typename TC>
static std::basic_string<TC> printStr(const std::basic_string<TC>& s, bool repr, char prefix) {
    if (!repr)
        return s;
    std::basic_string<TC> e;  
    if (prefix != 0)
        e += prefix;
    e += '\'';
    for(uint i = 0; i < s.size(); ++i) {
        TC c = s[i];
        if (c >= 0x20 && c <= 0x7e && c != '\'' && c != '"')
            e += c;
        else {
            e += '\\';
            switch (c) {
            case '\'': case '"': e += c; break;
            case '\n':  e+= 'n'; break;
            case '\r': e += 'r'; break;
            case '\t': e += 't'; break;
            case '\\': e += '\\'; break;
            default:   e += ((std::basic_ostringstream<TC>&)(std::basic_ostringstream<TC>() << 'x' << std::hex << (int)c)).str(); break; 
            }
        }
    }
    e += '\'';
    return e;
}

void print_recurse(const ObjRef& vref, ostream& out, RecurionTracker& tracker, bool repr) 
{
    // prevent infinite recursion
    auto guard = tracker.enter(vref);
    if (guard.wasHere()) {
        out << "[...]";
        return;
    }
    // TBD  maps
    Object *v = vref.get();
    if (v == nullptr) {
        out << "[nullptr]";
        return;
    }
    switch (v->type) {
    case Object::NONE: out << "None"; break;
    case Object::INT:  out << ((IntObject*)v)->v; break;
    case Object::BOOL: out << ( ((BoolObject*)v)->v ? "True" : "False" ); break;
    case Object::STR:  out << printStr(((StrObject*)v)->v, repr, 0); break;
    case Object::USTR: out << utf8FromWstr( printStr(((UnicodeObject*)v)->v, repr, 'u')); break;
    case Object::FLOAT:out << ((FloatObject*)v)->v; break;
    case Object::CODE: out << ((CodeObject*)v)->m_co.co_name; break;
    case Object::MODULE: out << ((ModuleObject*)v)->m_name; break;

    case Object::FUNC: {
        CallableObject *c = (CallableObject*)v;
        FuncObject* f = dynamic_cast<FuncObject*>(c);
        if (f != nullptr) {
            if (!f->m_code.isNull()) 
                print_recurse(ObjRef(f->m_code), out, tracker, false);
            else
                out << "<no-code>";
        }
        else {
            CFuncObject* cf = dynamic_cast<CFuncObject*>(c);
            CHECK(cf != nullptr, "Unknown callable object");
            out << "<c-function>";
        }
        break;
    }
    case Object::INSTANCE: {
        InstanceObject* iv = (InstanceObject*)v;
        if (iv->m_class.isNull()) 
            out << "<no-class> instance";
        else
            out << iv->m_class->m_name << " instance";
        break;
    }
    case Object::METHOD: {
        MethodObject *mv = (MethodObject*)v;
        if (mv->m_self.isNull()) {
            out << "unbound method ";
            print_recurse(ObjRef(mv->m_func), out, tracker, false);
        }
        else {
            out << "bound method ";
            print_recurse(ObjRef(mv->m_func), out, tracker, false);
            out << " of ";
            print_recurse(ObjRef(mv->m_self), out, tracker, false);
        }
        break;
    }
    case Object::LIST:
    case Object::TUPLE: {
        ListObject *lv = (ListObject*)v;
        out << "[";
        for(int i = 0; i < lv->size(); ++i) {
            if (i > 0)
                out << ", ";
            print_recurse(lv->v[i], out, tracker, true);
        }
        out << "]";
        break;
    }
    case Object::CLASS:
        out << "class " << ((ClassObject*)v)->m_name;
        break;
    default:
        out << "<no-str>"; break;
    }
}

void print(const ObjRef& vref, ostream& out, bool repr) {
    RecurionTracker rec;
    print_recurse(vref, out, rec, repr);
}


string stdstr(const ObjRef& vref, bool repr) {
    std::ostringstream ss;
    print(vref, ss, repr);
    return ss.str();
}

template<typename OT>
ObjRef OpImp::addType(Object *lhs, Object *rhs) {
    return vm->alloc(new OT( ((OT*)lhs)->v + ((OT*)rhs)->v ));
}
template<typename OT>
ObjRef OpImp::multType(Object *lhs, Object *rhs) {
    return vm->alloc(new OT( ((OT*)lhs)->v * ((OT*)rhs)->v ));
}
template<typename OT>
ObjRef OpImp::subType(Object *lhs, Object *rhs) {
    return vm->alloc(new OT( ((OT*)lhs)->v - ((OT*)rhs)->v ));
}
template<typename OT>
ObjRef OpImp::divType(Object *lhs, Object *rhs) {
    return vm->alloc(new OT( ((OT*)lhs)->v / ((OT*)rhs)->v ));
}
template<typename OT>
ObjRef OpImp::minusType(Object *arg) {
    return vm->alloc(new OT( -((OT*)arg)->v ));
}
template<typename OT>
static int64 lenType(Object* arg) {
    return checked_cast<OT>(arg)->v.size();
}


ObjRef OpImp::add(const ObjRef& lhsref, const ObjRef& rhsref) {
    // TBD - lists, tuples
    Object *lhs = lhsref.get(), *rhs = rhsref.get();
    if (lhs->type == rhs->type) {
        switch (rhs->type) {
        case Object::INT:   return addType<IntObject>(lhs, rhs);
        case Object::FLOAT: return addType<FloatObject>(lhs, rhs);
        case Object::STR:   return addType<StrObject>(lhs, rhs);
        case Object::USTR:  return addType<UnicodeObject>(lhs, rhs);
        }
    }
    if (lhs->type == Object::STR && rhs->type == Object::USTR) {
        UnicodeObject ulhs( ((StrObject*)lhs)->v, ENC_ASCII );
        return addType<UnicodeObject>(&ulhs, rhs);
    }
    if (lhs->type == Object::USTR && rhs->type == Object::STR) {
        UnicodeObject urhs( ((StrObject*)rhs)->v, ENC_ASCII );
        return addType<UnicodeObject>(lhs, &urhs);
    }
    if ((lhs->type == Object::FLOAT && rhs->type == Object::INT) ) {
        FloatObject floatRhs((float)((IntObject*)rhs)->v);
        return addType<FloatObject>(lhs, &floatRhs);
    }
    if ((lhs->type == Object::INT && rhs->type == Object::FLOAT) ) {
        FloatObject floatLhs((float)((IntObject*)lhs)->v);
        return addType<FloatObject>(&floatLhs, rhs);
    }    
    THROW("Can't add");
}

template<typename TC>
ObjRef OpImp::multStr(Object *str, Object *num) {
    const auto& s = static_cast<PSTROBJ_TYPE(TC)*>(str)->v;
    auto nw = vm->alloct(new PSTROBJ_TYPE(TC));
    int count = (int)static_cast<IntObject*>(num)->v;
    for(int i = 0; i < count; ++i)
        nw->v += s;
    return ObjRef(nw);
}

ObjRef OpImp::mult(const ObjRef& lhsref, const ObjRef& rhsref) {
    // TBD - lists, tuples
    Object *lhs = lhsref.get(), *rhs = rhsref.get();
    if (lhs->type == rhs->type) {
        switch (rhs->type) {
        case Object::INT:    return multType<IntObject>(lhs, rhs);
        case Object::FLOAT:  return multType<FloatObject>(lhs, rhs);
        }
    }
    if (lhs->type == Object::INT && rhs->type == Object::STR) {
        return multStr<char>(rhs, lhs);
    }
    if (lhs->type == Object::STR && rhs->type == Object::INT) {
        return multStr<char>(lhs, rhs);
    }
    if (lhs->type == Object::INT && rhs->type == Object::USTR) {
        return multStr<wchar_t>(rhs, lhs);
    }
    if (lhs->type == Object::USTR && rhs->type == Object::INT) {
        return multStr<wchar_t>(lhs, rhs);
    }
    if ((lhs->type == Object::FLOAT && rhs->type == Object::INT) ) {
        FloatObject floatRhs((float)((IntObject*)rhs)->v);
        return multType<FloatObject>(lhs, &floatRhs);
    }
    if ((lhs->type == Object::INT && rhs->type == Object::FLOAT) ) {
        FloatObject floatLhs((float)((IntObject*)lhs)->v);
        return multType<FloatObject>(&floatLhs, rhs);
    }
    THROW("Can't mult");
}

ObjRef OpImp::sub(const ObjRef& lhsref, const ObjRef& rhsref) {
    // TBD - lists, tuples
    Object *lhs = lhsref.get(), *rhs = rhsref.get();
    if (lhs->type == rhs->type) {
        switch (rhs->type) {
        case Object::INT:    return subType<IntObject>(lhs, rhs);
        case Object::FLOAT:  return subType<FloatObject>(lhs, rhs);
        }
    }
    else if ((lhs->type == Object::FLOAT && rhs->type == Object::INT) ) {
        FloatObject floatRhs((float)((IntObject*)rhs)->v);
        return subType<FloatObject>(lhs, &floatRhs);
    }
    else if ((lhs->type == Object::INT && rhs->type == Object::FLOAT) ) {
        FloatObject floatLhs((float)((IntObject*)lhs)->v);
        return subType<FloatObject>(&floatLhs, rhs);
    }
    THROW("Can't subtract");
}

ObjRef OpImp::div(const ObjRef& lhsref, const ObjRef& rhsref) {
    // TBD - lists, tuples
    Object *lhs = lhsref.get(), *rhs = rhsref.get();
    if (lhs->type == rhs->type) {
        switch (rhs->type) {
        case Object::INT:    return divType<IntObject>(lhs, rhs);
        case Object::FLOAT:  return divType<FloatObject>(lhs, rhs);
        }
    }
    else if ((lhs->type == Object::FLOAT && rhs->type == Object::INT) ) {
        FloatObject floatRhs((float)((IntObject*)rhs)->v);
        return divType<FloatObject>(lhs, &floatRhs);
    }
    else if ((lhs->type == Object::INT && rhs->type == Object::FLOAT) ) {
        FloatObject floatLhs((float)((IntObject*)lhs)->v);
        return divType<FloatObject>(&floatLhs, rhs);
    }
    THROW("Can't div");
}

ObjRef OpImp::uplus(const ObjRef& argref) {
    Object *arg = argref.get();
    if (arg->type == Object::INT || arg->type == Object::FLOAT) {
        return argref;
    }
    THROW("Can't unary positive");
}
ObjRef OpImp::uminus(const ObjRef& argref) {
    Object *arg = argref.get();
    switch (arg->type) {
    case Object::INT:    return minusType<IntObject>(arg);
    case Object::FLOAT:  return minusType<FloatObject>(arg);
    }
    THROW("Can't unary negative");
}
ObjRef OpImp::unot(const ObjRef& argref) {
    Object *arg = argref.get();
    if (arg->type == Object::BOOL) {
        return vm->alloc(new BoolObject( !((BoolObject*)arg)->v ));
    }
    THROW("Can't unary not");
}


static int64 intLen(const ObjRef& argref) {
    Object *arg = argref.get();
    int64 l = 0;
    switch (arg->type) {
    case Object::LIST:  return lenType<ListObject>(arg);
    case Object::TUPLE: return lenType<TupleObject>(arg);
    case Object::DICT:  return lenType<DictObject>(arg);
    case Object::STRDICT:  return lenType<StrDictObject>(arg);
    case Object::STR:   return lenType<StrObject>(arg);
    case Object::USTR:  return lenType<UnicodeObject>(arg);
    default:
    THROW("not len for type " << arg->typeName());
    }
}


ObjRef OpImp::len(const ObjRef& arg) {
    return vm->makeFromT(intLen(arg));
}

int64 lexical_cast(const string& s) {
    return strtoll(s.c_str(), nullptr, 10);
}
int64 lexical_cast(const wstring& s) {
    return wcstoull(s.c_str(), nullptr, 10);
}


ObjRef OpImp::int_(const ObjRef& arg) {
    if (arg->type == Object::INT)
        return arg;
    int64 i = 0;
    switch (arg->type) {
    case Object::STR:   i = lexical_cast(static_pcast<StrObject>(arg)->v); break;
    case Object::USTR:  i = lexical_cast(static_pcast<UnicodeObject>(arg)->v); break;
    case Object::BOOL:  i = static_pcast<BoolObject>(arg)->v ? 1 : 0; break;
    case Object::FLOAT: i = (int64)(static_pcast<FloatObject>(arg)->v); break;
    default:
    THROW("int() can't convert type " << arg->typeName() << " to int");
    }
    return vm->makeFromT(i);
}

template<typename TC>
static bool boolFromStr(const basic_string<TC>& s) {
    return !s.empty() && toLower(s) != LITERAL_TSTR(TC, "false");
}

ObjRef OpImp::bool_(const ObjRef& arg) {
    if (arg->type == Object::BOOL)
        return arg;
    bool b;
    switch (arg->type) {
    case Object::STR:   b = boolFromStr(static_pcast<StrObject>(arg)->v); break;
    case Object::USTR:  b = boolFromStr(static_pcast<UnicodeObject>(arg)->v); break;
    case Object::INT:   b = (static_pcast<IntObject>(arg)->v == 0) ? false : true; break;
    case Object::FLOAT: b = (static_pcast<FloatObject>(arg)->v == 0.0) ? false : true; break;
    case Object::NONE:  b = false; break;
    case Object::LIST:
    case Object::TUPLE:
    case Object::DICT:
    case Object::STRDICT: b = (intLen(arg) > 0); break;
    default:
    THROW("bool() can't convert type " << arg->typeName() << " to bool");
    }
    return vm->makeFromT(b);
}

template<typename T>
bool extractOrNone(const ObjRef& o, T* v) {
    if (o->type == Object::NONE)
        return false;
    *v = extract<T>(o);
    return true;
}

ObjRef OpImp::apply_slice(const ObjRef& o, int* startp, int* endp) 
{
    if (o->type != Object::STR) {
        THROW("slice implemented only on strings. got " << o->typeName());
    }
    const string& s = static_pcast<StrObject>(o)->v;
    int start = 0, end = 0;
    if (startp != nullptr)
        start = *startp;
    if (endp != nullptr)
        end = *endp;
    else
        end = (int)s.length();

    if (start < 0)
        start = imax(0, (int)s.length() + start);
    if (end < 0)
        end = (int)s.length() + end;
    int len = imin((int)s.length(), end - start);
    string res;
    if (len > 0)
        res = s.substr(start, len);
    return vm->makeFromT(res);
}




static int64 notMinusOne(int64 h) {
    if (h == -1)
        return -2;
    return h;
}

int64 hashNum(const Object* arg) {
    if (arg == nullptr || arg->type == Object::NONE) {
        return 0x1234; // here None and nullptr object are treated the same
    }
    switch (arg->type) {
    case Object::BOOL: 
        return ((BoolObject*)arg)->v ? 1:0;
    case Object::INT: {
        int64 n = ((IntObject*)arg)->v;
        return notMinusOne(n);
    }
    case Object::FLOAT: {
        double f = ((FloatObject*)arg)->v;
        double ip = 0.0;
        if (modf(f, &ip) == 0.0)
            return notMinusOne((int64)f);
        else 
            return notMinusOne(*(int64*)&f);
    }
    case Object::USTR:
    case Object::STR: { // same as CPython
        const StrBaseObject* s = (StrBaseObject*)arg;
        if (s->size() == 0)
            return 0;
        int h = s->at(0) << 7;
        for(int i = 0; i < s->size(); ++i)
            h = (h * 1000003) ^ s->at(i);
        h = h ^ (int)s->size();
        return notMinusOne(h);
    }
    case Object::TUPLE: {
        const vector<ObjRef> v = ((ListObject*)arg)->v;
        int h = 0x345678;
        for(size_t i = 0; i < v.size(); ++i)
            h = (h * 1000003) ^ (int)hashNum(v[i].get());
        h = h ^ (int)v.size();
        return notMinusOne(h);
    }
    case Object::CLASS:
    case Object::INSTANCE: {
        return notMinusOne( ((int64)arg) >> 4 );
    }
    }
    THROW("Not hashable");
}

int64 hashNum(const ObjRef& argref) {
    return hashNum(argref.get());
}

int64 hashStr(const string& s) {
    StrObject so(s);
    return hashNum(&so);
}

int64 binOp(int64 a, int64 b, uchar op) {
    switch (op) {
    case BINARY_OR:  case INPLACE_OR: return a | b;
    case BINARY_AND: case INPLACE_AND: return a & b;
    case BINARY_XOR: case INPLACE_XOR: return a ^ b;
    case BINARY_RSHIFT: case INPLACE_RSHIFT: return (*(uint64*)&a) >> b; // avoid sign extension
    case BINARY_LSHIFT: case INPLACE_LSHIFT: return a << b;
    }
    THROW("Unexpected op");
}


void Frame::doOpcode( SetObjCallback& setObj )
{
    CodeDefinition& c = m_code->m_co;
    Instruction ins(c.co_code[m_lasti]);
    if (ins.opcode >= HAVE_ARGUMENT) {
        CHECK(m_lasti + 2 < c.co_code.size(), "Unexpected m_lasti");
        ins.param = (c.co_code[m_lasti+1] & 0xFF) | (c.co_code[m_lasti+2] << 8);
    }
    OpImp op(m_vm);

    switch (ins.opcode)
    {
    case LOAD_FAST:  // can be done with just the index
		// push(lookup(locals(), c.co_varnames(ins.param)));
        push(m_fastlocals[ins.param]);
        break;
    case STORE_FAST:
		// locals()[c.co_varnames(ins.param)] = pop();
        m_fastlocals[ins.param] = pop();
        break;
    case LOAD_NAME: {   // can be done with just the index
        const string& name = c.co_names[ins.param];
        ObjRef v = tryLookup(locals(), name);
        if (v.isNull()) {
            v = lookupGlobal(name);
            CHECK(!v.isNull(), "Name not found `" << name << "`");
        }
        push(v);
        break;
    }
    case STORE_NAME:
        locals()[c.co_names[ins.param]] = pop();
        break;
    case LOAD_CONST:
        push(c.co_consts[ins.param]);
        break;
    case COMPARE_OP: {
        ObjRef rhs = pop();
        ObjRef lhs = pop();
        push(alloc(new BoolObject(op.compare(lhs, rhs, ins.param))));
        break;
    }
    case POP_JUMP_IF_FALSE:
        if (asBool(pop()) == false) {
            m_lasti = ins.param;
            return;
        }
        break;
    case POP_JUMP_IF_TRUE: 
        if (asBool(pop()) == true) {
            m_lasti = ins.param;
            return;
        }
        break;
    case JUMP_IF_FALSE_OR_POP:
        if (asBool(top()) == false) {
            m_lasti = ins.param;
            return;
        }
        pop();
        break;
    case JUMP_IF_TRUE_OR_POP:
        if (asBool(top()) == true) {
            m_lasti = ins.param;
            return;
        }
        pop();
        break;
    case PRINT_ITEM: {
        ObjRef v = pop();
        if (m_vm->m_out) {
            print(v, *m_vm->m_out->m_os, false);
            (*m_vm->m_out->m_os) << " "; // space after each item in print
        }
        break;
    }
    case PRINT_NEWLINE:
        if (m_vm->m_out) {
            m_vm->m_out->endL();
        }
        break;
    case PRINT_EXPR: {
        ObjRef v = pop();
        if (m_vm->m_out) {
            print(v, *m_vm->m_out->m_os, true);
            m_vm->m_out->endL();
        }
        break;
    }
    case JUMP_FORWARD:
        m_lasti += ins.param;
        break;
    case INPLACE_ADD: 
    case BINARY_ADD: {
        ObjRef rhs = pop();
        ObjRef lhs = pop();
        push(op.add(lhs, rhs));
        break;
    }
    case INPLACE_MULTIPLY:
    case BINARY_MULTIPLY: {
        ObjRef rhs = pop();
        ObjRef lhs = pop();
        push(op.mult(lhs, rhs));
        break;
    }
    case INPLACE_SUBTRACT:
    case BINARY_SUBTRACT: {
        ObjRef rhs = pop();
        ObjRef lhs = pop();
        push(op.sub(lhs, rhs));
        break;
    }
    case INPLACE_DIVIDE:
    case BINARY_DIVIDE: {
        ObjRef rhs = pop();
        ObjRef lhs = pop();
        push(op.div(lhs, rhs));
        break;
    }
    case UNARY_POSITIVE:
        push(op.uplus(pop()));
        break;
    case UNARY_NEGATIVE:
        push(op.uminus(pop()));
        break;
    case UNARY_NOT:
        push(op.unot(pop()));
        break;


    case STORE_GLOBAL:
        globals()[c.co_names[ins.param]] = pop();
        break;
    case RETURN_VALUE:
        setObj(SLOT_RETVAL, pop());
        return; // don't increment m_lasti so we'll know where we returned for debugging
    case LOAD_GLOBAL: {
        const string& name = c.co_names[ins.param];
        ObjRef v = lookupGlobal(name);
        CHECK(!v.isNull(), "Unable to find global `" << name << "`");
        push(v);
        break;
    }
    case CALL_FUNCTION: {
        int posCount = ins.param & 0xFF;
        int kwCount = ins.param >> 8;
        push(m_vm->callFunction(*this, posCount, kwCount));
        break;
    }
    case POP_TOP:
        pop();
        break;
    case MAKE_FUNCTION: {
        CHECK(ins.param == 0, "default function arguments not supported");
        CodeObjRef code = checked_cast<CodeObject>(pop());
        if (!checkFlag(code->m_co.co_flags, (uint)MCO_GENERATOR))
            push(alloc(new FuncObject(code, m_module))); // function created in this module; 
        else
            push(alloc(new GeneratorObject(code, m_module, m_vm)));             
        break;
    }
    case LOAD_LOCALS:
        push(alloc(new StrDictObject(*m_locals)));
        break;
    case BUILD_CLASS: {
        StrDictObjRef methods = checked_cast<StrDictObject>(pop());
        TupleObjRef bases = checked_cast<TupleObject>(pop());
        CHECK(bases->size() <= 1, "multiple base classes not supported");
        StrObjRef name = checked_cast<StrObject>(pop());
        for(auto it = methods->v.begin(); it != methods->v.end(); ++it) {
            // create unbounded methods for class
            if (it->first == "__metaclass__") // can be a function but should not be made into a method.
                continue; 
            if (it->second->type == Object::FUNC) {
                auto cb = checked_dynamic_pcast<CallableObject>(it->second);
                methods->v[it->first] = (cb->m_isStaticMethod) ? ObjRef(cb) : alloc(new MethodObject(cb, InstanceObjRef()));
            }
        }
        ObjRef metahook = tryLookup(methods->v, "__metaclass__");
        if (metahook.isNull() && bases->size() > 0) {
            metahook = bases->v[0]->attr("__metaclass__");
        }
        ObjRef cls;
        if (!metahook.isNull()) {
            cls = m_vm->call(metahook, name, bases, methods);
            ClassObjRef clso = dynamic_pcast<ClassObject>(cls);
            if (!clso.isNull()) {
                clso->m_module = m_module;
            }
            // this implementation of metaclass is not quite full. there is no support for __new__ or __init__ of the metaclass. so basically it can just be a function.
        }
        else {
            cls = alloc(new ClassObject(methods, bases->v, name->v, m_module, m_vm));
        }
        push(cls);
        break;
    }
    case LOAD_ATTR: {
        const string& name = c.co_names[ins.param];
        ObjRef o = pop();
        CHECK(!o.isNull(), "attribute of None object " << name);
        IAttrable *attrb = o->tryAs<IAttrable>();
        if (attrb) {
            ObjRef r = attrb->attr(name);
            CHECK(!r.isNull(), "attribute `" << name << "` does not exist in " << stdstr(o, false) );
            push(r);
        }
        else if (PrimitiveAttrAdapter::adaptedType(o->type)) {
            push(m_vm->alloc(new PrimitiveAttrAdapter(o, name, m_vm))); 
        }
        else {
            THROW("Object of type " << o->typeName() << " does not have attribute `" << name << "`");
        }
        break;
    }
    case STORE_ATTR: {
        ObjRef o = pop();
        CHECK(!o.isNull(), "set attribute of None object");
        IAttrable *attrb = o->as<IAttrable>();
        attrb->setattr(c.co_names[ins.param], pop());
        break;
    }
    case BUILD_LIST: 
        push(op.makeListFromStack<ListObject>(*this, ins.param));
        break;
    case BUILD_TUPLE: 
        push(op.makeListFromStack<TupleObject>(*this, ins.param));
        break;    
    case STORE_SUBSCR: {
        ObjRef key = pop();
        ObjRef cont = pop();
        cont->as<ISubscriptable>()->setSubscr(key, pop());
        break;
    }
    case BINARY_SUBSCR: {
        ObjRef key = pop();
        ObjRef cont = pop();
        push( cont->as<ISubscriptable>()->getSubscr(key, m_vm) );
        break;
    }
    case SETUP_LOOP:
        pushBlock(ins.opcode, m_lasti + 3 + ins.param);
        break;
    case GET_ITER: {
        ObjRef a = pop();
        push(a->as<IIterable>()->iter(m_vm));
        break;
    }
    case FOR_ITER: {
        IIterator *it = top()->as<IIterator>();
        ObjRef nx;
        if (it->next(nx))
            push(nx);
        else {
            pop();
            m_lasti += ins.param;
        }
        break;
    }
    case JUMP_ABSOLUTE:
        m_lasti = ins.param;
        return;
    case POP_BLOCK:
        popBlock();
        break;
    case BUILD_MAP:
        push(alloc(new DictObject(m_vm)));
        break;
    case STORE_MAP: {
        ObjRef key = pop();
        ObjRef val = pop();
        DictObjRef m = checked_cast<DictObject>(top());
        m->setSubscr(key, val);
        break;
    }
    case IMPORT_NAME: {
        ObjRef fromlist = pop();
        ObjRef level = pop();
        ModuleObjRef m = m_vm->getModule(c.co_names[ins.param]);
        push(ObjRef(m));
        break;
    }
    case IMPORT_STAR: {
        ObjRef module = pop();
        // ignored
        break;
    }
    case RAISE_VARARGS: {
        ObjRef a,b,c;
        if (ins.param > 0) a = pop();
        if (ins.param > 1) b = pop();
        if (ins.param > 2) c = pop();
        throw PyRaisedException(a, b, c);
        break;
    }
    case BINARY_OR:
    case BINARY_AND:
    case BINARY_XOR:
    case BINARY_RSHIFT:
    case BINARY_LSHIFT:
    case INPLACE_OR:
    case INPLACE_AND:
    case INPLACE_XOR:
    case INPLACE_RSHIFT:
    case INPLACE_LSHIFT: {
        ObjRef b = pop();
        ObjRef a = pop();
        int64 ret = binOp(checked_cast<IntObject>(a)->v, checked_cast<IntObject>(b)->v, ins.opcode);
        push(m_vm->alloc(new IntObject(ret)));
        break;
    }
    case UNARY_INVERT: // bitwise not, operator ~
        push(m_vm->alloc(new IntObject( ~ checked_cast<IntObject>(pop())->v)));
        break;       
    case LIST_APPEND: { // for list comprehension
        ListObjRef lst = checked_cast<ListObject>(m_stack.peek(ins.param));
        lst->append(pop());
        break;
    }
    case UNPACK_SEQUENCE: {
        auto ito = pop()->as<IIterable>()->iter(m_vm); // save the iterator object
        IIterator *it = ito->as<IIterator>();
        ObjRef o;
        int i = 0;
        while (it->next(o)) {
            m_stack.pushAt(i++, o);
            CHECK(i <= ins.param, "too many values to unpack");
        }
        CHECK(ins.param == i, "too few values to unpack");
        break;
    }
    case ROT_TWO:  // used with when unpacking literals a,b=[1,2]
    case ROT_THREE: 
    case ROT_FOUR: 
        m_stack.pushAt(ins.opcode - 1, pop());
        break;
    case YIELD_VALUE:
        setObj(SLOT_YIELD, pop());
        break; // increment m_lasti so the next iteration will start where we left off
    case SLICE_0:
    case SLICE_1:
    case SLICE_2:
    case SLICE_3: {
        int a = 0, b = 0, *aptr = nullptr, *bptr = nullptr;
        if ((ins.opcode - SLICE_0) & 2) {
            b = extract<int>(pop());
            bptr = &b;
        }
        if ((ins.opcode - SLICE_0) & 1) {
            a = extract<int>(pop());
            aptr = &a;
        }
        auto obj = pop();
        push(op.apply_slice(obj, aptr, bptr));
        break;
    }
    case BUILD_SLICE: {
        int step = 0, a = 0, b = 0;
        bool has_step = false, has_a = false, has_b = false;
        if (ins.param == 3)
            has_step = extractOrNone<int>(pop(), &step);
        has_b = extractOrNone<int>(pop(), &b);
        has_a = extractOrNone<int>(pop(), &a);
        push(m_vm->alloc(new SliceObject(has_a, a, has_b, b, has_step, step)));
        break;
    }
    default:        
        THROW("Unknown opcode " << ins.opcode);
    }

    ++m_lasti;
    if (ins.opcode >= HAVE_ARGUMENT)
        m_lasti += 2;

};



void PyVM::validateCode(const CodeObjRef& obj) 
{
    // check that all the opcodes in all code constants are implemented.
    const string& c = obj->m_co.co_code;
    size_t i = 0;
    while (i < c.length()) {
        uchar opc = c[i];
        CHECK( (opFlags(opc) & IMPL) == IMPL, "Opcode `" << opName(opc) << "`(" << i << ") not implemented in `" << obj->m_co.co_name << "`");
        ++i;
        if (opc >= HAVE_ARGUMENT)
            i += 2;
    }
    for(const auto& o: obj->m_co.co_consts) {
        CodeObjRef co = dynamic_pcast<CodeObject>(o);
        if (!co.isNull())
            validateCode(co);
    }
}


ObjRef Builtins::get(const string& name) {
    return attr(name);
}

// wrap built-in C functions that are defined in OpImp. currently supports only taking one argument and returning one argument
ICWrapPtr makeOpImpCWrap(ObjRef(OpImp::*f)(const ObjRef&), PyVM* vm) {
    return makeWrap<1>(vm, [=](CallArgs& args)->ObjRef {
        OpImp ops(vm);
        return (ops.*f)(args[0]);
    });
}

ObjRef staticmethod(ObjRef r, PyVM* vm) { // for @Helper - just return what you get
    auto mr = checked_dynamic_pcast<CallableObject>(r);
    mr->m_isStaticMethod = true;
    return r;
}

int vmVersion() {
    return PYVM_VERSION;
}
int checkVmVersion(int req) {
    CHECK(vmVersion() >= req, "Wrong PyVM version. required=" << req << " actual=" << vmVersion() << " This means you're using the wrong connector version" );
    return vmVersion();
}


// works only for IAttrables at the moment
ObjRef getattr(const vector<ObjRef>& args) {
    CHECK(args.size() == 2 || args.size() == 3, "getattr expects 2 or 3 arguments, got " << args.size());
    string name = extract<string>(args[1]);
    IAttrable *attrb = args[0]->tryAs<IAttrable>();
    if (attrb != nullptr){
        ObjRef v = attrb->attr(name);
        if (!v.isNull())
            return v;
    }
    // not found
    if (args.size() == 3)
        return args[2];
    THROW("object has no attribute '" << name << "'");
}


ObjRef round(CallArgs& args, PyVM* vm) {
    CHECK(args.pos.size() == 1 , "round expects 1 argument, got " << args.pos.size());
    ObjRef arg = args[0];
    double res;
    if (arg->type == Object::FLOAT){
        double val = extract<double>(arg);
        res =  std::round(val);
        return vm->makeFromT<double>(res);
    }
    if (arg->type == Object::INT){
        int val = extract<int>(arg);
        return vm->makeFromT(val);
    }
    
    THROW("Invalid object type for round func '" << arg->typeName() << "'");
}



bool hasattr(ObjRef o, const string& name) {
    IAttrable *attrb = o->tryAs<IAttrable>();
    if (attrb != nullptr){
        ObjRef v = attrb->attr(name);
        return !v.isNull();
    }
    return false;
}
   

ObjRef strdict(CallArgs& args, PyVM* vm) {
    return vm->alloc(new StrDictObject(args.kw));
}

ObjRef xrange(CallArgs& args, PyVM* vm) {
    int start = 0, end = 0, step = 1;
    CHECK(args.kw.size() == 0, "xrange() can't get keyworkd arguments");
    int sz = args.pos.size();
    int i = 0;
    if (sz == 2 || sz == 3) {
        start = extract<int>(args.pos[0]);
        ++i;
        if (sz == 3) {
            step = extract<int>(args.pos[2]);
            CHECK(step != 0, "xrange() step should not be 0");
        }
    }
    else {
        CHECK(args.pos.size() == 1, "xrange() takes 1-3 int arguments");
    }
    end = extract<int>(args.pos[i]);
    return vm->alloc(new XRange(start, end, step, vm));
}


//basic logging object, contained in builtin globals, should be one instance per PyVm
class PyLogger
{
public:
    PyLogger(){}
    virtual ~PyLogger() {}

    void debug(const std::vector<ObjRef>& args){
        plog(LOGLEVEL_DEBUG, args);
    }
    void error(const std::vector<ObjRef>& args){
        plog(LOGLEVEL_ERROR, args);
    }


private:
    void plog(LogLevel level, const std::vector<ObjRef>& args)
    {        
        std::stringstream stream;
        for(const auto& objRef: args){
            print(objRef, stream, false); // no space between arguments (unlike CPython)
        }
        log(level, stream.str());
    }


};

#if 0
static NameDict makeDictFrom(const bpt::ptree& pt, PyVM* vm)
{
    NameDict d;
    for(const auto& v: pt) {
        if (v.second.size()) {
            d[v.first] = vm->alloc(new StrDictObject(makeDictFrom(v.second, vm))) ;
        }
        else {
            d[v.first] = vm->makeFromT(v.second.data());
        }
    }
    return d;
}


ObjRef OpImp::runtime_import(const ObjRef& filenameObj) 
{
    string filename = extract<string>(filenameObj);
    string modname = extractFileNameWithoutExtension(filename);
    ModuleObjRef mod = tryLookup(vm->modules(), modname); // don't want to read the file every time some file calls runtime_import on the same thing
    if (!mod.isNull())
        return ObjRef(mod);

    CHECK(bool(vm->m_runtimeImportCallback), "runtime_import: no runtime import defined for reading `" << filename << "`");

    const string* code = vm->m_runtimeImportCallback(filename);
    if (code == nullptr) 
        return vm->makeNone();

    auto ext = fs::path(filename).extension();
#if 0 // not supported since IntCore doesn't not have CPython so it can't compile python code
    if (ext == ".py") {
        unique_ptr<CodeDefinition> code(new CodeDefinition);
        if (!Bridge::compile(buf, filename, code.get())) {
            LOG_DEBUG("runtime_import: failed compiling file `", filepath, "`");
            return vm->makeNone();
        }
        return ObjRef(vm->importModule(*code, modname));
    } 
#endif
#if 0
    namespace bpt = boost::property_tree;
    if (ext == ".json") {
        boost::property_tree::ptree ptree;
        std::stringstream jsonStream;
        jsonStream << *code;
        try {
            bpt::json_parser::read_json(jsonStream, ptree);
        }
        catch (const std::exception& e) {
            LOG_ERROR("runtime_import: Caught exception parsing response JSON: ", e.what(), " :: ", *code);
            return vm->makeNone();
        }
        auto mod = vm->addEmptyModule(modname);
        mod->m_globals = makeDictFrom(ptree, vm);
        return ObjRef(mod);
    }
#endif
    else {
        LOG_ERROR("runtime_import: unknown format `", ext, "`"); 
    }

    return vm->makeNone();
}
#endif


// support for __metaclass__
ObjRef type_(CallArgs& args, PyVM* vm) {
    CHECK(args.pos.size() == 3, "Wrong number of arguments to type()");
    auto name = extract<string>(args.pos[0]);
    auto bases = checked_cast<TupleObject>(args.pos[1])->v;
    auto methods = checked_cast<StrDictObject>(args.pos[2]);
    // the module is set in BUILD_CLASS
    return vm->alloc(new ClassObject(methods, bases, name, ModuleObjRef(), vm)); // module will be inited in BUILD_CLASS
}



Builtins::Builtins(PyVM* vm) 
    :ModuleObject("__builtin__", vm) 
{
    addGlobal(m_vm->makeFromT(true), "True");
    addGlobal(m_vm->makeFromT(false), "False");

    auto pyLoggerCls = this->class_<PyLogger>("Logger");
    pyLoggerCls->def(&PyLogger::debug, "debug");
    pyLoggerCls->def(&PyLogger::error, "error");
    std::shared_ptr<PyLogger> loggerIns(new PyLogger);
    auto insObj = pyLoggerCls->instanceSharedPtr(loggerIns);
    addGlobal((ObjRef)insObj, "logging");

    // functions that need to know about the VM, are declared in OpImp.
    defIc("len", makeOpImpCWrap(&OpImp::len, m_vm));
    defIc("hash", makeOpImpCWrap(&OpImp::hash, m_vm));
    defIc("str", makeOpImpCWrap(&OpImp::str, m_vm));
    defIc("repr", makeOpImpCWrap(&OpImp::repr, m_vm));
    defIc("hex", makeOpImpCWrap(&OpImp::hex, m_vm));
    defIc("int", makeOpImpCWrap(&OpImp::int_, m_vm));
    defIc("bool", makeOpImpCWrap(&OpImp::bool_, m_vm));
    def("round", round);
    def("strdict", strdict);
    def("staticmethod", staticmethod);
    def("xrange", xrange);

    def("hasattr", hasattr);
    def("getattr", getattr);
    addGlobal(ObjRef(emptyClass("AttributeError")), "AttributeError");
    def("vmVersion", vmVersion);
    def("checkVmVersion", checkVmVersion);
    def("msecTime", msecTime);
	def("debugBreak", debugBreak);
	def("MessageBoxCall", MessageBoxCall);
//    defIc("runtime_import", makeOpImpCWrap(&OpImp::runtime_import, m_vm));
    def("type", type_);
}


