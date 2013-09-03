%%
% TestMxArray.m: Tests for mxArray Python class.
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

classdef TestMxArray < tests.PyTestCase
 
    methods (Test)

        function testRoundTripFunctionHandle(testCase)
            fn = @(x) x^2;
            py_put('fn', fn);
            fn2 = py_get('fn');
            testCase.assertEqual(fn, fn2);
        end
        
        function testCallSingleArgFunctionHandle(testCase)
            fn = @(x) x^2;
            py_put('fn', fn);
            py_eval('y = fn(10)');
            testCase.pyAssertTrue('y == 100');
        end
        
        function testCallClosureHandle(testCase)
            closure_called = false;
            function example_closure()
                closure_called = true;
            end
            py_put('fn', @example_closure);
            py_eval('fn(nargout=0)');
            testCase.assertTrue(closure_called);
        end
        
        function testCallMultipleArgFunctionHandle(testCase)
            function [y, z] = example_fn(x)
                y = x^2;
                z = x^3;
            end
            py_put('fn', @example_fn);
            py_eval('y, z = fn(10, nargout=2)');
            testCase.pyAssertTrue('y == 100 and z == 1000');
        end

        function testGetProperty(testCase)
            py_put('x', tests.ExampleClass);
            testCase.pyAssertTrue('x.foo == 42');
        end

        function testCallMethod(testCase)
            py_put('x', tests.ExampleClass);
            testCase.pyAssertTrue('x.odd(3)')
        end
   
    end
        
end
