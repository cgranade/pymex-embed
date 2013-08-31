%%
% TestErrors.m: Unit tests for ensuring that errors are handled properly.
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

classdef TestErrors < tests.PyTestCase
    
    methods (Test)
    
        function testCmpErrors(testCase)
            py_import tests.broken_classes
            obj1 = tests.broken_classes.Incomparable();
            obj2 = tests.broken_classes.Incomparable();
            testCase.verifyError(@() obj1 < obj2, 'foo');
            testCase.verifyError(@() obj1 == obj2, 'foo');
        end
        
    end

end
