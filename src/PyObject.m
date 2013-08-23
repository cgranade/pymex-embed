%%
% PyObject.m: MATLAB class wrapping access to PyObject structures.
%%
% (c) 2013 Christopher E. Granade (cgranade@cgranade.com).
%    
% This file is a part of the pymex-embed project.
% Licensed under the AGPL version 3.
%%
% This program is free software: you can redistribute it and/or modify
% it under the terms of the GNU Affero General Public License as published by
% the Free Software Foundation, either version 3 of the License, or
% (at your option) any later version.
%
% This program is distributed in the hope that it will be useful,
% but WITHOUT ANY WARRANTY; without even the implied warranty of
% MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
% GNU Affero General Public License for more details.
%
% You should have received a copy of the GNU Affero General Public License
% along with this program.  If not, see <http://www.gnu.org/licenses/>.
%%

classdef PyObject < handle

    properties (Access = public) % FIXME: public only for debugging purposes.
        py_pointer = [];
    end
    
    methods (Access = private)
        function self = PyObject(py_ptr)
            % FIXME: catch PyNone!
            self.py_pointer = py_ptr;
        end
    end
    
    methods (Static)
    
        function newobj = new(py_ptr)
            if py_ptr == 0
                newobj = [];
            else
                newobj = PyObject(py_ptr);
            end
        end
    
    end

    methods
        
        %% MATLAB MAGIC METHODS %%
        
        function delete(self)
            pymex_fns(py_function_t.DECREF, self);
        end
        
        function b = subsref(self, subs)
            if strcmp(subs.type, '.')
                b = getattr(self, subs.subs);
            elseif strcmp(subs.type, '()')
                b = call(self, subs.subs{:});
            elseif strcmp(subs.type, '{}')
                b = getitem(self, subs.subs{:});
            end
        end
        
        function disp(self)
            fprintf('%s\n', str(self))
        end
        
        function display(self)
            fprintf('%s\n', str(self))
        end
        
        function product = mtimes(self, other)
            product = pymex_fns(py_function_t.MUL, self, other);
        end
        
        %% OTHER METHODS %%
        
        function retval = call(self, varargin)
            % TODO: unpack retval into a varargout.
            retval = pymex_fns(py_function_t.CALL, self, varargin);
        end
        
        function s = dir(self)
            s = call(py_builtins.dir, self);
        end
        
        function s = str(self)
            s = pymex_fns(py_function_t.STR, self);
        end
        
        function s = repr(self)
            s = call(py_builtins.repr, self);
        end
        
        function obj = getattr(self, name)
            obj = pymex_fns(py_function_t.GETATTR, self, name);
        end
        
        function value = getitem(self, key)
            value = pymex_fns(py_function_t.GETITEM, self, key);
        end
        
    end

end
