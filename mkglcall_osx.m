#mkoctfile -v --mex -I/usr/X11/include -L/usr/X11/lib -Wall -Wextra -framework OpenGL -lXext -lX11 -lXrandr -lXxf86vm -framework GLUT -lGL -lGLEW glcall.c /usr/local/lib/libglut.a
#mkoctfile -v --mex -I/usr/X11/include -L/usr/X11/lib -Wall -Wextra -lXext -lX11 -lXrandr -lXxf86vm -lGL -L/opt/local/lib -lGLEW -lglut -lXi -lGLU glcall.c /opt/local/lib/libglut.a
#mkoctfile -v --mex -Wall -Wextra -lXext -lX11 -lXrandr -lXxf86vm -lGL -L/opt/local/lib -lGLEW -lglut -lXi -lGLU glcall.c /opt/local/lib/libglut.a
# FreeGLUT, GLEW from MacPorts
mkoctfile -v --mex -Wall -Wextra -lX11 -lXrandr -lXxf86vm -lGL -L/opt/local/lib -lGLEW -lglut -lGLU glcall.c
