
#include <stdio.h>
#include <stdlib.h>

//#include <GL/gl.h>
#include <GL/freeglut.h>

//////////
#include "mex.h"
#ifndef HAVE_OCTAVE
# include "matrix.h"
#endif

#ifdef DONT_HAVE_MWSIZE
# define mwSize unsigned long
#endif

//////////

// 1 lhs, 0 rhs
#define OUT_GLCSTRUCT (plhs[0])

// > 0 rhs
#define IN_COMMAND (prhs[0])

// init
#define NEWWIN_IN_POS (prhs[1])
#define NEWWIN_IN_EXTENT (prhs[2])
#define NEWWIN_IN_NAME (prhs[3])
#define NEWWIN_OUT_WINID (plhs[0])

// draw
#define DRAW_IN_PRIMITIVETYPE (prhs[1])
#define DRAW_IN_VERTEXDATA (prhs[2])
#define DRAW_IN_INDICES (prhs[3])


enum glcalls_
{
    GLC_NEWWINDOW,
    GLC_DRAW,
    GLC_ENTERMAINLOOP,
    NUM_GLCALLS,  // must be last
};

const char *glcall_names[] =
{
    "newwindow",
    "draw",
    "entermainloop",
};

////////// UTIL //////////
static char errstr[128];

#define GLC_MEX_ERROR(Text, ...) do {           \
        sprintf(errstr, Text, ## __VA_ARGS__);  \
        mexErrMsgTxt(errstr);                  \
    } while (0)

// XXX: make this more useful by making it possible to specify combinations
//      of data type, {vertex, matrix, scalar} requirements, etc...
void verifyparam(const mxArray *ar, const char *arname, const char *typedesc)
{
    switch (typedesc[0])
    {
    case 's':  // string
        if (!mxIsChar(ar))
            GLC_MEX_ERROR("%s must be a character array", arname);
        break;
    case 'd':  // double scalar
        if (mxGetNumberOfElements(ar) != 1 || !mxIsDouble(ar))
            GLC_MEX_ERROR("%s must be a double scalar", arname);
        break;
    case 'i':  // int32 scalar
        if (mxGetNumberOfElements(ar) != 1 || !mxIsInt32(ar))
            GLC_MEX_ERROR("%s must be a int32 scalar", arname);
        break;
    case 'v':  // vector
    case 'V':  // double vector
        if (mxGetNumberOfDimensions(ar) != 2)
            GLC_MEX_ERROR("%s must be a vector", arname);
        if (typedesc[0]!='V' || !mxIsDouble(ar))
            GLC_MEX_ERROR("%s must be of double type", arname);
        if (typedesc[1])
        {
            int numelts = atoi(&typedesc[1]);
            if (mxGetNumberOfElements(ar) != (unsigned)numelts)
                GLC_MEX_ERROR("%s must be of length %d", arname, numelts);
        }
        break;
    }
}

int util_dtoi(double d, double minnum, double maxnum, const char *arname)
{
    if (!(d >= minnum && d <= maxnum))
        GLC_MEX_ERROR("Passed invalid %s: must be between %d and %d", arname, (int)minnum, (int)maxnum);
    return (int)d;
}

////////// FUNCTIONS //////////

static void mousefunc(int button, int state, int x, int y)
{
    button = button;
    state = state;
    x = x;
    y = y;

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glColor3f(0.5f, 0.5f, 0.5f);  // TMP

    mexEvalString("glcall_display_example()");  // TMP: make cfg'able
    glutPostRedisplay();
}

static void display(void)
{
    glutSwapBuffers();
}

