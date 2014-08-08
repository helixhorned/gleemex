% MKGLCALL([BUILD_RELEASE [, USE_RELEASE [, GL2PS]]]) Attempt to compile
% glcall.c into a MEX file suitable for the combination of M language
% interpreter and OS/compiler.
%
% Currently supported combinations are:
%  - Octave running on Linux (any supported architecture)
%  - MATLAB running on Windows/x64
%  - MATLAB running on Linux (TESTED LONG AGO)
% To a lesser extent,
%  - Octave running on Windows, using MinGW32
%  - MATLAB running Windows/x86 (TODO: NEEDS UPDATE)
%
% The two optional arguments BUILD_RELEASE and USE_RELEASE (both false by
% default) control two separate levels of debugging.
% NOTE: Currently, only effective for MATLAB/x64.
%  - If BUILD_RELEASE is true, build a release (optimized, no debug symbols)
%    version of glcall. Else, build a debugging version.
%  - If USE_RELEASE is true, prefer using a release version of FreeGLUT to a
%    debugging one.
%
% GL2PS: if true (default: false), build with gl2ps support, enabling the
%  glc.beginpage command. Currently, only for Linux.
function mkglcall(build_release, use_release, gl2ps)
    if (nargin < 1)
        build_release = false;
    end
    if (nargin < 2)
        use_release = false;
    end
    if (nargin < 3)
        gl2ps = false;
    end

    if (~exist('OCTAVE_VERSION', 'builtin'))
        % MATLAB

        if (ispc)
            % Windows
            args = {'-g', '-Iourinclude', '-L.', '-lfreeglutd', 'glcall.c'};
            if (use_release)
                args{4} = '-lfreeglut';
            end
            if (build_release)
                args(1) = [];
            end

            if (strcmp(computer('arch'), 'win64'))
                mex(args{:});
            else
                error('Windows/x86-MATLAB build unsupported')
                % no 32-bit libs yet
%                mex -Iourinclude -lglew32 glcall.c
            end
        else
            mex -lGL -lGLU -lGLEW -lglut glcall.c
        end

        return
    end

    % Octave
    % For building using a different compiler, run from the command line with CC=... prepended
    % (e.g. CC='clang -fsanitizer=address,undefined' mkoctfile ...)
    if (~isempty(strfind(computer(), 'linux')))
        % linux (generic)
        OPTS = {'--mex', '-g', '-W', '-Wall', '-Wextra', '-Werror-implicit-function-declaration', ...
                '-lGL', '-lGLU', '-lGLEW', '-lglut'};
        if (gl2ps)
            OPTS{end+1} = '-lgl2ps';
            OPTS{end+1} = '-DGLEEMEX_USE_GL2PS';
        end
        mkoctfile(OPTS{:}, 'glcall.c');
    else
        % mingw32
        mkoctfile --mex -g -W -Wall -Wextra -Iourinclude -Llib32 -lopengl32 -lglu32 -lglew32 -lfreeglut glcall.c
    end
end
