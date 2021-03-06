% GL = GLCONSTANTS()
%  Returns certain OpenGL constants (stripped of the GL_ prefix) in the struct
%  GL so they can be used like e.g. "GL.POINTS" for functions that expect them.
%
% Currently, the following sets of constants are returned:
%
%  * Primitives (GL_POINTS .. GL_POLYGON)
%  * The matrix modes GL_MODELVIEW and GL_PROJECTION
%  * the constants GL_SCISSOR_TEST and GL_DEPTH_TEST
%  * all glPush/PopAttrib bits (most have no effect, though)
%
%  * the GLUT special keys (w/o freeglut ones), stripped of the GLUT_ prefix
%  * GLUT_BUTTON_{LEFT,MIDDLE,RIGHT}  (1<<X), stripped of the GLUT_ prefix
%
%  * a lot more (UNDOCUMENTED), see the source of this function.
%
function GL = glconstants()

GL = struct();

%% Primitives
GL.POINTS = uint32(0);
GL.LINES = uint32(1);
GL.LINE_LOOP = uint32(2);
GL.LINE_STRIP = uint32(3);
GL.TRIANGLES = uint32(4);
GL.TRIANGLE_STRIP = uint32(5);
GL.TRIANGLE_FAN = uint32(6);
GL.QUADS = uint32(7);
GL.QUAD_STRIP = uint32(8);
GL.POLYGON = uint32(9);
GL.DRAW_AS_LINE = @(primitivetype)primitivetype+16;

%% Matrix Modes
GL.MODELVIEW = uint32(5888);  % 0x1700
GL.PROJECTION = uint32(5889);  % 0x1701
GL.TEXTURE = uint32(5890);  % 0x1702

%% tests
GL.SCISSOR_TEST = int32(3089);  % 0x0C11
GL.DEPTH_TEST = int32(2929);  % 0x0B71

%% Texture mapping
%GL.TEXTURE_1D = int32(3552);  % 0x0DE0

%% Blending
GL.BLEND = int32(3042);  % 0x0BE2

%% Points
GL.POINT_SMOOTH = int32(2832);  % 0x0B10
GL.POINT_SIZE	= int32(2833);  % 0x0B11

%% Lines
GL.LINE_SMOOTH = int32(2848);  % 0x0B20
GL.LINE_STIPPLE = int32(2852);  % 0x0B24
GL.LINE_STIPPLE_PATTERN = int32(2853);  % 0x0B25
GL.LINE_WIDTH = int32(2849);  % 0x0B21

%% Polygons
GL.POLYGON_SMOOTH = int32(2881);  % 0x0B41
GL.POLYGON_OFFSET_POINT = int32(10753);  % 0x2A01
GL.POLYGON_OFFSET_LINE = int32(10754);  % 0x2A02
GL.POLYGON_OFFSET_FILL = int32(32823);  % 0x8037

%% glPush/PopAttrib bits
GL.CURRENT_BIT = uint32(1);
GL.POINT_BIT = uint32(2);
GL.LINE_BIT = uint32(4);
GL.POLYGON_BIT = uint32(8);
GL.POLYGON_STIPPLE_BIT = uint32(16);
GL.PIXEL_MODE_BIT = uint32(32);
GL.LIGHTING_BIT = uint32(64);
GL.FOG_BIT = uint32(128);
GL.DEPTH_BUFFER_BIT = uint32(256);
GL.ACCUM_BUFFER_BIT = uint32(512);
GL.STENCIL_BUFFER_BIT = uint32(1024);
GL.VIEWPORT_BIT = uint32(2048);
GL.TRANSFORM_BIT = uint32(4096);
GL.ENABLE_BIT = uint32(8192);
GL.COLOR_BUFFER_BIT = uint32(16384);
GL.HINT_BIT = uint32(32768);
GL.EVAL_BIT = uint32(65536);
GL.LIST_BIT = uint32(131072);
GL.TEXTURE_BIT = uint32(262144);
GL.SCISSOR_BIT = uint32(524288);
GL.ALL_ATTRIB_BITS = uint32(1048575);  % 0xFFFFF

%% texture filter types
GL.NEAREST = int32(9728);  % 0x2600
% GL.LINEAR = 0x2601 is below

%% blend equations
GL.BLEND_EQUATION = int32(32777);  % 0x8009
GL.MIN = int32(32775);  % 0x8007
GL.MAX = int32(32776);  % 0x8008
GL.FUNC_ADD = int32(32774);  % 0x8006
GL.FUNC_SUBTRACT = int32(32778);  % 0x800A
GL.FUNC_REVERSE_SUBTRACT = int32(32779);  % 0x800B

%% Fog
GL.FOG = int32(2912);  % 0x0B60
GL.LINEAR = int32(9729);  % 0x2601
GL.EXP = int32(2048);  % 0x0800
GL.EXP2 = int32(2049);  % 0x0801

%% Depth buffer
GL.NEVER = int32(512);  % 0x0200
GL.LESS = int32(513);  % 0x0201
GL.EQUAL = int32(514);  % 0x0202
GL.LEQUAL = int32(515);  % 0x0203
GL.GREATER = int32(516);  % 0x0204
GL.NOTEQUAL = int32(517);  % 0x0205
GL.GEQUAL = int32(518);  % 0x0206
GL.ALWAYS = int32(519);  % 0x0207
%GL.DEPTH_TEST = int32(2929);  % 0x0B71
%GL.DEPTH_BITS = int32(3414);  % 0x0D56
%GL.DEPTH_CLEAR_VALUE = int32(2931);  % 0x0B73
GL.DEPTH_FUNC = int32(2932);  % 0x0B74
%GL.DEPTH_RANGE = int32(2928);  % 0x0B70
GL.DEPTH_WRITEMASK = int32(2930);  % 0x0B72
%GL.DEPTH_COMPONENT = int32(6402);  % 0x1902


