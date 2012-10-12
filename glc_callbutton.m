% GLC_CALLBUTTON(XYWH, MXY, CALLBACKFN [, ARG1 [, ARG2 ...]])
%
% The ARGs are passed to CALLBACKFN if it is a function handle,
% otherwise (if CALLBACKFN is a string) 'arg' is available in
% the evaluating environment.
function glc_callbutton(xywh, mxy, callbackfn, varargin)
    if (glc_pointinxywh(mxy, xywh))
        if (ischar(callbackfn))
            if (nargin >= 4)
                arg = varargin{1};
            end
            eval(callbackfn);
        else
            callbackfn(varargin{:});
        end
    end
end
