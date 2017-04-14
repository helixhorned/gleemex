
Gleemex User Manual
===================

Introduction
------------

Gleemex is a high-level interface to OpenGL functionality for GNU Octave and
MATLAB. It uses FreeGLUT to provide windowing and peripheral access for various
platforms, GLEW for determining OpenGL features, and the MEX API to communicate
with the host program. It allows writing interactive graphical applications
entirely in the M scripting language.


### Motivation

The graphics systems of MATLAB and Octave are geared towards producing plots of
moderate complexity for offline display and printout, but are less suited for
writing applications that interact with the user. Gleemex enables the creation
of event-driven graphical tools, on one hand aiming at simplicity of usage, but
without sacrificing performance where it is needed. Its main virtues are in

* Being a _higher level_ interface to OpenGL. Thus, rather than exposing OpenGL
  functions directly, they are wrapped in commands that perform a composite
  function. At the same time, it tries to add as little overhead as possible.

* Providing _safety_ as a consequence of the above. It must not be possible to
  invoke erratic behavior (such as crashes caused by accessing memory out of
  some allowed bounds) by providing invalid data to Gleemex. Input is validated
  where necessary. If undefined behavior can be invoked from M code, it is
  always a Gleemex bug.

* Aiming at simplicity of both usage and concept. For example,

    - After creating a window and adding the desired callbacks, starting the
      FreeGLUT main loop makes it run in a single thread of execution. This is in
      contrast to the native MATLAB "handle graphics" system, that allows for
      some form of concurrency, sometimes making program behavior hard to predict.

    - Drawing proceeds in a "fire and forget" fashion familiar from game
      programming. Instead of having to _manage_ objects (such as lines) by
      writing separate code for their creation, potential update, and deletion, a
      customary Gleemex app only keeps some _state_ in a global M variable. In the
      display callback, that state is then used to render one frame.

* Being strict with regard to errors. Any error from M code while the main loop
  is active makes Gleemex close all of its windows and report it. This eases
  debugging of graphical apps, since otherwise an erroneous condition may
  produce followup failures, obscuring the original cause.


### Installation

[FreeGLUT]: http://freeglut.sourceforge.net/
[GLEW]: http://glew.sourceforge.net/

The Gleemex core is implemented in a single source file, `glcall.c`. On a
supported combination of OS, architecture and M-language implementation, and
assuming a compatible C compiler is installed (see `help mex`), running
`mkglcall` usually suffices to build a dynamically loaded MEX library that can
be used with the given setup.

Gleemex depends on [FreeGLUT] and [GLEW]. For convenience, on Windows, GLEW is
packaged together with Gleemex and does not need to be installed separately.


The `glcall` API
----------------

The `glcall` MEX function is the only entry point to Gleemex. It consolidates
various sub-functions which are selected depending on the first argument passed
to it, the _command_. Invoking `glcall` without input arguments returns a
struct containing a mapping of names to numeric constants denoting these
commands, as well as those of callback functions to be registered once a window
has been created.
This struct should be stored in a global named `glc`.

Another function, `glconstants`, returns a struct containing mappings for
OpenGL and FreeGLUT constants, together with some auxiliary definitions. By
convention, it is stored into a global `GL`. [Note: Conceptually, one does
not _need_ to use the names `glc` and `GL`. However, currently Gleemex
references these named globals in M-file auxiliary functions "behind a user's
back" without first assigning to them. In the future, the restriction to use
these particular names may be lifted by substituting the numerical values for
these references.]

Thus, in the "main" function of an application, one usually initializes these
two globals for future use as follows:

`hello_gleemex.m`
~~~matlab
function hello_gleemex()
    global GL glc
    glc = glcall();
    GL = glconstants();

    % ...
end
~~~


### Callback functions

Callback functions are user-defined functions that are invoked when a
particular event, such as a mouse press, happens. Because the MEX API provides
no means of inspecting _function handle_ objects, callbacks are passed to
Gleemex **by name**. For example, the preceding hello-world app could register
a display callback named `hg_display` like this:

~~~matlab
function hello_gleemex()
    % ...
    glcall(glc.setcallback, glc.cb_display, 'hg_display');
    % ...
end

function hg_display()
    global GL glc
    glcall(glc.clear, [1 0 0]);  % clear the window to a red color
end
~~~

How the actual function corresponding to a particular name is looked up
differs between Octave and MATLAB. In Octave, it is possible to call secondary
functions by name when the primary function is active, but only when the latter
(here, `hello_gleemex`) has been invoked from the top level!

In MATLAB, secondary functions appear to never be considered. For this reason,
they can either be written in separate files from the start, or have to be
extracted from the common file. A helper function `extractsecfuncs` is provided
for this task.


### Initialization, setup and teardown commands

[FGAPI]: http://freeglut.sourceforge.net/docs/api.php

<a name="glc_newwindow"/>

#### `[WINID, GLUTWINID] = glcall(glc.newwindow, POS, EXTENT, WINDOWNAME [, OPTS])`

[`glutCreateSubWindow`]: https://www.opengl.org/resources/libraries/glut/spec3/node17.html

Creates a new (sub-)window with the given properties and a separate, new OpenGL
context. To make the window do something useful, it needs to have callbacks
attached afterwards.

CAUTION: Gleemex windows and MATLAB handle graphics figures should never be open
at the same time. Mixing them may entail crashes or other undesired behavior.

* `WINDOWNAME`: the name of the window, to be shown in the top bar

* `POS`: the initial (x, y) position of the window, must be of type `double`

* `EXTENT`: the initial (width, height) pair of the window, must be of type `double`

  When creating a top-level window (`OPTS.subwindow` not given or false), `POS`
  and `EXTENT` are passed to [`glutInitWindowPosition`][FGAPI] and
  [`glutInitWindowSize`][FGAPI], respectively. Otherwise, they are passed to
  [`glutCreateSubWindow`].

