function glc_setup2d(w, h, zrange)
    global glc GL

    if (nargin < 3)
        zrange = [-1 1];
    end

    glcall(glc.viewport, [0 0 w h]);
    glcall(glc.setmatrix, GL.PROJECTION, [[0 w, 0 h]+0.5, zrange]);
    glcall(glc.setmatrix, GL.MODELVIEW, []);
end
