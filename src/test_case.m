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

%% py_eval Basics %%
py_eval('x = 42');
x = py_get('x');
disp('Should print "42".');
disp(x);

%% Using py_eval to get Python builtins %%
py_eval('t = type');
py_type = py_get('t');
disp('Should print "<type ''type''>');
disp(py_type);
disp('Note that if py_get can''t convert a Python value, it wraps it in a PyObject:');
disp(class(py_type));

%% Calling PyObjects %%
% Double arrays are converted to Python floats.
disp(py_type(42));

%% Test py_put %%
% We can put values, too.
py_put('x', 'foo');
% Printing from within Python is currently very weird.
disp('should print foo');
py_eval print x

py_put('x', 42.0);
disp('should print 42.0');
py_eval print x  

%% Pulling from within Python %%
x = 42.1;
py_eval('import pymex');
py_eval('x = pymex.get("x")');
xq = py_get('x');
disp(xq);

%% Tests that produce wrong output %%
py_put('x', struct('x', 1, 'y', 'foo', 'z', struct('a', {'alpha', 'beta', 42}, 'b', 'echo')))
py_eval print x;

%% Test py_import and PyObject %%
% This test is very, very flaky!!

os = py_import('os');

