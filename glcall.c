
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

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

// setcallback
#define SETCALLBACK_IN_TYPE (prhs[1])
#define SETCALLBACK_IN_FUNCNAME (prhs[2])

enum glcalls_setcallback_
{
    CB_DISPLAY = 0,
    CB_RESHAPE,
    CB_KEYBOARD,
    CB_SPECIAL,
    CB_MOUSE,
    CB_MOTION,
    CB_PASSIVEMOTION,
    NUM_CALLBACKS,  // must be last
};

const char *glcall_callback_names[] = 
{
    "cb_display",
    "cb_reshape",
    "cb_keyboard",
    "cb_special",
    "cb_mouse",
    "cb_motion",
    "cb_passivemotion",
};


/*** GLCALL commands enum ***/
enum glcalls_
{
    GLC_NEWWINDOW = 0,
    GLC_DRAW,
    GLC_ENTERMAINLOOP,
    GLC_SETMATRIX,
    GLC_SETCALLBACK,
    NUM_GLCALLS,  // must be last
};

const char *glcall_names[] =
{
    "newwindow",
    "draw",
    "entermainloop",
    "setmatrix",
    "setcallback",
};


////////// DATA //////////

#define MAXCBNAMELEN 63
static char callback_funcname[NUM_CALLBACKS][MAXCBNAMELEN+1];

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

static void verifyparam(const mxArray *ar, const char *arname, uint32_t vpflags)
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

static int util_dtoi(double d, double minnum, double maxnum, const char *arname)
{
    if (!(d >= minnum && d <= maxnum))
        GLC_MEX_ERROR("%s must be between %d and %d", arname, (int)minnum, (int)maxnum);
    return (int)d;
}

////////// FUNCTIONS //////////

static void clear_callback_names(void)
{
    memset(callback_funcname, 0, sizeof(callback_funcname));
}

#define MAX_CB_ARGS 5  // REMEMBER
#define CHECK_CALLBACK(CallbackID) (callback_funcname[CallbackID][0]!='\0')

static int call_mfile_callback(int callbackid, int numargs, const int *args)
{
    int err, i;
    mxArray *mxargs[MAX_CB_ARGS];

    mxAssert(numargs <= MAX_CB_ARGS, "numargs > MAX_CB_ARGS, update MAX_CB_ARGS macro!");

    for (i=0; i<numargs; i++)
        mxargs[i] = mxCreateDoubleScalar((double)args[i]);

#ifdef HAVE_OCTAVE
    mexSetTrapFlag(1);
    err = mexCallMATLAB(0,NULL, numargs,mxargs, callback_funcname[callbackid]);
#else
    err = 0;
    if (mexCallMATLABWithTrap(0,NULL, numargs,mxargs, callback_funcname[callbackid]))
        err = 1;
#endif

    return err;
}

// 1: ALT
// 10: CTRL
// 100: SHIFT
static int getModifiers()
{
    int mods = glutGetModifiers();
    return (!!(mods&GLUT_ACTIVE_ALT)) + (10*!!(mods&GLUT_ACTIVE_CTRL)) + (100*!!(mods&GLUT_ACTIVE_SHIFT));
}

static void mouse_cb(int button, int state, int x, int y)
{
    if (CHECK_CALLBACK(CB_MOUSE))
    {
        int args[MAX_CB_ARGS] = {button, state, x, y, 0};
        args[4] = getModifiers();
        call_mfile_callback(CB_MOUSE, 5, args);
    }
}

static void motion_cb(int x, int y)
{
    if (CHECK_CALLBACK(CB_MOTION))
    {
        int args[MAX_CB_ARGS] = {x, y};
        call_mfile_callback(CB_MOTION, 2, args);
    }
}

static void passivemotion_cb(int x, int y)
{
    if (CHECK_CALLBACK(CB_PASSIVEMOTION))
    {
        int args[MAX_CB_ARGS] = {x, y};
        call_mfile_callback(CB_PASSIVEMOTION, 2, args);
    }
}

static void keyboard_cb(unsigned char key, int x, int y)
{
    if (CHECK_CALLBACK(CB_KEYBOARD))
    {
        int args[MAX_CB_ARGS] = {key, x, y, 0};
        args[3] = getModifiers();
        call_mfile_callback(CB_KEYBOARD, 3, args);
    }
}

