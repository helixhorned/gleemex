classdef GLCSimplePlotData < handle
    properties
        omx
        mxy
        wh
        axxywh
        zoom
        ang
        fog
        lineidxs
        keycb
        displaycb
        moretext
        ud

        havelist

        numsamples
        data
        numdims
        idxs
        colors
        mean
        min
        max
    end

    methods
        % Constructor.
        function self = GLCSimplePlotData()
            % Do absolutely nothing.
        end
    end
end
