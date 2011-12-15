
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

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

// setmatrix
#define SETMATRIX_IN_MODE (prhs[1])
#define SETMATRIX_IN_MATRIX (prhs[2])


enum glcalls_
{
    GLC_NEWWINDOW = 0,
    GLC_DRAW,
    GLC_ENTERMAINLOOP,
    GLC_SETMATRIX,
    NUM_GLCALLS,  // must be last
};

const char *glcall_names[] =
{
    "newwindow",
    "draw",
    "entermainloop",
    "setmatrix",
};


////////// DATA //////////
static uint32_t wwidth, wheight;

////////// UTIL //////////
static char errstr[128];

#define GLC_MEX_ERROR(Text, ...) do {           \
        sprintf(errstr, Text, ## __VA_ARGS__);  \
        mexErrMsgTxt(errstr);                   \
    } while (0)

enum verifyparam_flags
{
    // 0: no requirement

    /* Classes (data types) */
    VP_CELL = 1,
    VP_STRUCT,
    VP_LOGICAL,
    VP_CHAR,
    VP_DOUBLE,  // 5
    VP_SINGLE,
    VP_INT8,
    VP_UINT8,
    VP_INT16,
    VP_UINT16,  // 10
    VP_INT32,
    VP_UINT32,
    VP_INT64,
    VP_UINT64,
    VP_RESERVED_,  // 15
    VP_CLASS_MASK = 0x0000000f,

    /* scalar/vector/matrix requirements */
    VP_SCALAR = 0x00000100,
    VP_VECTOR = 0x00000200,
    VP_MATRIX = 0x00000300,
    VP_SVM_MASK = 0x00000300,

    VP_VECLEN_SHIFT = 16,
    VP_VECLEN_MASK = 0x00ff0000,  // shift down 16 bits to get length
};

static mxClassID class_ids[] = {
    0,
    mxCELL_CLASS, mxSTRUCT_CLASS, mxLOGICAL_CLASS, mxCHAR_CLASS, mxDOUBLE_CLASS, mxSINGLE_CLASS,
    mxINT8_CLASS, mxUINT8_CLASS, mxINT16_CLASS, mxUINT16_CLASS, mxINT32_CLASS, mxUINT32_CLASS,
    mxINT64_CLASS, mxUINT64_CLASS,
};
static const char *class_names[] = {
    "",
    "cell", "struct", "logical", "char", "double", "single",
    "int8", "uint8", "int16", "uint16", "int32", "uint32",
    "int64", "uint64",
};

void verifyparam(const mxArray *ar, const char *arname, uint32_t vpflags)
{
    uint32_t vpclassidx = vpflags&VP_CLASS_MASK;

    if (vpclassidx && class_ids[vpclassidx] != mxGetClassID(ar))
    {
        GLC_MEX_ERROR("%s must have class %s", arname, class_names[vpclassidx]);
    }

    switch (vpflags & VP_SVM_MASK)
    {
    case VP_SCALAR:
        if (mxGetNumberOfElements(ar) != 1)
            GLC_MEX_ERROR("%s must be scalar", arname);
        break;

    case VP_VECTOR:
    {
        int bad = 1;
        mwSize sz[2];

        if (mxGetNumberOfDimensions(ar) == 2)
        {
            sz[0] = mxGetM(ar);
            sz[1] = mxGetN(ar);

            if (sz[0]==1 || sz[1]==1)
            {
                if (vpflags&VP_VECLEN_MASK)
                {
                    uint32_t reqdveclen = (vpflags&VP_VECLEN_MASK)>>VP_VECLEN_SHIFT;

                    if (mxGetNumberOfElements(ar) == reqdveclen)
                        bad = 0;
                }
                else
                {
                    bad = 0;
                }
            }
        }

        if (bad)
            GLC_MEX_ERROR("%s must be a vector", arname);
        break;
    }

    case VP_MATRIX:
        if (mxGetNumberOfDimensions(ar) != 2)
            GLC_MEX_ERROR("%s must be a matrix", arname);
        break;
    }
}