static void special_cb(int key, int x, int y)
{
    if (CHECK_CALLBACK(CB_SPECIAL))
    {
        int args[MAX_CB_ARGS] = {key, x, y, 0};
        args[3] = getModifiers();
        call_mfile_callback(CB_SPECIAL, 4, args);
    }
}

static void display_cb(void)
{
    if (CHECK_CALLBACK(CB_DISPLAY))
    {
        call_mfile_callback(CB_DISPLAY, 0, NULL);
    }
    else
    {
        glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    }

    glutSwapBuffers();
}

static void reshape_cb(int w, int h)
{
    if (CHECK_CALLBACK(CB_RESHAPE))
    {
        int args[MAX_CB_ARGS] = {w, h};
        call_mfile_callback(CB_RESHAPE, 2, args);
    }
    else
    {
        glViewport(0, 0, (GLsizei)w, (GLsizei)h);
    }

#if 0
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
#endif
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
            int i, fieldnum;

            for (i=0; i<NUM_GLCALLS; i++)
            {
                tmpar = mxCreateNumericMatrix(1,1, mxINT32_CLASS, mxREAL);
                *(int *)mxGetData(tmpar) = i;
                mxSetFieldByNumber(glcstruct, 0, i, tmpar);
            }

            for (i=0; i<NUM_CALLBACKS; i++)
            {
                tmpar = mxCreateNumericMatrix(1,1, mxINT32_CLASS, mxREAL);
                *(int *)mxGetData(tmpar) = i;

                fieldnum = mxAddField(glcstruct, glcall_callback_names[i]);
                mxSetFieldByNumber(glcstruct, 0, fieldnum, tmpar);
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

        // these two are always there
        glutDisplayFunc(display_cb);
        glutReshapeFunc(reshape_cb);

        // X: make register/unregister on demand (when GLCALL(glc.setcallback, ...)
        //    is called)? Doesn't seem really necessary...
        glutMouseFunc(mouse_cb);
        glutMotionFunc(motion_cb);
        glutPassiveMotionFunc(passivemotion_cb);
        glutKeyboardFunc(keyboard_cb);
        glutSpecialFunc(special_cb);

        glutMainLoop();

        numentered--;
        inited = 0;

        clear_callback_names();
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
    return;

    case GLC_SETCALLBACK:
    {
        int32_t callbackid, i, slen;
        char tmpbuf[MAXCBNAMELEN+1], c;

        if (nlhs != 0 || nrhs != 3)
            mexErrMsgTxt("Usage: GLCALL(glc.setcallback, glc.cb_<callbacktype>, 'mfuncname'), pass '' to reset");

        verifyparam(SETCALLBACK_IN_TYPE, "GLCALL: setcallback: CALLBACKTYPE", VP_SCALAR|VP_INT32);
        callbackid = *(int32_t *)mxGetData(SETCALLBACK_IN_TYPE);
        if ((unsigned)callbackid >= NUM_CALLBACKS)
            mexErrMsgTxt("GLCALL: setcallback: CALLBACKTYPE must be a valid callback ID");

        verifyparam(SETCALLBACK_IN_FUNCNAME, "GLCALL: setcallback: FUNCNAME", VP_VECTOR|VP_CHAR);
        slen = mxGetNumberOfElements(SETCALLBACK_IN_FUNCNAME);
        if (slen > MAXCBNAMELEN)
            GLC_MEX_ERROR("GLCALL: setcallback: FUNCNAME must not exceed %d chars", MAXCBNAMELEN);

        mxGetString(SETCALLBACK_IN_FUNCNAME, tmpbuf, sizeof(tmpbuf));
        for (i=0; i<slen; i++)
        {
            c = tmpbuf[i];

            if ((i==0 && (c=='_' || (c>='0' && c<='9'))) ||
                    (c!='_' && !(c>='0' && c<='9') && !(c>='a' && 'c'<='z') && !(c>='A' && c<='Z')))
                mexErrMsgTxt("GLCALL: setcallback: FUNCNAME must be a valid MATLAB identifier ([A-Za-z][A-Za-z0-9_]+)");
        }

        memcpy(&callback_funcname[callbackid], tmpbuf, sizeof(tmpbuf));
    }
    return;

    }  // end switch(cmd)
}
