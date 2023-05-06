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
#include "PyVM.h"
#include "log.h"
#include "utils.h"
#include "PyCompile.h"

#include <sstream>
#include <fstream>


// int g_maxStackSize;

//map<string, int> g_lookups;

template ObjRef PyVM::makeFromT(const PoolPtr<Object>& v);



void Frame::argsFromStack(Frame& from, int posCount, int kwCount, CallArgs& args) {
    for(int i = 0; i < kwCount; ++i) {
        ObjRef v = from.pop();
        ObjRef k = from.pop();
        args.kw[extract<string>(k)] = v;
    }
    args.pos.reserve(posCount + 1); // avoid push_back allocating - all positional args + possible self
    for(int i = 0; i < posCount; ++i) {
        args.pos.push_back(from.pop());
    }
    from.pop(); // the callable object
}

/*
static void testNoneAndSet(NameDict& dest, const string& name, const ObjRef& v) {
    auto it = dest.find(name);
    CHECK(it == dest.end(), "Key already there `" << name << "`");
    dest[name] = v;
}
*/

static void testNoneAndSet(Frame::TFastLocalsList& dest, int index, const ObjRef& v) {
    CHECK(index < dest.size() && dest[index].isNull(), "Argument already threre " << index);
    dest[index] = v;
}

void Frame::localsFromStack(Frame& from, ObjRef self, int posCount, int kwCount) 
{
    const CodeDefinition& c = code()->m_co;
    int selfCount = self.isNull()?0:1;

    auto& dest = m_fastlocals; // size initialized with co_nlocals which includes the arguments and the *argv
    TupleObjRef starArgs;

    CHECK(checkFlag(c.co_flags, (uint)MCO_NEWLOCALS), "co_flags doesn't have CO_NEWLOCALS"); // using fast locals
    CHECK(!checkFlag(c.co_flags, (uint)MCO_VARKEYWORDS), "CO_VARKEYWORDS not supported");
    if (!checkFlag(c.co_flags, (uint)MCO_VARARGS)) {
        CHECK(posCount + kwCount + selfCount == c.co_argcount, "unexpected number of arguments " << posCount + kwCount + selfCount << "!=" << c.co_argcount);
    }
    else {
        starArgs = m_vm->alloct(new TupleObject);
        testNoneAndSet(dest, c.co_argcount, static_pcast<Object>(starArgs));
    }
    // go over the args in the stack, match to locals
/*    for(int i = 0; i < kwCount; ++i) { // arguments passed by key-value  TBD3
        ObjRef val = from.pop();
        string aname = checked_cast<StrObject>(from.pop())->v;
        int ci = 0;
        for(; ci < (int)c.co_argcount; ++ci) {
            const string& cname = c.co_varnames[ci];
            if (cname == aname) {
                //testNoneAndSet(dest, cname, val);
                testNoneAndSet(dest, ci, val);
                break;
            }
        }
        CHECK(ci < (int)c.co_argcount, "Unknown key argument name " << aname);
    }*/
    for(int i = 0; i < posCount; ++i) { // arguments passed by position
        int posi = selfCount + posCount - i - 1;
        ObjRef a = from.pop();
        //testNoneAndSet(dest, c.co_varnames(posi), from.pop());
        if (posi < (int)c.co_argcount)
            testNoneAndSet(dest, posi, a);
        else
            starArgs->prepend(a); // arguments received by *args
    }
    if (selfCount) {
        //testNoneAndSet(dest, c.co_varnames(0), self);
        testNoneAndSet(dest, 0, self);
    }
    // check that all args were assigned: (redundant)
//     for(int i = 0; i < (int)c.co_argcount; ++i) {
//         CHECK(dest.find(c.co_varnames(i)) != dest.end(), "argument did not get a value " << c.co_varnames(i));
//     }
    from.pop(); // the callable object
}

NameDict& Frame::globals() { 
    return m_module->m_globals; 
}

ObjRef Frame::lookupGlobal(const string& name) {
    ObjRef ret = tryLookup(globals(), name);
    if (!ret.isNull())
        return ret;
    return m_vm->m_builtins->get(name);
}

ObjRef Frame::run() {
    EObjSlot retslot = SLOT_RETVAL;
    ObjRef retval;
    SetObjCallback setObj = [&](EObjSlot slot, const ObjRef& v) {
        retslot = slot; 
        retval = v;
    };

    do {
        doOpcode(setObj);
    } while (retval.isNull());

    m_vm->m_lastFramei = m_lasti;
    m_retslot = retslot; 
    return retval;
}

void Frame::clear() {
    m_code.reset();
    m_module.reset();
    m_stack.clear(); // vector clear
    m_fastlocals.clear(); // container clear
}

void Frame::setCode(const CodeObjRef& code) {
    m_code = code;
    m_fastlocals.resize(m_code->m_co.co_nlocals);
}


