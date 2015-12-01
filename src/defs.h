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

#define DISALLOW_COPY_AND_ASSIGN(Cls) private: Cls(const Cls&) = delete; Cls& operator=(const Cls&) = delete;

typedef unsigned __int64 uint64;
typedef signed __int64 int64;
typedef unsigned short ushort;
typedef unsigned char uchar;
typedef unsigned int uint;

template<typename T>
inline bool checkFlag(T var, T flag) {
    return ((var & flag) == flag);
}

template<typename T>
inline T imin(T a, T b) {
    return (a < b)?a:b;
}

template<typename T>
inline T imax(T a, T b) {
    return (a > b)?a:b;
}


template<typename TC>
const TC* _chooseLiteral(const char* c, const wchar_t* w);
template<> inline const char* _chooseLiteral<char>(const char* c, const wchar_t* w) { return c; }
template<> inline const wchar_t* _chooseLiteral<wchar_t>(const char* c, const wchar_t* w) { return w; }

// with this define you can use a one literal string templated as either char* or wchar_t*
// tc is a type, char or wchar_t, str is an ansi string literal
#define LITERAL_TSTR(tc, str) _chooseLiteral<tc>(str, L##str)
