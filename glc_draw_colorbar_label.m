% glc_draw_colorbar_label(RECT, LABEL [, OFFSET [, HEIGHT]])
function glc_draw_colorbar_label(rect, label, offset, height)
    if (nargin < 3)
        offset = 16;
    end
    if (nargin < 4)
        height = 14;
    end

    xy_origin = rect(1:2) + [0, (rect(4)-rect(2))/2];
    glc_rotatetext(xy_origin, 90,  [0 offset], height, label, [0 -1]);
end
