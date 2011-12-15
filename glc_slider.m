% SLISTRUCT = GLC_SLIDER(VALS, INITIALIDX, VALCHANGE_CB)
%
% Construct a slider struct.
%  - VALS: vector of values (must be non-empty and numeric)
%  - INITIALIDX: initial index into VALS
%  - VALCHANGE_CB: handle to a callback function @(vals, newidx), called
%     with the struct member VALS and the newly assigned index as NEWIDX
function slistruct = glc_slider(vals, initialidx, valchange_cb)

    assert(isnumeric(vals) && isvector(vals) && ~isempty(vals));
    assert(initialidx >= 1 && initialidx <= numel(vals));
    assert(isa(valchange_cb, 'function_handle'));  % args: unchecked

    slistruct.vals = vals;
    slistruct.idx = initialidx;
    slistruct.cb = valchange_cb;
end
