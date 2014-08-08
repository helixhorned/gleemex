% HAVE = glc_have_beginpage()
% Return whether Gleemex was compiled with GL2PS support, that is, if the
% `glc.beginpage` command is available.
function have = glc_have_beginpage()
    have = isfield(glcall(), 'beginpage');
end
