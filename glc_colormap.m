% CMAPTEXNAME = GLC_COLORMAP(CMAP_OR_FUNC [, CMAPTEXNAME])
%
% Sets the GLCALL internal colormap and uploads its 2D RBG texture
% (width 256 x height 2).
%
% CMAP_OR_FUNC should be either
%  - a 3xN or Nx3 double (0..1) or uint8 (0..255) array
%  - a handle to a colormap function, e.g. @hot
function cmaptexname = glc_colormap(cmap_or_func, cmaptexname)
    global glc

    if (~(nargout <= 1 && nargin == 2 || nargout == 1 && nargin == 1))
        error('Usage: CMAPTEXNAME = GLC_COLORMAP(CMAP_OR_FUNC [, CMAPTEXNAME])');
    end

    if (nargin < 2)
        cmaptexname = 0;
    end

    if (isa(cmap_or_func, 'uint8'))
        cmap_img = cmap_or_func;
    elseif (isa(cmap_or_func, 'double'))
        cmap_img = uint8(cmap_or_func * 255);
    elseif (isa(cmap_or_func, 'function_handle'))
        cmap_img = uint8(cmap_or_func(256) * 255);  % should be 256x3
    else
        error('CMAP_OR_FUNC must be a uint8 or double matrix, or a function handle');
    end

    if (size(cmap_img, 1) ~= 3)
        cmap_img = cmap_img.';
    end
    glcall(glc.colormap, cmap_img);  % will throw error if cmap_img hasn't dims 3x256

    % make texture have dims (3x)256x2 ...
    cmap_img(:,:,2) = cmap_img(:,:,1);

    if (cmaptexname == 0)
        % upload first time
        cmaptexname = glcall(glc.texture, cmap_img);
    else
        % reupload
        glcall(glc.texture, cmap_img, cmaptexname);
    end
end
