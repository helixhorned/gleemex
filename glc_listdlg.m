% [SEL, OK] = GLC_LISTDLG('ListString',liststring, ...)
% [cvals, OK] = GLC_LISTDLG('ControlDesc',cdesc, 'ControlVals',cvals, ...)
%
% Replacement for MATLAB's LISTDLG(), see its doc for most options.
%
% Non-MATLAB options:
%  'FinishCb',<func>: must be a handle to a function accepting two input
%    arguments: 'ok' and 'sel'. In control-GUI mode, 'sel' is the 'ControlVals'
%    variable, otherwise it is a row vector of selected line indices.
%    Called on any exit condition except closing by clicking [x].
%
%  'Subwindow',<logical>: if true, create a subwindow instead of a top-level
%    window.  See glc_test_subwindows.m for an example.
%
%  'SelectionMode','edit': DEPRECATED
%
%  'ControlDesc',<cdesc-struct>;
%  'ControlVals',<cvals-struct>:
%    Control-GUI mode.  (TODO: better doc.)
%    These two must occur together and imply 'SelectionMode','edit'.
%
%  'ControlCb', @(cvals)(...): specifies a callback function to be run with
%    the updated 'ControlVals' variable whenever it changes.
%
% If glc_listdlg() was called when another window was current (its 'parent
% window'), then it will be restored to the current window when the list
% dialog finishes normally (not by [x]).
function [sel,ok]=glc_listdlg(varargin)

    global GL glc glc_ld

    GL = glconstants();
    glc = glcall();

    if (mod(nargin,2)~=0)
        error('Must pass an even number of input arguments');
    end

    haveliststring = false;

    % CONTROLGUI
    havecontrol = 0;
    controldesc = [];
    controlvals = [];
    % A callback function that is run whenever 'controlvals' changes.
    controlcb = @(cvals)0;

    liststring = {};
    selmode_multiple = true;
    selmode_edit = false;  % whether to use this list dialog as a GUI of sorts
    listsize = [160 300];
    initialval = 1;
    name = '';
    promptstr = '';
    okstr = 'OK';
    cancelstr = 'Cancel';

    % either function handle @f(ok, sel), or string eval'd with 'sel' and 'ok' in scope
    finish_cb = '';
    % create in subwindow?
    subwindowp = false;
    listpos = [200 200];

    for i=1:2:nargin-1
        if (~ischar(varargin{i}))
            error('Keys must be string values');
        end

        val = varargin{i+1};

        switch lower(varargin{i}),
          case 'uh','fus','ffs',
            warning('uh, fus, ffs not implemented');
          case 'liststring',
            assert(iscellstr(val), 'ListString value must be a cell array of strings');
            haveliststring = true;
            liststring = val(:);
          case 'selectionmode',
            if (strcmpi(val, 'single'))  % case-insens!
                selmode_multiple = false;
            elseif (strcmpi(val, 'multiple'))
                selmode_multiple = true;
            elseif (strcmpi(val, 'edit'))  % non-MATLAB
                selmode_edit = true;
                selmode_multiple = false;
            else
                error('SelectionMode value must be one of ''single'' or ''multiple''');
            end
          case 'listsize',
            assert(isvector(val) && isnumeric(val) && numel(val)==2, 'ListSize value must be a numeric 2-vector');
            assert(all(val>=1), 'All elements in ListSize value must be greater or equal 1');

            listsize = double(val);
          case 'initialvalue',
            assert(isvector(val) && isnumeric(val), 'InitialValue value must be a numeric vector');
            assert(all(round(val)==val), 'InitialValue vector must contain integer numbers');

            initialval = val;
          case 'name',
            assert(isvector(val) && ischar(val), 'Name value must be a char vector');

            name = val(:)';
          case 'promptstring',
            assert(iscellstr(val) || (isvector(val) && ischar(val)), ...
                   'PromptString value must be string or string cell');

            if (iscellstr(val))
                warning('PromptString as cell string: not implemented');
                promptstring = '';
            else
                promptstring = val;
            end
          case 'okstring',
            assert(ischar(val) && (isvector(val) || isempty(val)), 'OKString value must be a string');
            okstr = val;
          case 'cancelstring',
            assert(ischar(val) && (isvector(val) || isempty(val)), 'CancelString value must be a string');
            cancelstr = val;

            %% non-standard
          case 'finishcb',
            assert((isvector(val) && ischar(val)) || isa(val, 'function_handle'), ...
                   'FinishCb must be either a string or a function handle @f(ok, sel)');
            finish_cb = val;
          case 'subwindow',
            assert(isscalar(val) && islogical(val));
            subwindowp = val;
          case 'listpos',
            assert(isvector(val) && isnumeric(val) && numel(val)==2, 'ListPos value must be a numeric 2-vector');
            assert(all(val>=1), 'All elements in ListPos value must be greater or equal 1');
            listpos = double(val);

            %% CONTROLGUI
          case 'controlvals',  % A struct of values to change.
            havecontrol = bitor(havecontrol, 1);
            controlvals = val;
          case 'controldesc',  % The description of what constraints the above underlies.
            havecontrol = bitor(havecontrol, 2);
            controldesc = val;
          case 'controlcb',
            assert((isvector(val) && ischar(val)) || isa(val, 'function_handle'), ...
                   'ControlCb must be either a string or a function handle @f(cvals)');
            controlcb = val;

          otherwise,
            warning(sprintf('unrecognized option ''%s''', varargin{i}));
        end
    end

    %% some more input checking

    need_glc_ld_init = false;
    TYPE = struct('NUMBER',1, 'STRING',2, 'TOGBTN',3, 'MULTISEL',4, 'KEYBIND',5);
    if (~isstruct(glc_ld))
        % we need the symbolic names in glc_listdlg_{construct_str,validate_control}
        glc_ld = struct('TYPE',TYPE);
        need_glc_ld_init = true;
    end

    % CONTROLGUI
    if (havecontrol ~= 0)
        if (havecontrol ~= 3)
            error('If passing ''controlvals'', must also pass ''controldesc'', and vice versa');
        end

        controldesc = glc_listdlg_validate_control(controldesc, controlvals);
        liststring = glc_listdlg_construct_str(controldesc, controlvals);

        haveliststring = true;
        selmode_edit = true;

        % There's no way to undo the changes in the control-GUI since they're applied
        % immediately.
        cancelstr = '';
    end

    numvals = numel(liststring);

    if (~haveliststring)
        error('''ListString'' key/value is mandatory');
    end

    if (~selmode_multiple && numel(initialval)~=1)
        error('When in single selection mode, must have exactly one initial value');
    end

    if (any(initialval < 1 | initialval > numvals))
        error('Some elements in INITIALVAL out of bounds');
    end

    s = struct();
    s.liststring = liststring;
    s.selmode_multiple = selmode_multiple;
    s.oldstring = '';
    s.editstring = {'','',''};  % { const, head, tail }; temp, only while editing
    s.selmode_edit = selmode_edit;
    s.listsize = listsize;
    s.initialval = initialval;
    s.name = name;
    s.promptstr = promptstr;
    s.okstr = okstr;
    s.cancelstr = cancelstr;

    s.controldesc = controldesc;
    s.controlvals = controlvals;
    s.controlcb = controlcb;

    s.finish_cb = finish_cb;

    s.selected = false(size(liststring));
    s.selected(initialval) = true;

    s.editing = 0;  % > 0: list index of entry to edit
    s.done = 0;  % 1:OK, 2:Cancel
    s.ofs = 0;
    s.downreq = 0;  % set to 1 if pressed 'down' key, 10 if 'pgdn'

    s.oksel = {0, []};
    s.clicked = [];
    s.lastclickidx = 0;
    s.mxy = [1 1];
    s.wh = listsize+20+[40 80];  % +20: deviation from MATLAB default

    s.bbox = [20 s.wh(1)-20; ...
                        20+24+20 s.wh(2)-20];
    s.parentwinid = glcall(glc.get, GL.WINDOW_ID);  % 0 if none or uninited

    s.TYPE = TYPE;  % will be a struct filled with TYPE.* constants

    if (isempty(s.name))
        s.name = ' ';
    end

    if (need_glc_ld_init)
        % init woes: in MATLAB or Octave, we can't do either
        %  - S(n) = S2, where S and S2 are structs with different fields
        %  - S(n) = S2, where S is [] (globals on init)
        % WARNING: this means that it's a bad idea to assign to nonexistent
        %  fields of glc_ld in callbacks, in other words they should be
        %  'declared' above first
        glc_ld = glc_listdlg_dummyize_struct(s);
        glc_ld.TYPE = TYPE;
    end

    winid = glcall(glc.newwindow, listpos, s.wh, s.name, ...
                   struct('subwindow',subwindowp));

    % should be before setting the callbacks
    glc_ld(winid) = s;

    glcall(glc.setcallback, glc.cb_reshape, 'glc_listdlg_reshape');
    glcall(glc.setcallback, glc.cb_motion, 'glc_listdlg_motion');
    glcall(glc.setcallback, glc.cb_keyboard, 'glc_listdlg_keyboard');
    glcall(glc.setcallback, glc.cb_mouse, 'glc_listdlg_mouse');
    glcall(glc.setcallback, glc.cb_display, 'glc_listdlg_display');

    if (subwindowp)
        % note that we may have not entered the main loop yet
        glcall(glc.set, GL.WINDOW_ID, s.parentwinid);
    else
        glcall(glc.entermainloop);
    end

    [ok, sel] = glc_listdlg_get_ok_sel(winid);
end

function controldesc=glc_listdlg_validate_control(controldesc, controlvals)
    global glc_ld
    TYPE = glc_ld(1).TYPE;

    assert(isvector(controldesc) && isstruct(controldesc));
    assert(numel(controlvals)==1 && isstruct(controlvals));

    numcontrols = numel(controldesc);

    assert(isfield(controldesc, 'name'));
    assert(isfield(controldesc, 'key'));
    assert(isfield(controldesc, 'extra'));

    for i=1:numcontrols
        assert(ischar(controldesc(i).name) && isvector(controldesc(i).name));

        key = controldesc(i).key;
        assert(ischar(key) && isvector(key));
        assert(isfield(controlvals, key));

        ex = controldesc(i).extra;

        v = controlvals.(key);
        thetype = 0;
        if (numel(v)==1 && isfloat(v))
            % number, constrained by a function
            controldesc(i).type = TYPE.NUMBER;

            assert(numel(ex)==1 && isstruct(ex));
            % 'updownfunc' expected to be
            %  newval = func(oldval, direction)
            % (direction is -1 or 1)
            assert(isfield(ex, 'updownfunc'));
            assert(isa(ex.updownfunc, 'function_handle'));

            assert(isfield(ex, 'format'));
            assert(ischar(ex.format) && isvector(ex.format));
        elseif (ischar(v) && (isempty(v) || isvector(v)))
            % editable string
            controldesc(i).type = TYPE.STRING;

            % if non-empty, the validation function is expected to be
            %  is_ok = func(str)
            % or a logical telling whether editing is permitted.
            assert(isempty(ex) || isa(ex, 'function_handle') || ...
                   (numel(ex)==1 && islogical(ex)));
        elseif (numel(v)==1 && islogical(v))
            % toggle button
            controldesc(i).type = TYPE.TOGBTN;
            assert(isempty(ex));
        elseif (numel(v)==1 && strcmp(class(v), 'int32'))
            % multi-state selection
            controldesc(i).type = TYPE.MULTISEL;
            assert(iscellstr(ex) && isvector(ex) && v>=1 && v<=numel(ex));
        elseif (numel(v)==1 && strcmp(class(v), 'uint32'))
            % key bind
            controldesc(i).type = TYPE.KEYBIND;
            assert(isempty(ex));
        else
            error('Invalid value type');
        end
    end
end

function key=glc_listdlg_get_keyname(v)
    key = '<unknown>';
    if (v>=33 && v<=126)
        key = ['<' v '>'];
    elseif (v>=65536+1 && v<=65536+12)
        key = sprintf('<F%d>', v-65536);
    elseif (v>=65536+100 && v<=65536+108)
        KEYS = { 'LEFT', 'UP', 'RIGHT', 'DOWN', 'PGUP', ...
                 'PGDN', 'HOME', 'END', 'INSERT' };
        key = ['<' KEYS{v-(65536+100)+1} '>'];
    else
        switch (v)
          case 8,
            key = '<BACKSPACE>';
          case 9,
            key = '<TAB>';
          case 13,
            key = '<ENTER>';
          case 32,
            key = '<SPACE>';
          case 127,
            key = '<DEL>';
        end
    end
%    key = sprintf('%s %d', key, v);
end

function liststring = glc_listdlg_construct_str(controldesc, controlvals)
    global glc_ld
    TYPE = glc_ld(1).TYPE;

    numcontrols = numel(controldesc);

    liststring = cell(1, numcontrols);
    for i=1:numcontrols
        ex = controldesc(i).extra;
        str = [controldesc(i).name ': '];

        key = controldesc(i).key;
        v = controlvals.(key);

        switch (controldesc(i).type)
          case TYPE.NUMBER,
            % number, constrained by a function
            str = [str sprintf(ex.format, v)];
          case TYPE.STRING,
            if (~isempty(v))
                % editable string
                str = [str v];
            else
                % static string
                str(end-1:end) = [];  % cut off the ': '
            end
          case TYPE.TOGBTN,
            % toggle button
            ps = 'false';
            if (v) ps = 'true'; end
            str = [str ps];
          case TYPE.MULTISEL,
            % multi-state selection
            str = [str ex{v}];
          case TYPE.KEYBIND,
            % key binding dialog
            % TODO: better user-configurable verification. (E.g. duplicates allowed?
            % etc. But in a general fashion, i.e. through a callback function.)
            str = [str glc_listdlg_get_keyname(v)];
        end

        liststring{i} = str;
    end
end

function [ok,sel]=glc_listdlg_get_ok_sel(winid)
    global glc_ld

    ok = (glc_ld(winid).done==1);

    if (glc_ld(winid).selmode_edit)
        sel = glc_ld(winid).controlvals;
    else
        sel = find(glc_ld(winid).selected);
        if (ok)
            sel = sel(:)';
        else
            sel = [];
        end
    end
end

function s=glc_listdlg_dummyize_struct(s)
    assert(numel(s)==1 && isstruct(s));

    fn = fieldnames(s);
    for i=1:length(fn)
        s.(fn{i}) = [];
    end
end

function glc_listdlg_motion(buttonsdown, x, y)
    global GL glc glc_ld

    winid = glcall(glc.get, GL.WINDOW_ID);

    if (~buttonsdown)
        glc_ld(winid).mxy = [x y];
        glc_ld(winid).mxy(2) = glc_ld(winid).wh(2)-glc_ld(winid).mxy(2);

        glcall(glc.redisplay);
    end
end

function glc_listdlg_mouse(button, downp, x, y, mods)
    global GL glc glc_ld

    winid = glcall(glc.get, GL.WINDOW_ID);

    if (button==GL.BUTTON_LEFT && downp)
        glc_ld(winid).clicked = [glc_ld(winid).mxy mods];
    end

    glcall(glc.redisplay);
end

function glc_listdlg_run_cb()
    global GL glc glc_ld

    w = glcall(glc.get, GL.WINDOW_ID);

    controlcb = glc_ld(w).controlcb;
    cvals = glc_ld(w).controlvals;

    if (isa(controlcb, 'function_handle'))
        controlcb(cvals);
    else
        eval(controlcb);
    end
end

function glc_listdlg_keyboard(asc, x, y, mods)
    global GL glc glc_ld
    TYPE = glc_ld(1).TYPE;

    w = glcall(glc.get, GL.WINDOW_ID);

    eidx = glc_ld(w).editing;
    cd = glc_ld(w).controldesc;

    if (eidx < 0)
        % doing keybind
        glc_ld(w).editing = 0;
        if (~strcmp(glc_listdlg_get_keyname(asc), '<unknown>'))
            glc_ld(w).controlvals.(cd(-eidx).key) = uint32(asc);
            glc_ld(w).liststring = glc_listdlg_construct_str(cd, glc_ld(w).controlvals);
        end
        glcall(glc.redisplay);
        return;
    elseif (eidx > 0)
        [c,l,r] = deal(glc_ld(w).editstring{:});

        if (asc >= 32 && asc <= 126)
            glc_ld(w).editstring{2}(end+1) = asc;
        elseif (asc==8)  % backspace
            glc_ld(w).editstring{2}(max(1,end):end) = [];
        elseif (asc == GL.KEY_LEFT)
            if (~isempty(l))
                glc_ld(w).editstring = { c, l(1:end-1), [l(end) r] };
            end
        elseif (asc == GL.KEY_RIGHT)
            if (~isempty(r))
                glc_ld(w).editstring = { c, [l r(1)], r(2:end) };
            end
        end
    elseif (isstruct(cd))
        ei = find(glc_ld(w).selected);

        if ((cd(ei).type==TYPE.NUMBER || cd(ei).type==TYPE.MULTISEL) && ...
            (asc == GL.KEY_LEFT || asc == GL.KEY_RIGHT))

            dir = 1 - 2*(asc == GL.KEY_LEFT);
            if (mods==GL.MOD_CTRL)
                dir = dir*10;
            end
            key = cd(ei).key;
            val = glc_ld(w).controlvals.(key);

            switch (cd(ei).type)
              case TYPE.NUMBER,
                updownfunc = cd(ei).extra.updownfunc;
                glc_ld(w).controlvals.(key) = updownfunc(val, dir);
              case TYPE.MULTISEL,
                glc_ld(w).controlvals.(key) = min(max(1, val+dir), numel(cd(ei).extra));
            end

            glc_listdlg_run_cb();
            % NOTE: it's a bit overkill to reconstruct the whole list, but who cares...
            glc_ld(w).liststring = glc_listdlg_construct_str(cd, glc_ld(w).controlvals);
        end
    end

    if (asc==GL.KEY_LEFT || asc==GL.KEY_RIGHT)
        if (isstruct(cd) && cd(find(glc_ld(w).selected)).type==TYPE.TOGBTN)
            % left/right arrows --> toggle boolean value
            asc = 13;
        end
    end

    switch asc,
      case 1,  % Ctrl+a
        if (glc_ld(w).selmode_multiple)
            if (all(glc_ld(w).selected))
                glc_ld(w).selected(:) = false;
            else
                glc_ld(w).selected(:) = true;
            end
        end

      case 13,  % enter
        if (glc_ld(w).selmode_edit)
            if (eidx == 0)
                eidx = find(glc_ld(w).selected);
                assert(numel(eidx)==1);

                if (isstruct(cd) && cd(eidx).type==TYPE.KEYBIND)
                    glc_ld(w).editing = -eidx;  % <0: doing key bind
                elseif (~isstruct(cd) || cd(eidx).type==TYPE.STRING)
                    if (~isempty(glc_ld(w).controlvals.(cd(eidx).key)))
                        % if string is editable, starting to type
                        glc_ld(w).editing = eidx;
                        str = glc_ld(w).liststring{eidx};
                        glc_ld(w).oldstring = str;
                        colon = strfind(str, ': ');
                        if (~isempty(colon))
                            colon = colon(1);
                            glc_ld(w).editstring = { str(1:colon+1), str(colon+2:end), '' };
                        else
                            glc_ld(w).editstring = { '', str, '' };
                        end
                    end
                else
                    switch (cd(eidx).type)
                      case TYPE.TOGBTN,
                        glc_ld(w).controlvals.(cd(eidx).key) = ~glc_ld(w).controlvals.(cd(eidx).key);
                        glc_listdlg_run_cb();
                        glc_ld(w).liststring = glc_listdlg_construct_str(cd, glc_ld(w).controlvals);
                    end
                end
            else
                % finished typing
                newstr = [glc_ld(w).editstring{2:3}];
                if (isstruct(cd))
                    % TODO: run validation function.
                    glc_ld(w).controlvals.(cd(eidx).key) = newstr;
                    glc_listdlg_run_cb();
                end
                glc_ld(w).liststring{eidx} = [glc_ld(w).editstring{:}];
                glc_ld(w).oldstring = '';
                glc_ld(w).editing = 0;
            end
        elseif (~isempty(glc_ld(w).okstr))
            if (any(glc_ld(w).selected))
                glc_ld(w).done = 1;
            end
        end

      case 27,  % escape
        if (glc_ld(w).selmode_edit)
            if (eidx > 0)
                % canceling typing
                glc_ld(w).liststring{eidx} = glc_ld(w).oldstring;
                glc_ld(w).oldstring = '';
                glc_ld(w).editing = 0;
            end
        elseif (~isempty(glc_ld(w).cancelstr))
            glc_ld(w).done = 2;
        end

      % up/down: move "background" or selection (with CTRL)
      case GL.KEY_UP,
        uppermost = [];
        offset = glc_ld(w).ofs;
        didmove = false;
        if (mods~=GL.MOD_CTRL)
            % move selection one up
            uppermost = find(glc_ld(w).selected);
            if (numel(uppermost)==1 && uppermost > 1 && offset<uppermost)
                uppermost = uppermost-1;
                glc_ld(w).selected(:) = false;
                glc_ld(w).selected(uppermost) = true;
                didmove = true;
            end
        end
        if (~didmove && offset > 0 || uppermost==offset)
            glc_ld(w).ofs = offset-1;
        end

      case GL.KEY_PAGE_UP,
        glc_ld(w).ofs = max(0, glc_ld(w).ofs-10);

      case GL.KEY_DOWN,
        glc_ld(w).downreq = 1 - 2*(mods~=GL.MOD_CTRL);
      case GL.KEY_PAGE_DOWN,
        glc_ld(w).downreq = 10;
    end

    glcall(glc.redisplay);
end

function glc_listdlg_reshape(width, height)
    global GL glc glc_ld

    w = glcall(glc.get, GL.WINDOW_ID);

    glc_ld(w).wh = [width height];

    glc_ld(w).bbox = [20 glc_ld(w).wh(1)-20; ...
                        20+24+30 glc_ld(w).wh(2)-20];

    glcall(glc.viewport, [0 0 width height]);
    glcall(glc.setmatrix, GL.PROJECTION, [0 width, 0 height, -1 1]+0.5);
    glcall(glc.setmatrix, GL.MODELVIEW, []);

    glcall(glc.redisplay);
end

function glc_listdlg_display()
    global GL glc glc_ld
    TYPE = glc_ld(1).TYPE;

    w = glcall(glc.get, GL.WINDOW_ID);

    glcall(glc.clear, [1 1 1]);

    numvals = numel(glc_ld(w).liststring);

    haveokstr = ~isempty(glc_ld(w).okstr);
    havecancelstr = ~isempty(glc_ld(w).cancelstr);

    if (haveokstr)
        glc_drawbutton([20 20; 60 24].', glc_ld(w).okstr, glc_ld(w).mxy, false);
    end
    if (havecancelstr)
        glc_drawbutton([20+60+20 20; 100 24].', glc_ld(w).cancelstr, glc_ld(w).mxy, false);
    end

    if (~isempty(glc_ld(w).clicked))
        if (haveokstr)
            glc_callbutton([20 20; 60 24].', glc_ld(w).mxy, ...
                           'global glc_ld; glc_ld(arg).done=1;', w);
        end
        if (havecancelstr)
            glc_callbutton([20+60+20 20; 100 24].', glc_ld(w).mxy, ...
                           'global glc_ld; glc_ld(arg).done=2;', w);
        end
    end

    bbox = glc_ld(w).bbox;

    dy = bbox(4)-bbox(2);
    lineheight = 16;

    numlines = floor(max(0, dy/lineheight));

    if (glc_ld(w).downreq)
        j = glc_ld(w).downreq;
        moveselp = false;
        if (j < 0)
            moveselp = true;
            j = -j;
        end

        offset = glc_ld(w).ofs;

        lowermost = [];
        didmove = false;
        if (moveselp)
            lowermost = find(glc_ld(w).selected);
            if (numel(lowermost)==1 && lowermost < numvals && lowermost <= numlines+offset)
                lowermost = lowermost+1;
                glc_ld(w).selected(:) = false;
                glc_ld(w).selected(lowermost) = true;
                didmove = true;
            end
        end

        while (j > 0 && numlines+offset < numvals && (~didmove || lowermost > offset+1))
            offset = offset+1;
            j = j-1;
        end
        glc_ld(w).ofs = offset;

        glc_ld(w).downreq = false;
    end

    %% Per-line mouse handling.
    offset = glc_ld(w).ofs;
    xrange = bbox([1 3]);
    yorigin = glc_ld(w).wh(2)-20;

    point_in_i = glc_textlines_pointinrect(numlines, xrange, yorigin, lineheight, ...
                                           glc_ld(w).mxy);
    idx = 0;
    if (point_in_i >= 1 && point_in_i+offset <= numvals)
        idx = point_in_i+offset;

        if (~isempty(glc_ld(w).clicked))
            if (glc_ld(w).selmode_multiple)
                if (glc_ld(w).clicked(3)==GL.MOD_SHIFT)
                    if (glc_ld(w).lastclickidx > 0)
                        bounds = sort([idx glc_ld(w).lastclickidx]);
                        glc_ld(w).selected(bounds(1):bounds(2)) = true;
                    end
                elseif (glc_ld(w).clicked(3)==GL.MOD_CTRL)
                    glc_ld(w).selected(idx) = ~glc_ld(w).selected(idx);
                else
                    glc_ld(w).selected(:) = false;
                    glc_ld(w).selected(idx) = true;
                end
            else
                glc_ld(w).selected(:) = false;
                glc_ld(w).selected(idx) = true;
            end

            glc_ld(w).clicked = [];
            glc_ld(w).lastclickidx = idx;
        end
    end

    xmargin = 12;
    liststring = glc_ld(w).liststring;

    for runi=1:2
        for i=1:numlines
            idx = i + offset;
            if (idx > numvals)
                break;
            end

            y1 = yorigin - i*lineheight;

            bb = [xrange; ...
                  y1, y1+lineheight];

            cd = glc_ld(w).controldesc;
            if (runi==1)
                color = [1 1 1];
                if (glc_ld(w).selected(idx))
                    if (point_in_i==i)
                        color = [0.7 0.7 0.9];
                    else
                        color = [0.6 0.6 0.9];
                    end
                    if (glc_ld(w).editing == -i)
                        % doing keybind
                        color = color([3 1 2]);
                    end
                elseif (point_in_i==i)
                    color = [0.92 0.92 0.92];
                end

                % Draw background.
                glcall(glc.draw, GL.QUADS, glc_expandrect(bb), struct('colors', color));

                if (isstruct(cd) && glc_ld(w).selected(idx))
                    if (cd(idx).type==TYPE.MULTISEL)  % multi-state selection
                        cv = glc_ld(w).controlvals;
                        selidx = cv.(cd(idx).key);

                        if (selidx > 1)
                            glcall(glc.text, [xrange(1)-15 y1+1], lineheight-2, '<');
                        end
                        if (selidx < numel(cd(idx).extra))
                            glcall(glc.text, [xrange(2)+6 y1+1], lineheight-2, '>');
                        end
                    end
                end
            else
                % Draw text: must distinguish editing and plain cases.
                textorigin =  bb([1 2])+[xmargin 2];
                tmpargs = { textorigin, lineheight-2, liststring{idx} };

                if (glc_ld(w).editing == idx)
                    tmpargs{3} = [glc_ld(w).editstring{1:2}];
                    textlen = glcall(glc.text, tmpargs{:});
                    glcall(glc.text, textorigin+[textlen 0], lineheight-2, '|', ...
                           struct('colors',[0 0 0]));
                    tmpargs{3} = [glc_ld(w).editstring{:}];
                end

                glcall(glc.push, GL.LINE_BIT);
                ex = [];
                if (isstruct(cd))
                    ex = cd(idx).extra;
                end
                if (islogical(ex) && ex)
                    % highlight the text
                    glcall(glc.set, GL.LINE_WIDTH, 1.5)
                end

                glcall(glc.text, tmpargs{:});

                glcall(glc.pop, GL.LINE_BIT);
            end
        end
    end

    %% Draw the bounding box
    glcall(glc.draw, GL.QUADS+16, glc_expandrect(bbox));

    % Draw the "continuation triangles"
    centerx = (bbox(3)+bbox(1))/2;
    if (glc_ld(w).ofs > 0)
        glcall(glc.draw, GL.TRIANGLES, ...
               [centerx-20 centerx+20 centerx; ...
                bbox(4) bbox(4) bbox(4)+10]);
    end
    if (numlines+glc_ld(w).ofs < numvals)
        glcall(glc.draw, GL.TRIANGLES, ...
               [centerx-20 centerx+20 centerx; ...
                bbox(2) bbox(2) bbox(2)-10]);
    end

    glc_ld(w).clicked = [];

    if (glc_ld(w).done)
        glcall(glc.closewindow);

        parentwinid = glc_ld(w).parentwinid;
        if (parentwinid > 0)
            glcall(glc.set, GL.WINDOW_ID, parentwinid);  % might be already closed though
        end

        finish_cb = glc_ld(w).finish_cb;

        [ok, sel] = glc_listdlg_get_ok_sel(w);
        if (isa(finish_cb, 'function_handle'))
            finish_cb(ok, sel);
        else
            eval(finish_cb);  % finish_cb is a string
        end
    end
end
