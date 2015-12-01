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
#include "ObjPool.h"
#include "baseObject.h"
#include "VarArray.h"
#include "log.h"
#include "CodeDefinition.h"

#include <iostream>
#include <functional>
#include <vector>
#include <map>
#include <sstream>
#include <memory>

#ifdef USE_BOOST
  #include <boost/container/flat_map.hpp>
#endif

#pragma warning (disable: 4355) // 'this' : used in base member initializer list

using namespace std;


class ModuleObject;
typedef PoolPtr<ModuleObject> ModuleObjRef;
class Builtins;
typedef PoolPtr<Builtins> BuiltinsObjRef;
class CodeObject;
typedef PoolPtr<CodeObject> CodeObjRef;
class ClassObject;
typedef PoolPtr<ClassObject> ClassObjRef;

typedef map<string, ModuleObjRef> ModulesDict;

#ifdef USE_BOOST
typedef boost::container::flat_map<string, ObjRef> NameDict;
#else
typedef map<string, ObjRef> NameDict;
#endif

typedef map<int64, ObjRef> IntDict;


string stdstr(const ObjRef& vref, bool repr=false);
void print(const ObjRef& vref, ostream& out, bool repr);
int vmVersion();

class PyVM;
class Frame;

enum EObjSlot {
    SLOT_RETVAL = 0,
    SLOT_YIELD = 1
};

typedef std::function<void(EObjSlot s, const ObjRef&)> SetObjCallback;

//extern int g_maxStackSize;

template<typename T>
class Stack {
public:
    void push(const T& ref) {
        m_stack.push_back(ref);
        //if (g_maxStackSize < m_stack.size())
        //    g_maxStackSize = (int)m_stack.size();
    }
    // push an element some distance from the top. pushAt(0,r) is equivalent to push(r)
    void pushAt(int fromTop, const T& ref) {
        CHECK((int)m_stack.size() >= fromTop, "pushAt underflow");
        //m_stack.insert(m_stack.size() - fromTop, ref);
        m_stack.insert(m_stack.end() - fromTop, ref);
    }
    T pop() {
        CHECK(m_stack.size() > 0, "stack underflow");
        ObjRef r = m_stack.back();
        m_stack.pop_back();
        return r;
    }

    // i - offset from the top. 0=TOS
    T peek(int i) {
        CHECK((int)m_stack.size() > i, "peek underflow");
        return m_stack[m_stack.size() - 1 - i];
    }
    T top() {
        return peek(0);
    }
    int size() const {
        return (int)m_stack.size();
    }
    void clear() {
        m_stack.clear();
    }
    vector<T> data() { // for testers
        vector<T> r;
        m_stack.foreach([&](const T& v) { r.push_back(v); });
        //r = m_stack;
        return r; 
    }
    void reserve(int sz) {
        m_stack.reserve(sz);
    }
private:
    // on average operation the stack does not go over 6 items
    VarArray<T, 6> m_stack;
    //vector<T> m_stack;
};

extern map<string, int> g_lookups;

template<typename T>
T tryLookup(const map<string, T>& d, const string& name) {
    auto it = d.find(name);
    if (it == d.end())
        return T();
    return it->second;
}

inline ObjRef tryLookup(const NameDict& d, const string& name) {
    //++g_lookups[name];
    auto it = d.find(name);
    if (it == d.end())
        return ObjRef();
    return it->second;
}

template<typename T>
T lookup(const map<string, T>& d, const string& name) {
    auto it = d.find(name);
    CHECK(it != d.end(), "KeyError: could not find `" << name << "`");
    return it->second;
}

inline ObjRef lookup(const NameDict& d, const string& name) {
    //++g_lookups[name];
    auto it = d.find(name);
    CHECK(it != d.end(), "KeyError: could not find `" << name << "`");
    return it->second;
}

//struct nullptrType {};
//template<typename T>
//struct IsType { enum { nullptrType = false; }; };
//template<> struct IsType<nullptrType> { enum { nullptrType = true }; };


class StreamPrinter {
public:
    StreamPrinter(ostream* os = nullptr) : m_os(os) {}
    virtual ~StreamPrinter() {}
    virtual void endL() {
        if (m_os != nullptr)
            (*m_os) << std::endl;
    }
public:
    ostream* m_os;
};

