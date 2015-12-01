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
#include "except.h"


// based on QVarLengthArray from QT
// doesn't yet support move semantics
template<typename T, int StaticSize>
class VarArray {
public:
    VarArray() 
        : m_size(0), m_alloc(StaticSize), m_ptr(reinterpret_cast<T*>(m_arrbuf)), m_allocbuf(nullptr)
    {}
    ~VarArray() {
        clear();
    }

    typedef T* iterator;
    typedef T value_type;
    typedef const T* const_iterator;

    void reserve(int newAlloc) {
        if (newAlloc > m_alloc) // means we need to allocate dynamically
            realloc(m_size, newAlloc);
    }
    // use this to pre-init a fixed number of default constructed objects
    void resize(int newSize) {
        realloc(newSize, imax(newSize, m_alloc));
    }

    void clear() {
        T *i = m_ptr + m_size;
        while (i != m_ptr) {
            --i;
            i->~T();
        }
        if (m_allocbuf != nullptr) {
            delete[] m_allocbuf;
            m_allocbuf = nullptr;
        }
        m_size = 0;
        m_alloc = StaticSize;
        m_ptr = reinterpret_cast<T*>(m_arrbuf);
    }

    void push_back(const T& v) {
        incAlloc();
        int idx = m_size++;
        // placement new running copy constructor
        new (m_ptr + idx) T(v);
    }
    
    void pop_back() {
        ASSERT(m_size > 0, "Unexpected size 0");
        --m_size;
        (m_ptr + m_size)->~T();
        if (m_alloc > StaticSize && m_size < m_alloc / 2)
            realloc(m_size, m_alloc / 2);
    }
    
    T& back() {
        return m_ptr[m_size - 1];
    }
    const T& back() const {
        return m_ptr[m_size - 1];
    }

    void insert(iterator before, const T& v) {
        int beforeIdx = int(before - m_ptr);
        ASSERT(beforeIdx >= 0 && beforeIdx <= m_size, "Unexpected beforeIdx ");

        incAlloc();
        ++m_size;

        T *base = m_ptr + beforeIdx;
        T *dst = m_ptr + m_size;
        T *src = dst - 1;
        new (src) T(); // new element at the end of the line, will be assigned to shortly

        while (src != base) {
            --dst;
            --src;
            *dst = *src;
        }
        // just copied from src which is base, can set value to base
        *base = v;
    }

    int size() const {
        return m_size;
    }


    T& operator[](int idx) {
        ASSERT(idx >= 0 && idx < m_size, "Unexpected idx");
        return m_ptr[idx];
    }
    const T& operator[](int idx) const {
        ASSERT(idx >= 0 && idx < m_size, "Unexpected idx");
        return m_ptr[idx];
    }

    template<typename F>
    void foreach(F& f) const {
        for(int i = 0; i < m_size; ++i) {
            f(m_ptr[i]);
        }
    }

    iterator begin() {
        return m_ptr;
    }
    iterator end() {
        return m_ptr + m_size;
    }
    const_iterator begin() const {
        return m_ptr;
    }
    const_iterator end() const {
        return m_ptr + m_size;
    }

private:
    // inc by one. make sure m_size can be incremented by 1
    void incAlloc() {
        if (m_size == m_alloc)
            realloc(m_size, m_size * 2);
    }

    // always for m_allocated
    void realloc(int newSize, int newAlloc) 
    {
        CHECK(newAlloc >= newSize, "Unexpected alloc size");

        T *oldPtr = m_ptr;
        int oldSize = m_size;
        const int copySize = imin(newSize, oldSize);

        if (newAlloc != m_alloc)  // need to copy stuff
        {
            m_allocbuf = new char[newAlloc * sizeof(T)];
            m_ptr = reinterpret_cast<T*>(m_allocbuf);
            m_size = 0;
            m_alloc = newAlloc;

            // copy all the old elements
            try {
                while (m_size < copySize) {
                    new (m_ptr + m_size) T(*(oldPtr + m_size)); // copy c'tor. can throw
                    (oldPtr + m_size)->~T();
                    m_size++;
                }
            }
            catch(...) {
                // the c'tor is going to clean up to m_size but we have stuff in the old array that needs to go as well
                int sClean = m_size;
                while (sClean < oldSize)
                    (oldPtr + (sClean++))->~T();
                if (oldPtr != (T*)(m_arrbuf) && oldPtr != m_ptr)
                    delete[] reinterpret_cast<char*>(oldPtr);
                throw;
            }
        }
        m_size = copySize;
        // destroy remaining old objects
        while (oldSize > newSize)
            (oldPtr + (--oldSize))->~T();

        if (oldPtr != (T*)m_arrbuf && oldPtr != m_ptr)
            delete[] reinterpret_cast<char*>(oldPtr);

        // call default constructor for new objects (which can throw)
        while (m_size < newSize)
            new (m_ptr + (m_size++)) T;
    }

private:
    DISALLOW_COPY_AND_ASSIGN(VarArray);

    int m_size;  // number of T elements added
    int m_alloc; // we have enough allocated for this many T elements 
    // points to either m_arrray or equals to m_allocated.
    // once it goes to m_allocated, it never goes back to m_arrbuf
    T* m_ptr;
    // not an array of T since we don't want to call ctors
    char m_arrbuf[sizeof(T) * StaticSize];
    char *m_allocbuf;
};


