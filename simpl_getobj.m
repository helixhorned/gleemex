% INTERNAL. Classdef only.
function spd = simpl_getobj()
    global simpl

    winid = simpl_getwin();
    spd = simpl{winid};
    assert(isa(spd, 'GLCSimplePlotData'), 'INTERNAL ERROR: GLCSimplePlotData not initialized')
end
