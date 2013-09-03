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

// GLOBALS /////////////////////////////////////////////////////////////////////

PyObject *py_mxArray = NULL;
PyObject *py_struct = NULL;

// TYPEDEFS ////////////////////////////////////////////////////////////////////

#define py_pointer_box_t unsigned long long int

// UTILITY FUNCTIONS ///////////////////////////////////////////////////////////

/**
 * Ensures that py_mxArray has been initialized.
 */
void init_py_mxArray() {
    if (py_mxArray == NULL) {
        PyObject *pymex_module, *pymex_dict;
        // Assume the pymex module has already been imported, so we can
        // borrow the reference.
        pymex_module = PyImport_AddModule("pymex");
        pymex_dict = PyModule_GetDict(pymex_module);
        py_mxArray = PyDict_GetItemString(pymex_dict, "mxArray");
        // Borrowed reference; we'll leave it this way, though that might
        // cause a bug if the module goes away. Not sure how to handle
        // this.
    }

    // If it's still NULL, something went wrong.
    if (py_mxArray == NULL) {
        PyObject *err;

        err = PyErr_Occurred();
        if (err != NULL) {
            PyObject *descript;
            descript = PyObject_Str(err);
            mexPrintf(PyString_AsString(descript));
            Py_XDECREF(descript);
        }
        PyErr_Print();
        mexErrMsgTxt("Something went wrong getting the mxArray class constructor.");
    }
}

