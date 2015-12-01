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

// included from objects.h


template<int Acount, typename TL>
struct CWrap : public ICWrap {
    CWrap(TL f) : m_f(f) {}
    virtual ObjRef call(CallArgs& args) {
        return m_f(args);
    }

    virtual int argsCount() { return Acount; }
    virtual const string& name() { return m_name; }
    virtual void setName(const string& name) {
        m_name = name;
    }

    TL m_f; // the lambda function
    string m_name;
};

// take a callable object that has argument (vector<ObjRef>& args), make an ICWrap* of it
template<int acount, typename LT>
ICWrapPtr makeWrap(PyVM* vm, const LT& l) {
    return vm->alloct<ICWrap>(new CWrap<acount, LT>(l));
}

class PyVM;

// ------------------------------------------ functions -----------------------------------------------

// see http://stackoverflow.com/questions/7858817/unpacking-a-tuple-to-call-a-matching-function-pointer
// for details about this crazy meta programming variadic template voodoo.
// its purpose is to build a parameter-pack so that a function with any number of arguments can be called using a vector that includes
//   ObjRefs of the arguments

// these definitions purpose is to generate seq<0,1,2,3,4>, given gens<5>
template<int ...> struct seq 
{};
template<int N, int ...S> struct gens : gens < N - 1, N - 1, S... > 
{};
template<int ...S> struct gens < 0, S... > 
{ typedef seq<S...> type; };

template <typename R, typename ...Args>
struct save_func_for_later 
{
    static std::tuple<Args...> the_params; // used for knowning the types of the pack
    R(*func)(Args...);

    R delayed_dispatch(CallArgs& a) {
        return callFunc(a, typename gens<sizeof...(Args)>::type() );
    }

    template<int ...S>
    R callFunc(CallArgs& a, seq<S...>) { // the parameter pack here is actually integers "0,1,2,3..."
        // Extract needs to know the type of each argument
        // -S-1 since the arguments are in last to first order
        return func(Extract< typename std::tuple_element<S, decltype(the_params)>::type >()(a[sizeof...(Args)-S-1]) ...);
    }
};

// wrap a function that returns R. return an ICWrap that wraps a lambda that (calls the function, given a CallArgs&).
template<typename R, typename ...As>
ICWrapPtr makeCWrap( R(*f)(As...), PyVM* vm) {
    return makeWrap<sizeof...(As)>(vm, [=](CallArgs& args)->ObjRef {
        save_func_for_later<R, As...> saved = { f };
        R ret = saved.delayed_dispatch(args);
        return vm->makeFromT<R>(ret);
    });
}

// same thing but with a function that returns void
template<typename ...As>
ICWrapPtr makeCWrap( void(*f)(As...), PyVM* vm) {
    return makeWrap<sizeof...(As)>(vm, [=](CallArgs& args)->ObjRef {
        save_func_for_later<void, As...> saved = { f };
        saved.delayed_dispatch(args);
        return vm->makeNone();
    });
}

// std::function
template <typename R, typename ...Args>
struct save_stdfunc_for_later
{
    static std::tuple<Args...> the_params; // used for knowning the types of the pack
    std::function<R(Args...)> func;

    R delayed_dispatch(CallArgs& a) {
        return callFunc(a, typename gens<sizeof...(Args)>::type());
    }

    template<int ...S>
    R callFunc(CallArgs& a, seq<S...>) { // the parameter pack here is actually integers "0,1,2,3..."
        // Extract needs to know the type of each argument
        // -S-1 since the arguments are in last to first order
        return func(Extract< typename std::tuple_element<S, decltype(the_params)>::type >()(a[sizeof...(Args)-S - 1]) ...);
    }
};


template<typename ...As>
ICWrapPtr makeCWrap( std::function<void(As...)> f, PyVM* vm) {
    return makeWrap<sizeof...(As)>(vm, [=](CallArgs& args)->ObjRef {
        save_stdfunc_for_later<void, As...> saved = { f };
        saved.delayed_dispatch(args);
        return vm->makeNone();
    });
}


// some specialized signatures needed in our code

template<>
inline ICWrapPtr makeCWrap( ObjRef(*f)(ObjRef, PyVM*), PyVM* vm) {
    return makeWrap<1>(vm, [=](CallArgs& args)->ObjRef {
        return f(args[0], vm);
    });
} 

template<>
inline ICWrapPtr makeCWrap( ObjRef(*f)(CallArgs&, PyVM*), PyVM* vm) {
    return makeWrap<-1>(vm, [=](CallArgs& args)->ObjRef {
        args.posReverse();
        ObjRef ret = f(args, vm);
        return ret;
    });
}

