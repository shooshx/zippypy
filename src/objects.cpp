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
#pragma warning (disable: 4996) // copy unsafe
#include "objects.h"
#include "PyVM.h"
#include "OpImp.h"


using namespace std;

const char* Object::typeName(Type type) {
    switch (type) {
    case NONE:  return "NoneType";
    case BOOL:  return "bool";
    case INT:   return "int";
    case STR:   return "str";
    case TUPLE: return "tuple";
    case LIST:  return "list";
    case DICT:  return "dict";
    case CODE:  return "code";
    case MODULE: return "module";
    case FUNC:  return "function";
    case CLASS: return "classobj";
    case INSTANCE: return "instance";
    case METHOD: return "method";
    case USTR: return "unicode";
    case SLICE: return "slice";
    default:
        return "<unknown>";
    }
}

template<typename T>
static Object::Type typeValue() {
    class NoType;
    return sizeof(NoType); // compile error for types that are not instantiated in object.cpp
}
#define MAKE_TYPEVALUE(obj, val) template<> Object::Type Object::typeValue<obj >()   { return Object::val; }
MAKE_TYPEVALUE(BoolObject, BOOL)
MAKE_TYPEVALUE(StrObject,   STR)
MAKE_TYPEVALUE(IntObject,   INT)
MAKE_TYPEVALUE(TupleObject, TUPLE)
MAKE_TYPEVALUE(ListObject,  LIST)
MAKE_TYPEVALUE(DictObject, DICT)
MAKE_TYPEVALUE(StrDictObject, STRDICT)
MAKE_TYPEVALUE(CodeObject, CODE)
MAKE_TYPEVALUE(ModuleObject, MODULE)
MAKE_TYPEVALUE(FuncObject, FUNC)
MAKE_TYPEVALUE(CFuncObject, FUNC)
MAKE_TYPEVALUE(ClassObject, CLASS)
MAKE_TYPEVALUE(InstanceObject, INSTANCE)
MAKE_TYPEVALUE(MethodObject, METHOD)
MAKE_TYPEVALUE(IteratorObject, ITERATOR)
//MAKE_TYPEVALUE(GenericIterObject, ITERATOR)
MAKE_TYPEVALUE(UnicodeObject, USTR)
MAKE_TYPEVALUE(Object, NONE)
MAKE_TYPEVALUE(FloatObject, FLOAT)
MAKE_TYPEVALUE(SliceObject, SLICE)
#undef MAKE_TYPEVALUE


InstanceObjRef ClassObject::createInstance() {
    InstanceObjRef i(m_vm->alloct(new InstanceObject(ClassObjRef(this))));
    //makeMethods(i);
    return i;
}

ObjRef ModuleObject::defIc(const string& name, const ICWrapPtr& ic) {
    ic->setName(name);
    return addGlobal(m_vm->alloc(new CFuncObject(ic)), name);
}

ClassObjRef ModuleObject::emptyClass(const string& name) {
    ClassObjRef ret = m_vm->alloct(new ClassObject(m_vm->alloct(new StrDictObject()), vector<ObjRef>(/*bases*/), name, ModuleObjRef(this), m_vm));
    addGlobal(ObjRef(ret), name);
    return ret;
}

string MethodObject::funcname() const {
    if (m_self.isNull())
        return "nullptrSELF." + m_func->funcname();
    if (m_self->m_class.isNull())
        return "nullptrCLASS." + m_func->funcname();
    return m_self->m_class->funcname() + "." + m_func->funcname();
}

void Builtins::add(const string& name, const ObjRef& v) {
    addGlobal(v, name);;
}



// see "All about co_lnotab" http://svn.python.org/projects/python/branches/pep-0384/Objects/lnotab_notes.txt
int CodeObject::lineFromIndex(int qi) const {
    const string& tab = m_co.co_lnotab;
    if ((tab.size() % 2) != 0)
        return -1; //strange format
    int lineno = m_co.co_firstlineno, addr = 0;
    for(int i = 0; i < (int)tab.size(); i += 2) {
        addr += tab[i];
        if (addr > qi)
            return lineno;
        lineno += tab[i + 1];
    }
    return lineno;
}


int ISubscriptable::extractIndex(const ObjRef& key, size_t size) {
    int i = extract<int>(key);
    if (i < 0)
        i = (int)size + i;
    CHECK(i >= 0 && i < (int)size, "Out of range index " << i << ":" << size);
    return i;
}

//------------------------------------------------------------------------------------------

