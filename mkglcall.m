try
    octave_core_file_name;

    % Octave
    mkoctfile -mex -W -Wall -Wextra -Werror-implicit-function-declaration -lGL -lGLU -lGLEW -lglut glcall.c
catch
    % MATLAB
    mex -lglew32 glcall.c
end
