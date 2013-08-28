%%
% TestPyObject.m: Unit tests for PyObject wrapper.
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

classdef TestPyObject < tests.PyTestCase
 
    methods (Test)

        function testGetItem(testCase)
            s = struct('a', 'a_key', 'b', 42.0);
            py_put('x', s);
            x = py_get('x');
            testCase.assertEqual(x{'a'}, 'a_key');
            testCase.assertEqual(x{'b'}, 42.0);
        end
        
        function testMul(testCase)
            % FIXME: this relies on complexes not marshalling; change to
            %        a stub class.
            py_eval('x = 1+2j');
            x = py_get('x');
            py_put('y', x*2);
            testCase.pyAssertTrue('y == 2+4j');
        end
        
        function testComparisons(testCase)
            py_eval('from tests.stub_classes import A, B1, B2, C');
            A = py_get('A');
            B1 = py_get('B1');
            B2 = py_get('B2');
            C = py_get('C');
            testCase.assertTrue(A < B1);
            testCase.assertTrue(B1 == B2);
            testCase.assertTrue(B1 <= B2);
            testCase.assertTrue(C > B2);
            testCase.assertTrue(C >= A);
            testCase.assertTrue(A ~= C);
        end
   
    end
        
end