ObjRef FuncObject::call(Frame& from, Frame& frame, int posCount, int kwCount, const ObjRef& self) {
    frame.setCode(m_code);
    frame.localsFromStack(from, self, posCount, kwCount);
    return frame.run();
}


ObjRef CFuncObject::call(Frame& from, Frame& frame, int posCount, int kwCount, const ObjRef& self) {
    CallArgs args;
    frame.argsFromStack(from, posCount, kwCount, args);
    if (!self.isNull())
        args.pos.push_back(self);
    int fcount = wrap->argsCount();
    if (fcount != -1) // means variable number of arguments, see cfunc.h
        CHECK(args.pos.size() == fcount && args.kw.size() == 0, "Wrong number of arguments " << args.pos.size() << "!=" << fcount);
    ObjRef ret = wrap->call(args);
    return ret;
}


ObjRef MethodObject::call(Frame& from, Frame& frame, int posCount, int kwCount, const ObjRef& inself) {
    // calling unbounded method is possible with the first argument as self. unlike CPython, the first argument is not checked to be of the right class
    return m_func->call(from, frame, posCount, kwCount, ObjRef(m_self));
}

/*
void ClassObject::makeMethods(const InstanceObjRef& i) {
    // first let the parent put its method
    if (!m_base.isNull())
        m_base->makeMethods(i);
    // then this class's method, possibly overrriding
    if (!m_methods.isNull()) {
        for(auto it = m_methods->v.begin(); it != m_methods->v.end(); ++it) {
            if (it->second->type == METHOD)
                i->m_dict[it->first] = m_vm->alloc(new MethodObject( ((MethodObject*)it->second.get())->m_func ,i));
        }
    }
}
*/

ObjRef ClassObject::call(Frame& from, Frame& frame, int posCount, int kwCount, const ObjRef& self) 
{
    InstanceObjRef i(frame.m_vm->alloct(new InstanceObject(ClassObjRef(this))));
    // set up methods with self
    //makeMethods(i);
    ObjRef inito = i->simple_attr("__init__");
    if (!inito.isNull()) {
        MethodObjRef init = checked_cast<MethodObject>(inito);
        ObjRef ret = init->call(from, frame, posCount, kwCount, ObjRef());
        CHECK(ret.isNull() || ret->type == Object::NONE, "__init__() must return None"); 
    } // we can either call __init__() or the cpp ctor, not both since the arguments are removed from the stack
    else if (!m_cwrap.isNull() && !m_cwrap->m_ctor.isNull()) {
        CallArgs args;
        frame.argsFromStack(from, posCount, kwCount, args);
        i->m_cwrap = m_cwrap->m_ctor->construct(m_vm, args);
    }

    return ObjRef(i);
}

//--------------------------------------- VM ------------------------------------------------------

PyVM::PyVM() 
    : m_out(new LoggerPrinter(LOGLEVEL_DEBUG))
    , m_currentFrame(nullptr), m_lastFramei(-1)
    , m_noneObject(alloc(new Object)), m_trueObject(alloc(new BoolObject(true))), m_falseObject(alloc(new BoolObject(false)))
{
    m_defaultModule = alloct(new ModuleObject("__main__", this));
    m_builtins = alloct(new Builtins(this));
}

PyVM::~PyVM() {
   // m_defaultModule
    clear();
}

string PyVM::instructionPointer() {
    std::ostringstream os;
    os << "(" << m_lastFramei << ") ";
    Frame* f = m_currentFrame;
    while (f != nullptr) {
        os << f->m_lasti << " < ";
        f = f->m_lastFrame;
    }
    return os.str();
}

// where all functions go to and out of
ObjRef PyVM::callFunction(Frame& from, int numArg, int kwCount) {
    ObjRef func = from.m_stack.peek(numArg + kwCount*2);

    func->checkProp(Object::ICALLABLE);
    CallableObjRef funcref = static_pcast<CallableObject>(func);

    NameDict locals;
    Frame frame(this, funcref->m_module, &locals); // module needed for globals
    try {
        return funcref->call(from, frame, numArg, kwCount, ObjRef());
    }
    catch(PyException& e) {
        std::ostringstream s;
        s << "in " << funcref->funcname() << " ";
        if (!frame.code().isNull())
            s << frame.code()->lineFromIndex(frame.m_lasti) << " ";
        s << "[" << frame.m_lasti << "]";
        e.addTrack(s.str());
        throw;
    }

}



ObjRef PyVM::eval(const CodeObjRef& code, ModuleObjRef module) {
    validateCode(code);
    Frame frame(this, module, &module->m_globals); // in the module top level execution locals()==globals()
    frame.setCode(code);
    return frame.run();
}

ModuleObjRef PyVM::addEmptyModule(const string& name) {
    ModuleObjRef module(alloct(new ModuleObject(name, this)));
    module->m_globals["__name__"] = alloc(new StrObject(name));
    m_modules[name] = module;
    return module;
}

