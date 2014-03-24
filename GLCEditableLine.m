classdef GLCEditableLine < handle
    properties
        dohist  % enable history?
        histi  % Current index into linehist. 0: not using it.
        linehist  % the history of formerly executed lines
        bakline  % backup of line entered when started to use history

        curi  % the index of the '|' cursor in .curline
        curline  % the currently edited line
    end

    methods
        % SELF = GLCEditableLine(DOHIST)
        %
        % Constructor.
        %  DOHIST: scalar logical, enable history?
        function self = GLCEditableLine(dohist)
            if (~(isscalar(dohist) && islogical(dohist)))
                error('DOHIST must be a scalar logical')
            end

            self.dohist = dohist;
            self.histi = 0;
            self.linehist = {};
            self.bakline = '';

            self.curi = 1;
            self.curline = '|';
        end

        %% Methods to be run from Gleemex callbacks

        % .display(XRANGE, YORIGIN [, OPTS])
        function display(self, xrange, yorigin, opts)
            global GL glc

            if (nargin < 4)
                opts = struct();
            end

            glc_textlines({self.curline}, 1, xrange, yorigin, opts);
        end

        % .clearLine()
        % Clears command line.
        function clearLine(self)
            self.setLine('');
        end

        % STR = .getLine()
        % Returns the entered line (without pipe symbol) as a string STR.
        function str = getLine(self)
            str = self.curline([1:self.curi-1 self.curi+1:end]);
        end

        function enter = handleKey(self, key, mods)
            global GL glc

            enter = false;

            if (key >= 32 && key < 127)
                % printable char
                self.curins(key);
                return
            elseif (key == GL.KEY_ENTER)
                enter = true;
                self.stopHistory();
                self.saveLine();
                return
            end

            switch (key)
              case GL.KEY_BACKSPACE,
                self.curdelbefore();
              case GL.KEY_DELETE,
                self.curdelafter();
              case GL.KEY_LEFT,
                self.curleft();
              case GL.KEY_RIGHT,
                self.curright();
              case { GL.KEY_HOME, 1 },  % HOME, Ctrl-A
                self.curbeg();
              case { GL.KEY_END, 5 },  % END, Ctrl-E
                self.curend();
              case { GL.KEY_UP, GL.KEY_DOWN },
                self.handleHistory(1 - 2*(key == GL.KEY_UP));
            end

%            fprintf('%d\n', key);
        end
    end

    methods (Access=protected)
        function setLine(self, str)
            assert(ischar(str) && (isempty(str) || isvector(str)))

            self.curline = [str '|'];
            self.curi = numel(str)+1;
        end

        function stopHistory(self)
            self.bakline = '';
            self.histi = 0;
        end

        % .handleHistory(DIR)
        % DIR: direction, -1 is backward in history, 1 is forward
        function handleHistory(self, dir)
            numhist = numel(self.linehist);

            if (~self.dohist || numhist==0)
                return
            end

            assert(isequal(dir, 1) || isequal(dir, -1));

            hi = self.histi;

            if (hi == 0 && dir == 1 || hi == 1 && dir == -1)
                return
            end

            if (hi == 0)
                % Starting to use history
                self.bakline = self.getLine();
                assert(dir == -1);
                hi = numhist + 1;
            elseif (hi == numhist && dir == 1)
                % Stopping using history
                self.setLine(self.bakline);
                self.stopHistory();
                return
            end

            hi = hi + dir;
            assert(hi >= 1 && hi <= numhist);
            self.histi = hi;

            self.setLine(self.linehist{hi});
        end

        function saveLine(self)
            if (~self.dohist)
                return
            end

            curline = self.getLine();
            if (isempty(self.linehist) || ~strcmp(self.linehist{end}, curline))
                self.linehist{end+1} = curline;
            end
        end

        %% Cursor movement etc.

        % delete char before cursor
        function curdelbefore(self)
            if (self.curi > 1)
                self.curline(self.curi-1) = [];
                self.curi = self.curi-1;
            end
        end

        % delete char after cursor
        function curdelafter(self)
            if (self.curi < numel(self.curline))
                self.curline(self.curi+1) = [];
            end
        end

        function curins(self, asc)
            curline = self.curline;
            self.curline = [curline(1:self.curi-1) asc '|' curline(self.curi+1:end)];
            self.curi = self.curi+1;
        end

        function curleft(self)
            if (self.curi > 1)
                self.curline([self.curi-1 self.curi]) = self.curline([self.curi self.curi-1]);
                self.curi = self.curi-1;
            end
        end

        function curright(self)
            if (self.curi < numel(self.curline))
                self.curline([self.curi+1 self.curi]) = self.curline([self.curi self.curi+1]);
                self.curi = self.curi+1;
            end
        end

        function curbeg(self)
            while (self.curi > 1)
                self.curleft();
            end
        end

        function curend(self)
            while (self.curi < numel(self.curline))
                self.curright();
            end
        end
    end
end