bool ensure_mat_scalar(const mxArray* m_value) {
    // FIXME: return bool, rather than raising error.
    if (mxGetM(m_value) != 1 || mxGetN(m_value) != 1) {
        return false;
    } else {
        return true;
    }
}

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
    mwIndex* dims, bool flatten1
) {
    
    PyObject *py_list, *new_el;
    int idx_el = 0;
    
    if (subs == NULL) {
        subs = mxCalloc(nsubs, sizeof(mwIndex));
    }
    
    if (dims == NULL) {
        dims = mxGetDimensions(cell_array);
    }
    
    // Check if this dimension is 1 and if so, recurse immediately,
    // or (if we're at the bottom), return the last element.
    if (flatten1 && dims[idx_dim] == 1) {
        subs[idx_dim] = 0;
        
        // Decide to recurse of return base case.
        if (idx_dim == nsubs - 1) {
            new_el = mat2py(
                mxGetCell(cell_array, mxCalcSingleSubscript(
                    cell_array, nsubs, subs
                )), false
            );
        } else {
            // Jump ahead one level.
            new_el = py_list_from_cell_array(
                cell_array, idx_dim + 1, nsubs, subs, dims, flatten1
            );
        }
        
        // We should always return a list of some kind, so if we're at the 0th
        // dimension and about to return a non-list, do something different.
        if (!PyList_Check(new_el)) {
            py_list = PyList_New(1);
            PyList_SetItem(py_list, 0, new_el);
            return py_list;
        } else {
            return new_el;
        }
    }
    
    // We're still here, so there must be something to do.
    py_list = PyList_New(dims[idx_dim]);
    for (idx_el = 0; idx_el < dims[idx_dim]; idx_el++) {
        subs[idx_dim] = idx_el;
        if (idx_dim != nsubs - 1) {
            new_el = py_list_from_cell_array(
                cell_array, idx_dim + 1, nsubs, subs, dims, flatten1
            );
        } else {
            new_el = mat2py(
                mxGetCell(cell_array, mxCalcSingleSubscript(
                    cell_array, nsubs, subs
                )), false
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

// INIT FUNCTIONS //////////////////////////////////////////////////////////////

void init_marshal_types() {
    // TODO: move init_py_mxArray here.

    PyObject *pymex_module, *pymex_dict, *mtypes_module, *mtypes_dict;
    // Assume the pymex module has already been imported, so we can
    // borrow the reference.
    pymex_module = PyImport_AddModule("pymex");
    pymex_dict = PyModule_GetDict(pymex_module);
    mtypes_module = PyDict_GetItemString(pymex_dict, "mtypes");
    mtypes_dict = PyModule_GetDict(mtypes_module);

    py_struct = PyDict_GetItemString(mtypes_dict, "struct");

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
    
    if (py_value == NULL) {
        mexErrMsgTxt("Python value to marshal was NULL. This shouldn't happen.");
    }
    
    
    if (is_boxed_mxarray(py_value)) {
        mat_value = unbox_mxarray(py_value);
        // Py_XDECREF(py_value);
        return mat_value;
    }
    
    // TODO: Right now, we aren't converting any types, but are just boxing
    //       them up.
    if (PyString_Check(py_value)) {
        char *bufs[1];
        bufs[0] = PyString_AsString(py_value);
        mat_value = mxCreateCharMatrixFromStrings(1, bufs);
        Py_XDECREF(py_value);
    } else if (PyBool_Check(py_value)) {
        bool c_value = PyObject_IsTrue(py_value);
        mat_value = mxCreateLogicalScalar(c_value);
        Py_XDECREF(py_value);
    } else if (PyFloat_Check(py_value)) {
        mat_value = mxCreateDoubleScalar(PyFloat_AsDouble(py_value));
        Py_XDECREF(py_value);
    } else if (PyLong_Check(py_value)) {
        long long int int_value = PyLong_AsLongLong(py_value);
        mwSize dims[2] = {1, 1};
        mat_value = mxCreateNumericArray(2, dims, mxINT64_CLASS, mxREAL);
        *(long long int*)mxGetData(mat_value) = int_value;        
        Py_XDECREF(py_value);    
    } else if (PyInt_Check(py_value)) {
        long int int_value = PyInt_AsLong(py_value);
        mwSize dims[2] = {1, 1};
        mat_value = mxCreateNumericArray(2, dims, mxINT32_CLASS, mxREAL);
        *(long int*)mxGetData(mat_value) = int_value;
        Py_XDECREF(py_value);
    } else if (PyList_Check(py_value)) {
        // Make a 1xn cell array, and then pack everything into it by
        // calling py2mat recursively. This will make ugly structures
        // for nested Python lists, but it's kind of unavoidable due to
        // the ability of Python lists to be jagged.
        
        int idx_cell;
        int len = PyList_Size(py_value);
        mat_value = mxCreateCellMatrix(1, len);
        for (idx_cell = 0; idx_cell < len; idx_cell++) {
            mxSetCell(mat_value, idx_cell, py2mat(PyList_GetItem(py_value, idx_cell)));
        }
        Py_XDECREF(py_value);
    } else if (py_struct != NULL && PyObject_IsInstance(py_value, py_struct)) {
        int idx;
        PyObject *items;
        int len;
        char **field_names;


        items = PyDict_Items(py_value);
        len = PyList_Size(items);

        field_names = mxCalloc(len, sizeof(char*));
        for (idx = 0; idx < len; ++idx) {
            field_names[idx] = PyString_AsString(PyTuple_GetItem(PyList_GetItem(items, idx), 0));
        }
        
        mat_value = mxCreateStructMatrix(1, 1, len, field_names);
        for (idx = 0; idx < len; ++idx) {
            mxSetField(mat_value, 0, field_names[idx], py2mat(PyTuple_GetItem(PyList_GetItem(items, idx), 1)));
        }

        Py_XDECREF(items);
        Py_XDECREF(py_value);
    } else {
        mat_value = box_pyobject(py_value);
    }
    
    return mat_value;
    
}

/**
 * Given a MATLAB array, creates and returns a pointer to an appropriate
 * PyObject. If a new object cannot be created, returns NULL.
 *
 * @param flatten1: If true, dimensions of length one will be removed.
 *     [Default: false]
 */
PyObject* mat2py(const mxArray* m_value, bool flatten1) {
    
    PyObject* new_obj = NULL;
    char* buf;
    int nsubs;
    int n_fields;
    int idx_field;
    char* key;
    PyObject* value = NULL;
    
    if (m_value == NULL) {
        mexErrMsgTxt("MATLAB value to marshal was NULL. This shouldn't happen.");
    }
    
    // First, check if the MATLAB value is a boxed PyObject.
    if (is_boxed_pyobject(m_value)) {
        new_obj = unbox_pyobject(m_value);
        Py_INCREF(new_obj);
        return new_obj;
    }
    
    // If it wasn't, figure out how to marshal the native-MATLAB data.
    switch (mxGetClassID(m_value)) {
        
        case mxCELL_CLASS:
            nsubs = mxGetNumberOfDimensions(m_value);
            return py_list_from_cell_array(m_value, 0, nsubs, NULL, NULL, flatten1);
        
        case mxSTRUCT_CLASS:
            // TODO: enforce 1x1 shape.
            // Treat MATLAB structures as Python dicts.
            // Note that this breaks roundtrips (Python dicts are supersets
            // of MATLAB structs), but it's as close as we'll get.
            n_fields = mxGetNumberOfFields(m_value);
            new_obj = PyDict_New();
            for (idx_field = 0; idx_field < n_fields; ++idx_field) {
                key = mxGetFieldNameByNumber(m_value, idx_field);
                value = mat2py(mxGetFieldByNumber(m_value, 0, idx_field), false);
                PyDict_SetItemString(new_obj, key, value);
            }
            return new_obj;
        
        case mxDOUBLE_CLASS:
            if (ensure_mat_scalar(m_value)) {
                // new, so already owned.
                new_obj = PyFloat_FromDouble(mxGetScalar(m_value)); 
                return new_obj;
            } else break;
            
        case mxINT32_CLASS:
            if (ensure_mat_scalar(m_value)) {
                new_obj = PyInt_FromLong(*(long int*)mxGetData(m_value));
                return new_obj;
            } else break;
            
        case mxINT64_CLASS:
            if (ensure_mat_scalar(m_value)) {
                new_obj = PyLong_FromLongLong(*(long long int*)mxGetData(m_value));
                return new_obj;
            } else break;
            
        case mxCHAR_CLASS:
            get_matlab_str(m_value, &buf);
            // new, so already owned.
            new_obj = PyString_FromString(buf);
            return new_obj;
            
    }
    
    // If we got here, then we need to box it up.
    return box_mxarray(m_value);
    
}


// BOXING AND UNBOXING ////////////////////////////////////////////////////////
// These functions are for encapsulating un-interpreted values when
// marshalling. Uninterpreted Python objects are wrapped in the PyObject
// MATLAB class, while unintepreted MATLAB arrays are wrapped in the
// mxArray Python class. This allows for arbitrary data types to be sent
// back and forth, even if specific support is not built into pymex-embed.

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



bool is_boxed_mxarray(const PyObject* py_object) {
    init_py_mxArray();
    return (PyObject_IsInstance(py_object, py_mxArray) == 1);
}

mxArray* unbox_mxarray(const PyObject* py_object) {
    PyObject *ptr_attr;
    mxArray *ptr_val;
    
    // TODO: add type check.
    
    ptr_attr = PyObject_GetAttrString(py_object, "_mxArray__ptr");
    if (ptr_attr == NULL) {
        mexErrMsgTxt("Error unboxing an mxArray. Got a NULL pointer.");
    }
    ptr_val = (mxArray*) PyLong_AsUnsignedLongLong(ptr_attr);

    Py_XDECREF(ptr_attr);

    return ptr_val;
}

PyObject* box_mxarray(const mxArray* m_array) {
    
    PyObject *boxed_value = NULL;
    mxArray* dup_array;

    init_py_mxArray();

    // We need a duplicate of the array that we can persist.
    dup_array = mxDuplicateArray(m_array);
    mexMakeArrayPersistent(dup_array);

    // Now we call the constructor for the Python class mxArray
    // with the cell box as an argument.
    boxed_value = PyObject_CallFunction(py_mxArray, "K", (unsigned long long int) dup_array);
    // The boxed value is a new reference, so we already own it.
    return boxed_value;
    
}