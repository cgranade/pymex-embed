/**
 * pymex_fns.c: MEX function embedding Python as an interpreter.
 **
 * (c) 2013 Christopher E. Granade (cgranade@cgranade.com).
 *    
 * This file is a part of the pymex-embed project.
 * Licensed under the AGPL version 3.
 **
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **/

// INCLUDES ////////////////////////////////////////////////////////////////////

#include <Python.h>
#include <mex.h>
#include <stdio.h>
#include "pymex_marshal.h"
#ifdef LINUX
    #include <dlfcn.h>
#endif

// CONSTANTS ///////////////////////////////////////////////////////////////////

typedef enum {
    EVAL = 0,
    IMPORT = 1,
    DECREF = 2,
    STR = 3,
    PUT = 4,
    GET = 5,
    GETATTR = 6,
    CALL = 7,
    GETITEM = 8,
    MUL = 9,
    EQ = 10,
    LT = 11,
    GT = 12,
    LE = 13,
    GE = 14,
    NE = 15,
} function_t;

// GLOBALS /////////////////////////////////////////////////////////////////////

bool has_initialized = false;
PyObject* __main__ = NULL;
PyObject* MatlabError = NULL;

mxArray *MEX_NULL = NULL;

// PROTOTYPES //////////////////////////////////////////////////////////////////

void import(int, mxArray**, int, const mxArray**);
void eval(int, mxArray**, int, const mxArray**);
void decref(int, mxArray**, int, const mxArray**);
void str(int, mxArray**, int, const mxArray**);
void put(int, mxArray**, int, const mxArray**);
void get(int, mxArray**, int, const mxArray**);
void getattr(int, mxArray**, int, const mxArray**);
void call(int, mxArray**, int, const mxArray**);
void getitem(int, mxArray**, int, const mxArray**);
void mul(int, mxArray**, int, const mxArray**);
void cmp(int, int, mxArray**, int, const mxArray**);

// PYTHON METHODS AND FUNCTIONS ////////////////////////////////////////////////
// These functions are exposed to the embedded Python runtime via the
// Py_InitModule function called inside mexFunction(), below.

static PyObject* pymex_mateval(PyObject* self, PyObject* str) {
    // Because METH_0 is defined for this method, we need not parse the
    // args tuple; the single argument str is unpacked from it for us.
    mxArray* result = mexEvalStringWithTrap(PyString_AS_STRING(str));
    
    if (result == NULL) {
        Py_INCREF(Py_None);
        return Py_None;
    } else {
        mxArray* m_err_msg = mxGetProperty(result, 0, "message");
        if (m_err_msg != NULL) {
            char* c_err_msg;
            get_matlab_str(m_err_msg, &c_err_msg);
            PyErr_SetString(MatlabError, c_err_msg);
        } else {
            PyErr_SetString(MatlabError, "Unknown MATLAB error occured.");
        }
        
        // A NULL must make its way all the way back to the Python
        // interpreter for the PyErr_SetString call to raise an exception.
        return NULL;
    }
    
}

static PyObject* pymex_matwrite(PyObject* self, PyObject* str) {

    char *c_str;
   
    if (str == NULL) {
        PyErr_SetString(PyExc_TypeError, "Got null instead of a string.");
        return NULL;
    } else if (!PyString_Check(str)) {
        PyErr_SetString(PyExc_TypeError, "Expected string argument.");
        return NULL;
    }
    
    c_str = PyString_AsString(str);
    
    mexPrintf(c_str);
    
    return Py_None;

}

static PyObject* pymex_get(PyObject* self, PyObject* args, PyObject* kwargs) {
    
    char *name;
    char *workspace = "base";
    
    static char *kwlist[] = {"name", "workspace", NULL};
    
    mxArray* mat_var;
    PyObject* py_var;
    
    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "s|s", kwlist, &name, &workspace)) {
    //if (!PyArg_ParseTupleAndKeywords(args, kwargs, "ss", kwlist, &name, &workspace)) {
        mexWarnMsgTxt("PyArg_ParseTupleAndKeywords failed.");
        return NULL;
    }
    
    mat_var = mexGetVariable(workspace, name);
    
    if (mat_var != NULL) {    
        py_var = mat2py(mat_var, false);
        mxDestroyArray(mat_var);
    } else {
        PyErr_SetString(PyExc_NameError,
            "No such MATLAB variable.");
        return NULL;
    }
    
    if (py_var != NULL) {
        Py_INCREF(py_var);
        return py_var;
    } else {
        PyErr_SetString(PyExc_NotImplementedError,
            "MATLAB value class not yet supported.");
        return NULL;
    }
    
}

