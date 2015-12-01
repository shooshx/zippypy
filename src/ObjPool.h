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
#include "except.h"
#include "defs.h"
#include "log.h"


template<typename T>
class ObjPool;

template<typename T>
class PoolPtr 
{
public:
    PoolPtr() : m_p(nullptr) {}
    PoolPtr(const PoolPtr& o) : m_p(o.m_p) {
        if (m_p != nullptr) {
            ++m_p->count.count;
        }
    }
    PoolPtr(PoolPtr&& o) {
        m_p = o.m_p;
        o.m_p = nullptr;
    }
    explicit PoolPtr(T* p) :m_p(p) {
        if (m_p != nullptr)
            ++m_p->count.count;
    }
    template<typename U>
    explicit PoolPtr(const PoolPtr<U>& o) : m_p(nullptr) {
        if (o.get() == nullptr)
            return;
        m_p = dynamic_cast<U*>(o.get());
        CHECK(m_p != nullptr, "Incompatible conversion");
        ++m_p->count.count;
    }

    ~PoolPtr() {
        reset();
    }
    PoolPtr& operator=(const PoolPtr& o) {
        if (&o == this)
            return *this;
        reset();
        m_p = o.m_p;
        if (m_p != nullptr) {
            ++m_p->count.count;
        }
        return *this;
    }
    void reset() {
        if (m_p == nullptr)
            return;
        if (--m_p->count.count == 0) {
            m_p->count.pool->remove(m_p);
        }
        m_p = nullptr;
    }
    T* get() const {
        return m_p;
    }
    T* operator->() const {
        return m_p;
    }
    const T& operator*() const {
        return *m_p;
    }
    T& operator*() {
        return *m_p;
    }
    bool isNull() const {
        return m_p == nullptr;
    }
    int use_count() const {
        if (m_p == nullptr)
            return 0;
        return m_p->count.count;
    }

private:
    T* m_p;

    friend class ObjPool<T>;
};

template<typename U, typename T>
PoolPtr<U> static_pcast(const PoolPtr<T>& o) {
    return PoolPtr<U>(static_cast<U*>(o.get()));
}
template<typename U, typename T>
PoolPtr<U> dynamic_pcast(const PoolPtr<T>& o) {
    return PoolPtr<U>(dynamic_cast<U*>(o.get()));
}


// doubly linked list that hold objects of type T. T objects should have public data member count of type RefCount<T>
template<typename T>
class DList {
private:
    T *m_head, *m_tail;
    int m_size;

public:
    struct Entry {
        Entry() : next(nullptr), prev(nullptr) {}
        T *next, *prev;
    };

    struct iterator {
        iterator(T* p) : ptr(p) {}
        T* get() { return ptr; }
        bool operator!=(iterator rhs) { return ptr != rhs.ptr; }
        bool operator==(iterator rhs) { return ptr == rhs.ptr; }
        iterator& operator++() {// prefix
            CHECK(ptr != nullptr, "unexpected nullptrptr");
            ptr = ptr->count.ent.next;
            return *this;
        }
        T* ptr;
    };

    DList() : m_head(nullptr), m_tail(nullptr), m_size(0) {
        // two dummy nodes that are always there.
        m_head = new T();
        m_tail = new T();
        m_head->count.ent.next = m_tail;
        m_tail->count.ent.prev = m_head;
    }
    ~DList() {
        if (m_size != 0) {
            LOG_ERROR("Object pool not empty. size=", m_size, " global objects?");
        }
        delete m_head;
        delete m_tail;
    }
    // push at head
    void push_front(T* v) { 
        Entry* n = &v->count.ent;// new Entry(m_head, v, m_head->next);
        n->prev = m_head;
        n->next = m_head->count.ent.next;
        m_head->count.ent.next->count.ent.prev = v;
        m_head->count.ent.next = v;
        ++m_size;
    }
    iterator begin() {
        return iterator(m_head->count.ent.next);
    }
    iterator end() {
        return iterator(m_tail); // one after last
    }
    void erase(T* v) {
        if (v == nullptr)
            return;
        CHECK(v != m_head && v != m_tail, "Unexpected state DList");
        Entry* e = &v->count.ent;
        e->next->count.ent.prev = e->prev;
        e->prev->count.ent.next = e->next;
        --m_size;
    }
    int size() const {
        return m_size;
    }
};

template<typename T>
struct RefCount {
    RefCount() : count(0), pool(nullptr)
    {}
    ~RefCount() {
      //  delete remover;
    }
    int count;
    ObjPool<T> *pool;
    typename DList<T>::Entry ent;
private:
    DISALLOW_COPY_AND_ASSIGN(RefCount);
};


//#define OBJ_CNT_PRINT_DELTA_MS  10*60*1000 //10 mins

/** A pool of reference counter objects of type T
 * T should have a public data member named 'count' of type RefCount<T>
 * The pool can hold objects that inherit from T
 * it holds a doubly linked list of T objects. the next and prev pointes are inside RefCount::ent
 */
template<typename T>
class ObjPool 
{
public:
    ObjPool() :m_hadRemove(false) //, m_lastTimePrintedObjCount(0)
    {}
    // takes ownership of p
    PoolPtr<T> add(T* p) {  
        PoolPtr<T> ret(p);
        m_objs.push_front(p);
        p->count.pool = this; 
        return ret;
    }

    void remove(T* v) {
        //printObjCntIfNeeded(); disabled since it garbages the log. instead, print the count before destruction of the pool in PyVM::clear()
        m_objs.erase(v);
        delete v;
        m_hadRemove = true;
    }

 /*   void printObjCntIfNeeded(){
        uint64 currentMsec = os::OSUtils::msecTimeFast();
        if ((currentMsec - m_lastTimePrintedObjCount) > OBJ_CNT_PRINT_DELTA_MS){
            LOG_DEBUG("There are ", m_objs.size(), " objects in pyvm object pool");
            m_lastTimePrintedObjCount = currentMsec;
        }
    }*/

    template<typename F>
    bool foreach(const F& f) {
        PoolPtr<T> p(m_objs.begin().ptr);
        typename DList<T>::iterator it(p.get());
        while( it != m_objs.end()) {
            // need to get the reference to the next one before it is possibly destroyed (delay its destruction)
            ++it;
            PoolPtr<T> nextp( (it != m_objs.end()) ? (it.ptr) : nullptr ); 
            if (!f(p))
                return false;
            p = nextp;
        }
        return true;
    }

    int size() const { 
        return m_objs.size();
    }
    T* listHead() {
        return m_objs.begin().get();
    }   

    // the problem this avoids is that if the object we're on is just freed 
    // since it was part of a circle, we can't get to the next one
    template<typename F>
    void gradForeach(const F& f) {
        while (true) {
            m_hadRemove = false;
            auto it = m_objs.begin();
            for(; it != m_objs.end(); ++it) {
                f(PoolPtr<T>(it.get()));
                if (m_hadRemove) // maybe someone removed the iterator.
                    break;
            }
            if (it == m_objs.end())
                break;
        }
    }

    int countRefs() {
        int sum = 0;
        foreach([&](const PoolPtr<T>& p)->bool { 
            sum += p->count.count - 1;
            return true;
        });
        return sum;
    }

private:
    DList<T> m_objs;
    bool m_hadRemove; // used in gradForeach
    //uint64 m_lastTimePrintedObjCount;

};

