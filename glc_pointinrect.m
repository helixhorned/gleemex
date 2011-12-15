function [yes, fracs]=glc_pointinrect(mxy, xywh)
    yes = (mxy(1)>=xywh(1) && mxy(1) <= xywh(1)+xywh(3) && ...
           mxy(2)>=xywh(2) && mxy(2) <= xywh(2)+xywh(4));

    fracs = [(mxy(1)-xywh(1))./xywh(3), (mxy(2)-xywh(2))./xywh(4)];
end
