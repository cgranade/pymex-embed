%%
% TestImport.m: Unit tests for importing packages.
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

classdef TestImport < tests.PyTestCase
    
    methods (Test)
    
        function testImportRootNoArgOut(testCase)
            py_import os
            testCase.assertEqual(getattr(os, '__name__'), 'os')
        end
        
        function testImportRootArgOut(testCase)
            os = py_import('os');
            testCase.assertEqual(getattr(os, '__name__'), 'os')
        end
        
        function testImportSubpackageNoArgOut(testCase)
            py_import tests.deep_package.a.b
            testCase.assertEqual(tests.deep_package.a.b.sentinel, int32(42))
        end
        
        function testImportSubpackageArgOut(testCase)
            b = py_import('tests.deep_package.a.b');
            testCase.assertEqual(b.sentinel, int32(42))
        end
    
    end

end
