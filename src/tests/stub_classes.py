# -*- coding: utf-8 -*-
##
# stub_classes.py: Simple classes for testing pymex functionality.
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
class ComparisonStub(object):
    """
    Wraps a comparible object (e.g. an int) to ensure that the data type is not
    marshaled, and exposes all of the Pythonic comparison operators.
    """
    def __init__(self, wrapped):
        self.wrapped = wrapped
        
    def __eq__(self, other):
        return isinstance(other, ComparisonStub) and self.wrapped == other.wrapped
        
    def __le__(self, other):
        return isinstance(other, ComparisonStub) and self.wrapped <= other.wrapped
        
## CONSTANTS ##################################################################

A, B1, B2, C = map(ComparisonStub, "abbc")