ObjRef UnicodeObject::fromStr(const ObjRef& s, PyVM* vm) {
    return vm->alloc(new UnicodeObject(checked_dynamic_pcast<StrObject>(s)));
}

ObjRef ClassObject::attr(const string& name) {
    ObjRef v = tryLookup(m_dict->v, name);
    if (!v.isNull())
        return v;
    if (!m_base.isNull())
        return m_base->attr(name);
    return v;
}

PoolPtr<MethodObject> MethodObject::bind(const InstanceObjRef& newself) {
    CHECK(m_self.isNull(), "Can't bind an already bound method");
    CHECK(!newself.isNull(), "Can't bind to a nullptr reference");
    // get to the vm through the class
    return newself->m_class->m_vm->alloct(new MethodObject(m_func, newself)); 
}


ObjRef InstanceObject::simple_attr(const string& name) 
{
    // try in the instance
    ObjRef v = tryLookup(m_dict, name);
    if (!v.isNull())
        return v;
    // try in the class
    if (!m_class.isNull()) {
        v = m_class->attr(name); 
        if (!v.isNull()) {
            // if it's a method, need to create a new bounded method object
            // the methods are not saved in the instance object to avoid a cycle instance->method->(m_self)instance
            // this is the way it is done in CPython
            if (v->type == Object::METHOD) {
                MethodObjRef m = checked_cast<MethodObject>(v);
                // same as bind(), without the checks
                return m_class->m_vm->alloc(new MethodObject(m->m_func, InstanceObjRef(this)));
            }
            return v;
        }
    }
    return ObjRef();
}


ObjRef InstanceObject::attr(const string& name) 
{
    ObjRef v = simple_attr(name);
    if (!v.isNull())
        return v;

    ObjRef getatt = simple_attr("__getattr__");
    if (!getatt.isNull()) {
        vector<ObjRef> gargs;
        gargs.push_back(m_class->m_vm->makeFromT(name));
        try {
            return m_class->m_vm->callv(getatt, gargs);
        }
        catch (const PyRaisedException& e) {
            auto inst = checked_cast<InstanceObject>(e.inst);
            if (inst->m_class->m_name == "AttributeError")
                return ObjRef(); // indicates the attribute does not exist
            throw;
        }
    }
    return v;
}


int toSlash(int c) {
    if (c == '\\')
        return '/';
    return c;
}
template<typename T, typename TOp>
static T transformed(const T& str, TOp& op) {
    T ret = str;
    transform(ret.begin(), ret.end(), ret.begin(), op);
    return ret;
}

enum StrOp { SO_EQUALS, SO_CONTAINS, SO_BEGINS, SO_ENDS };


static void checkArgCountS(Object::Type t, const CallArgs::TPosVector& args, int c, const string& name) {
    CHECK(args.size() == c, "method " << Object::typeName(t) << "." << name << " takes exactly " << c << "arguments (" << args.size() << " given)");
}
void PrimitiveAttrAdapter::checkArgCount(const ObjRef obj, const CallArgs::TPosVector& args, int c) {
    checkArgCountS(obj->type, args, c, m_name);
}


// casei: case insensitive compare
// slashi: slash insensitive compare
template<typename TC>
static bool stringQuery(const ObjRef& thisv, const CallArgs::TPosVector& args, StrOp op, StrModifier mod) 
{
    checkArgCountS(Object::STR, args, 1, "cmp"); // STR is just for logging
    const basic_string<TC>* a = extractStrPtr<TC>(args[0], mod);
    const basic_string<TC>* v = extractStrPtr<TC>(thisv, mod); 

    if (a->size() > v->size())
        return false;
    switch (op) {
    case SO_CONTAINS:
        return v->find(*a) != basic_string<TC>::npos;
    case SO_BEGINS:
        return memcmp(v->data(), a->data(), a->size() * sizeof(TC)) == 0;
    case SO_ENDS:
        return memcmp(v->data() + v->size() - a->size(), a->data(), a->size() * sizeof(TC)) == 0;
    case SO_EQUALS:
        return *a == *v;
    }
    return false;
}


