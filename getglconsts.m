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
function glconsts = getglconsts()

glconsts = struct();

%% Primitives
glconsts.POINTS = uint32(0);
glconsts.LINES = uint32(1);
glconsts.LINE_LOOP = uint32(2);
glconsts.LINE_STRIP = uint32(3);
glconsts.TRIANGLES = uint32(4);
glconsts.TRIANGLE_STRIP = uint32(5);
glconsts.TRIANGLE_FAN = uint32(6);
glconsts.QUADS = uint32(7);
glconsts.QUAD_STRIP = uint32(8);
glconsts.POLYGON = uint32(9);

%% Matrix Modes
glconsts.MODELVIEW = uint32(5888);  % 0x1700
glconsts.PROJECTION = uint32(5889);  % 0x1701
glconsts.TEXTURE = uint32(5890);  % 0x1702

%% tests
glconsts.SCISSOR_TEST = int32(3089);  % 0x0C11
glconsts.DEPTH_TEST = int32(2929);  % 0x0B71

%% Blending
glconsts.BLEND = int32(3042);  % 0x0BE2

%% Points
glconsts.POINT_SMOOTH = int32(2832);  % 0x0B10
glconsts.POINT_SIZE	= int32(2833);  % 0x0B11

%% Lines
glconsts.LINE_SMOOTH = int32(2848);  % 0x0B20

%% glPush/PopAttrib bits
glconsts.CURRENT_BIT = uint32(1);
glconsts.POINT_BIT = uint32(2);
glconsts.LINE_BIT = uint32(4);
glconsts.POLYGON_BIT = uint32(8);
glconsts.POLYGON_STIPPLE_BIT = uint32(16);
glconsts.PIXEL_MODE_BIT = uint32(32);
glconsts.LIGHTING_BIT = uint32(64);
glconsts.FOG_BIT = uint32(128);
glconsts.DEPTH_BUFFER_BIT = uint32(256);
glconsts.ACCUM_BUFFER_BIT = uint32(512);
glconsts.STENCIL_BUFFER_BIT = uint32(1024);
glconsts.VIEWPORT_BIT = uint32(2048);
glconsts.TRANSFORM_BIT = uint32(4096);
glconsts.ENABLE_BIT = uint32(8192);
glconsts.COLOR_BUFFER_BIT = uint32(16384);
glconsts.HINT_BIT = uint32(32768);
glconsts.EVAL_BIT = uint32(65536);
glconsts.LIST_BIT = uint32(131072);
glconsts.TEXTURE_BIT = uint32(262144);
glconsts.SCISSOR_BIT = uint32(524288);
glconsts.ALL_ATTRIB_BITS = uint32(1048575);  % 0xFFFFF
