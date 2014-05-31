% XYXY = glc_toxyxy(XYWH)
% Converts an array XYWH with (x1,y1, w,h) entries into the array XYXY with
% (x1,y1, x2,y2) entries. XYWH may have any size for which the number of elements
% is be divisible by four.
function xyxy = glc_toxyxy(xywh)
    glc_assert(isnumeric(xywh), 'xywh must be a numeric array')

    sz = size(xywh);

    if (sz(1)==1 && sz(2)==4)
        xyxy = [xywh(1:2) xywh(1:2)+xywh(3:4)];
        return
    end

    xywh = reshape(xywh, 4, []);
    xyxy = [xywh(1:2, :); xywh(1:2, :)+xywh(3:4, :)];

    xyxy = reshape(xyxy, sz);
end
