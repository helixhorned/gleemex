% GL = GETGLCONSTS()
%  Returns certain OpenGL constants (stripped of the GL_ prefix) in the struct
%  GL so they can be used like e.g. "GL.POINTS" for functions that expect them.
%
% Currently, the following sets of constants are returned:
%
%  * Primitives (GL_POINTS .. GL_POLYGON)
%  * The matrix modes GL_MODELVIEW and GL_PROJECTION
%  * the constants GL_SCISSOR_TEST and GL_DEPTH_TEST
%
function glconsts = getglconsts()

glconsts = struct();

% Primitives
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

% Matrix Modes
glconsts.MODELVIEW = uint32(5888);  % 0x1700
glconsts.PROJECTION = uint32(5889);  % 0x1701
glconsts.TEXTURE = uint32(5890);  % 0x1702

% tests
glconsts.SCISSOR_TEST = int32(3089);  % 0x0C11
glconsts.DEPTH_TEST = int32(2929);  % 0x0B71