/*
ModuleObjRef PyVM::importModule(const CodeDefinition& moduleDef, const string& name)
{    // co_name would always be <module> so there's no sense in this
    string extName = name.empty() ? moduleDef.co_name : name;
    auto module = addEmptyModule(extName);
    eval(moduleDef, module);
    return module;
}
*/

ModuleObjRef PyVM::importPycStream(istream& is, const string& path, bool hasHeader)
{
    ObjRef obj = CodeDefinition::parsePyc(is, this, hasHeader);
    auto code = checked_cast<CodeObject>(obj);
    string modName;
    if (!code->m_co.co_filename.empty())
        modName = extractFileNameWithoutExtension(code->m_co.co_filename); 
    else if (!code->m_co.co_name.empty() && code->m_co.co_name != "<module>")
        modName = code->m_co.co_name;
    else {
        CHECK(!path.empty(), "Missing module name");
        modName = extractFileNameWithoutExtension(path);
    }
    auto module = addEmptyModule(modName);
    eval(code, module);
    return module;
}

ModuleObjRef PyVM::importPycFile(const string& pycpath) 
{
    ifstream ifs(pycpath, ios::binary);
    if (!ifs.good())
        return ModuleObjRef();
    return importPycStream(ifs, pycpath, true);
}

ModuleObjRef PyVM::importPycBuf(const string& pyctext, bool hasHeader)
{
    istringstream iss(pyctext);
    return importPycStream(iss, string(), hasHeader);
}


ModuleObjRef PyVM::getModule(const string& name) {
    ModuleObjRef mod = tryLookup(m_modules, name);
    if (!mod.isNull())
        return mod;
    if (m_importCallback) {
        auto impret = m_importCallback(name);
        if (impret.first.get() != nullptr)
            return importPycStream(*impret.first.get(), name, impret.second);
    }
    THROW("Did not find module " << name);
}

vector<string> split(const string &s, char delim) {
    vector<string> elems;
    stringstream ss(s);
    string item;
    while (std::getline(ss, item, delim)) {
        elems.push_back(item);
    }
    return elems;
}

// restrictions of names:
// - code can't import one module to another
// - can't call one class from another
ObjRef PyVM::lookupQual(const string& name, ModuleObjRef* mod) 
{
    ModuleObjRef module = m_defaultModule; 
    string funcname = name;
    if (funcname.find('.') != string::npos) {
        vector<string> names = split(funcname, '.');
        CHECK(names.size() == 2, "Only one module nesting allowed: " << name);
        module = getModule(names[0]);
        funcname = names[1];
    }
    ObjRef func = module->attr(funcname);
    if (!func.isNull()) {
        if (mod)
            *mod = module;
        return func;
    }
    func = m_builtins->attr(funcname);
    if (!func.isNull()) {
        if (mod)
            *mod = static_pcast<ModuleObject>(m_builtins);
        return func;
    }
    THROW("Did not find function " << name);
}

// from cpp code
ObjRef PyVM::callv(const string& funcname, const vector<ObjRef>& posargs) {
    //ModuleObjRef module;
    ObjRef func = lookupQual(funcname, nullptr); // parse module.funcname and get the object
    return callv(func, posargs);
}

// from cpp code
ObjRef PyVM::callv(const ObjRef& ofunc, const vector<ObjRef>& posargs) {
    ofunc->checkProp(Object::ICALLABLE);
    CallableObjRef func = static_pcast<CallableObject>(ofunc);
    Frame dummyFrame(this, func->m_module, nullptr);
    dummyFrame.push(ofunc);
    for(auto it = posargs.begin(); it != posargs.end(); ++it)
        dummyFrame.push(*it);
    return callFunction(dummyFrame, (int)posargs.size(), 0);
}

void PyVM::addGlobalFunc(const CodeDefinition& cdef) {
    m_defaultModule->addGlobal(alloc(new FuncObject(alloct(new CodeObject(cdef)), m_defaultModule)), cdef.co_name);
}

void PyVM::memDump(ostream& os) {
    os << "Memory Dump: " << m_alloc.size() << " objects\n";
    m_alloc.foreach([&](const ObjRef& o)->bool {
        os << "(" << o.use_count() << ") " << o->typeName() << ": " << stdstr(o, true) << "\n";
        return true;
    });
}

void PyVM::clear() {
    LOG_DEBUG("PyVM clear with ", m_alloc.size(), " objects");
    m_modules.clear();
    m_alloc.gradForeach([&](const ObjRef& o) {
        o->clear();
    });

}

void PyVM::addBuiltin(const ClassObjRef& v) {
    addBuiltin(v->funcname(), ObjRef(v) );
}

#if USE_CPYTHON

void PyVM::runInteractive() 
{
    while (true) {
        string pycline = getInteractiveLine();           
        if (pycline.empty())
            break;
        istringstream iss(pycline);
        ObjRef c = CodeDefinition::parsePyc(iss, this, false);
        eval(dynamic_pcast<CodeObject>(c), mainModule());
    }
}

#endif