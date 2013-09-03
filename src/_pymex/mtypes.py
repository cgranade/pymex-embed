# -*- coding: utf-8 -*-
##
# mtypes.py: Classes for use in generating specialized MATLAB types.
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

import re

## FUNCTIONS ##################################################################

def is_valid_matlab_ident(ident):
    if not isinstance(ident, str):
        return False
    return bool(re.match(r"[a-z][a-z0-9_]*", ident))

## CLASSES ####################################################################

class struct(dict):
    def __setitem__(self, key, value):
        if is_valid_matlab_ident(key):
            super(struct, self).__setitem__(key, value)
        else:
            raise ValueError("{} is not a valid MATLAB identifier.".format(key))
