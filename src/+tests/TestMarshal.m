%%
% TestMarshal.m: Unit tests for data marshalling across MATLAB/Python boundary.
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

classdef TestMarshal < matlab.unittest.TestCase

    methods (Access = private)
        
        function pyAssertTrue(testCase, s)
            % WARNING: uses the name _assert_var.
            py_eval(['_assert_var = ' s]);
            testCase.assertTrue(py_get('_assert_var'));
        end
    end
    
    methods (Test)
    
        function testPutString(testCase)
            py_put('x', 'foo');
            testCase.pyAssertTrue('x == "foo"');
            testCase.pyAssertTrue('isinstance(x, str)');
        end
        
        function testGetString(testCase)
            py_eval('x = "foo"');
            x = py_get('x');
            testCase.assertEqual(x, 'foo');
            testCase.assertTrue(ischar(x));
        end
        
        function testPutDoubleScalar(testCase)
            py_put('x', 42.0);
            testCase.pyAssertTrue('x == 42.0');
            testCase.pyAssertTrue('isinstance(x, float)');
        end
        
        function testGetDoubleScalar(testCase)
            py_eval('x = 42.0');
            x = py_get('x');
            testCase.assertEqual(x, 42.0);
            testCase.assertTrue(isa(x, 'double'));
        end
        
        function testPutInt64Scalar(testCase)
            py_put('x', int64(42));
            testCase.pyAssertTrue('x == 42');
            testCase.pyAssertTrue('isinstance(x, int)');
        end
        
        function testGetInt64Scalar(testCase)
            py_eval('x = 42');
            x = py_get('x');
            testCase.assertEqual(x, int64(42));
            testCase.assertTrue(isa(x, int64));
        end
        
        function testPutHorizCell(testCase)
            py_put('x', {1, 'a'})
            testCase.pyAssertTrue('x == [[1, "a"]]');
            testCase.pyAssertTrue('isinstance(x, list)');
        end
        
        function testPutVertCell(testCase)
            py_put('x', {1; 'a'})
            testCase.pyAssertTrue('x == [[1], ["a"]]');
            testCase.pyAssertTrue('isinstance(x, list)');
        end
        
        function testGetList(testCase)
            py_eval('x = [1, "a"]');
            x = py_get('x');
            testCase.assertEqual(x, {1, 'a'});
            testCase.assertTrue(iscell(x));
        end
    
    end

end