* `OPTS`: A struct containing additional options:

    - `subwindow`: If true, creates a sub-window in the current window instead of
      a new top-level one.

    - `menus`: A special kind of struct (called _menu struct_ from now on) that
      defines a menu to be opened when a particular mouse button is pressed in the
      window to be created. The menu has some top-level properties and may
      recursively include sub-menus. Consequently, some of its fields may only
      appear in the outermost struct, while others are also allowed to exist in
      those nested deeper.

        + `button` (top level only): the button that needs to be pressed in order to
          open the menu
        + `cbfunc`: The name of the callback function to be invoked when the user
          activates a menu entry. It may appear in both root and child menu structs,
          with semantics detailed below.
        + `entries`: A cell array `{ e_1, e_2, ..., e_n }` sequence of the menu's
          entries, in the order they ultimately appear. An element `e_i` can be
          either a leaf that can be activated, or another sub-menu. Where this cell's
          element is a string, the entry is considered a leaf, and the string is
          taken as its label in the menu. Otherwise, the element is expected to be a
          menu struct describing the child, and its label is taken from the child's
          `label` field.
        + `label`: (child only): the label of the sub-menu which this menu
          struct describes in its parent's menu

    - When the user activates a menu entry, a suitable callback function is
      searched first. Conceptually, this is done by traversing the menu tree,
      starting at the selected entry and working towards the root (i.e. following
      the parent links). When a menu struct containing a `cbfunc` field is
      encountered, the named function is invoked with two arguments:
      `cbfunc(label, idx)`.

        + `label`: the label of the menu entry, as given in the menu struct
        + `idx`: TODO (not yet specified)

The return value `WINID` is a small integer identifying the window for as long
as it is live. Note that it is **not** the identifier passed from FreeGLUT but
one specific to Gleemex. Its values are in the range [1 .. `MAXACTIVEWINDOWS`],
where at most `MAXACTIVEWINDOWS` can be open at the same time (currently,
32). The identifiers of windows that have been closed may subsequently be
reused.

