% GLC_CALLBUTTON(XYWH, MXY, CALLBACKFN)
function glc_callbutton(xywh, mxy, callbackfn)
    if (glc_pointinrect(mxy, xywh))
        callbackfn();
    end
end
