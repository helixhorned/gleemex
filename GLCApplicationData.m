classdef GLCApplicationData < handle
    properties
        % a 2-vector [width, height]
        wh

        % a 2-vector of the mouse's [x, y]
        mxy

        % a bitfield of GL.BUTTON_* values
        mbutton
    end

    properties (Access=private)
        windowID
        glutWindowID

        oldWindowID
    end

    methods
        % Constructor
        function self = GLCApplicationData()
            self.wh = [800 600];  % arbitrary sensible default
            self.mxy = [1 1];
            self.mbutton = 0;
        end

        % .updateWindowPos(W, H [, SETUP2D])
        % To be called from the reshape callback of the application.
        function updateWindowPos(self, w, h, varargin)
            self.wh = [w h];
            if (~isempty(varargin))
                setup2d = varargin{1};
                assert(islogical(setup2d) && numel(setup2d)==1, ...
                       'SETUP2D must be a scalar logical')
                if (setup2d)
                    glc_setup2d(w, h);
                end
            end
        end

        % .updateMousePos(X, Y)
        % To be called from the mouse callback of the application.
        function updateMousePos(self, mbutton, x, y)
            % NOTE: invert y
            self.mxy = [x self.wh(2)-y];
            self.mbutton = mbutton;
        end

        % WINID = .attachNewWindow(POS, NAME, ...)
        function winid = attachNewWindow(self, pos, name, varargin)
            global glc
            [self.windowID, self.glutWindowID] = glcall(...
                glc.newwindow, pos, self.wh, name, varargin{:});
            winid = self.windowID;
        end

        % [OK, OLDWINID] = .makeCurrent()
        % Tries to make the window of this GLCApplicationData object current.
        function [ok, oldwinid] = makeCurrent(self)
            global glc GL
            assert(~isempty(self.windowID), 'Must have called .attachNewWindow()')
            oldwinid = glcall(glc.get, GL.WINDOW_ID);
            ok = glcall(glc.set, GL.WINDOW_ID, [self.windowID, self.glutWindowID]);
            if (ok)
                self.oldWindowID = oldwinid;
            end
        end

        % OK = .restoreOldWindow()
        function restoreOldWindow(self)
            global glc GL
            glc_assert(~isempty(self.oldWindowID), 'Must have called .makeCurrent()')
            ok = glcall(glc.set, GL.WINDOW_ID, self.oldWindowID);
        end

        % OK = .postRedisplay()
        function ok = postRedisplay(self)
            ok = self.makeCurrent();
            if (ok)
                global glc
                glcall(glc.redisplay);
                self.restoreOldWindow();
            end
        end
    end

    methods (Static)
        % GLCApplicationData.register(WINID, OBJ)
        function register(winid, obj)
            global glc_appdata

            if (~isobject(obj))
                error('OBJ must be an classdef object')
            end

            if (~(metaclass(obj) <= ?GLCApplicationData))
                error('OBJ must be derived from GLCApplicationData')
            end

            if (isempty(glc_appdata))
                glc_appdata = {};
            end

            glc_appdata{winid} = obj;
        end

        % [OBJS, WINIDS] = GLCApplicationData.getAll(CLASSNAME, ACTIVEP)
        %
        % WINIDS only valid if ACTIVEP is true.
        function [objs, winids] = getAll(className, activep)
            global glc_appdata
            assert(isvarname(className), 'CLASSNAME must be a valid class name')
            assert(islogical(activep) && isscalar(activep), 'ACTIVEP must be a scalar logical')

            objs = {};
            winids = int32([]);

            glc = glcall();
            GL = glconstants();

            for i=1:numel(glc_appdata)
                ok = false;

                if (isa(glc_appdata{i}, className))
                    ok = (glcall(glc.set, GL.WINDOW_ID, int32(i)) == activep);
                end

                if (ok)
                    objs{end+1} = glc_appdata{i};
                    if (activep)
                        winids(end+1) = int32(i);
                    end
                end
            end
        end

        function nset = setCallbacks(prefix, varargin)
            if (~ischar(prefix) || ~isvector(prefix))
                error('PREFIX must be a nonempty string')
            end

            if (~isempty(varargin))
                suffix = varargin{1};

                if (~ischar(suffix) || ~(isvector(suffix) || isempty(suffix)))
                    error('SUFFIX must be a string')
                end
            else
                suffix = '';
            end

            nset = 0;
            glc = glcall();
            cbnames = { 'display', 'reshape', 'keyboard', 'mouse', 'motion', 'position' };

            for i=1:numel(cbnames)
                funcname = [prefix cbnames{i} suffix];
                if (~isvarname(funcname))
                    error('Constructed function names must be valid');
                end

                try
                    if (exist('OCTAVE_VERSION','builtin'))
                        evalin('caller', ['@' funcname ';']);
                    else
                        assert(exist(funcname, 'file') ~= 0);
                    end

                    glcall(glc.setcallback, glc.(['cb_' cbnames{i}]), funcname);
                    nset = nset+1;
                end
            end
        end
    end
end
