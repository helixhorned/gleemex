classdef GLCScatterPlot < handle
    properties (Access=protected)
        % (x1,y1, w,h) wrt lower left corner
        llwh

        % A vector (xgap, ygap) of gaps between the individual scatter plots
        xygap

        % A cell array vector of length <numvars> containing the name of each variable,
        % or the empty array (none requested).
        varnames

        % Drawing limits for each variable
        mins
        maxs

        % Ticks for each variable. A cell array of length NUMVARS
        ticks

        % A (numsamples, numvars) matrix of values, or the empty array (no data yet
        % passed).
        data

        % The size of the drawn GL.POINT primitives
        pointsz

        % The text height of the tick labels
        ticktextheight

        % The text height of the variable labels
        varlabelheight

        % Display the tiles from bottom to top?
        upwards
    end

    methods
        % Constructor.
        % GSC = GLCScatterPlot()
        function self = GLCScatterPlot()
            self.setTileDirection(false);
            self.setTickTextHeight(8);
            self.setVarLabelHeight(10);
        end

        %% Getters/setters

        % Display variables.

        % SELF = .getPosExt(ULWH)
        function self = setPosExt(self, ulwh)
            glc_assert(isnumeric(ulwh) && isvector(ulwh) && numel(ulwh)==4, ...
                       'ULWH must be a numeric 4-vector')
            self.llwh = ulwh - [0 ulwh(4)-1, 0 0];
        end

        % SELF =.setGaps(XYGAP)
        function self = setGaps(self, xygap)
            glc_assert(isnumeric(xygap) && numel(xygap)==2, 'XYGAP must be a numeric pair')
            self.xygap = xygap;
        end

        % SELF = .setPointSize(POINTSZ)
        function self = setPointSize(self, pointsz)
            glc_assert(isnumeric(pointsz) && numel(pointsz)==1, 'POINTSZ must be a numeric scalar')
            self.pointsz = pointsz;
        end

        % SELF = .setTickTextHeight(HEIGHT)
        function self = setTickTextHeight(self, height)
            GLCScatterPlot.checkTextHeight(height);
            self.ticktextheight = height;
        end

        % SELF = .setVarLabelHeight(HEIGHT)
        function self = setVarLabelHeight(self, height)
            GLCScatterPlot.checkTextHeight(height);
            self.varlabelheight = height;
        end

        % SELF = .setTileDirection(UPWARDS)
        function self = setTileDirection(self, upwards)
            glc_assert(islogical(upwards) && numel(upwards)==1, ...
                       'UPWARDSP must be a logical scalar')
            self.upwards = upwards;
        end

        % State variables.

        % NUMVARS = .getNumVars()
        function numvars = getNumVars(self)
            numvars = size(self.data, 2);
        end

        function setData(self, data)
            glc_assert(isnumeric(data) && ndims(data)==2, 'DATA must be a numeric matrix')
            glc_assert(~isempty(data), 'DATA must not be empty')
            glc_assert(size(data, 2) <= 100, 'DATA must have 100 columns or less')

            self.data = data;
            self.checkInvalidate();

            self.setPointSize(self.calcPointSize());
        end

        % .setVarNames(NAMES)
        function setVarNames(self, names)
            glc_assert(self.getNumVars() > 0, 'Must have called setupData()')
            glc_assert(iscellstr(names) && isvector(names), 'NAMES must be a string cell vector')
            glc_assert(numel(names) == self.getNumVars(), 'NAMES must have NUMVARS elements')

            self.varnames = names;
        end

        % .setLimits(MINS, MAXS)
        function setLimits(self, mins, maxs)
            self.mins = self.checkLimit(mins);
            self.maxs = self.checkLimit(maxs);
            self.calcTicks();
        end

        % .calcLimitsAuto()
        function calcLimitsAuto(self)
            self.mins = min(self.data, [], 1);
            self.maxs = max(self.data, [], 1);
            self.calcTicks();
        end

        % .calcTicks(MAXTICKS)
        function calcTicks(self, maxticks)
            if (nargin < 2)
                maxticks = 7;
            end
            numvars = self.getNumVars();
            self.ticks = cell(1, numvars)

            for i=1:numvars
                self.ticks{i} = glc_genticks([self.mins(i) self.maxs(i)], maxticks);
            end
        end

        % XYWH = .calcPosExt(I, J)
        % Get position and extent for glc_drawscatter()'s XYWH.
        function xywh = calcPosExt(self, i, j)
            if (self.upwards)
                j = self.getNumVars() - j + 1;
            end

            llwh = self.llwh;
            xygap = self.xygap;

            xywh = llwh + [(i-1)*(llwh(3)+xygap(1)), ...
                           -(j-1)*(llwh(4)+xygap(2)), 0, 0];
        end

        %% Drawing

        % .draw()
        function draw(self)
            global glc

            self.checkFullState();

            if (isempty(self.mins))
                self.calcLimitsAuto();
            end

            numvars = self.getNumVars();
            pointsz = self.pointsz;

            for i=1:numvars  % x-axis variable
                for j=1:numvars  % y-axis variable
                    xywh = self.calcPosExt(i,j);
                    lims = [self.mins(i) self.maxs(i) self.mins(j) self.maxs(j)];

                    if (i == j)
                        data = zeros(2, 0);
                    else
                        data = self.data(:, [i j]).';
                    end

                    glc_drawscatter(xywh, lims, data, pointsz);

                    if (i == j && ~isempty(self.varnames) && self.varlabelheight > 0)
                        % Draw variable label
                        glcall(glc.text, xywh(1:2) + 0.5*xywh(3:4), ...
                               self.varlabelheight, self.varnames{i}, [0 0]);
                    end
                end
            end

            %% Draw tick marks
            side = 1;
            textheight = self.ticktextheight;

            for i=1:numvars
                lims = [self.mins(i) self.maxs(i)];

                if (~self.upwards)
                    top = 1;
                    bottom = numvars;
                else
                    top = numvars;
                    bottom = 1;
                end

                % Top/bottom
                if (side > 0)
                    pe = self.calcPosExt(i, top);  % top
                    pe = pe + [0 pe(4), 0 -pe(4)];
                else
                    pe = self.calcPosExt(i, bottom);  % bottom
                    pe(4) = 0;
                end
                glc_drawticks(glc_toxyxy(pe), lims, self.ticks{i}, side*4, -textheight);

                % Left/right
                if (side > 0)
                    pe = self.calcPosExt(numvars, i);  % right
                    pe = pe + [pe(3) 0, -pe(3) 0];
                else
                    pe = self.calcPosExt(1, i);  % left
                    pe(3) = 0;
                end
                glc_drawticks(glc_toxyxy(pe), lims, self.ticks{i}, -side*4, -textheight);

                side = -side;
            end
        end
    end

    methods (Access=protected)
        function pointsz = calcPointSize(self)
            numsamples = size(self.data, 1);

            if (numsamples <= 10)
                pointsz = 4;
            elseif (numsamples <= 100)
                pointsz = 3;
            elseif (numsamples <= 1000)
                pointsz = 2;
            else
                pointsz = 1;
            end
        end

        function lim = checkLimit(self, lim, name)
            if (~isnumeric(lim) || ~isvector(lim))
                error([name ' must be a numeric vector']);
            end

            numvars = self.getNumVars();

            if (numel(lim) == 1)
                lim = repmat(lim, 1, numvars);
            elseif (numel(lim) ~= numvars)
                error([name ' must be a scalar ot have length NUMVARS'])
            end
        end

        function checkFullState(self)
            glc_assert(~isempty(self.llwh), 'Must have set position and extent with setPosExt()')
            glc_assert(~isempty(self.xygap), 'Must have set x/y gaps with setGaps()')
            glc_assert(~isempty(self.data), 'Must have registered data with setData()')
        end

        function checkInvalidate(self)
            numvars = self.getNumVars();

            if (numel(self.varnames) ~= numvars)
                self.varnames = [];
            end

            if (numel(self.mins) ~= numvars)
                self.mins = [];
                self.maxs = [];
            end
        end
    end

    methods (Static)
        function checkTextHeight(height)
            glc_assert(isnumeric(height) && numel(height)==1, 'HEIGHT must be a numeric scalar')
            glc_assert(height >= 0, 'HEIGHT must be non-negative')
        end
    end
end
