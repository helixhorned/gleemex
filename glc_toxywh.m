% XYWH = glc_toxywh(XYXY)
% Converts an array XYXY with (x1,y1, x2,y2) entries into the array XYWH with
% (x1,y1, w,h) entries. XYXY may have any size for which the number of elements
% is be divisible by four.
function xywh = glc_toxywh(xyxy)
    glc_assert(isnumeric(xyxy), 'XYXY must be a numeric array')

    sz = size(xyxy);

    if (sz(1)==1 && sz(2)==4)
        xywh = [xyxy(1:2) xyxy(3:4)-xyxy(1:2)];
        return
    end

    xyxy = reshape(xyxy, 4, []);
    xywh = [xyxy(1:2, :); xyxy(3:4, :)-xyxy(1:2, :)];

    xywh = reshape(xywh, sz);
end