template<>
inline ICWrapPtr makeCWrap( ObjRef(*f)(const vector<ObjRef>&), PyVM* vm) {
    return makeWrap<-1>(vm, [=](CallArgs& args)->ObjRef {
        args.posReverse();
        // need to copy it to a real vector
        vector<ObjRef> argsCopy(args.pos.begin(), args.pos.end());
        ObjRef ret = f(argsCopy);
        return ret;
    });
}

inline ICWrapPtr makeCWrap(std::function<ObjRef(const vector<ObjRef>&, PyVM*)> f, PyVM* vm) {
    return makeWrap<-1>(vm, [=](CallArgs& args)->ObjRef {
        args.posReverse();
        // need to copy it to a real vector
        vector<ObjRef> argsCopy(args.pos.begin(), args.pos.end());
        ObjRef ret = f(argsCopy, vm);
        return ret;
    });
}

inline ICWrapPtr makeCWrap(std::function<ObjRef(ObjRef, PyVM*)> f, PyVM* vm) {
    return makeWrap<1>(vm, [=](CallArgs& args)->ObjRef {
        return f(args[0], vm);
    });
}


/// ----------------------------------------------- methods -------------------------------------------------
template<typename C>
C* extractCInst(const ObjRef o);

template<typename C>
std::shared_ptr<C> extractCSharedPtr(const ObjRef o);

// wrapper for a class method pointer that calls it with a parameter pack. see above
template <typename C, typename R, typename ...Args>
struct save_method_for_later
{
    static std::tuple<Args...> the_params;
    R(C::*func)(Args...);

    R delayed_dispatch(CallArgs& a, C* self) {
        return callFunc(typename gens<sizeof...(Args)>::type(), a, self);
    }

    template<int ...S>
    R callFunc(seq<S...>, CallArgs& a, C* self) {
        return (self->*func)( Extract< typename std::tuple_element<S, decltype(the_params)>::type >()(a[sizeof...(Args)-S - 1]) ...);
    }
};

// wrap a class C method pointer that returns R and takes any arguments
template<typename C, typename R, typename ...As>
ICWrapPtr makeCWrap(R(C::*f)(As...), PyVM* vm) {
    return makeWrap<sizeof...(As) + 1>(vm, [=](CallArgs& args)->ObjRef {
        save_method_for_later<C, R, As...> saved = { f };
        C* self = extractCInst<C>(args[sizeof...(As)]);
        R ret = saved.delayed_dispatch(args, self);
        return vm->makeFromT<R>(ret);
    });
}

// wrap a class C method points that returns void and takes any arguments.
template<typename C, typename ...As>
ICWrapPtr makeCWrap(void(C::*f)(As...), PyVM* vm) {
    return makeWrap<sizeof...(As) + 1>(vm, [=](CallArgs& args)->ObjRef {
        save_method_for_later<C, void, As...> saved = { f };
        C* self = extractCInst<C>(args[sizeof...(As)]);
        saved.delayed_dispatch(args, self);
        return vm->makeNone();
    });
}



// class C return R and take a vector of ObjRef
template<typename C, typename R>
ICWrapPtr makeCWrap( R(C::*f)(const vector<ObjRef>&), PyVM* vm) {
    return makeWrap<-1>(vm, [=](CallArgs& args)->ObjRef {
        CHECK(args.pos.size() >= 1, "No self argument");
        C* self = extractCInst<C>(args.pos.back()); // self is the last argument.
        args.pos.pop_back();
        args.posReverse();
        vector<ObjRef> argsCopy(args.pos.begin(), args.pos.end());
        R ret = (self->*f)(argsCopy);
        return vm->makeFromT<R>(ret);
    });
}

// class C return void and take a vector of ObjRef
template<typename C>
ICWrapPtr makeCWrap( void(C::*f)(const vector<ObjRef>&), PyVM* vm) {
    return makeWrap<-1>(vm, [=](CallArgs& args)->ObjRef {
        CHECK(args.pos.size() >= 1, "No self argument");
        C* self = extractCInst<C>(args.pos.back()); // self is the last argument.
        args.pos.pop_back();
        args.posReverse();
        vector<ObjRef> argsCopy(args.pos.begin(), args.pos.end());
        (self->*f)(argsCopy);
        return vm->makeNone();
    });
}

template<typename C>
inline ICWrapPtr makeCWrap( ObjRef(C::*f)(CallArgs&, PyVM*), PyVM* vm) {
    return makeWrap<-1>(vm, [=](CallArgs& args)->ObjRef {
        CHECK(args.pos.size() >= 1, "No self argument");
        C* self = extractCInst<C>(args.pos.back());
        args.pos.pop_back();
        args.posReverse();
        ObjRef ret = (self->*f)(args, vm);
        return ret;
    });
}
