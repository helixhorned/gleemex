function [sel,ok]=glc_listdlg(varargin)

    global GL glc glc_listdlg_s

    GL = getglconsts();
    glc = glcall();

    if (mod(nargin,2)~=0)
        error('Must pass an even number of input arguments');
    end

    haveliststring = false;

    liststring = {};
    selmode_multiple = true;
    listsize = [160 300];
    initialval = 1;
    name = '';
    promptstr = '';
    okstr = 'OK';
    cancelstr = 'Cancel';

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
            if (isequal(val, 'single'))  % case-insens?
                selmode_multiple = false;
            elseif (isequal(val, 'multiple'))
                selmode_multiple = true;
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
            assert(isvector(val) && ischar(val), 'OKString value must be a char vector');
            okstr = val;
          case 'cancelstring',
            assert(isvector(val) && ischar(val), 'CancelString value must be a char vector');
            cancelstr = val;
          otherwise,
            warning(sprintf('unrecognized option ''%s''', varargin{i}));
        end
    end

    % some more input checking
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
    s.listsize = listsize;
    s.initialval = initialval;
    s.name = name;
    s.promptstr = promptstr;
    s.okstr = okstr;
    s.cancelstr = cancelstr;

    s.selected = logical(zeros(size(liststring),'uint8'));
    s.selected(initialval) = true;

    s.done = 0;  % 1:OK, 2:Cancel
    s.ofs = 0;
    s.downreq = 0;  % set to 1 if pressed 'down' key, 10 if 'pgdn'

    s.clicked = [];
    s.lastclickidx = 0;
    s.mxy = [1 1];
    s.wh = listsize+20+[40 80];  % +20: deviation from MATLAB default

    s.bbox = [20 s.wh(1)-20; ...
                        20+24+20 s.wh(2)-20];
    s.tmpwinid = 0;

    if (isempty(s.name))
        s.name = ' ';
    end

    if (~isstruct(glc_listdlg_s))
        % init woes: in MATLAB, we can't do either
        %  - S(n) = S2, where S and S2 are structs with different fields
        %  - S(n) = S2, where S is [] (globals on init)
        % WARNING: this means that it's a bad idea to assign to nonexistent
        %  fields of glc_listdlg_s in callbacks, in other words they should be
        %  'declared' above first
        glc_listdlg_s = glc_listdlg_dummyize_struct(s);
    end

    winid = glcall(glc.newwindow, [200 200], s.wh, s.name);

    % should be before setting the callbacks
    glc_listdlg_s(winid) = s;

    glcall(glc.setcallback, glc.cb_reshape, 'glc_listdlg_reshape');
    glcall(glc.setcallback, glc.cb_motion, 'glc_listdlg_motion');
    glcall(glc.setcallback, glc.cb_keyboard, 'glc_listdlg_keyboard');
    glcall(glc.setcallback, glc.cb_mouse, 'glc_listdlg_mouse');
    glcall(glc.setcallback, glc.cb_display, 'glc_listdlg_display');

    glcall(glc.entermainloop);

    ok = (glc_listdlg_s(winid).done==1);
    sel = find(glc_listdlg_s(winid).selected);
    if (ok)
        sel = sel(:)';
    else
        sel = [];
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
    global GL glc glc_listdlg_s

    winid = glcall(glc.get, GL.WINDOW_ID);

    if (~buttonsdown)
        glc_listdlg_s(winid).mxy = [x y];
        glc_listdlg_s(winid).mxy(2) = glc_listdlg_s(winid).wh(2)-glc_listdlg_s(winid).mxy(2);

        glcall(glc.postredisplay);
    end
end

function glc_listdlg_mouse(button, downp, x, y, mods)
    global GL glc glc_listdlg_s

    winid = glcall(glc.get, GL.WINDOW_ID);

    if (button==GL.LEFT_BUTTON && downp)
        glc_listdlg_s(winid).clicked = [glc_listdlg_s(winid).mxy mods];
    end

    glcall(glc.postredisplay);
end

function glc_listdlg_keyboard(asc, x, y, mods)
    global GL glc glc_listdlg_s

    winid = glcall(glc.get, GL.WINDOW_ID);

    switch asc,
      case 1,  % Ctrl+a
        if (glc_listdlg_s(winid).selmode_multiple)
            if (all(glc_listdlg_s(winid).selected))
                glc_listdlg_s(winid).selected(:) = false;
            else
                glc_listdlg_s(winid).selected(:) = true;
            end
        end

      case 13,  % enter
        if (any(glc_listdlg_s(winid).selected))
            glc_listdlg_s(winid).done = 1;
        end

      case 27,  % escape
        glc_listdlg_s(winid).done = 2;

      case GL.KEY_UP,
        if (glc_listdlg_s(winid).ofs > 0)
            glc_listdlg_s(winid).ofs = glc_listdlg_s(winid).ofs-1;
        end
      case GL.KEY_PAGE_UP,
        glc_listdlg_s(winid).ofs = max(0, glc_listdlg_s(winid).ofs-10);

      case GL.KEY_DOWN,
        glc_listdlg_s(winid).downreq = 1;
      case GL.KEY_PAGE_DOWN,
        glc_listdlg_s(winid).downreq = 10;

    end

    glcall(glc.postredisplay);
end

