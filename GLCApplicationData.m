classdef GLCApplicationData < handle
    properties
        % a 2-vector [width, height]
        wh

        % a 2-vector of the mouse's [x, y]
        mxy
    end

    methods
        % Constructor
        function self = GLCApplicationData()
            self.wh = [800 600];  % arbitrary sensible default
            self.mxy = [1 1];
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
        function updateMousePos(self, x, y)
            % NOTE: invert y
            self.mxy = [x self.wh(2)-y];
        end
    end

    methods (Static)
        % GLCApplicationData.register(WINID, OBJ)
        function register(winid, obj)
            global glc_appdata

            if (~isobject(obj))
                error('OBJ must be an OO-sense object')
            end

            if (~GLCApplicationData.isclass(obj, 'GLCApplicationData'))
                error('OBJ must be derived from GLCApplicationData')
            end

            if (isempty(glc_appdata))
                glc_appdata = {};
            end

            glc_appdata{winid} = obj;
        end

        % OBJ = GLCApplicationData.getSuspended(CLASSNAME)
        %
        %
        function obj = getSuspended(className)
            global glc_appdata

            % NOTE: also handles the case of glc_appdata empty (not initialized)
            for i=1:numel(glc_appdata)
                if (isa(glc_appdata{i}, className))
                    obj = glc_appdata{i};
                    return
                end
            end

            %% Didn't find one.
            name = className;
            if (numel(name) > 4 && isequal(name(end-3:end), 'Data'))
                name(end-3:end) = [];
            end
            error('Didn''t find a previously suspended %s session', name)
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
                if (exist([funcname '.m'], 'file'))
                    glcall(glc.setcallback, glc.(['cb_' cbnames{i}]), funcname);
                    nset = nset+1;
                end
            end
        end
    end

    methods (Static, Access=private)
        % IS = GLCApplicationData.isclass(OBJ, CLASSNAME)
        function is = isclass(obj, classname)
            if (isa(obj, classname))
                is = true;
                return
            end

            mo = metaclass(obj);
            superClasses = mo.SuperClasses;

            for i=1:numel(superClasses)
                if (isequal(superClasses{i}.Name, classname))
                    is = true;
                    return
                end
            end

            is = false;
        end
    end
end
