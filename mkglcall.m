try
    octave_core_file_name;

    % Octave
    mkoctfile -mex -W -Wall -Wextra -Werror-implicit-function-declaration -lGL -lGLU -lGLEW -lglut glcall.c
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
