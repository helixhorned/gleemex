try
    OCTAVE_VERSION;
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
    return
end

    % Octave
    % For building using a different compiler, run from the command line with CC=... prepended
    % (e.g. CC='clang -fcatch-undefined-behavior' mkoctfile ...)
    if (~isempty(strfind(computer(), 'linux')))
        % linux (generic)
        mkoctfile --mex -g -W -Wall -Wextra -Werror-implicit-function-declaration -lGL -lGLU -lGLEW -lglut glcall.c
    else
        % mingw32
        mkoctfile --mex -g -W -Wall -Wextra -Iourinclude -Llib32 -lopengl32 -lglu32 -lglew32 -lfreeglut glcall.c
    end
