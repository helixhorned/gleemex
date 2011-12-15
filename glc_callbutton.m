% GLC_CALLBUTTON(XYWH, MXY, CALLBACKFN)
function glc_callbutton(xywh, mxy, callbackfn)
    if (glc_pointinrect(mxy, xywh))
        if (ischar(callbackfn))
            eval(callbackfn);
        else
            callbackfn();
        end
    end
end
