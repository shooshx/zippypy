#include "Bridge.h"

#include "intigua/script/python_boost.h"
#include "intigua/script/script.pb.h"

#include "intigua/script/Interpreter.h"
#include "intigua/script/PyGILGuard.h"

#include "intigua/script/pyvm/objects.h"
#include "intigua/os/DataUtils.h"


namespace intigua {
namespace script {


namespace bp = boost::python;

static void encodePy(PyObject* codeobj, CodeDefinition& c);

static void encodeConst(const bp::object& o, ConstValue* v) {
    PyObject *po = o.ptr();
    if (po == Py_None)
        v->set_type(ConstValue::NONE);
    else if (PyBool_Check(po)) {
        v->set_type(ConstValue::BOOL);
        v->set_bool_(bp::extract<bool>(o));
    }
    else if (PyInt_Check(po)) {
        v->set_type(ConstValue::INT);
        v->set_sint(bp::extract<int>(o));
    }
    else if (PyLong_Check(po)) {
        v->set_type(ConstValue::INT);
        PyObject* po = o.ptr();
        int overflow = 0;
        int64 sn = PyLong_AsLongLongAndOverflow(po, &overflow);
        if (overflow == 0)
            v->set_sint(sn);
        else {
            uint64 un = PyLong_AsUnsignedLongLong(po);
            if (PyErr_Occurred() == nullptr) {
                v->set_uint(un);
            }
            else {
                PyErr_Clear();
                // extract the it as string
                PyObject* as_str = PyObject_Str(po);
                CHECK(as_str != nullptr);
                const char* the_c_str = PyString_AsString(as_str);
                string s(the_c_str);
                Py_DECREF(as_str);
                THROW("Numeric constant too big " << s);
            }
        }

    }
    else if (PyString_Check(po)) {
        v->set_type(ConstValue::STR);
        v->set_str(bp::extract<string>(o));
    }
    else if (PyCode_Check(po)) {
        v->set_type(ConstValue::CODE);
        encodePy(po, *v->mutable_code());
    }
    else if (PyTuple_Check(po)) {
        v->set_type(ConstValue::TUPLE);
        bp::tuple tv(o);
        for(int i = 0, sz = (int)bp::len(tv); i < sz; ++i)
            encodeConst(bp::object(tv[i]), v->add_tupleitems());
    }
    else if (PyFloat_Check(po)) {
        v->set_type(ConstValue::FLOAT);
        v->set_flt(bp::extract<double>(o));
    }
    else if (PyUnicode_Check(po)) {
        v->set_type(ConstValue::USTR);
        wstring us = bp::extract<wstring>(o);
        string s;
        CHECK(os::DataUtils::toUtf8(us, &s));
        v->set_str(s);
    }
    else {
        THROW(IntiguaException("type not supported"));
    }

}

static void encodePy(PyObject *pco, CodeDefinition& c) 
{
    bp::object co(bp::handle<>(bp::borrowed(pco)));
    c.set_co_name(bp::extract<string>(co.attr("co_name")));
    c.set_co_argcount(bp::extract<int>(co.attr("co_argcount")));
    c.set_co_nlocals(bp::extract<int>(co.attr("co_nlocals")));
    {
        bp::tuple varnames(co.attr("co_varnames"));
        for(int i = 0, sz = (int)bp::len(varnames); i < sz; ++i)
            c.add_co_varnames(bp::extract<string>(varnames[i]));
    }
    CHECK(bp::len(bp::tuple(co.attr("co_cellvars"))) == 0);
    CHECK(bp::len(bp::tuple(co.attr("co_freevars"))) == 0);
    c.set_co_code(bp::extract<string>(co.attr("co_code")));
    {
        bp::tuple consts(co.attr("co_consts"));
        for(int i = 0, sz = (int)bp::len(consts); i < sz; ++i) {
            encodeConst(bp::object(consts[i]), c.add_co_consts());
        }
    }
    {
        bp::tuple names(co.attr("co_names"));
        for(int i = 0, sz = (int)bp::len(names); i < sz; ++i)
            c.add_co_names(bp::extract<string>(names[i]));
    }
    c.set_co_filename(bp::extract<string>(co.attr("co_filename")));
    c.set_co_firstlineno(bp::extract<int>(co.attr("co_firstlineno")));
    c.set_co_lnotab(bp::extract<string>(co.attr("co_lnotab")));
    c.set_co_stacksize(bp::extract<int>(co.attr("co_stacksize")));
    c.set_co_flags(bp::extract<int>(co.attr("co_flags")));
}


bool Bridge::compile(const string& codestr, const string& filename, CodeDefinition* code) 
{
    PyGILGuard gilg;
    PyObject *pco = Py_CompileString(codestr.c_str(), filename.c_str(), Py_file_input);
    if (pco == nullptr) {
        string s = handlePyException();
        LOG_ERROR_T(SCRIPT, "Error compiling python code: ", s);
        return false;
    }
    bp::handle<> ignored(pco);
    try {
        encodePy(pco, *code);
    }
    catch (const IntiguaException& e) {
        LOG_ERROR_T(SCRIPT, "Error encoding python code from `", filename, "` ", e.what());
        return false;
    }
    LOG_DEBUG_T(SCRIPT, "Compiled `", filename, "`(", codestr.size(), " bytes) successfully");
    return true;
}


PyCodeObject* getInteractiveLine();


#if 0
void Bridge::testInteractive() {
    script::Interpreter intr;
    intr.init(false);
    PyVM vm;
    vm.setStdout(&std::cout);
    while (true) {
        CodeDefinition c;
        {
            PyGILGuard gilg;
            PyCodeObject *pco = getInteractiveLine();           
            if (pco == nullptr)
                break;
            encodePy((PyObject*)pco, c);
        }
        vm.eval(c, vm.mainModule());
    }

}

#endif



}}