%% Get tokens
GL.MODELVIEW_MATRIX = int32(2982);  % 0x0BA6
GL.PROJECTION_MATRIX = int32(2983);  % 0x0BA7


%% GLC specific

% GET tokens
% KEEPINSYNC GLC_GET_TOKENS in glcall.c.
GL.WINDOW_ID = int32(-100);
GL.WINDOW_SIZE = int32(-101);
GL.MOUSE_POS = int32(-102);  % SET only
GL.MENU_ENABLE = int32(-103);  % SET only
GL.WINDOW_POS = int32(-104);
GL.POLYGON_OFFSET = int32(-105);  % SET only


%% GLUT stuff follows

% modifiers, GLUT_ACTIVE_*
GL.MOD_SHIFT = 1;
GL.MOD_CTRL = 2;
GL.MOD_ALT = 4;
GL.MODS_ALL = 7;

% mouse buttons, 1<<X
GL.BUTTON_LEFT = 1;
GL.BUTTON_MIDDLE = 2;
GL.BUTTON_RIGHT = 4;
GL.MWHEEL_UP = 8;
GL.MWHEEL_DOWN = 16;
GL.BUTTONS_ALL = 7;

% GLUT API macro definitions -- the special key codes:
GL.KEY_F1 = 65536+1;
GL.KEY_F2 = 65536+2;
GL.KEY_F3 = 65536+3;
GL.KEY_F4 = 65536+4;
GL.KEY_F5 = 65536+5;
GL.KEY_F6 = 65536+6;
GL.KEY_F7 = 65536+7;
GL.KEY_F8 = 65536+8;
GL.KEY_F9 = 65536+9;
GL.KEY_F10 = 65536+10;
GL.KEY_F11 = 65536+11;
GL.KEY_F12 = 65536+12;
GL.KEY_LEFT = 65536+100;
GL.KEY_UP = 65536+101;
GL.KEY_RIGHT = 65536+102;
GL.KEY_DOWN = 65536+103;
GL.KEY_PAGE_UP = 65536+104;
GL.KEY_PAGE_DOWN = 65536+105;
% Alternative names for PAGE_{UP,DOWN}:
GL.KEY_PGUP = 65536+104;
GL.KEY_PGDN = 65536+105;
GL.KEY_HOME = 65536+106;
GL.KEY_END = 65536+107;
GL.KEY_INSERT = 65536+108;

% Gleemex key names
GL.KEY_BACKSPACE = 8;
GL.KEY_TAB = 9;
GL.KEY_ENTER = 13;
GL.KEY_ESCAPE = 27;
GL.KEY_SPACE = 32;
GL.KEY_DELETE = 127;

% Should be really in GLAux or something?
GL.TYPEFUNC_TEXTURE = @uint32;
GL.INVALID_TEXTURE = uint32(0);

% Additional bits to glc.draw's PRIMITIVETYPE
GL.AS_LINE = uint32(16);
GL.AS_LINE_FRONT = uint32(16 + 32);
GL.AS_LINE_BACK = uint32(32 + 64);
GL.AS_LINE_BITS = uint32(16 + 32 + 64);

%% For gl2ps/glc.beginpage
% (defines from gl2ps.h)

% -- Output file formats
GL.PS_PS = int32(0);
GL.PS_EPS = int32(1);
%GL.PS_TEX = int32(2);
GL.PS_PDF = int32(3);
GL.PS_SVG = int32(4);
%GL.PS_PGF = int32(5);

% -- Sorting algorithms
GL.PS_NO_SORT = int32(1);
GL.PS_SIMPLE_SORT = int32(2);
GL.PS_BSP_SORT = int32(3);

% -- Message levels and error codes
% (in Gleemex, only for the result of gl2psEndPage())
GL.PS_SUCCESS = int32(0);
%GL.PS_INFO = int32(1);
%GL.PS_WARNING = int32(2);
GL.PS_ERROR = int32(3);
GL.PS_NO_FEEDBACK = int32(4);
GL.PS_OVERFLOW = int32(5);
GL.PS_UNINITIALIZED = int32(6);

% -- Options for gl2psBeginPage
GL.PS_NONE                 = int32(0);
GL.PS_DRAW_BACKGROUND      = int32(1);
GL.PS_SIMPLE_LINE_OFFSET   = int32(2);
%GL.PS_SILENT               = int32(4);
GL.PS_BEST_ROOT            = int32(8);
GL.PS_OCCLUSION_CULL       = int32(16);
%GL.PS_NO_TEXT              = int32(32);
GL.PS_LANDSCAPE            = int32(64);
GL.PS_NO_PS3_SHADING       = int32(128);
%GL.PS_NO_PIXMAP            = int32(256);
%GL.PS_USE_CURRENT_VIEWPORT = int32(512);
GL.PS_COMPRESS             = int32(1024);
GL.PS_NO_BLENDING          = int32(2048);
GL.PS_TIGHT_BOUNDING_BOX   = int32(4096);
