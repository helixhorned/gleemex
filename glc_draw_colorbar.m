% GLC_DRAW_COLORBAR(RECT, BOUNDS, LABEL, CMAPTEXNAME [, NUMTICKS [, TEXTHEIGHT]])
%
%  RECT - [x1 y1 x2 y2] bounding box
%  BOUNDS - [min max]
%  LABEL - the string to print near the color bar
%  CMAPTEXNAME - the color map 2D texture name (ID)
%  NUMTICKS - the number of tick marks, divided between min and max
%             bound. Default: 7
%  TEXTHEIGHT - the text height passed to glc.text.  Default: 10
function glc_draw_colorbar(rect, bounds, label, cmaptexname, numticks, textheight)
    global glc GL

    if (~exist('numticks', 'var'))
        numticks = 7;
    end

    if (~exist('textheight', 'var'))
        textheight = 10;
    end

    glcall(glc.draw, GL.QUADS, glc_expandrect(rect), struct(...
        'tex', cmaptexname, 'texcoords', glc_expandrect([0 1; 0 1]), 'colors',[1 1 1]));
    glcall(glc.draw, GL.QUADS+16, glc_expandrect(rect));

    if (~isempty(label))
        xy_origin = rect(1:2) + [0, (rect(4)-rect(2))/2];
        glc_rotatetext(xy_origin, 90,  [0 16], 14, label, [0 -1]);
    end

    for i=0:numticks-1
        frac = (i/(numticks-1));
        y = rect(2) + frac*(rect(4)-rect(2)); % - textheight/2;

        glcall(glc.text, [rect(3)+8, y], textheight, ...
               sprintf('%.02f', bounds(1)+frac*(bounds(2)-bounds(1))), [-1 0]);
    end
end
