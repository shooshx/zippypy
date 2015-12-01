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
#include <iostream>

enum LogLevel {
    LOGLEVEL_DEBUG,
    LOGLEVEL_ERROR
};


inline void tostream(std::ostream& os) {
}

template<typename T, typename... Args>
inline void tostream(std::ostream& os, const T& v, const Args&... args) {
    os << v;
    tostream(os, args...);
}

template<typename... Args>
void log(LogLevel lvl, const Args&... args) {
    std::ostream* ostr;
    if (lvl == LOGLEVEL_DEBUG)
        ostr = &std::cout;
    else
        ostr = &std::cerr;

    tostream(*ostr, args...);
    *ostr << "\n";
}

#define LOG_DEBUG(...)  do { log(LOGLEVEL_DEBUG, __VA_ARGS__ ); } while (false)
#define LOG_ERROR(...)  do { log(LOGLEVEL_ERROR, __VA_ARGS__ ); } while (false)
