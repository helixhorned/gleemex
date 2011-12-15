function [yes, fracs]=glc_pointinrect(mxy, xyxy)
    yes = (mxy(1)>=xyxy(1) && mxy(1) <= xyxy(3) && ...
           mxy(2)>=xyxy(2) && mxy(2) <= xyxy(4));

    fracs = [(mxy(1)-xyxy(1))./(xyxy(3)-xyxy(1)), (mxy(2)-xyxy(2))./(xyxy(4)-xyxy(2))];
end
