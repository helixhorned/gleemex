% GLC_SLIDER_DRAW(SLISTRUCT, POSRECT, ARROWWIDTH)
%
% Draw a slider.
%  SLISTRUCT - as returned from glc_slider
%  POSRECT - [x1 y1 x2 y2] or the bounding rectangle
%  ARROWWIDTH - width of the arrow buttons to the left and right,
%   (x2-x1)-2*ARROWWIDTH must be > 0
function glc_slider_draw(slistruct, posrect, arrowwidth)
    global glc GL

%    glcall(glc.draw, GL.LINE_LOOP, glc_expandrect(posrect));
    lrect = [posrect(1) posrect(2) posrect(1)+arrowwidth posrect(4)];
    rrect = [posrect(3)-arrowwidth, posrect(2) posrect(3) posrect(4)];

    innerposrect = posrect + [arrowwidth 0 -arrowwidth 0];

    width = innerposrect(3)-innerposrect(1);

    p1 = [innerposrect(1) + (slistruct.idx-1)/(numel(slistruct.vals))*width; innerposrect(2)];
    p2 = [innerposrect(1) + slistruct.idx/(numel(slistruct.vals))*width; innerposrect(4)];

    % slider rect
    glcall(glc.draw, GL.QUADS, glc_expandrect([p1, p2]), struct('colors',[0.8 0.8 0.2]));

    % bounding rect (w/o arrows)
    glcall(glc.draw, GL.QUADS+16, [glc_expandrect(lrect) glc_expandrect(innerposrect) glc_expandrect(rrect)]);
end
