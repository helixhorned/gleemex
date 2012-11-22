try
    OCTAVE_VERSION;

    % Octave
    % For building using a different compiler, run from the command line with CC=... prepended
    % (e.g. CC='clang -fcatch-undefined-behavior' mkoctfile ...)
    mkoctfile --mex -g -W -Wall -Wextra -Werror-implicit-function-declaration -lGL -lGLU -lGLEW -lglut glcall.c
catch
    % MATLAB
    if (ispc)
        if (strcmp(computer('arch'), 'win64'))
            mex -Iourinclude -Llib64 -lglew32 -lfreeglutd glcall.c
        else
            % no 32-bit libs yet
            mex -Iourinclude -lglew32 glcall.c
        end
    else
        mex -lGL -lGLU -lGLEW -lglut glcall.c
    end
end