static void reshape(int w, int h)
{
	glViewport(0, 0, (GLsizei)w, (GLsizei)h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
//    if (ortho)
    {
        GLfloat aspect = (GLfloat)w / (GLfloat)h;
        glOrtho(-2.0*aspect, 2.0*aspect,  -2.0, 2.0,  0.0, 100.0);
    }
//    else
//        gluPerspective (60, (GLfloat)w / (GLfloat)h, 1.0, 200.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

//    wheight = h;
//    wwidth = w;
}

////////// MEX ENTRY POINT //////////

void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
    int cmd;

    static int inited = 0;

    if (nrhs==0)
    {
        if (nlhs==1)
        {
            // the user queries available glcalls
            mxArray *glcstruct = mxCreateStructMatrix(1,1, NUM_GLCALLS, glcall_names);
            mxArray *tmpar;
            int i;

            for (i=0; i<NUM_GLCALLS; i++)
            {
                tmpar = mxCreateNumericMatrix(1,1, mxINT32_CLASS, mxREAL);
                *(int *)mxGetData(tmpar) = i;
                mxSetFieldByNumber(glcstruct, 0, i, tmpar);
            }
            OUT_GLCSTRUCT = glcstruct;

            return;
        }
        else
            mexErrMsgTxt("Usage: GLCALLINFO_STRUCT = GLCALL()");
    }

    // nrhs > 0

    verifyparam(IN_COMMAND, "COMMAND", "i");
    cmd = *(int *)mxGetData(IN_COMMAND);

    if (cmd != GLC_NEWWINDOW && !inited)
        mexErrMsgTxt("Must call 'newwindow' subcommand to initialize GLUT before any other command!");

    //////////
    switch (cmd)
    {
    case GLC_NEWWINDOW:
    {
        double *posptr, *extentptr;

        int pos[2], extent[2];
        char windowname[80];

        if (nlhs != 1 || nrhs != 4)
            mexErrMsgTxt("Usage: WINID = GLCALL(glc.newwindow, POS, EXTENT, WINDOWNAME)");
        verifyparam(NEWWIN_IN_POS, "POS", "V2");
        verifyparam(NEWWIN_IN_EXTENT, "EXTENT", "V2");
        verifyparam(NEWWIN_IN_NAME, "WINDOWNAME", "s");

        posptr = mxGetPr(NEWWIN_IN_POS);
        extentptr = mxGetPr(NEWWIN_IN_EXTENT);

        pos[0] = util_dtoi(posptr[0], 0, 1680, "POS(1)");
        pos[1] = util_dtoi(posptr[1], 0, 1050, "POS(2)");

        extent[0] = util_dtoi(extentptr[0], 320, 1680, "EXTENT(1)");
        extent[1] = util_dtoi(extentptr[1], 200, 1050, "EXTENT(2)");

        mxGetString(NEWWIN_IN_NAME, windowname, sizeof(windowname)-1);

        // init!
        if (!inited)
        {
            char *argvdummy[1] = {"QWEASDproggy"};
            int argcdummy = 1;

            glutInit(&argcdummy, argvdummy);  // XXX
            glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_CONTINUE_EXECUTION);

            inited = 1;
        }
        else
            mexErrMsgTxt("GLCALL: newwindow: multiple windows not implemented!");

        glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH | GLUT_RGBA);
        glutInitWindowPosition(pos[0], pos[1]);
        glutInitWindowSize(extent[0], extent[1]);

        {
            int winid = glutCreateWindow(windowname);
            mxArray *outar = mxCreateNumericMatrix(1,1, mxINT32_CLASS, mxREAL);
            *(int *)mxGetData(outar) = winid;

            NEWWIN_OUT_WINID = outar;
        }
    }
    return;

    case GLC_DRAW:
    {
        unsigned int primitivetype;
        int numdims, numtotalverts, numverts, i;

        unsigned int *indices;

        if (nlhs != 0 || (nrhs != 3 && nrhs != 4))
            mexErrMsgTxt("Usage: GLCALL(glc.draw, GL.<PRIMITIVE_TYPE>, VERTEXDATA [, INDICES])");

        if (!mxIsUint32(DRAW_IN_PRIMITIVETYPE))
            mexErrMsgTxt("GLCALL: draw: PRIMITIVE_TYPE must be of uint32 type");
        primitivetype = *(int *)mxGetData(DRAW_IN_PRIMITIVETYPE);

        if (!(/*primitivetype >= GL_POINTS &&*/ primitivetype <= GL_POLYGON))
            mexErrMsgTxt("GLCALL: draw: invalid GL primitive type");

        if (!mxIsDouble(DRAW_IN_VERTEXDATA))
            mexErrMsgTxt("GLCALL: draw: VERTEXDATA must be of double type");

        if (nrhs > 3)
        {
            if (!mxIsUint32(DRAW_IN_INDICES))
                mexErrMsgTxt("GLCALL: draw: VERTEXDATA must be of uint32 type");
        }

        numdims = mxGetM(DRAW_IN_VERTEXDATA);
        numtotalverts = mxGetN(DRAW_IN_VERTEXDATA);

        if (numdims==0 || numtotalverts==0)
            return;  // draw nothing   XXX: indices array goes unchecked...

        if (!(numdims >=2 && numdims <= 4))
            mexErrMsgTxt("GLCALL: draw: VERTEXDATA must have between 2 and 4 rows (number of coordinates)");

        if (nrhs > 3)
        {
            numverts = mxGetNumberOfElements(DRAW_IN_INDICES);  // XXX: must be vector?
            indices = mxGetData(DRAW_IN_INDICES);

            // bounds check!
            for (i=0; i<numverts; i++)
                if (indices[i] >= (unsigned)numverts)
                    mexErrMsgTxt("GLCALL: draw: INDICES must contain values between 0 and size(VERTEXDATA,2)-1");
        }
        else
        {
            indices = mxMalloc(numtotalverts * sizeof(indices[0]));
            for (i=0; i<numtotalverts; i++)
                indices[i] = i;
            numverts = numtotalverts;
        }

        glEnableClientState(GL_VERTEX_ARRAY);

        glVertexPointer(numdims, GL_DOUBLE, 0, mxGetPr(DRAW_IN_VERTEXDATA));
        glDrawElements(primitivetype, numverts, GL_UNSIGNED_INT, indices);

        if (nrhs <= 3)
            mxFree(indices);
    }
    return;

    case GLC_ENTERMAINLOOP:
    {
        static int numentered = 0;

        if (nlhs != 0 || nrhs != 1)
            mexErrMsgTxt("Usage: GLCALL(glc.entermainloop)");

        if (numentered)
            mexErrMsgTxt("GLCALL: entermainloop: entered recursively!");
        numentered++;

        glutMouseFunc(mousefunc);
        glutDisplayFunc(display);
        glutReshapeFunc(reshape);

        glutMainLoop();

        numentered--;
        inited = 0;
    }
    return;
    }
}
