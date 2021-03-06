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
import sys
import _pymex.mat_funcs as mf

## CLASSES ####################################################################

class PymexStdout(object):
    def write(self, val):
        pymex.matwrite(val)

class PymexStderr(object):
    def __init__(self):
        try:
            self._fprintf = mf.matfunc('fprintf')
        except:
            self._fprintf = 'fprintf'
        
    def write(self, val):
        # FIXME: this is a placeholder until feval is implemented
        # as a callable object.
        pymex.feval(self._fprintf, 2.0, val, nargout=0)

## FUNCTIONS ##################################################################

def redirect_io():
    old_stdout = sys.stdout
    sys.stdout = PymexStdout()
    old_stderr = sys.stderr
    sys.stderr = PymexStderr()
