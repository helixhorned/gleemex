try
    [1](1)
    % Octave
    mkoctfile -mex -W -Wall -Wextra -Werror-implicit-function-declaration -lGL -lGLU -lGLEW -lglut glcall.c
catch
    % MATLAB
    mex -lglew32 glcall.c
end
