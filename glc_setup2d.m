function glc_setup2d(w, h)
    global glc GL

    glcall(glc.viewport, [0 0 w h]);
    glcall(glc.setmatrix, GL.PROJECTION, [0 w, 0 h, -1 1]+0.5);
    glcall(glc.setmatrix, GL.MODELVIEW, []);
end
