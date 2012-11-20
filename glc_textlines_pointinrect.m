% POINT_IN_I = GLC_TEXTLINES_POINTINRECT(NUMLINES, XRANGE, YORIGIN)
function point_in_i = glc_textlines_pointinrect(numlines, xrange, yorigin, lineheight, mxy)
    if (nargin ~= 5)
        error('Must have exactly 5 input arguments');
    end

    for i=1:numlines
        y1 = yorigin - i*lineheight;

        bb = [xrange; ...
              y1, y1+lineheight];

        if (glc_pointinrect(mxy, bb))
            point_in_i = i;
            return;
        end
    end

    point_in_i = 0;
end
