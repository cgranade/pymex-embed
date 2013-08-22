/**
 * pymex_marshal.c: Functions for marshalling data between MATLAB and Python.
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

#include "pymex_marshal.h"

// CONSTANTS ///////////////////////////////////////////////////////////////////

const char* PY_OBJECT_CLASS_NAME = "PyObject";
const char* PY_OBJECT_PTR_FIELD = "py_pointer";

const mxClassID POINTER_CLASS = mxUINT64_CLASS;

// TYPEDEFS ////////////////////////////////////////////////////////////////////

#define py_pointer_box_t unsigned long long int

// UTILITY FUNCTIONS ///////////////////////////////////////////////////////////

/**
 * Gets the contents of a MATLAB string as a C string (zero-terminated char*).
 *
 * @param m_str: Pointer to a MATLAB array containing a string to be extracted.
 * @param c_str: Pointer that will be assigned to the new C string containing
 *     the contents of `m_str`.
 */
void get_matlab_str(const mxArray* m_str, char** c_str) {
    int status;
    
    // Make a buffer using mxCalloc, leaving room for the \0.
    int buf_size = (mxGetM(m_str) * mxGetN(m_str)) + 1;
    *c_str = mxCalloc(buf_size, sizeof(char));
    
    // Attempt to convert the MATLAB string into a C string.
    status = mxGetString(m_str, *c_str, buf_size);
    if (status != 0) {
        mexWarnMsgTxt("Not enough space. String is truncated.");
    }
}

/**
 * Converts a MATLAB scalar containing a pointer into a PyObject*.
 */
PyObject* py_obj_from_mat_scalar(const mxArray* m_scalar) {
    return *(PyObject**)mxGetData(m_scalar);
}

/**
 * Recursively converts dimensions of a cell array to Python lists.
 */
PyObject* py_list_from_cell_array(
    const mxArray* cell_array, int idx_dim, mwSize nsubs, mwIndex* subs,
    mwIndex* dims
) {
    
    PyObject *py_list, *new_el;
    int idx_el = 0;
    
    if (subs == NULL) {
        subs = mxCalloc(nsubs, sizeof(mwIndex));
    }
    
    if (dims == NULL) {
        dims = mxGetDimensions(cell_array);
    }
    
    py_list = PyList_New(dims[idx_dim]);
    for (idx_el = 0; idx_el < dims[idx_dim]; idx_el++) {
        subs[idx_dim] = idx_el;
        if (idx_dim != nsubs - 1) {
            new_el = py_list_from_cell_array(
                cell_array, idx_dim + 1, nsubs, subs, dims
            );
        } else {
            new_el = mat2py(
                mxGetCell(cell_array, mxCalcSingleSubscript(
                    cell_array, nsubs, subs
                ))
            );
            if (new_el == NULL) {
                mexWarnMsgTxt("Unsupported value in cell array; substituting with None.");
                new_el = Py_None;
                Py_INCREF(Py_None);
            }
        }
        PyList_SetItem(py_list, idx_el, new_el);
    }
    return py_list;
    
}

// MARSHALLING FUNCTIONS ///////////////////////////////////////////////////////

/**
 * Given a Python object, creates and returns a MATLAB array for that object.
 * Types (float, int, str, ...) that have 1:1 reprsentations in MATLAB will be
 * converted, while any more complicated types will be boxed using the
 * MATLAB class PyObject.
 *
 * Note that this function DECREFs any value that isn't boxed into a PyObject
 * MATLAB class.
 */