static PyMethodDef PymexMethods[] = {
    {"mateval", pymex_mateval, METH_O,
        "Evaluates MATLAB code inside the PyMEX host."},
    {"matwrite", pymex_matwrite, METH_O,
        "Write a string to the MATLAB command window or console."},
    {"get", (PyCFunction)pymex_get, METH_VARARGS | METH_KEYWORDS,
        "Returns the value of a MATLAB variable."},
        
    // Terminate the array with a NULL method entry.
    {NULL, NULL, 0, NULL}
};


// MEX ENTRY POINTS ////////////////////////////////////////////////////////////

void cleanup() {
    Py_Finalize();
}

void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]) {

    PyObject *pymex_module, *dict;
    char buf[200];
    char *stdout_class = "\n\
import pymex\n\
import sys\n\
\n\
class PymexStdout(object):\n\
    def write(self, val):\n\
        pymex.matwrite(val)\n\
\n\
old_stdout = sys.stdout\n\
sys.stdout = PymexStdout()\n\
";
    
    // Create the various variables we'll need in the switch below.
    function_t function = *(unsigned char*)(mxGetData(prhs[0]));

	// Check whether we have already called Py_Initialize, and do it if need be.    
    if (!has_initialized) {
        PyObject *_pymex_module, *_pymex_dict;
        
        #ifdef LINUX
            void* dlresult;
            dlresult = dlopen("libpython2.7.so", RTLD_GLOBAL|RTLD_LAZY);
            if (dlresult == NULL) {
                mexErrMsgTxt("Failed to dlopen python2.7.so.");
            }
        #endif
        
        // Initialize Python environment.
        Py_Initialize();
        
        // Find the __main__ module.
        __main__ = PyImport_AddModule("__main__");
        
        // Register our cleanup function so that Py_Finalize will always get
        // called.
        mexAtExit(cleanup);
        
        // Load embedded methods into the Python runtime environment.
        pymex_module = Py_InitModule("pymex", PymexMethods);
        
        // Make a new MatlabException to handle traps.
        dict = PyModule_GetDict(pymex_module);
        MatlabError = PyErr_NewException("pymex.MatlabError",
            PyExc_StandardError, NULL);
        PyDict_SetItemString(dict, "MatlabError", MatlabError);
        
        // FIXME: this is a dirty hack to ensure '' is on sys.path,
        //        and has the side effect of leaking "sys" into globals().
        PyRun_SimpleString("import sys\nsys.path.insert(0, '')\n");
        
        // Add pure-Python definitions from the private package _pymex
        // into the pymex module.
        // This should roughly break down as
        // pymex.__dict__.update(_pymex.__dict__).
        _pymex_module = PyImport_ImportModule("_pymex");
        if (_pymex_module == NULL) {
            mexWarnMsgTxt("Did not import _pymex correctly!");
        } else {
            _pymex_dict = PyModule_GetDict(_pymex_module);
            PyDict_Merge(dict, _pymex_dict, 0);
        }
        // Py_XDECREF(_pymex_module);
        // (The dict was borrowed, so no DECREF.)
        
        // Add a new class to the pymex module to redirect stdout.
        PyRun_SimpleString(stdout_class);
        
        // Finally, set aside an empty array for returning as MATLAB's answer
        // to null.
        MEX_NULL = mxCreateNumericMatrix(0, 0, mxDOUBLE_CLASS, mxREAL);
    }
    
    // Assume that nrhs >= 1, and that prhs[0] is of type int8 (classID == 8).
    switch(function) {
        case EVAL:
            eval(nlhs, plhs, nrhs - 1, prhs + 1);
            break;
            
        case IMPORT:
            import(nlhs, plhs, nrhs - 1, prhs + 1);
            break;
            
        case DECREF:
            decref(nlhs, plhs, nrhs - 1, prhs + 1);
            break;
            
        case STR:
            str(nlhs, plhs, nrhs - 1, prhs + 1);
            break;
            
        case PUT:
            put(nlhs, plhs, nrhs - 1, prhs + 1);
            break;
            
        case GET:
            get(nlhs, plhs, nrhs - 1, prhs + 1);
            break;
            
        case GETATTR:
            getattr(nlhs, plhs, nrhs - 1, prhs + 1);
            break;
            
        case CALL:
            call(nlhs, plhs, nrhs - 1, prhs + 1);
            break;
            
        case GETITEM:
            getitem(nlhs, plhs, nrhs - 1, prhs + 1);
            break;
            
        case MUL:
            mul(nlhs, plhs, nrhs - 1, prhs + 1);
            break;
            
        case EQ:
            cmp(Py_EQ, nlhs, plhs, nrhs - 1, prhs + 1);
            break;
            
        case LT:
            cmp(Py_LT, nlhs, plhs, nrhs - 1, prhs + 1);
            break;
            
        case GT:
            cmp(Py_GT, nlhs, plhs, nrhs - 1, prhs + 1);
            break;
            
        case LE:
            cmp(Py_LE, nlhs, plhs, nrhs - 1, prhs + 1);
            break;
            
        case GE:
            cmp(Py_GE, nlhs, plhs, nrhs - 1, prhs + 1);
            break;
            
        case NE:
            cmp(Py_NE, nlhs, plhs, nrhs - 1, prhs + 1);
            break;
            
        default:
            sprintf(buf, "Invalid function label %d received.", function);
            mexErrMsgTxt(buf);
            break;
    }
}