The FreeGLUT window index can be obtained in the second output argument
`GLUTWINID`. Currently, the only place where Gleemex accepts a FreeGLUT window
index is as the second element of a pair to [`glc.set`](#glc_set) / `GL.WINDOW_ID`.

<a name="glc_closewindow"/>

#### `glcall(glc.closewindow [, WINID])`

Closes the window given by the Gleemex identifier `WINID`, or the current
window if `WINID` is omitted.

<a name="glc_setcallback"/>

#### `glcall(glc.setcallback, CALLBACK_KIND, CBFUNC_NAME)`

[FGWindowCallback]: http://freeglut.sourceforge.net/docs/api.php#WindowCallback

Sets the specified callback `CALLBACK_KIND` of the current window to the
function named `CBFUNC_NAME`. The following values are permissible for
`CALLBACK_KIND`; the names in angle brackets denote the respective GLUT
registration functions.

- `glc.cb_display`, \[`glutDisplayFunc`]

  Invoked with no arguments when the windows is about to be redrawn. At the
  end, Gleemex always issues a call to `glutSwapBuffers()`.

<a name="cb_reshape"/>

- `glc.cb_reshape`, \[`glutReshapeFunc`]

  Invoked with two arguments `(width, height)` when the window has been resized,
  and on initialization. Thus, it is possible to e.g. set up a projection matrix
  in this callback. If no custom reshape callback is registered, Gleemex
  registers one that issues `glViewport(0, 0, width, height)`.

- `glc.cb_position`, \[[`glutPositionFunc`][FGWindowCallback]]

  Invoked with two arguments `(x, y)` when the window is repositioned/moved
  programatically or by the user.

- `glc.cb_keyboard`, \[`glutKeyboardFunc`, `glutSpecialFunc`]

  Invoked with four arguments `(key, x, y, mods)` when a keyboard key has been
  pressed. Gleemex consolidates the two GLUT callback functions into one for
  ease of use. The `key` argument can be checked for equality with either a
  single-character string such as `'a'`, or with constants `GL.KEY_*`. The
  `mods` argument denotes which modifier keys are pressed and can be checked
  against the `GL.MOD_*` bit constants.

  NOTE: For "key chords" such as `Ctrl + a`, FreeGLUT may pass different
  combinations of `key` and `mods` on different platforms.

<a name="cb_mouse"/>

- `glc.cb_mouse`, \[`glutMouseFunc`]

  Invoked with five arguments `(button, downp, x, y, mods)` when a mouse button
  is pressed (`downp` is true) or released (`downp` is false). The `button`
  arguments can be checked against the `GL.BUTTON_*` and `GL.MWHEEL_*` bit
  constants, `mods` are the pressed modifier keys as with the keyboard callback.

  NOTE: The reported position has the origin at the origin at the **topmost**
  and leftmost pixel. Thus, it is reversed with [`glc_setup2d`](#glc_setup2d)
  in the y screen dimension.

<a name="cb_motion"/>

- `glc.cb_motion`, \[`glutMotionFunc`, `glutPassiveMotionFunc`]

  Invoked with three arguments `(buttons, x, y)` when the mouse is moved. The
  `buttons` argument has the `GL.BUTTON_*` bits set which are depressed at the
  time the movement happens. The `(x, y)` position is reported like for the
  `cb_mouse` callback.

  NOTE: Currently, `buttons` may not be entirely reliable in multi-window
  scenarios.

#### `glcall(glc.entermainloop)`

Enters the FreeGLUT main loop once. Issuing `glc.entermainloop` while the main
loop is active is a no-op.

#### `glcall(glc.leavemainloop)`

Instructs FreeGLUT to stop executing the main loop. This will close **all**
Gleemex windows and return to the prompt of the M-language implementation.
In most cases, [`glc.closewindow`](#glc_closewindow) should be preferred.

#### `[STR1, STR2, EX] = glcall(glc.geterrstr)` \[MATLAB only]

Returns information about the last error.

Currently, on Octave, when an error happens, Gleemex immediately issues
`mexErrMsgText` and thus effectively jumps back to the prompt. **TODO**: it is
not clear by itself that this is correct, i.e. doesn't leak resources (or
worse).

On MATLAB, issuing `mexErrMsgText` while the FreeGLUT main loop is active has
been found to produce crashes, which is why a different approach is taken: the
error message is backed up in persistent variables, and control is returned to
the prompt by normally returning from all active Gleemex functions and
terminating the FreeGLUT main loop.

The `glc.geterrstr` command can be used to query the messages of the last error
(`STR1` and `STR2`) as well as obtain the `MException` associated with it,
`EX`, from the MATLAB prompt. Thus, after your Gleemex app terminates
abnormally, you would usually issue

~~~matlab
global glc
[a,b,c] = glcall(glc.geterrstr);
c.getReport()
~~~

to pretty-print the error together with a backtrace.


### Commands that change or query OpenGL state

<a name="glc_viewport"/>

#### `glcall(glc.viewport, XYWH)`

[`glViewport`]: http://www.opengl.org/sdk/docs/man2/xhtml/glViewport.xml

Sets the viewport by calling [`glViewport`] with the elements of `XYWH`, which
must be a 4-vector of type `double`.

#### `glcall(glc.scissor, XYWH)`

[`glScissor`]: http://www.opengl.org/sdk/docs/man2/xhtml/glScissor.xml

Defines a scissor box by calling [`glScissor`] with the elements of `XYWH`,
which must be a 4-vector of type `int32`.

#### `OLDSTATE = glcall(glc.toggle, KV_PAIRS)`

[`glEnable`]: http://www.opengl.org/sdk/docs/man2/xhtml/glEnable.xml

Toggles the [enable][`glEnable`] state of various server-side GL capabilities.

* `KV_PAIRS`: A vector of even length containing successive pairs of `KEY` and
  `VALUE`. Must be of type `int32`; this is automatically ensured if `KV_PAIRS`
  is constructed as concatenation of values containing `GL.*` values, as
  detailed below.

    - `KEY`: An allowed GL constant denoting the capability: one of
      `GL.DEPTH_TEST`, `GL.SCISSOR_TEST`, `GL.BLEND`, `GL.POINT_SMOOTH`,
      `GL.LINE_SMOOTH`, `GL.LINE_STIPPLE`, `GL.POLYGON_SMOOTH`,
      `GL.POLYGON_OFFSET_POINT`, `GL.POLYGON_OFFSET_LINE`, `GL.POLYGON_OFFSET_FILL`,
      `GL.FOG`.

    - `VALUE`: 1 for enabling the capability, 0 for disabling it, or -1 for
      flipping its enable state.

* `OLDSTATE`: an `int32` vector of the same size and format as `KV_PAIRS`
  containing the old enable state of the requested capabilities. It can be
  passed to a later `glc.toggle` call to restore that state.

Example:
~~~matlab
    % enable depth testing and blending
    glcall(glc.toggle, [GL.DEPTH_TEST 1, GL.BLEND 1]);
~~~

#### `glcall(glc.push, WHAT)` and `glcall(glc.pop, WHAT)`

[`glPushMatrix`]: http://www.opengl.org/sdk/docs/man2/xhtml/glPushMatrix.xml
[`glPushAttrib`]: http://www.opengl.org/sdk/docs/man2/xhtml/glPushAttrib.xml

Pushes or pops the [matrix][`glPushMatrix`] or [server attribute][`glPushAttrib`]
stack. `WHAT` is a vector of constants denoting what to push or pop: allowed
values are

* `GL.PROJECTION`, `GL.MODELVIEW` and `GL.TEXTURE` for the respective matrices
* The `GL.*_BIT` constants for the sets of attributes documented in
  [`glPushAttrib`]

<a name="glc_set"/>

#### `glcall(glc.set, WHAT, VALUE)`

[`glPointSize`]: http://www.opengl.org/sdk/docs/man2/xhtml/glPointSize.xml
[`glLineWidth`]: http://www.opengl.org/sdk/docs/man2/xhtml/glLineWidth.xml
[`glLineStipple`]: http://www.opengl.org/sdk/docs/man2/xhtml/glLineStipple.xml

[`glBlendEquation`]: http://www.opengl.org/sdk/docs/man2/xhtml/glBlendEquation.xml
[`glDepthFunc`]: http://www.opengl.org/sdk/docs/man2/xhtml/glDepthFunc.xml
[`glDepthMask`]: http://www.opengl.org/sdk/docs/man2/xhtml/glDepthMask.xml
[`glPolygonOffset`]: http://www.opengl.org/sdk/docs/man2/xhtml/glPolygonOffset.xml

Sets the GL and FreeGLUT state variable denoted by `WHAT` to a new
value. `WHAT` can be one of the following named Gleemex constants:

* `GL.MENU_ENABLE`: `VALUE` is a scalar logical of whether to enable
  the menu in the current window.

* `GL.MOUSE_POS`: `VALUE` is a 2-vector of type `double` whose
  elements are passed to `glutWarpPointer`. The values must be greater or equal
  0 and less that or equal to the width/height of the window.

* `GL.WINDOW_ID`: TODO

* `GL.WINDOW_SIZE`: TODO

* `GL.POINT_SIZE`: `VALUE` is a `double`-typed scalar passed to [`glPointSize`].

* `GL.LINE_WIDTH`: `VALUE` is a `double`-typed scalar passed to [`glLineWidth`].

* `GL.LINE_STIPPLE_PATTERN`: `VALUE` is a `uint16`-typed scalar passed as the
  _pattern_ to [`glLineStipple`]. For the _factor_ argument, 1 is passed.

* `GL.BLEND_EQUATION`: `VALUE` is a GL constant permissible to
  [`glBlendEquation`]: `GL.MIN`, `GL.MAX`, `GL.FUNC_ADD`, `GL.FUNC_SUBTRACT` or
  `GL.FUNC_REVERSE_SUBTRACT`.

* `GL.DEPTH_FUNC`: `VALUE` is a GL constant permissible to [`glDepthFunc`]:
  `GL.NEVER`, `GL.LESS`, `GL.EQUAL`, `GL.LEQUAL`, `GL.GREATER`, `GL.NOTEQUAL`,
  `GL.GEQUAL` or `GL.ALWAYS`.

* `GL.DEPTH_WRITEMASK`: `VALUE` is a scalar logical, whose corresponding GL
  value (`GL.TRUE` or `GL.FALSE`) is passed to [`glDepthMask`].

* `GL.POLYGON_OFFSET`: `VALUE` is a pair of `single`-typed values, which are
  passed as the two arguments _factor_ and _units_ of [`glPolygonOffset`].

#### `VALUE = glcall(glc.get, WHAT)`

Gets the current value of the GL or FreeGLUT state variable denoted by `WHAT`.

* `GL.WINDOW_ID`: see [`glc.set`](#glc_set)
* `GL.WINDOW_SIZE`: see [`glc.set`](#glc_set)
* `GL.WINDOW_POS`: TODO
* `GL.POINT_SIZE`
* `GL.PROJECTION_MATRIX`, `GL.MODELVIEW_MATRIX`: `VALUE` is a 4-by-4
  `double`-typed matrix.

<a name="glc_setmatrix"/>

#### `glcall(glc.setmatrix, WHICH, M)`

[`glLoadMatrixd`]: http://www.opengl.org/sdk/docs/man2/xhtml/glLoadMatrix.xml
[`glOrtho`]: http://www.opengl.org/sdk/docs/man2/xhtml/glOrtho.xml
[`gluPerspective`]: http://www.opengl.org/sdk/docs/man2/xhtml/gluPerspective.xml

Sets the `GL.PROJECTION` or `GL.MODELVIEW` matrix (selected by `WHICH`) to `M`,
which must be a `double`-typed array and can be one of the following:

* An empty array `[]` is interpreted to reset the respective matrix to the
  identity matrix.
* A 4-by-4 matrix is passed unmodified to [`glLoadMatrixd`].
* For `GL.PROJECTION`, it is possible to pass a vector of length 4 or 6.
    - The elements of a 4-vector are passed to [`gluPerspective`].
    - The elements of a 6-vector are passed to [`glOrtho`].

#### `glcall(glc.mulmatrix, WHICH, M)`

[`glMultMatrixd`]: http://www.opengl.org/sdk/docs/man2/xhtml/glMultMatrix.xml
[`glTranslated`]: http://www.opengl.org/sdk/docs/man2/xhtml/glTranslate.xml
[`glRotated`]: http://www.opengl.org/sdk/docs/man2/xhtml/glRotate.xml

Multiplies the `GL.PROJECTION` or `GL.MODELVIEW` matrix (selected by `WHICH`)
with `M`, which must be a `double`-typed array and can be one of the following:

* A 4-by-4 matrix is passed unmodified to [`glMultMatrixd`].
* The elements of a 3-vector are passed to [`glTranslated`].
* The elements of a 4-vector are passed to [`glRotated`].

#### `glcall(glc.fog, FOG_MODE, FOG_PARAM, FOG_COLOR)`

[`glFog`]: http://www.opengl.org/sdk/docs/man2/xhtml/glFog.xml

Sets the [fog][`glFog`] parameters and color.

* `FOG_MODE`: one of `GL.LINEAR`, `GL.EXP` or `GL.EXP2`
* `FOG_PARAM`: for `GL.LINEAR` fog, a 2-vector specifying the near and far
  distances for the linear fog equation. Otherwise, the fog density. Must have
  numeric type `single`.
* `FOG_COLOR`: a 4-vector of `single` numeric type specifying the color of the
  fog


### Drawing and related commands

#### `glcall(glc.clear [, COLOR])`

[`glClear`]: http://www.opengl.org/sdk/docs/man2/xhtml/glClear.xml

[Clears][`glClear`] the depth buffer to 1. Also, if `COLOR` is provided as an
`(R, G, B [, A])` vector of type `double`, the color buffer is cleared to that
value.

#### `glcall(glc.draw, PRIMITIVE_TYPE, VERTEXDATA [, OPTSTRUCT])`

[`glDrawElements`]: http://www.opengl.org/sdk/docs/man2/xhtml/glDrawElements.xml
[`glBegin`]: http://www.opengl.org/sdk/docs/man2/xhtml/glBegin.xml
[`glNormalPointer`]: http://www.opengl.org/sdk/docs/man2/xhtml/glNormalPointer.xml

Passes vertex data to OpenGL, instructing it to draw them as primitives of
`PRIMITIVE_TYPE` and optionally providing face indices, colors and/or texture
coordinates.

* `PRIMITIVE_TYPE`: The kind of geometric [primitive][`glBegin`] to draw, one of\
  `GL.POINTS`, `GL.LINES`, `GL.LINE_LOOP`, `GL.LINE_STRIP`, `GL.TRIANGLES`,
  `GL.TRIANGLE_STRIP`, `GL.TRIANGLE_FAN`, `GL.QUADS`, `GL.QUAD_STRIP` or
  `GL.POLYGON`.\
  If 16 is added to `PRIMITIVE_TYPE`, only the outlines of polygons are drawn.
  (TODO: make a symbolic constant?)

* `VERTEXDATA`: A `(K, NumTotalVerts)` matrix of floating numeric type
  containing the vertex coordinates of the primitives.

    - `K`: The number of dimensions. Can be 2, 3 or 4, corresponding to x, y,
      z and/or w coordinates.
    - `NumTotalVerts`: The total number of vertices, must be consistent with the
      given `PRIMITIVE_TYPE`. For example, requesting `GL.LINES` expects
      `NumTotalVerts` to be even.

* `OPTSTRUCT`: A struct that can be used to pass additional data. May contain
  the following fields:

    - `colors`: The `(R, G, B [, A])` colors. Can be either a vector of length
      3 or 4, in which case all vertices are given that color, or a `(3 or 4,
      NumTotalVerts)` matrix, specifying a separate color for each vertex. Must
      have type `double`, `single` or `uint8`.\
      If no colors are passed, the default one is white when a texture is
      provided, and (0.5, 0.5, 0.5) otherwise.\
      As a special convenience feature, if a 5-tuple `[R G B A true]` is
      passed, then blending is enabled temporarily.

    - `indices`: Zero-based indices into the columns of `VERTEXDATA` used to
      construct the primitives. See [`glDrawElements`]. Must have _index type_
      (`uint8` or `uint32`).\
      When `indices` are passed, the number of drawn vertices equals the number of
      elements in `indices` (the dimensions don't matter), and each of its
      elements must be less than `NumTotalVerts`. This bound check has to be done
      every `glc.draw` call, so providing a large amount of indices may be
      expensive.

    - `normals`: a floating type matrix of the same size as `VERTEXDATA`,
       giving the [normals][`glNormalPointer`] for each vertex.

    - `tex`: The OpenGL _texture name_ (a non-zero `uint32` number as returned by
      [`glc.texture`](#glc_texture)) with which the primitives are to be
      textured.

    - `texcoords`: The s and optionally t texture coordinates at each vertex. Must
      be a `(1 or 2, NumTotalVerts)` matrix of floating numeric type. Must be
      passed whenever `tex` is passed.

#### `glcall(glc.text, POS, HEIGHT, TEXT [, XYALIGN] [, OPTS])`

Renders a line of text using a FreeGLUT's Roman stroke font.

Assuming that [`glc_setup2d`](#glc_setup2d) has been used to set up a 2D view,
the string `TEXT` is drawn at pixel position `POS` and with height `HEIGHT`
(which should be a `double`-typed 2-vector and scalar, respectively). `TEXT`
should not contain newlines.

* `XYALIGN`: an optional `double`-typed 2-vector `[xalign yalign]` specifying
  the horizontal and vertical alignment of the text.

    - -1: align on left/bottom (i.e. towards negative coordinates)
    - 0: align at the center
    - 1: align on right/top

* `OPTS`: an option struct which may contain the following fields:

    - `colors`: a 3-vector of type `double` giving the `(R, G, B)` color of the
       text
    - `xgap`: The distance between successive characters, expressed as a
       proportion of the width of the space character. The default is a value that
       was experimentally determined to look good. Must have `double` type.
    - `mono`: a scalar logical of whether to use the monospaced Roman font

<a name="glc_redisplay"/>

#### `glcall(glc.redisplay [, NOW])`

[`glutPostRedisplay`]: http://www.opengl.org/resources/libraries/glut/spec3/node20.html
[`glutSwapBuffers`]: http://www.opengl.org/resources/libraries/glut/spec3/node21.html

With no additional arguments, issues [`glutPostRedisplay`].  If `NOW` is
passed, it should be a scalar logical; if `NOW` is true, [`glutSwapBuffers`] is
issued instead.

#### `PIXELS = glcall(glc.readpixels [, XYWH])`

[`glReadPixels`]: http://www.opengl.org/sdk/docs/man2/xhtml/glReadPixels.xml

[Reads][`glReadPixels`] a block of pixels from the framebuffer using the `GL_RGB`
format. If `XYWH` is passed (which must be a `double`-typed 4-vector), its
elements are passed to the _x_, _y_, _width_ and _height_ arguments of
`glReadPixels`. Otherwise, the default is to grab the whole window.

`PIXELS` is returned as a 3-by-_width_-by-_height_ `uint8` array.


### Resource definition commands

<a name="glc_texture"/>

#### `TEXID = glcall(glc.texture, TEXIMG [, OPTS])` or `glcall(glc.texture, TEXIMG, TEXID [, OPTS])`

[`glTexImage2D`]: http://www.opengl.org/sdk/docs/man2/xhtml/glTexImage2D.xml
[`glTexParameter`]: http://www.opengl.org/sdk/docs/man2/xhtml/glTexParameter.xml

[Uploads][`glTexImage2D`] an image `TEXIMG` for a texture, either creating a new
GL texture name `TEXID` (first form) or reusing an already given texture name
(second form). Various options in `OPTS` allow either modifying certain aspects
of the texture rendering such as its filtering, or control how `TEXIMG` itself
is interpreted.

The format and type of a color component of the texture is determined by
various properties of `TEXIMG`:

* `TEXIMG` can be a `width`-by-`height` matrix. In this case, it is uploaded
  using the `GL_LUMINANCE` format. As a special case, if `OPTS.u32rgba` is
  given and true, `TEXIMG` must have class `uint32` and the 32-bit unsigned
  integers are interpreted as RGBA quadruplets (least significant byte is red).

* `TEXIMG` can be a `k`-by-`width`-by-`height` array, with `k` being the number
   of color components in the texture. It can be one of 2, 3 or 4, in which
   case the texture is uploaded in `GL_LUMINANCE_ALPHA`, `GL_RGB` or `GL_RGBA`
   format, respectively.

* `TEXIMG` may have either `single` numeric class, or one of `[u]int{8,16,32}`.

`OPTS` may be a struct containing the following fields:

* `u32rgba`: described above

* `minmag`: a scalar or 2-vector containing the GL constants denoting the
  minification and magnification [filters][`glTexParameter`]. Each one can be
  `GL.NEAREST` or `GL.LINEAR`. If a scalar is passed, both filters are set to
  the given mode. The default is `GL.LINEAR`.

Currently, there is no way to set the texture's wrapping mode: it is always set
to `GL_CLAMP_TO_EDGE`. Uploading mipmaps is not supported, either.

#### `glcall(glc.deletetextures, TEXIDS)`

[`glDeleteTextures`]: http://www.opengl.org/sdk/docs/man2/xhtml/glDeleteTextures.xml

[Deletes][`glDeleteTextures`] the textures named by the elements of the vector
`TEXIDS`.

<a name="glc_colormap"/>

#### `glcall(glc.colormap, COLORMAP)`

Uploads a colormap texture and binds it as a one-dimensional texture to a
texture unit different from `GL_TEXTURE0`. The texture is set to linear
filtering and `GL_CLAMP_TO_EDGE` wrapping.

* `COLORMAP`: must be a 3-by-256 `uint8` matrix of (R, B, G) colors.

This command allows using the so registered colormap from fragment shaders. See
the notes labeled **COLORMAP_DEF** below.


### Shader commands

<a name="glc_newprogram"/>

#### `PROGID [, UNIFORMS] = glcall(glc.newprogram, FRAGSHADERSRC [, VERTSHADERSRC])`

[`glCompileShader`]: http://www.opengl.org/sdk/docs/man2/xhtml/glCompileShader.xml
[`glAttachShader`]: http://www.opengl.org/sdk/docs/man2/xhtml/glAttachShader.xml
[`glLinkProgram`]: http://www.opengl.org/sdk/docs/man2/xhtml/glLinkProgram.xml

[`glGetActiveUniform`]: http://www.opengl.org/sdk/docs/man2/xhtml/glGetActiveUniform.xml

[GLSL_1_20]: http://www.opengl.org/registry/doc/GLSLangSpec.Full.1.20.8.pdf
[GLSL_1_50]: https://www.opengl.org/registry/doc/GLSLangSpec.1.50.09.pdf
[GLSL_newer]: http://www.opengl.org/documentation/glsl/

Creates a program object to be run on the programmable vertex and/or fragment
processor of the graphics processing unit. The program can have at most one
vertex shader and at most one fragment shader [attached][`glAttachShader`]. Their
sources are provided in `FRAGSHADERSRC` and `VERTSHADERSRC`, respectively,
where passing an empty string means that the particular shader should not be
[compiled][`glCompileShader`] as part of the program.

Vertex and fragment shaders are written in the OpenGL Shading Language. See the
[1.20][GLSL_1_20] or [1.50][GLSL_1_50] specification for an easier
introduction, or the [newer versions][GLSL_newer] for a more complete overview.

If either the creation of a shader or program object failed, or there was an
error with compilation or [linking][`glLinkProgram`], an M error is issued.

**COLORMAP_DEF**: If the fragment shader contains a uniform variable named
`cmap`, it must be declared with `uniform sampler1D` type. The vertex shader
must not contain a variable with that name. In the shader, you can sample the
colormap texture, for example

~~~glsl
uniform sampler1D cmap;
int main(void) {
  // ...
  float val = /* (some value in [0 .. 1] */;
  vec3 rgb = texture1D(cmap, val).rgb;  // get the RGB triplet for 'val'
  // ...
}
~~~

On success, returns

* `PROGID`: an identifier by which the created program can be referenced. The
  program is *not* enabled by creating it, use
  [`glc.useprogram`](#glc_useprogram) for this.

* `UNIFORMS`: a struct mapping [active uniform][`glGetActiveUniform`] variable
  names that are also valid MATLAB identifiers to Gleemex-specific handles
  permissible to [`glc.setuniform`](#glc_setuniform). Only names of
  user-defined uniform variables with the following types are listed (the
  variables may be either scalars or arrays of these _base types_):

    - base scalars: `GL_BOOL`, `GL_FLOAT`, `GL_INT`
    - base vectors: `GL_{BOOL,INT,FLOAT}_VEC{2,3,4}`

<a name="glc_useprogram"/>

#### `glcall(glc.useprogram [, PROGID])`

[`glUseProgram`]: http://www.opengl.org/sdk/docs/man2/xhtml/glUseProgram.xml

If `PROGID` is passed, [enables][`glUseProgram`] usage of that program previously
created with `glc.newprogram`. Otherwise, disables any program for the
programmable processors and reverts to OpenGL fixed functionality for both
vertex and fragment processing.

**COLORMAP_DEF**: When enabling a program, if the fragment shader contains
a `cmap` uniform variable, it it set to the texture unit to which the texture
uploaded with [`glc.colormap`](#glc_colormap) binds.

<a name="glc_setuniform"/>

#### `glcall(glc.setuniform, UNIFORMHANDLE, VAL)`

[`glUniform`]: http://www.opengl.org/sdk/docs/man2/xhtml/glUniform.xml

[Sets][`glUniform`] the value of a uniform variable in the currently active
program.

* `UNIFORMHANDLE`: The Gleemex handle denoting a uniform variable as returned
  as one of the keys of the `UNIFORMS` mapping from
  [`glc.newprogram`](#glc_newprogram).

* `VAL`: A value consistent with the type and size of the corresponding uniform
  variable.

    - For `GL_FLOAT`-typed uniforms, values of M type `single` are expected. For
      uniforms of type `GL_BOOL` or `GL_INT`, values of M type `int32` should be
      passed.

    - For scalar uniforms, `VAL` should contain as many elements as suggested by
      the _base type_: a single element for `GL_{BOOL,FLOAT,INT}` and 2, 3 or 4
      for `GL_*_VEC{2,3,4}` (respectively).

    - For array uniforms, `VAL` should have length equal to the length of the
       array, times the base element count as described with the previous point.


### GL2PS commands and functions

[GL2PS]: http://www.geuz.org/gl2ps

[GL2PS] is a library that can output geometry passed to OpenGL in
various vector formats. Gleemex provides an interface to GL2PS if it was
compiled with GL2PS support (see `mkglcall`).

The constants needed by the interface reside in the `GL` struct under names
like `GL.PS_TIGHT_BOUNDING_BOX`. That is, they're like the corresponding
`GL2PS_*` C defines with the `2` omitted.

An example of GL2PS usage is given by the `simpleplot` application packaged
together with Gleemex.

#### `HAVE = glc_have_beginpage()`

Return whether Gleemex was compiled with GL2PS support, that is, if the
`glc.beginpage` command is available.

#### `STATUS = glcall(glc.beginpage, FORMAT, SORTMODE, OPTIONS, BUFSIZE, FILENAME)`

[`gl2psBeginPage`]: http://www.geuz.org/gl2ps/#sec:gl2psBeginPage

[Begin][`gl2psBeginPage`] collecting OpenGL drawing commands for eventual output
to file `FILENAME` and issue a [redisplay](#glc_redisplay) request to
FreeGLUT. The collection will last until the ensuing display callback returns,
after which a call to `gl2psEndPage()` is issued.  Thus, whether it succeeded
only becomes known afterwards.

* `FORMAT`: one of `GL.PS_PS`, `GL.PS_EPS`, `GL.PS_PDF` or `GL.PS_SVG`. The two
  LaTeX formats are not (yet) available.

* `SORTMODE`: one of `GL.PS_NO_SORT`, `GL.PS_SIMPLE_SORT` or `GL.PS_BSP_SORT`.

* `OPTIONS`: a bitwise-OR of individual `GL.PS_*` [plot option bits][`gl2psBeginPage`],
  except that the following ones are not available:
  `PS_SILENT`, `PS_NO_TEXT`, `PS_NO_PIXMAP`; and `PS_USE_CURRENT_VIEWPORT`
  which is automatically set by Gleemex. If no options are to be passed,
  `GL.PS_NONE` should be used.

* `BUFSIZE`: the size of the feedback buffer

* `FILENAME`: the name of the file where the output is to be written

* `STATUS`: an indication of success or failure.
    - 0: starting the geometry collection was successful
    - 1: failed to open a write-only file named `FILENAME`
    - 2: `gl2psBeginPage()` returned `GL2PS_ERROR`. This could mean that the width
         or height of the [current viewport](#glc_viewport) is zero.

As noted above, the knowledge of ultimate success only becomes available after
the return of the subsequent display callback. The following form of
`glc.beginpage` allows one to examine this status then.

#### `ENDSTATUS = glcall(glc.beginpage)`
(Note that there is **no** separate "`glc.endpage`" command.)

Retrieves the return value of the last [`gl2psEndPage()`][`gl2psBeginPage`]
call as one of the GL2PS status constants: `GL.PS_SUCCESS`, `GL.PS_ERROR`,
`GL.PS_NO_FEEDBACK`, `GL.PS_OVERFLOW` or `GL.PS_UNINITIALIZED`.

Currently, when `glcall(glc.beginpage, ...)` returned 2 for `STATUS` or this
form's `ENDSTATUS` is anything other than `GL.PS_SUCCESS`, the file `FILENAME`
remains on the system and may be empty.

#### `STR = glc_endpage_errmsg(ENDSTATUS)`

Returns a string for the status code of `gl2psEndPage()`. It is the lowercased
version of the corresponding `GL.PS_*` error code define, for example
`'no_feedback'` for `GL.PS_NO_FEEDBACK`.

#### `[STATUS, FILENAME] = glc_beginpage_ext(FORMAT, SORTMODE, OPTIONS, BUFSIZE, FILEPREFIX)`

Calls `glcall(glc.beginpage, ...)` with the provided arguments, expect that
`FILEPREFIX` automatically gets an extension appended based on the value of
`FORMAT` and the `GL.PS_COMPRESS` bit of `OPTIONS`.

* `STATUS`: one of the `glcall(glc.beginpage, ...)` return values (0, 1 or 2), or
   -1 if Gleemex was compiled without GL2PS support.
* `FILENAME`: In case of success (`STATUS` is 0), `FILEPREFIX` with an appended
  extension.


Auxiliary functions
-------------------

Gleemex provides a number of convenience functions built on top of the basic
`glcall` routines as well as helper routines designed to aid in application
development.

### Convenience functions

<a name="glc_setup2d"/>

#### `glc_setup2d(WIDTH, HEIGHT [, ZRANGE])`

Sets up the viewport and projection matrix suitable for 2D drawing in a window
of size `WIDTH` and `HEIGHT`, and resets the modelview matrix to the identity
matrix.  In the so constructed view, position (1, 1) denotes the center of the
leftmost and bottommost pixel.

NOTE: `glc_setup2d` achieves the above-mentioned convention by calling
[`glc.setmatrix`](#glc_setmatrix) / `GL.PROJECTION` with a 6-vector, where the
first four values are 0, `WIDTH`, 0 and `HEIGHT`, to all of which 0.5 is added.
The last two values are those given by `ZRANGE`, or `[-1 1]` by default.

#### `glc_axes_setup(VIEWPORT_XYWH, PROJECTION [, ADDITIONAL_BITS])` -> `true`

Sets up an "axes" as a rectangular region of the current window given by
`VIEWPORT_XYWH`, which should be a 4-vector permissible to
[`glc.viewport`](#glc_viewport). Drawing commands between a call to
`glc_axes_setup` and a closing `glc_axes_finish` will never draw outside this
region. The `PROJECTION` argument can be anything permissible to
[`glc.setmatrix`](#glc_setmatrix); the modelview matrix is reset to the
identity matrix.

Always returns `true`.

NOTE: `glc_axes_setup` pushes the following matrices and server-side attribute
bits: `GL.PROJECTION`, `GL.MODELVIEW`,
`GL.VIEWPORT_BIT+GL.SCISSOR_BIT+GL.ENABLE_BIT`. Additional attribute bits may
be pushed by passing `ADDITIONAL_BITS`.

#### `glc_axes_finish()`

Restores the matrices and server attributes pushed by `glc_axes_setup`.

#### `CMAPTEXNAME = glc_colormap(CMAP_OR_FUNC [, CMAPTEXNAME])`

TODO

#### `glc_draw_colorbar(RECT, BOUNDS, LABEL, CMAPTEXNAME [, FORMAT [, NUMTICKS [, TEXTHEIGHT]]])`

TODO

### Non-graphical helper functions

#### `glc_checkextract()`

TODO

#### `[VAL, OK] = glc_getenv(VARNAME)`

Returns the value `VAL` of the variable named `VARNAME` in the base workspace if
one exists. `OK` is *true* then.

If no variable named `VARNAME` exists, `OK` is *false* and the empty array `[]`
is returned for `VAL`.


Creating applications
---------------------

Using the `GLCApplicationData` handle class provided by Gleemex, it is possible
to create Gleemex applications that may be present in multiple instances at the
same time. For a particular application, the user creates a class derived from
`GLCApplicationData` and populates it with the desired application-specific
functionality.

In the following, assume that `MyApplicationData` is a classdef class derived
from `GLCApplicationData`.

#### `ad = MyApplicationData()`

Creates an object of `MyApplicationData`; each object represents an instance of
a particular application, similar to the relation of a process to one
particular program at the OS level.

The `GLCApplicationData` constructor initializes some of its members to default
values:

* `ad.mxy = [1 1]  % current mouse position`
* `ad.mbutton = 0  % currently pressed mouse buttons`
* `ad.wh = [800 600]  % width and height of the app window`

CAUTION: These members may become private in the future.

The initial height and width may be overridden by passing a `WH` pair to the
`GLCApplicationData` constructor in that of the derived class, like this:

~~~matlab
        function self = MyApplicationData(wh)
            self@GLCApplicationData(wh);
            % additional initialization goes here...
        end
~~~


### Static methods and functions

#### `GLCApplicationData.register(winid, obj)`

Registers an object `obj` of a class derived from `GLCApplicationData` with the
window identifier `winid`, as returned by
[`glc.newwindow`](#glc_newwindow). This association is not cleared until
another call of `GLCApplicationData.register` with the same `winid` -- for
example, it persists after closing the app's window.

#### `[objs, winids] = GLCApplicationData.getAll(className, activep)`

Retrieves all objects and window identifiers of the class named `className` --
for example, `'MyApplicationData'` -- previously registered with
`GLCApplicationData.register`.

* `activep`: Should be a scalar logical telling whether active, i.e. currently
  open windows should be considered. If false, those that are not active are
  returned

* `objs`: a cell array vector having as many elements as there are (active or
  passive, depending on `activep`) windows of class `className`.

* `winids`: in the case of `activep` being *true*, a vector of the same length,
  containing the respective Gleemex window identifiers

#### `nset = GLCApplicationData.setCallbacks(prefix [, suffix])`

Registers callbacks for the current window. For each of the strings
`'display'`, `'reshape'`, `'keyboard'`, `'mouse'`, `'motion'` and `'position'`,
a function name is constructed by concatenating it in between `prefix` (which
must be non-empty) and `suffix` (which is empty by default). Then, if a file
existence check for this name suffixed with `'.m'` yields a positive result,
the corresponding callback function is [registered](#glc_setcallback).

The returned value `nset` is the number of successfully registered callbacks.

#### `obj = glc_appdata()`

Retrieves the object `obj` previously registered with
`GLCApplicationData.register` for the current window, as obtained with
`glcall(glc.get, GL.WINDOW_ID)`. If no application data was registered with the
current window, the behavior is undefined.


### Instance methods

#### `ad.updateWindowPos(w, h [, setup2d])`

Updates the window position with the new width `w` and height `h`, as obtained
from the [reshape callback](#cb_reshape). If `setup2d` is passed, it must be a
scalar logical; in case it is true, [`glc_setup2d`](#glc_setup2d)`(w, h)` is
also called.

#### `ad.updateMousePos(buttons, x, y)`

Updates the current mouse pointer position with the new mouse button state
`buttons` and coordinates (`x`, `y`), as obtained from the [mouse motion
callback](#cb_motion). Internally, `x` is stored as obtained, but the stored y
is the difference of the current window width and the passed `y`.

#### `winid = ad.attachNewWindow(pos, name [, opts])`

Creates a [new window](#glc_newwindow) and stores the Gleemex and FreeGLUT
window indices in private fields. The former one is returned in `winid`.
The optional input argument `opts` is passed to the `glc.newwindow` call.

#### `[ok, oldwinid] = ad.makeCurrent()`

Tries to make the window of this `GLCApplicationData` object current. A call to
the `attachNewWindow` method must have previously taken place, otherwise an
error is issued. Before the switch, the window index of the old current window
is backed up in a private field, for later restoration by `restoreOldWindow`.

* `ok`: a logical of whether making the application window succeeded. Failure
  can for example occur because the window has been destroyed.

* `oldwinid`: the Gleemex window identifier of the window that was current
  prior to the call to `ad.makeCurrent`.

#### `ok = ad.restoreOldWindow()`

Changes the current window to that which was current before the last call to
the `makeCurrent` method. The output argument `ok` tells whether that
succeeded.

#### `ok = ad.postRedisplay()`

Calls [`glc.redisplay`](#glc_redisplay) for this `ad`'s window (registered with
`attachNewWindow`), but wrapping it in backup and restoration of the current
one.  The output argument `ok` is from `ad.makeCurrent()` and tells whether
this operation was performed.
