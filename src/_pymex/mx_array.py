# -*- coding: utf-8 -*-
##
# mx_array.py: 
##
# (c) 2013 Christopher E. Granade (cgranade@cgranade.com).
#    
# This file is a part of the pymex-embed project.
# Licensed under the AGPL version 3.
##
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU Affero General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Affero General Public License for more details.
#
# You should have received a copy of the GNU Affero General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
##

## FEATURES ###################################################################

from __future__ import division

## IMPORTS ####################################################################

import pymex
from _pymex.mat_funcs import matfunc
import _pymex.mtypes as M

from functools import partial

## CLASSES ####################################################################

class mxArray(object):
    def __init__(self, ptr):
        self.__ptr = ptr
        # We can't use the function handle approach to feval, since
        # we need for the mxArray import to be complete before then.
        self._class = pymex.feval("class", self)
        self._props = set([item[0] for item in pymex.feval("properties", self._class)])
        self._methods = set([item[0] for item in pymex.feval("methods", self._class)])
    
    def __del__(self):
    	# TODO: we will need to destroy the pointer here!
    	# For now, we pass, but this causes a bloody huge memory leak
    	# until the TODO is fixed.
    	pass

    def __repr__(self):
        return "<mxArray at 0x{:x} (0x{:x} in MATLAB)>".format(id(self), self.__ptr)

    def __call__(self, *args, **kwargs):
        if self._class == 'function_handle':
            return pymex.feval(self, *args, **kwargs)
        else:
            raise TypeError("mxArray of MATLAB class {} is not callable.".format(self._class))

    def __add__(self, other):
        return matfunc('plus')(self, other)

    def __eq__(self, other):
        return matfunc('eq')(self, other)

    def __getattr__(self, name):
        if name in self._props:
            return matfunc('subsref')(self, M.struct(type='.', subs=name))
        elif name in self._methods:
            return partial(matfunc(name), self)
