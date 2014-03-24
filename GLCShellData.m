classdef GLCShellData < GLCApplicationData
    properties
        % A GLCEditableLine object handle
        el
    end

    methods
        % Constructor
        function self = GLCShellData()
            self.el = GLCEditableLine(true);

            % Override default GLCApplicationData width/height
            self.wh = [400 600];
        end
    end
end
