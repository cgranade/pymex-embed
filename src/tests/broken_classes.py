# -*- coding: utf-8 -*-
##
# broken_classes.py: Classes that cause myriad errors, to test robustness of
#     MEX wrapper.
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

from functools import total_ordering

## CLASSES ####################################################################

@total_ordering
class Incomparable(object):
    """
    A class of objects that complains whenever an attempt is made to compare it
    to another object.
    """ 
    def __eq__(self, other):
        raise NotImplementedError("Cannot test equality.")
        
    def __le__(self, other):
        raise NotImplementedError("Cannot compare ordering.")

class Unarithmetic(object):
    """
    A class of objects that cannot be added, subtracted, multiplied, etc. to
    other objects.
    """
    def __mul__(self, other):
        raise NotImplementedError("Cannot be multiplied.")
        
    def __add__(self, other):
        raise NotImplementedError("Cannot be added.")

## FUNCTIONS ###################################################################

def raises():
    raise RuntimeError("Example runtime error.")