// DEBUG FUNCTIONS /////////////////////////////////////////////////////////////

void print(PyObject* py_obj) {
    PyObject* py_str = PyObject_Str(py_obj);
    mexPrintf("%s\n", PyString_AsString(py_str));
    Py_XDECREF(py_str);    
}

// MEX FUNCTIONS ///////////////////////////////////////////////////////////////

void import(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]) {
    
    PyObject *py_name, *py_module;
    
    // We expect there to be a single argument, containing the name
    // of the module to import.
    if (nrhs < 1) {
        mexErrMsgTxt("Not enough arguments.");
        return;
    }
    
    py_name = mat2py(prhs[0], false);
    py_module = PyImport_Import(py_name); // New reference, so no incref needed.
    Py_DECREF(py_name);
    
    if (py_module == NULL) {
        if (PyErr_Occurred() != NULL) {
            PyErr_Print();
            mexErrMsgTxt("Python exception inside py_import.");
        }
    }
    
    if (nlhs == 0) {
        // FIXME: Find a way to DECREF this later!
        // Py_DECREF(py_module);
    } else {
        plhs[0] = py2mat(py_module);
    }
}

void eval(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]) {
    
    char* arg_buf;
    PyObject *retval, *py_dict;
    
    // We expect there to be a single argument for now,
    // consisting of a string to be run.
    if (nrhs < 1) {
        mexErrMsgTxt("Not enough arguments.");
        return;
    }
    
    get_matlab_str(prhs[0], &arg_buf);
    
    // Grab a borrowed reference to the __main__ module dict,
    // so that we can use it for globals() and locals().
    py_dict = PyModule_GetDict(__main__);
    
    // Now evaluate the string as a Python line.
    // This is a new reference, so we already own it.
    retval = PyRun_String(arg_buf, Py_single_input, py_dict, py_dict);
    
    // Check for an exception or a NULL return.
    if (PyErr_Occurred()) {
        PyErr_Print();
        mexErrMsgTxt("Python exception inside eval.");
    } else if (retval == NULL) {
        mexErrMsgTxt("Returned NULL, but no exception raised. This should never happen.");
    }
    
    if (nlhs >= 1) {
        plhs[0] = py2mat(retval);
    } else {
        // DECREF the new reference, since we won't be keeping it after all.
        Py_XDECREF(plhs[0]);
    }
    
}

