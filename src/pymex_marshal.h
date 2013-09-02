/**
 * pymex_marshal.h: Functions for marshalling data between MATLAB and Python.
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

// PRAGMAS AND INCLUDE GUARD ///////////////////////////////////////////////////

#pragma once
#ifndef PYMEX_MARSHAL_H
#define PYMEX_MARSHAL_H

// INCLUDES ////////////////////////////////////////////////////////////////////

#include <Python.h>
#include <mex.h>

// PROTOTYPES //////////////////////////////////////////////////////////////////

void get_matlab_str(const mxArray* m_str, char** c_str);

PyObject* py_obj_from_mat_scalar(const mxArray* m_scalar);
mxArray* mat_scalar_from_py_obj(const PyObject* py_obj);

PyObject* mat2py(const mxArray* m_value, bool flatten1);
//mxArray* py2mat(const PyObject* py_value);
mxArray* py2mat(const PyObject* py_value);

PyObject* py_list_from_cell_array(
    const mxArray* cell_array, int idx_dim, mwSize nsubs, mwIndex* subs,
    mwIndex* dims, bool flatten1
);

bool is_boxed_pyobject(const mxArray* mat_array);
PyObject* unbox_pyobject(const mxArray* mat_array);
mxArray* box_pyobject(const PyObject* py_object);

bool is_boxed_mxarray(const PyObject* py_object);
mxArray* unbox_mxarray(const PyObject* py_object);
PyObject* box_mxarray(const mxArray* m_array);

#endif
