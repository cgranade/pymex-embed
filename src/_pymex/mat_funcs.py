# -*- coding: utf-8 -*-
##
# redirect_io.py: Classes for redirecting I/O to MATLAB's Command Window.
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

## GLOBALS ####################################################################

str2func = None

## FUNCTIONS ##################################################################

def matfunc(name):
    global str2func
    if str2func is None:
        str2func = pymex.feval('str2func', 'str2func')

    r = pymex.feval(str2func, name)
    return r
