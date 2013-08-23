%%
% py_builtins.m: Class providing builtin functions from Python for use within
%     MATLAB.
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

classdef (Sealed) py_builtins

    % Note that we do not use an enumeration block here, as we do not want
    % Java-style enums, but rather something where the constants are literal
    % int values. Even subclassing from int8 does not result in the expected
    % behavior, as such values are not passed correctly via MEX.

    % Prevent construction.
    methods (Access = private)
        function self = py_builtins()
        end
    end
    
    %% FUNCTION VALUES %%
    % Generated using:
    % >>> import __builtin__
    % >>> print "\n".join([(" " * 8) + ("{0} = py_get('{0}')".format(builtin_name)) for builtin_name in dir(__builtin__) if not builtin_name.startswith('_')])
    properties (Access = public, Constant)
        ArithmeticError = py_get('ArithmeticError')
        AssertionError = py_get('AssertionError')
        AttributeError = py_get('AttributeError')
        BaseException = py_get('BaseException')
        BufferError = py_get('BufferError')
        BytesWarning = py_get('BytesWarning')
        DeprecationWarning = py_get('DeprecationWarning')
        EOFError = py_get('EOFError')
        Ellipsis = py_get('Ellipsis')
        EnvironmentError = py_get('EnvironmentError')
        Exception = py_get('Exception')
        False = py_get('False')
        FloatingPointError = py_get('FloatingPointError')
        FutureWarning = py_get('FutureWarning')
        GeneratorExit = py_get('GeneratorExit')
        IOError = py_get('IOError')
        ImportError = py_get('ImportError')
        ImportWarning = py_get('ImportWarning')
        IndentationError = py_get('IndentationError')
        IndexError = py_get('IndexError')
        KeyError = py_get('KeyError')
        KeyboardInterrupt = py_get('KeyboardInterrupt')
        LookupError = py_get('LookupError')
        MemoryError = py_get('MemoryError')
        NameError = py_get('NameError')
        None = py_get('None')
        NotImplemented = py_get('NotImplemented')
        NotImplementedError = py_get('NotImplementedError')
        OSError = py_get('OSError')
        OverflowError = py_get('OverflowError')
        PendingDeprecationWarning = py_get('PendingDeprecationWarning')
        ReferenceError = py_get('ReferenceError')
        RuntimeError = py_get('RuntimeError')
        RuntimeWarning = py_get('RuntimeWarning')
        StandardError = py_get('StandardError')
        StopIteration = py_get('StopIteration')
        SyntaxError = py_get('SyntaxError')
        SyntaxWarning = py_get('SyntaxWarning')
        SystemError = py_get('SystemError')
        SystemExit = py_get('SystemExit')
        TabError = py_get('TabError')
        True = py_get('True')
        TypeError = py_get('TypeError')
        UnboundLocalError = py_get('UnboundLocalError')
        UnicodeDecodeError = py_get('UnicodeDecodeError')
        UnicodeEncodeError = py_get('UnicodeEncodeError')
        UnicodeError = py_get('UnicodeError')
        UnicodeTranslateError = py_get('UnicodeTranslateError')
        UnicodeWarning = py_get('UnicodeWarning')
        UserWarning = py_get('UserWarning')
        ValueError = py_get('ValueError')
        Warning = py_get('Warning')
        ZeroDivisionError = py_get('ZeroDivisionError')
        abs = py_get('abs')
        all = py_get('all')
        any = py_get('any')
        apply = py_get('apply')
        basestring = py_get('basestring')
        bin = py_get('bin')
        bool = py_get('bool')
        buffer = py_get('buffer')
        bytearray = py_get('bytearray')
        bytes = py_get('bytes')
        callable = py_get('callable')
        chr = py_get('chr')
        classmethod = py_get('classmethod')
        cmp = py_get('cmp')
        coerce = py_get('coerce')
        compile = py_get('compile')
        complex = py_get('complex')
        copyright = py_get('copyright')
        credits = py_get('credits')
        delattr = py_get('delattr')
        dict = py_get('dict')
        dir = py_get('dir')
        divmod = py_get('divmod')
        enumerate = py_get('enumerate')
        eval = py_get('eval')
        execfile = py_get('execfile')
        exit = py_get('exit')
        file = py_get('file')
        filter = py_get('filter')
        float = py_get('float')
        format = py_get('format')
        frozenset = py_get('frozenset')
        getattr = py_get('getattr')
        globals = py_get('globals')
        hasattr = py_get('hasattr')
        hash = py_get('hash')
        help = py_get('help')
        hex = py_get('hex')
        id = py_get('id')
        input = py_get('input')
        int = py_get('int')
        intern = py_get('intern')
        isinstance = py_get('isinstance')
        issubclass = py_get('issubclass')
        iter = py_get('iter')
        len = py_get('len')
        license = py_get('license')
        list = py_get('list')
        locals = py_get('locals')
        long = py_get('long')
        map = py_get('map')
        max = py_get('max')
        memoryview = py_get('memoryview')
        min = py_get('min')
        next = py_get('next')
        object = py_get('object')
        oct = py_get('oct')
        open = py_get('open')
        ord = py_get('ord')
        pow = py_get('pow')
        print = py_get('print')
        property = py_get('property')
        quit = py_get('quit')
        range = py_get('range')
        raw_input = py_get('raw_input')
        reduce = py_get('reduce')
        reload = py_get('reload')
        repr = py_get('repr')
        reversed = py_get('reversed')
        round = py_get('round')
        set = py_get('set')
        setattr = py_get('setattr')
        slice = py_get('slice')
        sorted = py_get('sorted')
        staticmethod = py_get('staticmethod')
        str = py_get('str')
        sum = py_get('sum')
        super = py_get('super')
        tuple = py_get('tuple')
        type = py_get('type')
        unichr = py_get('unichr')
        unicode = py_get('unicode')
        vars = py_get('vars')
        xrange = py_get('xrange')
        zip = py_get('zip')
    end

end
