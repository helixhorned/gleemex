classdef GLCPersistentMessage < handle
    properties
        lastmsg
        keepuntil

        duration  % default duration in seconds
    end

    methods
        % SELF = GLCPersistentMessage([DURATION])
        function self = GLCPersistentMessage(varargin)
            self.lastmsg = '';
            self.keepuntil = 0;

            if (nargin > 0)
                duration = varargin{1};
                assert(isnumeric(duration) && numel(duration)==1, ...
                       'DURATION must be a numeric scalar')
                self.duration = duration;
            else
                self.duration = 3;
            end
        end

        % .tmessage(DURATION, FMT, ...)
        function tmessage(self, duration, fmt, varargin)
            self.lastmsg = sprintf(fmt, varargin{:});
            self.keepuntil = now() + duration/(24*3600);
        end

        % .message(FMT, ...)
        function message(self, fmt, varargin)
            self.tmessage(self.duration, fmt, varargin{:});
        end

        % .pmessage(FMT, ...)
        function pmessage(self, fmt, varargin)
            self.message(fmt, varargin{:});
            fprintf('%s\n', self.lastmsg);
        end

        function msg = getMessage(self)
            if (now() < self.keepuntil)
                msg = self.lastmsg;
            else
                msg = '';
            end
        end
    end
end
