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

// CONSTANTS ///////////////////////////////////////////////////////////////////

typedef enum {
    EVAL = 0,
    IMPORT = 1,
    DECREF = 2,
    STR = 3,
    PUT = 4,
} function_t;

// GLOBALS /////////////////////////////////////////////////////////////////////

bool has_initialized = false;
PyObject* __main__ = NULL;

// PROTOTYPES //////////////////////////////////////////////////////////////////

void get_matlab_str(const mxArray* m_str, char** c_str);

void import(int, mxArray**, int, const mxArray**);
void eval(int, mxArray**, int, const mxArray**);
void decref(int, mxArray**, int, const mxArray**);
void str(int, mxArray**, int, const mxArray**);
void put(int, mxArray**, int, const mxArray**);

// MEX ENTRY POINTS ////////////////////////////////////////////////////////////

void cleanup() {
    Py_Finalize();
}

void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]) {

    // Create the various variables we'll need in the switch below.
    function_t function = *(int*)(mxGetData(prhs[0]));

	// Check whether we have already called Py_Initialize, and do it if need be.    
    if (!has_initialized) {
        Py_Initialize();
        __main__ = PyImport_AddModule("__main__");
        mexAtExit(cleanup);
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
    }
}

// UTILITY FUNCTIONS ///////////////////////////////////////////////////////////

void get_matlab_str(const mxArray* m_str, char** c_str) {
    // Make a buffer using mxCalloc, leaving room for the \0.
    int buf_size = (mxGetM(m_str) * mxGetN(m_str)) + 1;
    *c_str = mxCalloc(buf_size, sizeof(char));
    
    // Attempt to convert the MATLAB string into a C string.
    int status = mxGetString(m_str, *c_str, buf_size);
    if (status != 0) {
        mexWarnMsgTxt("Not enough space. String is truncated.");
    }
}

PyObject* py_obj_from_mat_scalar(const mxArray* m_scalar) {
    return *(PyObject**)mxGetData(m_scalar);
}

// MEX FUNCTIONS ///////////////////////////////////////////////////////////////

void import(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]) {
    // We expect there to be a single argument, containing the name
    // of the module to import.
    if (nrhs < 1) {
        mexErrMsgTxt("Not enough arguments.");
        return;
    }
    
    char* name_buf;
    get_matlab_str(prhs[0], &name_buf);
    
    PyObject *py_name = PyString_FromString(name_buf);
    PyObject *py_module = PyImport_Import(py_name);
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
        plhs[0] = mxCreateNumericMatrix(1, 1, mxINT64_CLASS, mxREAL);
        ((long long int*)mxGetData(plhs[0]))[0] = (long long int) py_module;
    }
}

void eval(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]) {
    // We expect there to be a single argument for now,
    // consisting of a string to be run.
    if (nrhs < 1) {
        mexErrMsgTxt("Not enough arguments.");
        return;
    }
    
    char* arg_buf;
    get_matlab_str(prhs[0], &arg_buf);
    
    // Now evaluate the string as a Python line.
    PyRun_SimpleString(arg_buf);
}

void decref(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]) {
    if (nrhs != 1) {
        mexErrMsgTxt("Expected exactly one argument.");
        return;
    }
    
    if (mxGetClassID(prhs[0]) != mxINT64_CLASS) {
        mexErrMsgTxt("Expected an int64.");
        return;
    }
    
    Py_XDECREF(py_obj_from_mat_scalar(prhs[0]));
}

void str(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]) {
    if (nrhs != 1) {
        mexErrMsgTxt("Expected exactly one argument.");
        return;
    }
    
    if (mxGetClassID(prhs[0]) != mxINT64_CLASS) {
        mexErrMsgTxt("Expected an int64.");
        return;
    }
    
    if (nlhs != 1) {
        mexErrMsgTxt("Expected one output argument.");
        return;
    }
    
    PyObject *py_obj = py_obj_from_mat_scalar(prhs[0]);
    PyObject *py_str = PyObject_Str(py_obj);
    
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
    if (nrhs != 2) {
        mexErrMsgTxt("Expected exactly two argument.");
        return;
    }
    
    PyObject *new_obj = NULL;
    char* val_name;
    get_matlab_str(prhs[0], &val_name);
    
    // FIXME: currently assuming n = m = 1 (scalar case).
    
    mxArray const *m_val = prhs[1];
    char* buf;
    switch (mxGetClassID(m_val)) {
        
        case mxDOUBLE_CLASS:
            if (mxGetM(m_val) != 1 || mxGetN(m_val) != 1) {
                mexErrMsgTxt("Putting arrays not yet supported.");
            }
            new_obj = PyFloat_FromDouble(mxGetScalar(prhs[1]));
            Py_INCREF(new_obj);
            break;
            
        case mxCHAR_CLASS:
            get_matlab_str(m_val, &buf);
            new_obj = PyString_FromString(buf);
            Py_INCREF(new_obj);
            break;
            
    }
    
    if (new_obj != NULL) {
        PyObject* dict = PyModule_GetDict(__main__);
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