// takes 2 optional arguments
// arg 1: separator (if not given, using white-space)
// arg 2: if separator is given, should we add empty elements. default is true (while-space split always ignores empty elements)
template<typename TC>
static ObjRef split(const ObjRef& o, const CallArgs::TPosVector& args, PyVM* vm) 
{
    basic_string<TC> s = extract<basic_string<TC>>(o);
    ListObjRef ret = vm->alloct(new ListObject);
    if (args.size() == 1 || args.size() == 2) 
    {
        basic_string<TC> sep = extract<basic_string<TC> >(args[0]);
        bool addEmpty = true; // the default documented behaviour 
        if (args.size() == 2)
            addEmpty = extract<bool>(args[1]);
        size_t next = 0, current = 0;
        do {
            next = s.find(sep, current);
            basic_string<TC> v = s.substr(current, next - current);
            if (!v.empty() || addEmpty) {
                ret->append(vm->alloc(new PSTROBJ_TYPE(TC)(v)));
            }
            current = next + sep.size();
        } while (next != string::npos);
    }
    else if (args.size() == 0)
    {
        size_t next = 0, current = 0;
        do {
            next = s.find_first_of( LITERAL_TSTR(TC, " \t\n\r"), current);
            basic_string<TC> v = s.substr(current, next - current);
            if (!v.empty()) {
                ret->append(vm->alloc(new PSTROBJ_TYPE(TC)(v)));
            }
            current = next + 1;
        } while (next != string::npos);
    }
    else 
        THROW("Unexpected number of arguments (" << args.size() << ") in split()");

    return ObjRef(ret);
}



template<typename TC>
static ObjRef join(const ObjRef& s, const CallArgs::TPosVector& args, PyVM* vm) {
    checkArgCountS(Object::STR, args, 1, "join");
    auto ito = args[0]->as<IIterable>()->iter(vm); // save a reference to the object
    auto it = ito->as<IIterator>();
    ObjRef o;
    if (!it->next(o))
        return vm->makeFromT(basic_string<TC>());
    ObjRef result = o;
    OpImp ops(vm);
    while (it->next(o)) {
        CHECK(o->type == Object::STR || o->type == Object::USTR, "join(): wrong type expected: str or unicode got:" << o->typeName());
        result = ops.add(result, s);
        result = ops.add(result, o);
    }
    return result;
}

#ifdef _WINDOWS
#define PATH_MODIFIER_ STRMOD_PATH
#else
#define PATH_MODIFIER_ STRMOD_NONE
#endif


// to add a string method name, 
// - add it to "string_method_names.txt", 
// - enable it in the build (set "Exclude from build to 'No') doesn't matter which build config
// - compile "string_method_names.txt" to create a new "gen_string_method_names.h"
// - disable it again in the build (set "Exclude from build to 'Yes')
// the custom build step that generates this file is currently only generated in the windows build since there is still no scons rule for it
// this is why "gen_string_method_names.h" is in source control.
// after re-generating this file, make sure you commit the change in the genrated file as well.
#include "gen_string_method_names.h"

template<typename TC>
ObjRef PrimitiveAttrAdapter::stringMethod(const ObjRef& obj, const CallArgs::TPosVector& args) 
{
    EStringMethods nameEnum = stringMethod_enumFromStr(m_name);
    switch(nameEnum) 
    {
    case STRM_CONTAINS:
        return m_vm->makeFromT(stringQuery<TC>(obj, args, SO_CONTAINS, STRMOD_NONE));        
    case STRM_BEGINSWITH:
    case STRM_STARTSWITH:
        return m_vm->makeFromT(stringQuery<TC>(obj, args, SO_BEGINS, STRMOD_NONE));
    case STRM_ENDSWITH:
        return m_vm->makeFromT(stringQuery<TC>(obj, args, SO_ENDS, STRMOD_NONE));
    case STRM_EQUALS:
        return m_vm->makeFromT(stringQuery<TC>(obj, args, SO_EQUALS, STRMOD_NONE));

    case STRM_PATHCONTAINS:
        return m_vm->makeFromT(stringQuery<TC>(obj, args, SO_CONTAINS, PATH_MODIFIER_));        
    case STRM_PATHBEGINSWITH:
        return m_vm->makeFromT(stringQuery<TC>(obj, args, SO_BEGINS, PATH_MODIFIER_));        
    case STRM_PATHENDSWITH:
        return m_vm->makeFromT(stringQuery<TC>(obj, args, SO_ENDS, PATH_MODIFIER_));
    case STRM_PATHEQUALS:
        return m_vm->makeFromT(stringQuery<TC>(obj, args, SO_EQUALS, PATH_MODIFIER_));

    case STRM_ICONTAINS:
        return m_vm->makeFromT(stringQuery<TC>(obj, args, SO_CONTAINS, STRMOD_CASEI));        
    case STRM_IBEGINSWITH:
        return m_vm->makeFromT(stringQuery<TC>(obj, args, SO_BEGINS, STRMOD_CASEI));        
    case STRM_IENDSWITH:
        return m_vm->makeFromT(stringQuery<TC>(obj, args, SO_ENDS, STRMOD_CASEI));
    case STRM_IEQUALS:
        return m_vm->makeFromT(stringQuery<TC>(obj, args, SO_EQUALS, STRMOD_CASEI));

    case STRM_SPLIT:
        return split<TC>(obj, args, m_vm);
    case STRM_JOIN:
        return join<TC>(obj, args, m_vm);
    case STRM_LOWER:
        return m_vm->makeFromT(toLower(extract<basic_string<TC>>(obj)));
    case STRM_C_PTR:
        return m_vm->makeFromT( static_cast<StrBaseObject*>(m_obj.get())->ptr());
    case STRM_GLOB_PTR: {
        // leak this string
        basic_string<TC> *new_st = new basic_string<TC>(extract<basic_string<TC>>(obj));
        return m_vm->makeFromT((size_t)new_st->data()); 
    }
    case STRM_STRIP: {
		basic_string<TC> nonConstS = extract<basic_string<TC>>(obj);
		if (args.size() == 0){
			trimSpaces(nonConstS);
			return m_vm->makeFromT(nonConstS);
		}
		strip<TC>(nonConstS, extract<basic_string<TC>>(args[0]));
		return m_vm->makeFromT(nonConstS);
	}
    } // switch
    
    THROW("Unknown string method " << m_name);
   
}

