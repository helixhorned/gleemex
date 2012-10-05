% GLC_TEXTLINES(LISTSTRING, NUMLINES, XRANGE, YORIGIN [, OPTS])
%
% OPTS is a structure with the following optional fields:
%  * colors  (size: numel(liststring) x 3)
%  * lineheight
%  * offset
%  * xmargin
function glc_textlines(liststring, numlines, xrange, yorigin, opts)
    global glc GL

    numvals = numel(liststring);

    narginchk(4, 5);

    assert(iscellstr(liststring));

    % default options
    colors = [];
    lineheight = 16;
    offset = 0;
    xmargin = 12;

    if (nargin >= 4)
        if (isfield(opts, 'colors'))
            colors = opts.colors;
            assert(isnumeric(colors) && isequal(size(colors), [numvals 3]));
        end
        if (isfield(opts, 'lineheight'))
            lineheight = opts.lineheight;
            assert(isnumeric(lineheight) && numel(lineheight)==1);
        end
        if (isfield(opts, 'offset'))
            offset = opts.offset;
            assert(isnumeric(lineheight) && numel(lineheight)==1);
        end
        if (isfield(opts, 'xmargin'))
            xmargin = opts.xmargin;
            assert(isnumeric(lineheight) && numel(lineheight)==1);
        end
    end

    for runi=1:2
        for i=1:numlines
            idx = i + offset;
            if (idx > numvals)
                break;
            end

            y1 = yorigin - i*lineheight;

            bb = [xrange; ...
                  y1, y1+lineheight];

            if (runi==1)
                glcall(glc.draw, GL.QUADS, glc_expandrect(bb), struct('colors', colors(idx, :)));
            else
                glcall(glc.text, bb([1 2])+[xmargin 2], lineheight-2, liststring{idx});
            end
        end
    end
end
