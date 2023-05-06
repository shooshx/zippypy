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
#include "PyCompile.h"
#include "log.h"

#if USE_CPYTHON

//#define HAVE_ROUND // newer VS has this, uncomment in older VS
#undef _DEBUG // don't get the debug lib
#include <Python.h>
#include <marshal.h>
//#include <Python-ast.h>

bool initPython() 
{
    if (Py_IsInitialized()) 
        return true;

    Py_Initialize();
    if (Py_IsInitialized()) 
        return true;

    return false;
}

typedef intptr_t ssize_t;

// if the GIL needs to be locked, do that before calling this function
bool compileTextToPycBuf(const std::string& text, const std::string& filename, std::string* outpyc)
{
    if (!initPython())
        return false;

    PyObject *pco = Py_CompileString(text.c_str(), filename.c_str(), Py_file_input);
    if (pco == nullptr) {
        LOG_ERROR("Error compiling python code");
        PyErr_Print();
        return false;
    }

    PyObject* pycstr = PyMarshal_WriteObjectToString(pco, 2);
    ssize_t sz = 0;
    char* data = nullptr;
    PyBytes_AsStringAndSize(pycstr, &data, &sz);
    *outpyc = std::string(data, sz);

    Py_DECREF(pycstr);
    Py_DECREF(pco);

    return true;
}


std::string getInteractiveLine() 
{
    bool inited = false;
    if (!inited) {
        if (!initPython())
            return std::string();
        inited = true;
    }

    // use PyRun_InteractiveOneObjectEx

 /*   int errcode = 0;
    PyArena *arena = PyArena_New();
    mod_ty mod = PyParser_ASTFromFile(stdin, "blafilename", "utf-8", Py_single_input, ">>> ", "... ", nullptr, &errcode, arena);
    if (mod == nullptr) {
        PyArena_Free(arena);
        return std::string();
    }

    PyCodeObject *pco = PyAST_Compile(mod, "blafilename", nullptr, arena);
    PyArena_Free(arena);

    PyObject* pycstr = PyMarshal_WriteObjectToString((PyObject*)pco, 2);
    ssize_t sz = 0;
    char* data = nullptr;
    PyBytes_AsStringAndSize(pycstr, &data, &sz);
    std::string pycline = std::string(data, sz);

    Py_DECREF(pycstr);
    Py_DECREF(pco);

    return pycline;
    */
    return {};
}

#endif // HAS_CPYTHON