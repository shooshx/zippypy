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

#include <exception>
#include <string>
#include <sstream>

using namespace std;


class PyException : public exception
{
public:
    PyException(const string& desc) : m_desc(desc)
    {}
    virtual const char* what() const {
        return m_desc.c_str();
    }

    void addTrack(const string& t) {
        trackback += t + "\n";
    }

public:
    string trackback;
private:
    string m_desc;
};

#define CHECK(pred, msg) do { if (!(pred)) { stringstream ss; ss << msg; throw PyException(ss.str()); } } while(false)

#define ASSERT(pred, msg) CHECK(pred, msg)

#define THROW(msg) do { stringstream ss; ss << msg; throw PyException(ss.str()); } while(false)