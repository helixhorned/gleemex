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

        % Sequence number for the vector image saved with gl2ps
        imagenum
        % 0: not taken a screenshot, 1: just took one, 2: status will be available
        imagestate

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