int util_dtoi(double d, double minnum, double maxnum, const char *arname)
{
    if (!(d >= minnum && d <= maxnum))
        GLC_MEX_ERROR("%s must be between %d and %d", arname, (int)minnum, (int)maxnum);
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
    wwidth = w;
    wheight = h;

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
    int32_t cmd;

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
            mexErrMsgTxt("Usage: GLCALLINFO_STRUCT = GLCALL(), query available subcommands.");
    }

    // nrhs > 0

    verifyparam(IN_COMMAND, "COMMAND", VP_SCALAR|VP_INT32);
    cmd = *(int32_t *)mxGetData(IN_COMMAND);

    if (cmd != GLC_NEWWINDOW && !inited)
        mexErrMsgTxt("GLCALL: Must call 'newwindow' subcommand to initialize GLUT before any other command!");

    //////////
    switch (cmd)
    {
    case GLC_NEWWINDOW:
    {
        double *posptr, *extentptr;

        int pos[2], extent[2], winid;
        char windowname[80];

        if (nlhs > 1 || nrhs != 4)
            mexErrMsgTxt("Usage: [WINID =] GLCALL(glc.newwindow, POS, EXTENT, WINDOWNAME), create new window.");

        verifyparam(NEWWIN_IN_POS, "POS", VP_VECTOR|VP_DOUBLE|(2<<VP_VECLEN_SHIFT));
        verifyparam(NEWWIN_IN_EXTENT, "EXTENT", VP_VECTOR|VP_DOUBLE|(2<<VP_VECLEN_SHIFT));
        verifyparam(NEWWIN_IN_NAME, "WINDOWNAME", VP_VECTOR|VP_CHAR);

        posptr = mxGetPr(NEWWIN_IN_POS);
        extentptr = mxGetPr(NEWWIN_IN_EXTENT);

        pos[0] = util_dtoi(posptr[0], 0, 1680, "GLCALL: newwindow: POS(1)");
        pos[1] = util_dtoi(posptr[1], 0, 1050, "GLCALL: newwindow: POS(2)");

        extent[0] = util_dtoi(extentptr[0], 320, 1680, "GLCALL: newwindow: EXTENT(1)");
        extent[1] = util_dtoi(extentptr[1], 200, 1050, "GLCALL: newwindow: EXTENT(2)");

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

        winid = glutCreateWindow(windowname);

        if (nlhs > 0)
        {
            mxArray *outar = mxCreateNumericMatrix(1,1, mxINT32_CLASS, mxREAL);
            *(int *)mxGetData(outar) = winid;

            NEWWIN_OUT_WINID = outar;
        }
    }
    return;

    case GLC_DRAW:
    {
        // TODO: pass color array

        unsigned int primitivetype;
        mwSize i, numdims, numtotalverts, numverts;

        unsigned int *indices;

        if (nlhs != 0 || (nrhs != 3 && nrhs != 4))
            mexErrMsgTxt("Usage: GLCALL(glc.draw, GL.<PRIMITIVE_TYPE>, VERTEXDATA [, INDICES])");

        if (!mxIsUint32(DRAW_IN_PRIMITIVETYPE))
            mexErrMsgTxt("GLCALL: draw: PRIMITIVE_TYPE must be of uint32 type");
        primitivetype = *(int *)mxGetData(DRAW_IN_PRIMITIVETYPE);

        if (!(/*primitivetype >= GL_POINTS &&*/ primitivetype <= GL_POLYGON))
            mexErrMsgTxt("GLCALL: draw: invalid GL primitive type");

        verifyparam(DRAW_IN_VERTEXDATA, "GLCALL: draw: VERTEXDATA", VP_MATRIX|VP_DOUBLE);

        if (nrhs > 3)
            verifyparam(DRAW_IN_INDICES, "GLCALL: draw: INDICES", VP_VECTOR|VP_UINT32);

        numdims = mxGetM(DRAW_IN_VERTEXDATA);
        numtotalverts = mxGetN(DRAW_IN_VERTEXDATA);

        if (!(numdims >=2 && numdims <= 4))
            mexErrMsgTxt("GLCALL: draw: VERTEXDATA must have between 2 and 4 rows (number of coordinates)");

        if (nrhs > 3)
        {
            numverts = mxGetNumberOfElements(DRAW_IN_INDICES);
            indices = mxGetData(DRAW_IN_INDICES);

            if (numverts==0)
                return;

            // bounds check!
            for (i=0; i<numverts; i++)
                if (indices[i] >= (unsigned)numverts)
                    mexErrMsgTxt("GLCALL: draw: INDICES must contain values between 0 and size(VERTEXDATA,2)-1");
        }
        else
        {
            if (numtotalverts==0)
                return;

            indices = mxMalloc(numtotalverts * sizeof(indices[0]));
            for (i=0; i<numtotalverts; i++)
                indices[i] = i;
            numverts = numtotalverts;
        }

        // draw them at last!

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
            mexErrMsgTxt("Usage: GLCALL(glc.entermainloop), enter main loop.");

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

    case GLC_SETMATRIX:
    {
        uint32_t matrixmode;
        mwSize numel;

        static const char *usageText =
            "Usage: GLCALL(glc.setmatrix, GL.<MATRIX_MODE>, X), where X can be\n"
            "  * a 4x4 double matrix\n"
            "  * the empty matrix [], meaning 'load identity matrix', or\n"
            "  * in the case if GL.PROJECTION, a double vector [left right bottom top nearVal farVal]\n"
            "    which is passed to glOrtho(). No matrix manipulation is performed beforehand.";

        if (nlhs != 0 || nrhs != 3)
            mexErrMsgTxt(usageText);

        verifyparam(SETMATRIX_IN_MODE, "GLCALL: setmatrix: MATRIX_MODE", VP_SCALAR|VP_UINT32);
        matrixmode = *(uint32_t *)mxGetData(SETMATRIX_IN_MODE);

        if (matrixmode != GL_MODELVIEW && matrixmode != GL_PROJECTION)
            mexErrMsgTxt("GLCALL: setmatrix: MATRIX_MODE must be one of GL_MODELVIEW or GL_PROJECTION");

        if (!mxIsDouble(SETMATRIX_IN_MATRIX))
            mexErrMsgTxt("GLCALL: setmatrix: X must have class double");

        numel = mxGetNumberOfElements(SETMATRIX_IN_MATRIX);
        // XXX: no dim check this way, but also simpler
        if (numel == 0)
        {
            glMatrixMode(matrixmode);
            glLoadIdentity();
        }
        else if (numel == 16)
        {
            glMatrixMode(matrixmode);
            glLoadMatrixd(mxGetPr(SETMATRIX_IN_MATRIX));
        }
        else if (numel == 6)
        {
            const double *vec;

            if (matrixmode != GL_PROJECTION)
                mexErrMsgTxt("GLCALL: setmatrix: invalid call, passing a length-6 vector as X is"
                             " only allowed with GL_PROJECTION matrix mode.");

            vec = mxGetPr(SETMATRIX_IN_MATRIX);
            glMatrixMode(GL_PROJECTION);
            glOrtho(vec[0], vec[1], vec[2], vec[3], vec[4], vec[5]);
        }
        else
        {
            mexErrMsgTxt("GLCALL: setmatrix: invalid call, see GLCALL(glc.setmatrix) for usage.");
        }
    }

    }  // end switch(cmd)
}
