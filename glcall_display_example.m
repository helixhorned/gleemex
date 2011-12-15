function glcall_display_example()

GL = getglconsts(); glc = glcall();

glcall(glc.draw, GL.LINE_STRIP, [1 2 3 4 5; 4 5 3 2 1]./5);
