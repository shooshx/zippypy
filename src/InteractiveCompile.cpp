#include "intigua/script/python_boost.h"

//#include <python.h>
// weird warning in VS2013 here
#include <Python-ast.h>

// This needs to be in a separate file that doesn't include anything intigua because the Python-ast.h redefines 'string'

namespace intigua {
namespace script {


PyCodeObject* getInteractiveLine() {

    int errcode = 0;
    PyArena *arena = PyArena_New();
    mod_ty mod = PyParser_ASTFromFile(stdin, "blafilename", Py_single_input, ">>>", "...", nullptr, &errcode, arena);
    if (mod == nullptr) {
        PyArena_Free(arena);
        return nullptr;
    }

    PyCodeObject *co = PyAST_Compile(mod, "blafilename", nullptr, arena);
    PyArena_Free(arena);
    return co;
}


}}