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
#include <windows.h>
#include "utils.h"

bool wstrFromUtf8(const string& s, wstring *out) {
    if (s.empty()) {
        out->clear();
        return true;
    }
    int sz = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, s.c_str(), (int)s.size(), NULL, 0);
    if (sz == 0)
        return false;
    out->resize(sz);
    sz = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, s.c_str(), (int)s.size(), (LPWSTR)out->data(), sz);
    return sz != 0;
}
string utf8FromWstr(const wstring& s) {
    if (s.empty()) {
        return string();
    }
    int sz = WideCharToMultiByte(CP_UTF8, MB_ERR_INVALID_CHARS, s.c_str(), (int)s.size(), NULL, 0, NULL, NULL);
    if (sz == 0)
        return string();
    string out;
    out.resize(sz);
    sz = WideCharToMultiByte(CP_UTF8, MB_ERR_INVALID_CHARS, s.c_str(), (int)s.size(), (LPSTR)out.data(), sz, NULL, NULL);
    return out;
}

wstring wstrFromAnsi(const string& s) {
    wstring out;
    out.resize(s.size());
    std::copy(s.begin(), s.end(), out.begin());
    return out;
}
string ansiFromwstr(const wstring& s) {
    string out;
    out.reserve(s.size());
    for(wchar_t c: s)
        out.append(1, (char)c); // not correct at all
    return out;
}

#define SLASHES "/\\"

string extractFileNameWithoutExtension(const string& path) {
    // find the last slash
    size_t startPos = path.find_last_of(SLASHES);
    if (startPos == wstring::npos) 
        startPos = 0;
    else
        startPos += 1;
    size_t endPos = path.find_last_of('.');
    if (endPos == path.npos || endPos < startPos){
        endPos = path.length();
    }
    return path.substr(startPos,endPos - startPos);
}


uint64 msecTime(){
    return GetTickCount();
}

void debugBreak() {
    DebugBreak();
}
void MessageBoxCall() {
    MessageBoxA(nullptr, "Your Message Box", "Hello", MB_OK);
}

void consoleSetColor(int col) {
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), col);
}