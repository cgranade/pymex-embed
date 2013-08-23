%%
% py_function_t.m: Class copying `function_t` enumeration from pymex_fns.c.
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

classdef (Sealed) py_function_t

    % Note that we do not use an enumeration block here, as we do not want
    % Java-style enums, but rather something where the constants are literal
    % int values. Even subclassing from int8 does not result in the expected
    % behavior, as such values are not passed correctly via MEX.

    % Prevent construction.
    methods (Access = private)
        function self = py_function_t()
        end
    end
    
    %% ENUM VALUES %%
    properties (Access = public, Constant)
        EVAL = int8(0);
        IMPORT = int8(1);
        DECREF = int8(2);
        STR = int8(3);
        PUT = int8(4);
        GET = int8(5);
        GETATTR = int8(6);
        CALL = int8(7);
        GETITEM = int8(8);
        MUL = int8(9);
    end

end