// if anything is unicode, everything should be unicode
ObjRef PrimitiveAttrAdapter::stringMethodConv(const ObjRef& obj, CallArgs::TPosVector& args) {
    bool uni = obj->type == Object::USTR;
    for(auto ait = args.begin(); !uni && ait != args.end(); ++ait)
        uni |= (*ait)->type == Object::USTR;
    if (!uni)
        return stringMethod<char>(obj, args);
    // otherwise, conver all strings to unicode

    return stringMethod<wchar_t>(obj, args);
}

ObjRef PrimitiveAttrAdapter::listMethod(const ObjRef& obj, CallArgs::TPosVector& args) {
    auto l = static_pcast<ListObject>(obj);
    if (m_name == "append") {
        checkArgCount(obj, args, 1);
        l->append(args[0]);
        return m_vm->makeNone();
    }
    if(m_name == "pop"){
        checkArgCount(obj, args, 1);        
        return l->pop(extract<int>(args[0]));
    }
    if (m_name == "extend") {
        checkArgCount(obj, args, 1);
        auto ito = args[0]->as<IIterable>()->iter(m_vm); // save the iterator reference
        auto it = ito->as<IIterator>();
        ObjRef o;
        while (it->next(o))
            l->v.push_back(o);
        return m_vm->makeNone();
    }
    THROW("Unknown list method " << m_name);
}

template<typename TDict>
ObjRef PrimitiveAttrAdapter::dictMethod(const ObjRef& obj, CallArgs::TPosVector& args) 
{
    auto d = static_pcast<TDict>(obj);
    if (m_name == "keys") {
        auto ret = m_vm->alloct(new ListObject);
        for(auto it = d->v.begin(); it != d->v.end(); ++it)
            ret->append(m_vm->makeFromT(it->first));
        return ObjRef(ret);
    }
    if (m_name == "values"){
        auto ret = m_vm->alloct(new ListObject);
        for (auto it = d->v.begin(); it != d->v.end(); ++it)
            ret->append(m_vm->makeFromT(it->second));
        return ObjRef(ret);
    }
    if (m_name == "pop") {
        checkArgCount(obj, args, 1);
        return d->pop((args[0]));
    }
    if (m_name == "iteritems") {
        return m_vm->alloc(new MapKeyValueIterObject<TDict>(d, m_vm));
    }
    THROW("Unknown strdict method " << m_name);
}



ObjRef PrimitiveAttrAdapter::call(Frame& from, Frame& frame, int posCount, int kwCount, const ObjRef& self) 
{
    CallArgs args;
    frame.argsFromStack(from, posCount, kwCount, args);

    args.posReverse();  // arguments come in reverse order, we can just edit in place since it's not used after.
    
    switch (m_obj->type) {
    case Object::STR:
    case Object::USTR:
        return stringMethodConv(m_obj, args.pos);
    case Object::LIST:
        return listMethod(m_obj, args.pos);
    case Object::STRDICT:
        return dictMethod<StrDictObject>(m_obj, args.pos);
    case Object::DICT:
        return dictMethod<DictObject>(m_obj, args.pos);
    // if you add a case here, you also need to at it in adaptedType()
    default:
        THROW("Unknown primitive method " << m_name << " of " << Object::typeName(m_obj->type));
    }

};