class LoggerPrinter : public StreamPrinter {
public:
    LoggerPrinter(LogLevel lvl) : m_lvl(lvl) {
        m_os = &m_s;
    }
    virtual void endL() {
        log(m_lvl, m_s.str());
        m_s.str(string()); // clear the buffer
    }
private:
    DISALLOW_COPY_AND_ASSIGN(LoggerPrinter); // needed because we're taking m_s address
    std::ostringstream m_s;
    LogLevel m_lvl;
};


class PyVM {
public:
    PyVM();
    ~PyVM();

    void setStdout(ostream* s) {
        m_out.reset(new StreamPrinter(s));
    }

    void addGlobalFunc(const CodeDefinition& code);

    ObjRef callv(const ObjRef& func, const vector<ObjRef>& posargs);
    ObjRef callv(const string& funcname, const vector<ObjRef>& posargs);

    template<typename ...Args>
    ObjRef call(const ObjRef& func, Args&&... args) {
        vector<ObjRef> argv({ makeFromT(args)... });
        return callv(func, argv);
    }

    template<typename ...Args>
    ObjRef call(const string& funcname, Args&&... args) {
        vector<ObjRef> argv({ makeFromT(args)... });
        return callv(funcname, argv);
    }

    ObjRef eval(const CodeObjRef& code, ModuleObjRef module);

    ModuleObjRef addEmptyModule(const string& name);
   // ModuleObjRef importModule(const CodeDefinition& moduleDef, const string& name);
    ModuleObjRef getModule(const string& name);

    ModuleObjRef importPycStream(istream& is, const string& path, bool hasHeader);
    ModuleObjRef importPycFile(const string& pycpath);
    ModuleObjRef importPycBuf(const string& pyctext, bool hasHeader = false);

//     void addDummyModule(const string& name) {
//         m_modules[name] = ModuleObjRef();
//     }

    void clear();
    
    ObjRef alloc(Object *o) {
        return m_alloc.add(o);
    }
    template<typename T>
    PoolPtr<T> alloct(T* t) {
        Object* o = static_cast<Object*>(t);
        ObjRef r = m_alloc.add(o);
        return static_pcast<T>(r);
    }

    template<typename T>
    ObjRef makeFromT(T v);
    template<typename T> 
    ObjRef makeFromT(const PoolPtr<T>& v) {  return ObjRef(v); }
    ObjRef makeNone(void) {
       return m_noneObject;
    }
    template<typename ObjT, typename InitT> // give the object type to construct
    ObjRef makeFromT2(const InitT& v);
    
    template<typename A1, typename A2>
    ObjRef makeTuple(const A1& a1, const A2& a2);

    // memory introspection
    void memDump(ostream& os);
    int countObjects() {
        return m_alloc.size();
    }

    void validateCode(const CodeObjRef& def); // in instruction.cpp

    ModuleObjRef mainModule() {
        return m_defaultModule;
    }
    template<typename T> // T should be some Object
    void addBuiltin(const string& name, const PoolPtr<T>& v);
    void addBuiltin(const ClassObjRef& v); // name taken from the class

    string instructionPointer();
    ObjRef lookupQual(const string& name, ModuleObjRef* mod);
    ObjPool<Object>& objPool() { 
        return m_alloc; 
    }
    const ModulesDict& modules() const {
        return m_modules;
    }
    
    // the import callback returns a pair with the stream to read the pyc from and a bool that says if the stream has a header
    typedef std::function<pair<unique_ptr<istream>,bool>(const string&)> TImportCallback;

    void setImportCallback(TImportCallback callback) {
        m_importCallback = callback;
    }
    Frame* currentFrame() {
        return m_currentFrame;
    }

    void runInteractive();

private:
    DISALLOW_COPY_AND_ASSIGN(PyVM);
    friend class Frame;
    friend class OpImp;

    ObjRef callFunction(Frame& from, int posCount, int kwCount);

private:
    ObjPool<Object> m_alloc; // must be first member so it would be destructed last, after all references are down

    std::unique_ptr<StreamPrinter> m_out; 
    ModuleObjRef m_defaultModule; // module of __main__
    BuiltinsObjRef m_builtins;
    ModulesDict m_modules; // this is sys.modules
    ObjRef m_noneObject, m_trueObject, m_falseObject;
    TImportCallback m_importCallback;

