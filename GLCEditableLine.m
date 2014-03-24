classdef GLCEditableLine < handle
    properties
        dohist  % enable history?
        linehist  % the history of formerly executed lines

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
            self.linehist = {};

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
            self.curi = 1;
            self.curline = '|';
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
            end

%            fprintf('%d\n', key);
        end
    end

    methods (Access=protected)

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
