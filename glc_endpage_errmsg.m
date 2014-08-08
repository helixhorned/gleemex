% STR = glc_endpage_errmsg(ENDSTATUS)
% Returns a string for the status code of gl2psEndPage(), available in Gleemex
% by calling glcall(glc.beginpage) -- that is, calling the glc.beginpage
% without any other args. The string is the lowercased version of the
% corresponding GL.PS_* error code define, e.g. 'no_feedback' for
% GL.PS_NO_FEEDBACK.
function str = glc_endpage_errmsg(status)
    % Compare with GL.PS_* error codes.
    STATUS = { 'success', 'info', 'warning', 'error', ...
               'no_feedback', 'overflow', 'uninitialized' };
    str = STATUS{status+1};
end
