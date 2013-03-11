%%
% test_case.m: Simple test cases for various pymex functions.
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

%% Test py_eval %%
py_eval y = 0
disp('should print 0');
py_eval print y
py_eval y = lambda t: t**2
disp('should print something about a lambda');

%% Test py_put %%
py_put('x', 'foo');
disp('should print foo');
py_eval print x

py_put('x', 42.0);
disp('should print 42.0');
py_eval print x  

%% Test py_import and PyObject %%
% This test is very, very flaky!!

os = py_import('os');

% After running this next line, clear all causes MATLAB to crash hard.
% Something is very wrong in str().
disp(str(os));
