classdef GLCPersistentMessage < handle
    properties
        lastmsg
        keepuntil
        once

        duration  % default duration in seconds
    end

    methods
        % SELF = GLCPersistentMessage([DURATION])
        function self = GLCPersistentMessage(varargin)
            self.lastmsg = '';
            self.keepuntil = 0;
            self.once = false;

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
        %
        % Stores a "timed message" for display up to DURATION seconds from now.
        function tmessage(self, duration, fmt, varargin)
            self.lastmsg = sprintf(fmt, varargin{:});
            self.keepuntil = now() + duration/(24*3600);
            self.once = false;
        end

        % .onceMessage(FMT, ...)
        %
        % Stores a message that is to be retrieved only once.
        function onceMessage(self, fmt, varargin)
            self.lastmsg = sprintf(fmt, varargin{:});
            self.once = true;
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
            if (self.once || now() < self.keepuntil)
                msg = self.lastmsg;
                self.once = false;
            else
                msg = '';
            end
        end
    end
end
