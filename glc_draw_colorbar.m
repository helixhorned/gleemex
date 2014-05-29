% GLC_DRAW_COLORBAR(RECT, BOUNDS, LABEL, CMAPTEXNAME [, FORMAT [, NUMTICKS [, TEXTHEIGHT]]])
%
% RECT: [x1 y1 x2 y2] bounding box
% BOUNDS: [min max]
% LABEL: the string to print near the color bar
% CMAPTEXNAME: the color map 2D texture name (ID)
% FORMAT: defaults to '%.02f'
% NUMTICKS: the number of tick marks, divided between min and max
%  bound. If negative, glc_genticks() is used with -NUMTICKS ticks.
%  Default: 7
% TEXTHEIGHT: the text height passed to glcall(glc.text, ...).
%  Default: 10
function glc_draw_colorbar(rect, bounds, label, cmaptexname, format, numticks, textheight)
    global glc GL

    if (~exist('numticks', 'var'))
        numticks = 7;
    end

    if (~exist('textheight', 'var'))
        textheight = 10;
    end

    if (~exist('format', 'var'))
        format = '%.02f';
    end

    glcall(glc.draw, GL.QUADS, glc_expandrect(rect), struct(...
        'tex', cmaptexname, 'texcoords', [0 0 1 1]));
    glcall(glc.draw, GL.QUADS+16, glc_expandrect(rect));

    if (~isempty(label))
        xy_origin = rect(1:2) + [0, (rect(4)-rect(2))/2];
        glc_rotatetext(xy_origin, 90,  [0 16], 14, label, [0 -1]);
    end

    if (numticks < 0)
        ticks = glc_genticks(bounds, -numticks);
        glc_drawticks(rect([3 2 3 4]), bounds, ticks, -3, -textheight);
    else
        for i=0:numticks-1
            frac = (i/(numticks-1));
            y = rect(2) + frac*(rect(4)-rect(2));

            glcall(glc.text, [rect(3)+8, y], textheight, ...
                   sprintf(format, bounds(1)+frac*(bounds(2)-bounds(1))), [-1 0]);
        end
    end
end
