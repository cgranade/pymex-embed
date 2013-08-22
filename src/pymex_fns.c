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

// FIXME ///////////////////////////////////////////////////////////////////////
/**
 * - PyObject* pointers passed into mexFunction are always ints, frustrating
 *   any attempt to distinguish types from within this C compliation unit.
 *   Use mxGetClassName to check for this.
 */

// INCLUDES ////////////////////////////////////////////////////////////////////

#include <Python.h>
#include <mex.h>
#include <stdio.h>
#include "pymex_marshal.h"

// CONSTANTS ///////////////////////////////////////////////////////////////////

typedef enum {
    EVAL = 0,
    IMPORT = 1,
    DECREF = 2,
    STR = 3,
    PUT = 4,
    GET = 5,
    GETATTR = 6,
    CALL = 7
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
        py_var = mat2py(mat_var);
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
    
    // Create the various variables we'll need in the switch below.
    function_t function = *(unsigned char*)(mxGetData(prhs[0]));

	// Check whether we have already called Py_Initialize, and do it if need be.    
    if (!has_initialized) {
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
            
        default:
            sprintf(buf, "Invalid function label %d received.", function);
            mexErrMsgTxt(buf);
            break;
    }
}


// MEX FUNCTIONS ///////////////////////////////////////////////////////////////

void import(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]) {
    
    char* name_buf;
    PyObject *py_name, *py_module;
    
    // We expect there to be a single argument, containing the name
    // of the module to import.
    if (nrhs < 1) {
        mexErrMsgTxt("Not enough arguments.");
        return;
    }
    
    get_matlab_str(prhs[0], &name_buf);
    
    py_name = PyString_FromString(name_buf);
    py_module = PyImport_Import(py_name); // New reference, so no incref needed.
    Py_DECREF(py_name);
    
    if (py_module == NULL) {
        if (PyErr_Occurred() != NULL) {
            mexErrMsgTxt("Python exception inside py_import.");
            PyErr_Print();
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
    py_obj = mat2py(prhs[0]);    
    py_str = PyObject_Str(py_obj);
    
    if (py_str == NULL) {
        if (PyErr_Occurred() != NULL) {
            mexErrMsgTxt("Python exception inside str.");
            PyErr_Print();
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
    new_obj = mat2py(m_val);
    
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
    PyObject* py_val_name;
    
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
    
    // Does the variable exist?
    if (PyDict_Contains(dict, py_val_name)) {
        
        // If so, grab the reference and use Py_INCREF to own it.
        new_obj = PyDict_GetItem(dict, py_val_name);
        Py_INCREF(new_obj);
        
        // Pack the newly owned reference into a MATLAB scalar.
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
    obj = py_obj_from_mat_scalar(prhs[0]);
    
    // Fetch the name of the attribute to be queried.
    get_matlab_str(prhs[1], &val_name);
    // Convert the value name to a Python str.
    py_val_name = PyString_FromString(val_name);
    
    // Does the attribute exist?
    if (PyObject_HasAttr(obj, py_val_name)) {
        
        // If so, grab the reference. It's a new reference, so we don't need
        // to take ownership.
        new_obj = PyObject_GetAttr(obj, py_val_name);
        
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

    callee = py_obj_from_mat_scalar(prhs[0]);
    args_list = py_list_from_cell_array(prhs[1], 0, 1, NULL, NULL);
    args = PySequence_Tuple(args_list);
    Py_DECREF(args_list);
    
    // Check that the callee is, well, callable.
    if (PyCallable_Check(callee)) {        
        retval = PyObject_CallObject(callee, args);
        plhs[0] = py2mat(retval);
    } else {
        mexErrMsgTxt("Object is not callable.");
    }
    
    if (PyErr_Occurred() != NULL) {
        PyErr_Print();
        mexErrMsgTxt("Python exception during call.");
    }

}