void decref(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]) {
    
    if (nrhs != 1) {
        mexErrMsgTxt("Expected exactly one argument.");
        return;
    }
    
    Py_XDECREF(unbox_pyobject(prhs[0]));
    
}

void str(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]) {
    
    PyObject *py_obj, *py_str;
    char buf[500];
    
    // Try to get the MATLAB argument as a PyObject.
    py_obj = mat2py(prhs[0], false);    
    py_str = PyObject_Str(py_obj);
    
    if (py_str == NULL) {
        if (PyErr_Occurred() != NULL) {
            PyErr_Print();
            mexErrMsgTxt("Python exception inside str.");
        }
    }
    
    plhs[0] = mxCreateString(PyString_AsString(py_str));
    Py_DECREF(py_str);
    
}

void put(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]) {
    
    PyObject *new_obj = NULL, *dict;
    char* val_name;
    mxArray const *m_val;
    
    if (nrhs != 2) {
        mexErrMsgTxt("Expected exactly two arguments.");
        return;
    }
    
    get_matlab_str(prhs[0], &val_name);
    
    // FIXME: currently assuming n = m = 1 (scalar case).
    
    m_val = prhs[1];
    new_obj = mat2py(m_val, false);
    
    if (new_obj != NULL) {
        dict = PyModule_GetDict(__main__);
        if (dict == NULL) {
            mexErrMsgTxt("Could not get dict(__main__).");
            return;
        }
        PyDict_SetItemString(dict, val_name, new_obj);
        // FIXME: when the dict is decref'd, it seems as though
        //        variables set there don't get saved.
        //Py_DECREF(dict);
        // FIXME: decref if there's an exception, otherwise don't?
        //Py_DECREF(__main__);
    }
    
}

/**
 * MATLAB signature: obj = py_get(name)
 * 
 * Returns the contents of a Python variable as a MATLAB array.
 */
void get(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]) {
    PyObject *new_obj = NULL, *dict;
    char* val_name;
    PyObject *py_val_name, *__builtins__dict;
    
    plhs[0] = MEX_NULL;
    
    if (nrhs != 1) {
        mexErrMsgTxt("Expected exactly one argument.");
    }
    
    // Fetch the name of the value we are supposed to pull from Python's
    // __main__.
    get_matlab_str(prhs[0], &val_name);
    // Convert the value name to a Python str.
    py_val_name = PyString_FromString(val_name);
    
    // Find __main__'s dict.
    dict = PyModule_GetDict(__main__);
    __builtins__dict = PyEval_GetBuiltins();
    
    // Does the variable exist?
    if (PyDict_Contains(dict, py_val_name)) {
        
        // If so, grab the reference and use Py_INCREF to own it.
        new_obj = PyDict_GetItem(dict, py_val_name);
        Py_INCREF(new_obj);
        
        // Pack the newly owned reference into a MATLAB scalar.
        plhs[0] = py2mat(new_obj);
        
    } else if (PyDict_Contains(__builtins__dict, py_val_name)) {
        // No? Then try __builtins__.
        new_obj = PyDict_GetItem(__builtins__dict, py_val_name);
        Py_INCREF(new_obj);
        plhs[0] = py2mat(new_obj);        
    } else {
        mexErrMsgTxt("No such variable exists in the Python environment's __main__ module.");        
    }
    
    // In any case, decref the value name.
    Py_DECREF(py_val_name);
    
}

/**
 * MATLAB signature: value = getattr(object, name)
 * 
 * Returns an attribute of a Python object with a given pointer.
 */
