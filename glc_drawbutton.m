% GLC_DRAWBUTTON(XYWH, THETEXT, MARKEDP)
%  XYWH - [x y width height], height must be >=5
%  THETEXT - text string
%  MARKEDP - either a scalar logical, or a numeric [X Y] vector which is
%   checked against the button bounds
function glc_drawbutton(xywh, thetext, markedp, clickedp)

    global GL glc

    assert(nargin==4);

    x = xywh(1);
    y = xywh(2);
    width = xywh(3);
    height = xywh(4);

    verts = [x x+width x+width  x;
             y y       y+height y+height];

    drawhi = ((islogical(markedp) && markedp) || ...
              (markedp(1)>=x && markedp(1)<=x+width && markedp(2)>=y && markedp(2)<=y+height));

    if (drawhi)
        if (clickedp)
            col = [1 1 .5];
        else
            col = [.9 .9 .7];
        end
    else
        col = [1 1 1];
    end

    glcall(glc.draw, GL.QUADS, verts, struct('colors', col));
    glcall(glc.draw, GL.LINE_LOOP, verts, struct('colors', [0 0 0]));

%    glcall(glc.rendertext, [x+2 y+2 0], height-4, thetext);
    glcall(glc.rendertext, [x-1+width/2 y+1+height/2], height-4, thetext, [0 0]);
end
