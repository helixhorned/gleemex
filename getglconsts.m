% GL = GETGLCONSTS()
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
%  * GLUT_{LEFT,MIDDLE,RIGHT}_BUTTON  (1<<X), stripped of the GLUT_ prefix
%
function GL = getglconsts()

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

%% Polygons
GL.POLYGON_SMOOTH = int32(2881);  % 0x0B41

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


%% GLUT stuff follows

% mouse buttons, 1<<X
GL.LEFT_BUTTON = 1;
GL.MIDDLE_BUTTON = 2;
GL.RIGHT_BUTTON = 4;


%
% GLUT API macro definitions -- the special key codes:
%
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
GL.KEY_HOME = 65536+106;
GL.KEY_END = 65536+107;
GL.KEY_INSERT = 65536+108;