void getattr(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]) {
    // TODO: special case away the various Python types that have 1:1
    //       representations in MATLAB, including ints, floats and NumPy
    //       arrays of the appropriate dtypes.
    
    PyObject *new_obj = NULL, *obj;
    char* val_name;
    PyObject* py_val_name;
    
    if (nrhs != 2) {
        mexErrMsgTxt("Expected exactly two arguments.");
    }
    
    // Unbox the PyObject* from the MATLAB scalar.
    obj = mat2py(prhs[0], false);
    
    // Fetch the name of the attribute to be queried.
    get_matlab_str(prhs[1], &val_name);
    // Convert the value name to a Python str.
    py_val_name = PyString_FromString(val_name);
    
    // Does the attribute exist?
    if (PyObject_HasAttr(obj, py_val_name)) {
        
        // If so, grab the reference. It's a new reference, so we don't need
        // to take ownership.
        new_obj = PyObject_GetAttr(obj, py_val_name);
        
        // Check for Python exceptions.
        if (PyErr_Occurred() != NULL) {
            PyErr_Print();
            mexErrMsgTxt("Python exception inside getattr.");
        }
        
        // Pack the newly owned reference into a MATLAB scalar.
        plhs[0] = py2mat(new_obj);
        
    } else {
        mexErrMsgTxt("No such attribute exists.");
        plhs[0] = NULL;
    }
    
    // In any case, decref the attribute name.
    Py_DECREF(py_val_name);
    
}

/**
 * MATLAB signature: value = call(object, varargin)
 * 
 * Calls a Python object with given arguments (but without keyword arguments).
 */
void call(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]) {

    PyObject *args, *args_list = NULL, *retval = NULL, *callee;

    callee = mat2py(prhs[0], false);
    
    // Ensure it's a cell array!
    if (!mxIsCell(prhs[1])) {
        mexErrMsgTxt("Expected cell array of args.");
    }
    
    args_list = mat2py(prhs[1], true);
    args = PySequence_Tuple(args_list);
    Py_DECREF(args_list);
    
    // Check that the callee is, well, callable.
    if (PyCallable_Check(callee)) {        
        retval = PyObject_CallObject(callee, args);
        if (retval == NULL) {
            if (PyErr_Occurred() != NULL) {
                PyErr_Print();
                mexErrMsgTxt("Python exception during call.");
            } else {
                mexErrMsgTxt("Call failed for unknown reason.");
            }
        }
        plhs[0] = py2mat(retval);
    } else {
        mexErrMsgTxt("Object is not callable.");
    }
    
    if (PyErr_Occurred() != NULL) {
        PyErr_Print();
        mexErrMsgTxt("Python exception during call.");
    }

}

/**
 * MATLAB signature: value = getitem(object, key)
 * 
 * Accesses an item from the Python object "object", emulating `object[key]`
 * notation. 
 */
void getitem(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]) {

    PyObject *target = mat2py(prhs[0], false), *key = mat2py(prhs[1], false);
    PyObject *py_value;
    
    py_value = PyObject_GetItem(target, key);
    
    if (py_value == NULL) {
        mexErrMsgTxt("Exception getting item.");
    }
    
    if (PyErr_Occurred() != NULL) {
        PyErr_Print();
        mexErrMsgTxt("Python exception during call.");
    }
    
    // New reference!
    plhs[0] = py2mat(py_value);
    
}

void mul(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]) {

    if (nrhs != 2) {
        mexErrMsgTxt("Expected exactly two arguments.");
    }
    
    PyObject *a = mat2py(prhs[0], false), *b = mat2py(prhs[1], false);
    
    plhs[0] = py2mat(PyNumber_Multiply(a, b));
    Py_XDECREF(a);
    Py_XDECREF(b);
    
    if (PyErr_Occurred() != NULL) {
        PyErr_Print();
        mexErrMsgTxt("Python exception inside mul.");
    }
    
}

void cmp(int op, int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]) {

    if (nrhs != 2) {
        mexErrMsgTxt("Expected exactly two arguments.");
    }
    
    PyObject *a = mat2py(prhs[0], false), *b = mat2py(prhs[1], false);
    
    plhs[0] = py2mat(PyObject_RichCompare(a, b, op));
    Py_XDECREF(a);
    Py_XDECREF(b);
    
    if (PyErr_Occurred() != NULL) {
        PyErr_Print();
        mexErrMsgTxt("Python exception inside cmp.");
    }
    
}
