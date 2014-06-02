classdef GLCScatterPlot < handle
    properties (Access=protected)
        % (x1,y1, w,h) wrt lower left corner
        llwh

        % A vector (xgap, ygap) of gaps between the individual scatter plots
        xygap

        % A cell array vector of length <numvars> containing the name of each variable,
        % or the empty array (none requested).
        varnames

        % Drawing limits for each variable. Each a (1, NUMVARS, NUMDSETS) vector.
        % (That is, one obtained by reducing .data along the points dimension).
        mins
        maxs

        % Ticks for each variable and data set. A cell array of size (NUMVARS,
        % NUMDSETS).
        ticks

        % A (NUMPOINTS, NUMVARS, NUMDSETS) matrix of values, or the empty array (no
        % data yet passed).
        data

        % The size of the drawn GL.POINT primitives
        pointsz

        % The text height of the tick labels
        ticktextheight

        % The max. text height of the variable labels
        maxvarlabelheight
        % The really used one, is <= maxvarlabelheight
        varlabelheight
        % Set to true whenever varlabelheight should be updated from maxvarlabelheight
        needLabelHeightUpdate

        % Display the tiles from bottom to top?
        upwards

        % Data set indices. A pair [DSX, DSY] of indices into the third dimension of
        % .data.
        dsi

        %% Colors

        % A struct('colors', COLORS), where COLORS is a
        % a 3xNUMPOINTS matrix of class uint8, single or double.
        % (Or the empty  if none have been registered.)
        colors

        % Show colored points?
        showcolors
    end

    methods
        % Constructor.
        % GSC = GLCScatterPlot()
        function self = GLCScatterPlot()
            self.setTileDirection(false);
            self.setTickTextHeight(8);
            self.setVarLabelHeight(10);
            self.setColors([]);
            self.dsi = [1 1];  % Can't use setAxesDataSets() since .data is empty
        end

        %% Getters/setters

        % Display variables.

        % SELF = .getPosExt(ULWH)
        function self = setPosExt(self, ulwh)
            glc_assert(isnumeric(ulwh) && isvector(ulwh) && numel(ulwh)==4, ...
                       'ULWH must be a numeric 4-vector')
            self.llwh = ulwh - [0 ulwh(4)-1, 0 0];
            self.needLabelHeightUpdate = true;
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
            self.maxvarlabelheight = height;
            self.needLabelHeightUpdate = true;
        end

        % SELF = .setTileDirection(UPWARDS)
        function self = setTileDirection(self, upwards)
            glc_assert(islogical(upwards) && numel(upwards)==1, ...
                       'UPWARDS must be a logical scalar')
            self.upwards = upwards;
        end

        % SELF = .setShowColors(DOSHOW)
        function self = setShowColors(self, doshow)
            glc_assert(islogical(doshow) && numel(doshow)==1, ...
                       'DOSHOW must be a logical scalar')
            self.showcolors = doshow;
        end

        % State variables.

        function numpoints = getNumPoints(self)
            numpoints = size(self.data, 1);
        end

        % NUMVARS = .getNumVars()
        function numvars = getNumVars(self)
            numvars = size(self.data, 2);
        end

        % NUMDSETS = .getNumDataSets()
        function numdsets = getNumDataSets(self)
            numdsets = size(self.data, 3);
        end

        % SELF = .setData(DATA)
        function self = setData(self, data)
            glc_assert(isnumeric(data) && ndims(data) <= 3, 'DATA must be a numeric array with <= 3 dims')
            glc_assert(size(data, 2) <= 100, 'DATA must have 100 columns (variables) or less')

            if (isempty(data))
                self.data = [];
            else
                self.data = data;
            end
            self.checkInvalidate();
            self.needLabelHeightUpdate = true;

            self.setPointSize(self.calcPointSize());
        end

        % SELF = .setAxesDataSets(DSI)
        function self = setAxesDataSets(self, dsi)
            glc_assert(isnumeric(dsi) && numel(dsi)==2, 'DSI must be a numeric pair')
            glc_assert(all(round(dsi) == dsi), 'DSI must be integers')
            glc_assert(all(dsi >= 1 & dsi <= self.getNumDataSets()), 'DSI must be in [1 .. NUMDSETS]')

            self.dsi = dsi;
        end

        % SELF = .setVarNames(NAMES)
        function self = setVarNames(self, names)
            glc_assert(self.getNumVars() > 0, 'Must have called setupData()')
            glc_assert(iscellstr(names) && isvector(names), 'NAMES must be a string cell vector')
            glc_assert(numel(names) == self.getNumVars(), 'NAMES must have NUMVARS elements')

            self.varnames = names;
            self.needLabelHeightUpdate = true;
        end

        % SELF = .setLimits(MINS, MAXS)
        function self = setLimits(self, mins, maxs)
            self.mins = self.checkLimit(mins);
            self.maxs = self.checkLimit(maxs);
            self.calcTicks();
        end

        % SELF = .setColors([COLORS])
        function self = setColors(self, colors)
            if (isempty(colors))
                self.colors = [];
            else
                glc_assert(ndims(colors)==2 && size(colors)==[3 self.getNumPoints()], ...
                           'COLORS must have size [3 NUMPOINTS]')
                cls = class(colors);
                glc_assert(any(strcmp(cls, { 'uint8', 'single', 'double' })), ...
                           'COLORS must have class uint8, single or double')

                self.colors = struct('colors', colors);
            end
        end

        % Calculation of display state

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
            numdsets = self.getNumDataSets();
            self.ticks = cell(numvars, numdsets);

            for v=1:numvars
                for d=1:numdsets
                    self.ticks{v,d} = glc_genticks([self.mins(1,v,d) self.maxs(1,v,d)], maxticks);
                end
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

            % Data set index for x and y axes:
            dsx = self.dsi(1);
            dsy = self.dsi(2);

            if (dsx == dsy)
                labelofs = 0.5*self.llwh(3:4);
                labelalign = [0 0];
            else
                labelofs = [0.5, 1].*self.llwh(3:4) - [0, 4];
                labelalign = [0 1];
            end

            self.checkVarLabelHeight();

            for i=1:numvars  % x-axis variable
                for j=1:numvars  % y-axis variable
                    xywh = self.calcPosExt(i,j);
                    lims = [self.mins(1,i,dsx) self.maxs(1,i,dsx), ...
                            self.mins(1,j,dsy) self.maxs(1,j,dsy)];

                    nodraw = (i == j && dsx == dsy);

                    if (nodraw)
                        data = zeros(2, 0);
                    else
                        data = [self.data(:, i, dsx).'; ...
                                self.data(:, j, dsy).'];
                    end

                    if (self.showcolors && self.haveColors() && ~nodraw)
                        glc_drawscatter(xywh, lims, data, pointsz, self.colors);
                    else
                        glc_drawscatter(xywh, lims, data, pointsz);
                    end

                    if (i == j && ~isempty(self.varnames) && self.varlabelheight > 0)
                        % Draw variable label
                        glcall(glc.text, xywh(1:2) + labelofs, ...
                               self.varlabelheight, self.varnames{i}, labelalign);
                    end
                end
            end

            %% Draw tick marks
            side = 1;
            textheight = self.ticktextheight;

            for i=1:numvars
                xlims = [self.mins(1,i,dsx) self.maxs(1,i,dsx)];
                ylims = [self.mins(1,i,dsy) self.maxs(1,i,dsy)];

                if (~self.upwards)
                    top = 1;
                    bottom = numvars;
                else
                    top = numvars;
                    bottom = 1;
                end

                % X axis: top/bottom
                if (side > 0)
                    pe = self.calcPosExt(i, top);  % top
                    pe = pe + [0 pe(4), 0 -pe(4)];
                else
                    pe = self.calcPosExt(i, bottom);  % bottom
                    pe(4) = 0;
                end
                glc_drawticks(glc_toxyxy(pe), xlims, self.ticks{i,dsx}, side*4, -textheight);

                % Y axis: left/right
                if (side > 0)
                    pe = self.calcPosExt(numvars, i);  % right
                    pe = pe + [pe(3) 0, -pe(3) 0];
                else
                    pe = self.calcPosExt(1, i);  % left
                    pe(3) = 0;
                end
                glc_drawticks(glc_toxyxy(pe), ylims, self.ticks{i,dsy}, -side*4, -textheight);

                side = -side;
            end
        end

        % HAVE = .haveFullState()
        function have = haveFullState(self)
            % KEEPINSYNC checkFullState()
            have = (~isempty(self.llwh) && ~isempty(self.xygap) && ~isempty(self.data));
        end
    end

    methods (Access=protected)
        function pointsz = calcPointSize(self)
            numsamples = self.getNumPoints();

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
            if (~isnumeric(lim) || size(lim, 1) ~= 1)
                error([name ' must be numeric array with one row']);
            end

            numvars = self.getNumVars();
            numdsets = self.getNumDataSets();
            targetsz = [1 numvars numdsets];

            if (~isequal(size(lim), targetsz))
                switch (size(lim))
                  case [1 1],
                    lim = repmat(lim, targetsz);
                  case [1 numvars],
                    lim = repmat(lim, [1 1 numdsets]);
                  otherwise,
                    error([name ' must be a scalar or have size [1, NUMVARS [, NUMDSETS]]'])
                end
            end
        end

        % .checkVarLabelHeight()
        % Calculate .varlabelheight from maxvarlabelheight when necessary, considering
        % the width of a single scatter plot. Must be called from a context
        % where drawing is valid.
        function checkVarLabelHeight(self)
            maxheight = self.maxvarlabelheight;
            margins = 4;  % sum of the left and right margins
            w = self.llwh(3) - margins;

            if (isempty(maxheight) || isempty(self.varnames) || w <= 0)
                self.varlabelheight = 0;
            elseif (self.needLabelHeightUpdate)
                global glc
                self.needLabelHeightUpdate = false;

                maxwidth = 0;
                for i=1:self.getNumVars()
                    textwidth = glcall(glc.text, [0 0], maxheight, self.varnames{i});
                    maxwidth = max(maxwidth, textwidth);
                end

                if (maxwidth > w)
                    f = w/maxwidth;
                    self.varlabelheight = floor(f*maxheight);
                else
                    self.varlabelheight = maxheight;
                end
            end
        end

        function checkFullState(self)
            glc_assert(~isempty(self.llwh), 'Must have set position and extent with setPosExt()')
            glc_assert(~isempty(self.xygap), 'Must have set x/y gaps with setGaps()')
            glc_assert(~isempty(self.data), 'Must have registered data with setData()')
        end

        function have = haveColors(self)
            have = ~isempty(self.colors);
        end

        function checkInvalidate(self)
            numvars = self.getNumVars();

            if (numel(self.varnames) ~= numvars)
                self.varnames = [];
            end

            if (size(self.mins, 2) ~= numvars)
                self.mins = [];
                self.maxs = [];
            end

            if (self.haveColors())
                if (size(self.colors.colors, 2) ~= self.getNumPoints())
                    self.setColors([]);
                end
            end

            if (any(self.dsi > self.getNumDataSets()))
                self.setAxesDataSets([1 1]);
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
