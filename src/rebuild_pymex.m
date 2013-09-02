%%
% rebuild_pymex.m: Wrapper for the MEX call needed to compile pymex_fns.c.
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

function rebuild_pymex(varargin)
    SRC_FILES = {'pymex_fns.c' 'pymex_marshal.c'};
    
    function s = mk_args(format, args)
        s = '';
        for arg = cellfun(@(x) sprintf(format, x), args, 'UniformOutput', false)
            s = [s ' ' arg{1}];
        end
        s = s(2:end);
    end

    if isunix
        INCLUDE = getpref('pymex', 'include',  '/usr/include/python2.7');
        LIBDIR = getpref('pymex', 'libdir', '');
        LIBS = {'python2.7', 'dl'};
        CFLAGS = '--std=c99 -fPIC -O0 -DLINUX';
        LDFLAGS = '\$LDFLAGS -Xlinker -export-dynamic';
        DEFINES = {'LINUX'};
        %mex -g CFLAGS="--std=c99 -fPIC -O0 -DLINUX" pymex_fns.c pymex_marshal.c LDFLAGS='\$LDFLAGS -Xlinker -export-dynamic' -I/usr/include/python2.7 -lpython2.7 -ldl
    elseif ispc
        INCLUDE = getpref('pymex', 'include', 'C:\Python27\Include');
        LIBDIR = getpref('pymex', 'libdir', 'C:\Python27\Libs');
        LIBS = {'python27'};
        CFLAGS = '';
        LDFLAGS = '';
        DEFINES = {'WINDOWS'};
        %mex -g pymex_fns.c pymex_marshal.c -I"C:\Program Files\Enthought\Canopy\App\appdata\canopy-1.0.3.1262.win-x86_64\include" -L"C:\Program Files\Enthought\Canopy\App\appdata\canopy-1.0.3.1262.win-x86_64\libs" -lpython27
    else
        disp('Not yet supported.');
    end
    
    EXTRA_OPTS = varargin;
    if ~isempty(LIBDIR)
        EXTRA_OPTS{end+1} = sprintf('-L%s', LIBDIR);
    end
    
    mex( ...
        '-g', ...
        sprintf('CFLAGS="%s"', CFLAGS), ...
        SRC_FILES{:}, ...
        sprintf('LDFLAGS="%s"', LDFLAGS), ...
        sprintf('-I%s', INCLUDE), ...
        EXTRA_OPTS{:}, ...
        mk_args('-l%s', LIBS), ...
        mk_args('-D%s', DEFINES) ...
    )
end