function glc_listdlg_reshape(w, h)
    global GL glc glc_listdlg_s

    winid = glcall(glc.get, GL.WINDOW_ID);

    glc_listdlg_s(winid).wh = [w h];

    glc_listdlg_s(winid).bbox = [20 glc_listdlg_s(winid).wh(1)-20; ...
                        20+24+30 glc_listdlg_s(winid).wh(2)-20];

    glcall(glc.viewport, [0 0 w h]);
    glcall(glc.setmatrix, GL.PROJECTION, [0 w, 0 h, -1 1]+0.5);
    glcall(glc.setmatrix, GL.MODELVIEW, []);

    glcall(glc.postredisplay);
end

function glc_listdlg_display()
    global GL glc glc_listdlg_s

    winid = glcall(glc.get, GL.WINDOW_ID);

    glcall(glc.clear, [1 1 1]);

    numvals = numel(glc_listdlg_s(winid).liststring);

    glc_drawbutton([20 20; 60 24].', glc_listdlg_s(winid).okstr, glc_listdlg_s(winid).mxy, false);
    glc_drawbutton([20+60+20 20; 100 24].', glc_listdlg_s(winid).cancelstr, glc_listdlg_s(winid).mxy, false);

    if (~isempty(glc_listdlg_s(winid).clicked))
        glc_listdlg_s(1).tmpwinid = winid;
        glc_callbutton([20 20; 60 24].', glc_listdlg_s(winid).mxy, ...
                       'global glc_listdlg_s; glc_listdlg_s(glc_listdlg_s(1).tmpwinid).done=1;');
        glc_callbutton([20+60+20 20; 100 24].', glc_listdlg_s(winid).mxy, ...
                       'global glc_listdlg_s; glc_listdlg_s(glc_listdlg_s(1).tmpwinid).done=2;');
    end

    bbox = glc_listdlg_s(winid).bbox;

    dy = bbox(4)-bbox(2);
    lineheight = 16;

    numlines = floor(max(0, dy/lineheight));

    if (glc_listdlg_s(winid).downreq)
        j = glc_listdlg_s(winid).downreq;
        while (j > 0 && numlines+glc_listdlg_s(winid).ofs < numvals)
            glc_listdlg_s(winid).ofs = glc_listdlg_s(winid).ofs+1;
            j = j-1;
        end

        glc_listdlg_s(winid).downreq = false;
    end

    point_in_i = 0;

    for i=1:numlines
        idx = i + glc_listdlg_s(winid).ofs;
        if (idx > numvals)
            break;
        end

        y1 = glc_listdlg_s(winid).wh(2)-20-i*lineheight;

        bb = [bbox([1 3]); ...
              y1, y1+lineheight];

        % mouse click
        point_in_rect_p = glc_pointinrect(glc_listdlg_s(winid).mxy, bb);

        if (point_in_rect_p)
            point_in_i = i;

            if (~isempty(glc_listdlg_s(winid).clicked))
                if (glc_listdlg_s(winid).selmode_multiple)
                    if (glc_listdlg_s(winid).clicked(3)==100)  % Shift
                        if (glc_listdlg_s(winid).lastclickidx > 0)
                            bounds = sort([idx glc_listdlg_s(winid).lastclickidx]);
                            glc_listdlg_s(winid).selected(bounds(1):bounds(2)) = true;
                        end
                    elseif (glc_listdlg_s(winid).clicked(3)==10)  % Ctrl
                        glc_listdlg_s(winid).selected(idx) = ~glc_listdlg_s(winid).selected(idx);
                    else
                        glc_listdlg_s(winid).selected(:) = false;
                        glc_listdlg_s(winid).selected(idx) = true;
                    end
                else
                    glc_listdlg_s(winid).selected(:) = false;
                    glc_listdlg_s(winid).selected(idx) = true;
                end

                glc_listdlg_s(winid).clicked = [];
                glc_listdlg_s(winid).lastclickidx = idx;
            end
        end
    end

    for i=1:numlines
        idx = i + glc_listdlg_s(winid).ofs;
        if (idx > numvals)
            break;
        end

        y1 = glc_listdlg_s(winid).wh(2)-20-i*lineheight;

        bb = [bbox([1 3]); ...
              y1, y1+lineheight];

        % XXX: rects may occlude text :/
        color = [];
        if (glc_listdlg_s(winid).selected(idx))
            if (point_in_i==i)
                color = [0.7 0.7 0.9];
            else
                color = [0.6 0.6 0.9];
            end
        elseif (point_in_i==i)
            color = [0.92 0.92 0.92];
        end
        if (~isempty(color))
            glcall(glc.draw, GL.QUADS, glc_expandrect(bb), struct('colors', color));
%        else
%            glcall(glc.draw, GL.QUADS+16, glc_expandrect(bb));
        end

        glcall(glc.rendertext, bb([1 2])+[12 2], lineheight-2, glc_listdlg_s(winid).liststring{idx});
    end

    glcall(glc.draw, GL.QUADS+16, glc_expandrect(bbox));

    centerx = (bbox(3)+bbox(1))/2;
    if (glc_listdlg_s(winid).ofs > 0)
        glcall(glc.draw, GL.TRIANGLES, ...
               [centerx-20 centerx+20 centerx; ...
                bbox(4) bbox(4) bbox(4)+10]);
    end
    if (numlines+glc_listdlg_s(winid).ofs < numvals)
        glcall(glc.draw, GL.TRIANGLES, ...
               [centerx-20 centerx+20 centerx; ...
                bbox(2) bbox(2) bbox(2)-10]);
    end

    glc_listdlg_s(winid).clicked = [];

    if (glc_listdlg_s(winid).done)
        glcall(glc.closewindow);
    end
end