mxArray* py2mat(const PyObject* py_value) {
    mxArray* mat_value;

    // TODO: Right now, we aren't converting any types, but are just boxing
    //       them up.
    if (PyString_Check(py_value)) {
        char *bufs[1];
        bufs[0] = PyString_AsString(py_value);
        mat_value = mxCreateCharMatrixFromStrings(1, bufs);
        Py_XDECREF(py_value);
    } else if (PyFloat_Check(py_value)) {
        mat_value = mxCreateDoubleScalar(PyFloat_AsDouble(py_value));
        Py_XDECREF(py_value);
    } else if (PyBool_Check(py_value)) {
        bool c_value = PyObject_IsTrue(py_value);
        mat_value = mxCreateLogicalScalar(c_value);
        Py_XDECREF(py_value);
    } else if (PyList_Check(py_value)) {
        // Make a 1xn cell array, and then pack everything into it by
        // calling py2mat recursively. This will make ugly structures
        // for nested Python lists, but it's kind of unavoidable due to
        // the ability of Python lists to be jagged.
        
        int len = PyList_Size(py_value);
        mat_value = mxCreateCellMatrix(1, len);
        for (int idx_cell = 0; idx_cell < len; idx_cell++) {
            mxSetCell(mat_value, idx_cell, py2mat(PyList_GetItem(py_value, idx_cell)));
        }
        Py_XDECREF(py_value);
        
    } else {
        mat_value = box_pyobject(py_value);
    }
    
    return mat_value;
    
}

/**
 * Given a MATLAB array, creates and returns a pointer to an appropriate
 * PyObject. If a new object cannot be created, returns NULL.
 */
PyObject* mat2py(const mxArray* m_value) {
    PyObject* new_obj = NULL;
    char* buf;
    int nsubs;
    
    // First, check if the MATLAB value is a boxed PyObject.
    if (is_boxed_pyobject(m_value)) {
        return unbox_pyobject(m_value);
    }
    
    // If it wasn't, figure out how to marshal the native-MATLAB data.
    switch (mxGetClassID(m_value)) {
        
        case mxCELL_CLASS:
            nsubs = mxGetNumberOfDimensions(m_value);
            return py_list_from_cell_array(m_value, 0, nsubs, NULL, NULL);
        
        case mxDOUBLE_CLASS:
            if (mxGetM(m_value) != 1 || mxGetN(m_value) != 1) {
                mexErrMsgTxt("Putting arrays not yet supported.");
            }
            // new, so already owned.
            new_obj = PyFloat_FromDouble(mxGetScalar(m_value)); 
            return new_obj;
            
        case mxCHAR_CLASS:
            get_matlab_str(m_value, &buf);
            // new, so already owned.
            new_obj = PyString_FromString(buf);
            return new_obj;
            
        case mxFUNCTION_CLASS:
            mexErrMsgTxt("Calling MATLAB functions from within Python is not yet supported.");
            return NULL;
            
    }
    
    // If we got here, return something sensible.
    return NULL;
    
}

/**
 * Returns true if and only if the given array is a PyObject, as determined by
 * the class name of the given array.
 */
bool is_boxed_pyobject(const mxArray* mat_array) {
    char const *class_name = mxGetClassName(mat_array);
    return !strcmp(class_name, PY_OBJECT_CLASS_NAME);
}

PyObject* unbox_pyobject(const mxArray* mat_array) {
    mxArray* field;
    void* data;
    mxClassID class;
    
    // Try to get the pointer property out.
    field = mxGetProperty(mat_array, 0, PY_OBJECT_PTR_FIELD);
    if (field == NULL) {
        mexErrMsgTxt("Pointer field was NULL.");
    }
    
    // Check that the class is correct.
    class = mxGetClassID(field);
    
    if (class != POINTER_CLASS) {
        mexErrMsgTxt("Field py_pointer did not contain a pointer.");
        return NULL;
    } else {
        // Get the pointer to the data, which should be a pointer
        // to the PyObject* (giving a type of PyObject**).
        // Since we want the 0th element of the field, then dereferencing
        // is fine.
        data = mxGetData(field);
        if (data == NULL) {
            mexErrMsgTxt("Data was NULL.");
            return NULL;
        } else {
            return *((PyObject**) data);
        }
    }
}

/**
 * Given a pointer to a PyObject, boxes that pointer inside a new instance of
 * the MATLAB class PyObject.
 */
mxArray* box_pyobject(const PyObject* py_object) {
    mxArray* boxed_obj, *lhs[1], *rhs[1];
    rhs[0] = mxCreateNumericMatrix(1, 1, POINTER_CLASS, mxREAL);
    ((py_pointer_box_t*)mxGetData(rhs[0]))[0] = (py_pointer_box_t) py_object;
    mexCallMATLAB(1, lhs, 1, rhs, "PyObject.new");
    return lhs[0];
}

