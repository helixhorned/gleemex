% [VAL, OK] = glc_getenv(VARNAME)
% Returns the value VAL of the variable named VARNAME in the base workspace if
% one exists. OK is true then.
% If no variable named VARNAME exists, OK is false and the empty array [] is
% returned for VAL.
function [val, ok] = glc_getenv(varname)
    assert(isvarname(varname), 'VARNAME must be a valid variable name')

    s = evalin('base', 'whos');

    if (~any(strcmp({ s.name }, varname)))
        val = [];
        ok = false;
        return
    end

    val = evalin('base', varname);
    ok = true;
end
