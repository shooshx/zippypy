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
#include "ObjPool.h"
#include "except.h"

struct Object;
typedef PoolPtr<Object> ObjRef;
class Frame;
class PyVM;


// Base Object inherits from these but and throw an exception
// objects that are really supposed to inherit from these have IATTRABLE oe ICALLABLE
// in typeProp. this is to avoid a consty dynamic_cast<> and to be able to implement my
// own cheap dynamic_cast<> using this bitmask.

struct IAttrable {
    // try to lookup the name, if not found, return nullptr ref
    virtual ObjRef attr(const string& name) = 0;
    virtual void setattr(const string& name, const ObjRef& o) = 0;
};

struct ICallable {
    virtual ObjRef call(Frame& from, Frame& frame, int posCount, int kwCount, const ObjRef& self) = 0;
    virtual string funcname() const = 0; // for logging
};



//#define PYVM_COUNT_OBJECTS

#ifdef PYVM_COUNT_OBJECTS
// to debug object creation and deletion define the above #define and
// declare this variable in your tester code
extern int g_pyvmObjectCount; 
#endif

// this is a superset of ConstValue from proto
struct Object : public IAttrable, public ICallable
{
public:
    enum Type {
        NONE = 0,
        BOOL = 1,
        INT = 2,
        STR = 3,
        TUPLE = 4,
        LIST = 5,
        DICT = 6,
        CODE = 7,
        MODULE = 8,
        FUNC = 9,
        CLASS = 10,
        INSTANCE = 11,
        METHOD = 12,
        FLOAT = 13,
        CINSTANCE_WRAP = 14,
        ITERATOR = 15,
        STRDICT = 16,
        PRIMITIVE_ADAPTER = 17,
        USTR = 18,
        CCTOR_WRAP = 19,
        GENERATOR = 20,
        CFUNC_WRAP = 21,
        SLICE = 22,
        XRANGE = 23
    };
    enum TypeProp {
        IATTRABLE = 1, // see tryAs<IAttrable>
        ICALLABLE = 2
    };

    RefCount<Object> count;
    Type type;
    int typeProp; // used instead of costy dynamic_cast

public:
    virtual ~Object() {
        #ifdef PYVM_COUNT_OBJECTS
        g_pyvmObjectCount--;
        #endif
    }
    // clear all internal references of an object, to cleanup any reference cycle
    // must only call 'reset()' of held ObjRefs. never recursively call other objects clear() since that could cause infinite recursion
    Object(Type _type = NONE) :type(_type), typeProp(0) {
        #ifdef PYVM_COUNT_OBJECTS
        g_pyvmObjectCount++;
        #endif
    }
    Object(Type _type, uint _typeProp) :type(_type), typeProp(_typeProp) {
        #ifdef PYVM_COUNT_OBJECTS
        g_pyvmObjectCount++;
        #endif
    }

    virtual void clear() {}

    virtual ObjRef attr(const string& name) {
        THROW("Unimplemented Object::attr");
    }
    virtual void setattr(const string& name, const ObjRef& o) {
        THROW("Unimplemented Object::setattr");
    }
    virtual ObjRef call(Frame& from, Frame& frame, int posCount, int kwCount, const ObjRef& self) {
        THROW("Unimplemented Object::call");
    }
    virtual string funcname() const  {
        THROW("Unimplemented Object::funcname");
    }
public:
    template<typename T>
    static Object::Type typeValue();

    inline void checkType(Object::Type t) {
        CHECK(type == t, "wrong type expected:" << typeName(t) << " got:" << typeName());
    }
    inline void checkProp(Object::TypeProp p) {
        CHECK(checkFlag(typeProp, (int)p) == true, "wrong prop expected:" << typeProp << " got:" << p);
    }
    template<typename T> // T is a primitive type like int or string
    inline void checkTypeT();

    const char* typeName() {
        return typeName(type);
    }
    static const char* typeName(Type t);
    template<typename T>
    static const char* typeName() {
        return typeName( Object::typeValue<T>() );
    }

    // get a pointer to an interface from the object.
    // Notice not to loose the original reference to the object for as long as this is needed.
    template<typename T>
    T* as() {
        T* ret = dynamic_cast<T*>(this);
        CHECK(ret != nullptr, "Wrong type");
        return ret;
    }
    // used for interfaces
    template<typename T>
    T* tryAs() {
        return dynamic_cast<T*>(this);
    }
private:
    DISALLOW_COPY_AND_ASSIGN(Object);
    
};

template<> inline IAttrable* Object::tryAs<IAttrable>() {
    if (checkFlag(typeProp, (int)IATTRABLE)) { 
        return static_cast<IAttrable*>(this);
    }
    return nullptr;
}
template<> inline ICallable* Object::tryAs<ICallable>() {
    if (checkFlag(typeProp, (int)ICALLABLE)) {
        return static_cast<ICallable*>(this);
    }
    return nullptr;
}