    // used for debugging
    Frame* m_currentFrame; // managed by Frame object c'tor and d'tor
    int m_lastFramei; // the m_lasti of the last frame that returned
};


template<> inline ObjRef PyVM::makeFromT(ObjRef v) {  
    return v; 
}
int64 hashStr(const string& s);

// this class is istantiated on the stack to save the current state of the object pool
// when it goes out of scope it calls 'clear()' for all objects created after it's instantiation
// This clears out any reference circles which occured in the run of the python code to avoid memory leaks.
// NOTICE: this also means that there should not be globally referenced objects created in that scope.
class StateClearer {
public:
    StateClearer(PyVM* vm) :m_vm(vm), m_savedHead(vm->objPool().listHead()) {        
    }
    ~StateClearer() {
        try {
            bool tillend = m_vm->objPool().foreach([this](const ObjRef& o)->bool {
                if (o.get() == m_savedHead.get())
                    return false; // stop iteration
                o->clear();
                return true;
            });
            if (tillend)
                LOG_ERROR("!!!!! StateClearer went too far (did not find savedHead)");
        }
        catch(const PyException& e) {
            LOG_ERROR("!!!!! Caught exception in StateClearer ", e.what());
        }
    }
private:
    ObjRef m_savedHead;
    PyVM* m_vm;
};


struct Block {
    Block(int _type, int _handler, int _stackSize) :type(_type), handlerAddr(_handler), stackSize(_stackSize) {}
    int type;
    int handlerAddr;
    int stackSize;
};

struct CallArgs {
    // usually there are no more than 4 positional arguments for an internal function
    typedef VarArray<ObjRef, 4> TPosVector;

    TPosVector pos;
    NameDict kw;

    ObjRef operator[](int i) {
        return pos[i];
    }
    void posReverse() {
        std::reverse(pos.begin(), pos.end());
    }
};


class Frame 
{
public:
    Frame(PyVM* vm, const ModuleObjRef& module, NameDict *locals)
        :m_lasti(0), m_vm(vm), m_module(module), m_locals(locals), m_retslot(SLOT_RETVAL)
    {
        m_lastFrame = m_vm->m_currentFrame;
        m_vm->m_currentFrame = this;
        //m_stack.reserve(6);
    }
    ~Frame() {
        m_vm->m_currentFrame = m_lastFrame;
    }

    void push(const ObjRef& ref) {
        m_stack.push(ref);
    }
    ObjRef pop() {
        return m_stack.pop();
    }
    ObjRef top() {
        return m_stack.top();
    }
    ObjRef alloc(Object *o) {
        return m_vm->alloc(o);
    }

    void pushBlock(int type, int handler) {
        m_blocks.push_back(Block(type, handler, m_stack.size()));
    }
    Block popBlock() {
        CHECK(m_blocks.size() > 0, "block stack underflow");
        Block r = m_blocks.back();
        m_blocks.pop_back();

        CHECK(r.stackSize <= m_stack.size(), "Wrong stack size");
        while (r.stackSize < m_stack.size())
            m_stack.pop();

        return r;
    }

    void doOpcode(SetObjCallback& setObj);
    ObjRef lookupGlobal(const string& name);

    void argsFromStack(Frame& from, int posCount, int kwCount, CallArgs& args);
   // void localsFromArgs(const vector<ObjRef>& args);
    void localsFromStack(Frame& from, ObjRef self, int posCount, int kwCount);

    NameDict& locals() { return *m_locals; }
    NameDict& globals();
    ObjRef run();

    void clear();

    void setCode(const CodeObjRef& code);
    const CodeObjRef& code() {
        return m_code;
    }

public:
    uint m_lasti; // index in the code object string of the curret instruction
    NameDict *m_locals; // points to a dict on the stack
    PyVM* m_vm;
    ModuleObjRef m_module; // for globals, needs an objet to reference, m_globals is not an object
    vector<Block> m_blocks; // every for,try,except,finally,with is a block
    Frame* m_lastFrame;  // previous frame on the stack
    Stack<ObjRef> m_stack; // value stack
    EObjSlot m_retslot; // the slot filled with the return value after a function call returns

	// "usually" there are no more than 5 locals in a function
    typedef VarArray<ObjRef, 5> TFastLocalsList;

private:
    CodeObjRef m_code;
    TFastLocalsList m_fastlocals;

};

