
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

/*#include <GL/gl.h>*/
#include <GL/freeglut.h>

/*////////*/
#include "mex.h"
#ifndef HAVE_OCTAVE
# include "matrix.h"
#endif

#ifdef DONT_HAVE_MWSIZE
# define mwSize unsigned long
#endif

/*////////*/

/* 1 lhs, 0 rhs */
#define OUT_GLCSTRUCT (plhs[0])

/* > 0 rhs */
#define IN_COMMAND (prhs[0])

/* init */
#define NEWWIN_IN_POS (prhs[1])
#define NEWWIN_IN_EXTENT (prhs[2])
#define NEWWIN_IN_NAME (prhs[3])
#define NEWWIN_IN_MULTISAMPLEP (prhs[4])
#define NEWWIN_OUT_WINID (plhs[0])

/* draw */
#define DRAW_IN_PRIMITIVETYPE (prhs[1])
#define DRAW_IN_VERTEXDATA (prhs[2])
#define DRAW_IN_OPTSTRUCT (prhs[3])

/* setmatrix */
#define SETMATRIX_IN_MODE (prhs[1])
#define SETMATRIX_IN_MATRIX (prhs[2])

/* mulmatrix */
#define MULMATRIX_IN_MODE (prhs[1])
#define MULMATRIX_IN_MATRIX (prhs[2])

/* setcallback */
#define SETCALLBACK_IN_TYPE (prhs[1])
#define SETCALLBACK_IN_FUNCNAME (prhs[2])

/* viewport */
#define VIEWPORT_IN_XYWH (prhs[1])

/* clear */
#define CLEAR_IN_COLOR (prhs[1])

/* newtexture */
#define NEWTEXTURE_IN_TEXAR (prhs[1])
#define NEWTEXTURE_IN_TEXNAME (prhs[2])
#define NEWTEXTURE_OUT_TEXNAME (plhs[0])

/* setwindowsize */
#define SETWINDOWSIZE_IN_WH (prhs[1])

/* getwindowsize */
#define GETWINDOWSIZE_OUT_WH (plhs[0])

/* rendertext */
#define RENDERTEXT_IN_POS (prhs[1])
#define RENDERTEXT_IN_HEIGHT (prhs[2])
#define RENDERTEXT_IN_TEXT (prhs[3])

/* toggle */
#define TOGGLE_IN_KV (prhs[1])

/* scissor */
#define SCISSOR_IN_XYWH (prhs[1])

/* deltextures */
#define DELTEXTURES_IN_TEXNAMES (prhs[1])

/* push/pop */
#define PUSH_IN_WHATAR (prhs[1])


enum glcalls_setcallback_
{
    CB_DISPLAY = 0,
    CB_RESHAPE,
    CB_KEYBOARD,
    CB_SPECIAL,
    CB_MOUSE,
    CB_MOTION,
    CB_PASSIVEMOTION,
    NUM_CALLBACKS,  /* must be last */
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
    GLC_MULMATRIX,
    GLC_SETCALLBACK,
    GLC_VIEWPORT,
    GLC_CLEAR,
    GLC_POSTREDISPLAY,
    GLC_GETERRSTR,
    GLC_NEWTEXTURE,
    GLC_SETWINDOWSIZE,
    GLC_GETWINDOWSIZE,
    GLC_RENDERTEXT,
    GLC_TOGGLE,
    GLC_SCISSOR,
    GLC_DELTEXTURES,
    GLC_PUSH,
    GLC_POP,
    NUM_GLCALLS,  /* must be last */
};

const char *glcall_names[] =
{
    "newwindow",
    "draw",
    "entermainloop",
    "setmatrix",
    "mulmatrix",
    "setcallback",
    "viewport",
    "clear",
    "postredisplay",
    "geterrstr",
    "newtexture",
    "setwindowsize",
    "getwindowsize",
    "rendertext",
    "toggle",
    "scissor",
    "deltextures",
    "push",
    "pop",
};


/*//////// DATA //////////*/

#define MAXCBNAMELEN 63
static char callback_funcname[NUM_CALLBACKS][MAXCBNAMELEN+1];
static int numentered = 0;

/*//////// UTIL //////////*/
static char errstr[128];

#ifndef HAVE_OCTAVE
static const mxArray *exceptionar;
static const char *errstrptr;

# define mexErrMsgTxt(msg) do {                 \
        if (numentered)                         \
        {                                       \
            if (errstrptr)                      \
                free(errstrptr);                \
            errstrptr = strdup(msg);            \
            glutLeaveMainLoop();                \
        }                                       \
        else                                    \
            mexErrMsgTxt(msg);                  \
    } while (0);
#endif

#define GLC_MEX_ERROR(Text, ...) do {           \
        sprintf(errstr, Text, ## __VA_ARGS__);  \
        mexErrMsgTxt(errstr);                   \
    } while (0)

enum verifyparam_flags
{
    /* 0: no requirement */

    /* Classes (data types) */
    VP_CELL = 1,
    VP_STRUCT,
    VP_LOGICAL,
    VP_CHAR,
    VP_DOUBLE,  /* 5 */
    VP_SINGLE,
    VP_INT8,
    VP_UINT8,
    VP_INT16,
    VP_UINT16,  /* 10 */
    VP_INT32,
    VP_UINT32,
    VP_INT64,
    VP_UINT64,
    VP_FP_TYPE,  /* 15 */
    VP_INDEX_TYPE,
    VP_CLASS_MASK = 0x0000001f,

    /* scalar/vector/matrix requirements */
    VP_SCALAR = 0x00000100,
    VP_VECTOR = 0x00000200,
    VP_MATRIX = 0x00000300,
    VP_DIMN = 0x00000400,
    VP_SVM_MASK = 0x00000300,
    VP_DIMN_MASK = 0x0000f000,
    VP_DIMN_SHIFT = 12,

    VP_VECLEN_SHIFT = 16,
    VP_VECLEN_MASK = 0x00ff0000,  /* shift down 16 bits to get length */
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

    if (vpclassidx)
    {
        if (vpclassidx < VP_FP_TYPE && class_ids[vpclassidx] != mxGetClassID(ar))
            GLC_MEX_ERROR("%s must have class %s", arname, class_names[vpclassidx]);
        else if (vpclassidx == VP_FP_TYPE && !(mxIsDouble(ar) || mxIsSingle(ar)))
            GLC_MEX_ERROR("%s must be of floating-point type", arname);
        else if (vpclassidx == VP_INDEX_TYPE && !(mxIsUint32(ar) || mxIsUint8(ar)))
            GLC_MEX_ERROR("%s must be of index type (uint8 or uint32)", arname);
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

    case VP_DIMN:
    {
        mwSize reqddim = (vpflags&VP_DIMN_MASK)>>VP_DIMN_SHIFT;

        if (mxGetNumberOfDimensions(ar) != reqddim)
            GLC_MEX_ERROR("%s must have dimension %d", arname, reqddim);
        break;
    }

    }
}

static int32_t util_dtoi(double d, double minnum, double maxnum, const char *arname)
{
    if (!(d >= minnum && d <= maxnum))
        GLC_MEX_ERROR("%s must be between %d and %d", arname, (int)minnum, (int)maxnum);
    return (int32_t)d;
}

static mxArray *createScalar(mxClassID cid, const void *ptr)
{
    mxArray *tmpar = mxCreateNumericMatrix(1,1, cid, mxREAL);

    switch (cid)
    {
    case mxLOGICAL_CLASS:
        *(int8_t *)mxGetData(tmpar) = !!*(int8_t *)ptr;
        break;
    case mxCHAR_CLASS:
    case mxINT8_CLASS:
    case mxUINT8_CLASS:
        *(int8_t *)mxGetData(tmpar) = *(int8_t *)ptr;
        break;
    case mxINT16_CLASS:
    case mxUINT16_CLASS:
        *(int16_t *)mxGetData(tmpar) = *(int16_t *)ptr;
        break;
    case mxINT32_CLASS:
    case mxUINT32_CLASS:
    case mxSINGLE_CLASS:
        *(int32_t *)mxGetData(tmpar) = *(int32_t *)ptr;
        break;
    case mxINT64_CLASS:
    case mxUINT64_CLASS:
    case mxDOUBLE_CLASS:
        *(int64_t *)mxGetData(tmpar) = *(int64_t *)ptr;
        break;
    default:
        mexErrMsgTxt("INTERNAL ERROR in createScalar: invalid cid!");
    }

    return tmpar;
}

/*//////// FUNCTIONS //////////*/

static void clear_callback_names(void)
{
    memset(callback_funcname, 0, sizeof(callback_funcname));
}

#define MAX_CB_ARGS 5  /* REMEMBER */
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
    {
        const mxArray *ex;

        ex = mexCallMATLABWithTrap(0,NULL, numargs,mxargs, callback_funcname[callbackid]);
        err = (ex != NULL);

        if (err)
        {
            if (exceptionar)
                mxDestroyArray(exceptionar);
            exceptionar = ex;
            mexMakeArrayPersistent(exceptionar);
        }
    }

    if (err)
    {
        char buf[128];
        sprintf(buf, "left main loop: error in callback %s: %d",
                 callback_funcname[callbackid], err);
        mexErrMsgTxt(buf);
    }

#endif
    if (err)
        glutLeaveMainLoop();

    return err;
}

/* 1: ALT*/
/* 10: CTRL*/
/* 100: SHIFT*/
static int getModifiers()
{
    int mods = glutGetModifiers();
    return (!!(mods&GLUT_ACTIVE_ALT)) + (10*!!(mods&GLUT_ACTIVE_CTRL)) + (100*!!(mods&GLUT_ACTIVE_SHIFT));
}

static void mouse_cb(int button, int state, int x, int y)
{
    if (CHECK_CALLBACK(CB_MOUSE))
    {
        int args[MAX_CB_ARGS] = {button, (state==GLUT_DOWN), x, y, 0};
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
}

/*//////// MEX ENTRY POINT //////////*/

void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
    int32_t cmd;

    static int inited = 0;

    if (nrhs==0)
    {
        if (nlhs==1)
        {
            /* the user queries available glcalls */
            mxArray *glcstruct = mxCreateStructMatrix(1,1, NUM_GLCALLS, glcall_names);
            mxArray *tmpar;
            int32_t i, fieldnum;

            for (i=0; i<NUM_GLCALLS; i++)
            {
                tmpar = createScalar(mxINT32_CLASS, &i);
                mxSetFieldByNumber(glcstruct, 0, i, tmpar);
            }

            for (i=0; i<NUM_CALLBACKS; i++)
            {
                tmpar = createScalar(mxINT32_CLASS, &i);

                fieldnum = mxAddField(glcstruct, glcall_callback_names[i]);
                mxSetFieldByNumber(glcstruct, 0, fieldnum, tmpar);
            }

            OUT_GLCSTRUCT = glcstruct;

            return;
        }
        else
            mexErrMsgTxt("Usage: GLCALLINFO_STRUCT = GLCALL(), query available subcommands.");
    }

    /* nrhs > 0 */

    verifyparam(IN_COMMAND, "COMMAND", VP_SCALAR|VP_INT32);
    cmd = *(int32_t *)mxGetData(IN_COMMAND);

    if ((cmd != GLC_NEWWINDOW && cmd != GLC_GETERRSTR) && !inited)
        mexErrMsgTxt("GLCALL: Must call 'newwindow' subcommand to initialize GLUT before any other command!");

    /*////////*/
    switch (cmd)
    {
    case GLC_NEWWINDOW:
    {
        double *posptr, *extentptr;

        int32_t pos[2], extent[2], winid, multisamplep=0;
        char windowname[80];

        if (nlhs > 1 || (nrhs != 4 && nrhs != 5))
            mexErrMsgTxt("Usage: [WINID =] GLCALL(glc.newwindow, POS, EXTENT, WINDOWNAME [, MULTISAMPLEP]),"
                         " create new window.");

        verifyparam(NEWWIN_IN_POS, "POS", VP_VECTOR|VP_DOUBLE|(2<<VP_VECLEN_SHIFT));
        verifyparam(NEWWIN_IN_EXTENT, "EXTENT", VP_VECTOR|VP_DOUBLE|(2<<VP_VECLEN_SHIFT));
        verifyparam(NEWWIN_IN_NAME, "WINDOWNAME", VP_VECTOR|VP_CHAR);
        if (nrhs >= 5)
        {
            verifyparam(NEWWIN_IN_MULTISAMPLEP, "MULTISAMPLEP", VP_SCALAR|VP_LOGICAL);
            multisamplep = *(uint8_t *)mxGetData(NEWWIN_IN_MULTISAMPLEP);
        }

        posptr = mxGetPr(NEWWIN_IN_POS);
        extentptr = mxGetPr(NEWWIN_IN_EXTENT);

        pos[0] = util_dtoi(posptr[0], 0, 1680, "GLCALL: newwindow: POS(1)");
        pos[1] = util_dtoi(posptr[1], 0, 1050, "GLCALL: newwindow: POS(2)");

        extent[0] = util_dtoi(extentptr[0], 320, 1680, "GLCALL: newwindow: EXTENT(1)");
        extent[1] = util_dtoi(extentptr[1], 200, 1050, "GLCALL: newwindow: EXTENT(2)");

        mxGetString(NEWWIN_IN_NAME, windowname, sizeof(windowname)-1);

        /* init!*/
        if (!inited)
        {
            char *argvdummy[1] = {"QWEASDproggy"};
            int argcdummy = 1;

            glutInit(&argcdummy, argvdummy);  /* XXX */
            glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_CONTINUE_EXECUTION);

            glEnable(GL_POINT_SMOOTH);

            inited = 1;
        }
        else
            mexErrMsgTxt("GLCALL: newwindow: multiple windows not implemented!");

        glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH | multisamplep*GLUT_MULTISAMPLE | GLUT_RGBA);
        glutInitWindowPosition(pos[0], pos[1]);
        glutInitWindowSize(extent[0], extent[1]);

        winid = glutCreateWindow(windowname);

        if (nlhs > 0)
        {
            NEWWIN_OUT_WINID = createScalar(mxINT32_CLASS, &winid);
        }
    }
    return;

    case GLC_DRAW:
    {
        /* TODO: pass color array */

        unsigned int primitivetype;
        mwSize i, numdims, numtotalverts, numverts;

        int singlecolorp;

        const mxArray *colorsar=NULL, *indicesar=NULL;

        const double *colors=NULL;
        const void *indices=NULL;  /* uint8_t or uint32_t */
        int indicestype = 0, vertdatatype;
        GLuint texname = 0;

        if (nlhs != 0 || (nrhs != 3 && nrhs != 4))
            mexErrMsgTxt("Usage: GLCALL(glc.draw, GL.<PRIMITIVE_TYPE>, VERTEXDATA [, OPTSTRUCT])");

        if (!mxIsUint32(DRAW_IN_PRIMITIVETYPE))
            mexErrMsgTxt("GLCALL: draw: PRIMITIVE_TYPE must be of uint32 type");
        primitivetype = *(int *)mxGetData(DRAW_IN_PRIMITIVETYPE);

        if (!(/*primitivetype >= GL_POINTS &&*/ primitivetype <= GL_POLYGON))
            mexErrMsgTxt("GLCALL: draw: invalid GL primitive type");

        verifyparam(DRAW_IN_VERTEXDATA, "GLCALL: draw: VERTEXDATA", VP_MATRIX|VP_FP_TYPE);
        if (mxIsDouble(DRAW_IN_VERTEXDATA))
            vertdatatype = GL_DOUBLE;
        else
            vertdatatype = GL_FLOAT;

        numdims = mxGetM(DRAW_IN_VERTEXDATA);
        numtotalverts = mxGetN(DRAW_IN_VERTEXDATA);

        if (!(numdims >=2 && numdims <= 4))
            mexErrMsgTxt("GLCALL: draw: VERTEXDATA must have between 2 and 4 rows (number of coordinates)");

        if (nrhs > 3)
        {
            const mxArray *texar, *texcoordsar;

            verifyparam(DRAW_IN_OPTSTRUCT, "GLCALL: draw: OPTSTRUCT", VP_SCALAR|VP_STRUCT);

            colorsar = mxGetField(DRAW_IN_OPTSTRUCT, 0, "colors");
            indicesar = mxGetField(DRAW_IN_OPTSTRUCT, 0, "indices");
            texar = mxGetField(DRAW_IN_OPTSTRUCT, 0, "tex");
            if (texar)
            {
                verifyparam(texar, "GLCALL: draw: OPTSTRUCT.tex", VP_SCALAR|VP_UINT32);

                texname = *(uint32_t *)mxGetData(texar);
                if (texname == 0)
                    mexErrMsgTxt("GLCALL: draw: OPTSTRUCT.tex must be greater zero");

                texcoordsar = mxGetField(DRAW_IN_OPTSTRUCT, 0, "texcoords");
                if (!texcoordsar)
                    mexErrMsgTxt("GLCALL: draw: When passing OPTSTRUCT.tex, must also have OPTSTRUCT.texcoords");

                verifyparam(texcoordsar, "GLCALL: draw: OPTSTRUCT.texcoords", VP_MATRIX|VP_DOUBLE);
                if (mxGetM(texcoordsar) != 2 || mxGetN(texcoordsar) != (unsigned long)numtotalverts)
                    mexErrMsgTxt("GLCALL: draw: OPTSTRUCT.texcoords must have "
                                 "2 rows and size(VERTEXDATA,2) columns");

                glEnable(GL_TEXTURE_2D);
                glBindTexture(GL_TEXTURE_2D, texname);

                glEnableClientState(GL_TEXTURE_COORD_ARRAY);
                glTexCoordPointer(2, GL_DOUBLE, 0, mxGetPr(texcoordsar));
            }
        }

        if (!texname)
        {
            glDisableClientState(GL_TEXTURE_COORD_ARRAY);
            glDisable(GL_TEXTURE_2D);
        }

        singlecolorp = 0;

        if (colorsar)
        {
            mwSize sz[2];

            verifyparam(colorsar, "GLCALL: draw: OPTSTRUCT.colors", VP_MATRIX|VP_DOUBLE);

            sz[0] = mxGetM(colorsar);
            sz[1] = mxGetN(colorsar);

            if ((sz[0]==3 && sz[1]==1) || (sz[0]==1 && sz[1]==3))
                singlecolorp = 1;
            else
            {
                if (sz[0] != 3 || sz[1] != numtotalverts)
                    mexErrMsgTxt("GLCALL: draw: OPTSTRUCT.colors must either have length 3 "
                                 " or have 3 rows and size(VERTEXDATA,2) columns");
            }

            colors = (const double *)mxGetData(colorsar);
        }

        if (indicesar)
        {
            verifyparam(indicesar, "GLCALL: draw: OPTSTRUCT.indices", VP_VECTOR|VP_INDEX_TYPE);

            numverts = mxGetNumberOfElements(indicesar);
            indices = mxGetData(indicesar);

            if (mxIsUint32(indicesar))
                indicestype = GL_UNSIGNED_INT;
            else
                indicestype = GL_UNSIGNED_BYTE;

            if (numverts==0)
                return;

            /* bounds check! */
            for (i=0; i<numverts; i++)
            {
                if (indicestype==GL_UNSIGNED_INT)
                {
                    if (((uint32_t *)indices)[i] >= (unsigned)numverts)
                        mexErrMsgTxt("GLCALL: draw: INDICES must contain values between 0 and size(VERTEXDATA,2)-1");
                }
                else
                {
                    if (((uint8_t *)indices)[i] >= (unsigned)numverts)
                        mexErrMsgTxt("GLCALL: draw: INDICES must contain values between 0 and size(VERTEXDATA,2)-1");
                }
            }
        }
        else
        {
            if (numtotalverts==0)
                return;

            numverts = numtotalverts;
        }

        /* draw them at last! */

        if (!colors || singlecolorp)
        {
            glDisableClientState(GL_COLOR_ARRAY);
            if (singlecolorp)
                glColor3d(colors[0], colors[1], colors[2]);
            else
                glColor3f(0.5f, 0.5f, 0.5f);
        }
        else
        {
            glEnableClientState(GL_COLOR_ARRAY);
            glColorPointer(3, GL_DOUBLE, 0, colors);
        }

        glEnableClientState(GL_VERTEX_ARRAY);
        glVertexPointer(numdims, vertdatatype, 0, mxGetPr(DRAW_IN_VERTEXDATA));

        if (!indicesar)
            glDrawArrays(primitivetype, 0, numverts);
        else
            glDrawElements(primitivetype, numverts, indicestype, indices);
    }
    return;

    case GLC_ENTERMAINLOOP:
    {
        if (nlhs != 0 || nrhs != 1)
            mexErrMsgTxt("Usage: GLCALL(glc.entermainloop), enter main loop.");

        if (numentered)
            mexErrMsgTxt("GLCALL: entermainloop: entered recursively!");
        numentered++;

        /* these two are always there */
        glutDisplayFunc(display_cb);
        glutReshapeFunc(reshape_cb);

        /* X: make register/unregister on demand (when GLCALL(glc.setcallback, ...) */
        /*    is called)? Doesn't seem really necessary... */
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
            "    which is passed to glOrtho() after loading the identity matrix";

        if (nlhs != 0 || nrhs != 3)
            mexErrMsgTxt(usageText);

        verifyparam(SETMATRIX_IN_MODE, "GLCALL: setmatrix: MATRIX_MODE", VP_SCALAR|VP_UINT32);
        matrixmode = *(uint32_t *)mxGetData(SETMATRIX_IN_MODE);

        if (matrixmode != GL_MODELVIEW && matrixmode != GL_PROJECTION)
            mexErrMsgTxt("GLCALL: setmatrix: MATRIX_MODE must be one of GL_MODELVIEW or GL_PROJECTION");

        if (!mxIsDouble(SETMATRIX_IN_MATRIX))
            mexErrMsgTxt("GLCALL: setmatrix: X must have class double");

        numel = mxGetNumberOfElements(SETMATRIX_IN_MATRIX);
        /* XXX: no dim check this way, but also simpler*/
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
        else if (numel == 6 || numel == 4)
        {
            const double *vec;

            if (matrixmode != GL_PROJECTION)
                mexErrMsgTxt("GLCALL: setmatrix: invalid call, passing a length-6 (or 4) vector as X is"
                             " only allowed with GL_PROJECTION matrix mode.");

            vec = mxGetPr(SETMATRIX_IN_MATRIX);
            glMatrixMode(GL_PROJECTION);
            glLoadIdentity();

            if (numel == 6)
                glOrtho(vec[0], vec[1], vec[2], vec[3], vec[4], vec[5]);
            else
                gluPerspective(vec[0], vec[1], vec[2], vec[3]);
        }
        else
        {
            mexErrMsgTxt("GLCALL: setmatrix: invalid call, see GLCALL(glc.setmatrix) for usage.");
        }
    }
    return;

    case GLC_MULMATRIX:
    {
        uint32_t matrixmode;
        mwSize numel;

        static const char *usageText =
            "Usage: GLCALL(glc.nulmatrix, GL.<MATRIX_MODE>, X), where X can be\n"
            "  * a 4x4 double matrix\n"
            "  * the empty matrix [], meaning no-op, or\n"
            "  * a length-3 vector, whose elements are passed to glTranslate, or\n"
            "  * a length-4 vector, whose elements are passed to glRotate";

        if (nlhs != 0 || nrhs != 3)
            mexErrMsgTxt(usageText);

        verifyparam(MULMATRIX_IN_MODE, "GLCALL: mulmatrix: MATRIX_MODE", VP_SCALAR|VP_UINT32);
        matrixmode = *(uint32_t *)mxGetData(MULMATRIX_IN_MODE);

        if (matrixmode != GL_MODELVIEW && matrixmode != GL_PROJECTION && matrixmode != GL_TEXTURE)
            mexErrMsgTxt("GLCALL: mulmatrix: MATRIX_MODE must be one of GL_MODELVIEW, "
                         "GL_PROJECTION or GL_TEXTURE");

        if (!mxIsDouble(MULMATRIX_IN_MATRIX))
            mexErrMsgTxt("GLCALL: mulmatrix: X must have class double");

        numel = mxGetNumberOfElements(MULMATRIX_IN_MATRIX);
        /* XXX: no dim check this way, but also simpler */

        glMatrixMode(matrixmode);

        if (numel == 0)
        {
            return;
        }
        else if (numel == 16)
        {
            glMultMatrixd(mxGetPr(MULMATRIX_IN_MATRIX));
        }
        else if (numel == 3 || numel == 4)
        {
            const double *vec;

            vec = mxGetPr(MULMATRIX_IN_MATRIX);
            if (numel == 3)
                glTranslated(vec[0], vec[1], vec[2]);
            else
                glRotated(vec[0], vec[1], vec[2], vec[3]);
        }
        else
        {
            mexErrMsgTxt("GLCALL: mulmatrix: invalid call, see GLCALL(glc.mulmatrix) for usage.");
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

    /* TODO: merge with GLCALL(glc.scissor) ... */
    case GLC_VIEWPORT:
    {
        const double *xywh_d;
        int32_t xywh[4];

        if (nlhs != 0 || nrhs != 2)
            mexErrMsgTxt("Usage: GLCALL(glc.viewport, [x y width height])");

        verifyparam(VIEWPORT_IN_XYWH, "GLCALL: viewport: XYWH", VP_VECTOR|VP_DOUBLE|(4<<VP_VECLEN_SHIFT));
        xywh_d = mxGetPr(VIEWPORT_IN_XYWH);

        /* XXX: magic constants bad! */
        xywh[0] = util_dtoi(xywh_d[0], -16384, 16384, "GLCALL: viewport: XYWH(1)");
        xywh[1] = util_dtoi(xywh_d[1], -16384, 16384, "GLCALL: viewport: XYWH(2)");
        xywh[2] = util_dtoi(xywh_d[2], 0, 16384, "GLCALL: viewport: XYWH(3)");
        xywh[3] = util_dtoi(xywh_d[3], 0, 16384, "GLCALL: viewport: XYWH(4)");

        glViewport(xywh[0], xywh[1], xywh[2], xywh[3]);
    }
    return;

    case GLC_CLEAR:
    {
        mwSize numel;
        const double *color;

        if (nlhs != 0 || nrhs != 2)
            mexErrMsgTxt("Usage: GLCALL(glc.viewport, [r g b [a]]), color must have 'double' type");

        verifyparam(CLEAR_IN_COLOR, "GLCALL: clear: COLOR", VP_VECTOR|VP_DOUBLE);
        numel = mxGetNumberOfElements(CLEAR_IN_COLOR);
        if (numel != 3 && numel != 4)
            mexErrMsgTxt("GLCALL: clear: COLOR must have length 3 or 4");

        color = mxGetPr(CLEAR_IN_COLOR);

        glClearColor(color[0], color[1], color[2], numel==4?color[3]:0);
        glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    }
    return;

    case GLC_POSTREDISPLAY:
    {
        if (nlhs != 0 || nrhs != 1)
            mexErrMsgTxt("Usage: GLCALL(glc.postredisplay)");

        glutPostRedisplay();
    }
    return;

    case GLC_GETERRSTR:
    {
#ifdef HAVE_OCTAVE
        mexErrMsgTxt("CLGALL.geterrstr only available on MATLAB");
#else
        const char *emptystr = "<empty>";
        const char *errstr_ptr = errstr;

        if (nlhs != 3 || nrhs != 1)
            return;

        plhs[0] = mxCreateCharMatrixFromStrings(1, errstrptr ? &errstrptr : &emptystr);
        plhs[1] = mxCreateCharMatrixFromStrings(1, &errstr_ptr);
        plhs[2] = exceptionar ? exceptionar : mxCreateDoubleScalar(0);
#endif
    }
    return;

    case GLC_NEWTEXTURE:
    {
        GLuint texname;
        const mwSize *dimsizes;  /* XXX: older versions w/o mwSize? */
        GLint tmpwidth;

        int32_t newtex = (nlhs == 1 && nrhs == 2);
        int32_t repltex = (nlhs == 0 && nrhs == 3);

        GLint internalFormat;
        GLenum format;
        GLenum type;

        mwSize width, height;

        if (!newtex && !repltex)
            mexErrMsgTxt("Usage: texname = GLCALL(glc.newtexture, texar), texar must be NxMx3 uint8 or NxM single\n"
                         "   or: GLCALL(glc.newtexture, texar, texname)  to replace an earlier texture");

        if (mxGetNumberOfDimensions(NEWTEXTURE_IN_TEXAR)==3)
        {
            /* RBG uint8 */
            verifyparam(NEWTEXTURE_IN_TEXAR, "GLCALL: newtexture: TEXAR", VP_UINT8|VP_DIMN|(3<<VP_DIMN_SHIFT));

            dimsizes = mxGetDimensions(NEWTEXTURE_IN_TEXAR);
            if (dimsizes[0] != 3)
                mexErrMsgTxt("GLCALL: newtexture: TEXAR's 3rd dim must have length 3");

            width = dimsizes[1];
            height = dimsizes[2];

            internalFormat = GL_RGB;
            format = GL_RGB;
            type = GL_UNSIGNED_BYTE;
        }
        else
        {
            /* grayscale float */
            verifyparam(NEWTEXTURE_IN_TEXAR, "GLCALL: newtexture: TEXAR", VP_SINGLE|VP_DIMN|(2<<VP_DIMN_SHIFT));

            dimsizes = mxGetDimensions(NEWTEXTURE_IN_TEXAR);
            width = dimsizes[0];
            height = dimsizes[1];

            internalFormat = GL_LUMINANCE;
            format = GL_LUMINANCE;
            type = GL_FLOAT;
        }

        if (width <= 0 || width > 16384 || height <= 0 || height > 16384)
            mexErrMsgTxt("GLCALL: mextexture: TEXAR's width and height must have length in [1, 16384]");

        if (repltex)
        {
            verifyparam(NEWTEXTURE_IN_TEXNAME, "GLCALL: newtexture: TEXNAME", VP_SCALAR|VP_UINT32);
            texname = *(uint32_t *)mxGetData(NEWTEXTURE_IN_TEXNAME);
            if (texname == 0)
                mexErrMsgTxt("GLCALL: newtexture: TEXNAME must be greater 0");
        }
        else
        {
            glGenTextures(1, &texname);
        }

        glBindTexture(GL_TEXTURE_2D, texname);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

        glPixelStorei(GL_UNPACK_ALIGNMENT, type==GL_FLOAT ? 4 : 1);

        /* target, level, internalFormat, width, height, border, format, type, data*/
        glTexImage2D(GL_PROXY_TEXTURE_2D, 0, internalFormat,  width, height,
                     0, format, type, NULL);
        glGetTexLevelParameteriv(GL_PROXY_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &tmpwidth);
        if (tmpwidth==0)
        {
            glDeleteTextures(1, &texname);
            mexErrMsgTxt("GLCALL: newtexture: cannot accomodate texture");
        }
        glTexImage2D(GL_TEXTURE_2D, 0, internalFormat,  width, height,
                     0, format, type, mxGetData(NEWTEXTURE_IN_TEXAR));

        if (newtex)
            NEWTEXTURE_OUT_TEXNAME = createScalar(mxUINT32_CLASS, &texname);
    }
    return;

    case GLC_SETWINDOWSIZE:
    {
        const double *wh_d;
        int32_t wh[2];

        if (nlhs != 0 || nrhs != 2)
            mexErrMsgTxt("Usage: GLCALL(glc.setwindowsize, [width height])");

        verifyparam(SETWINDOWSIZE_IN_WH, "GLC: setwindowsize: WH", VP_VECTOR|VP_DOUBLE|(2<<VP_VECLEN_SHIFT));
        wh_d = mxGetPr(SETWINDOWSIZE_IN_WH);

        wh[0] = util_dtoi(wh_d[0], 1, 16384, "GLC: setwindowsize: WH(1)");
        wh[1] = util_dtoi(wh_d[1], 1, 16384, "GLC: setwindowsize: WH(2)");

        glutReshapeWindow(wh[0], wh[1]);
    }
    return;

    case GLC_GETWINDOWSIZE:
    {
        mxArray *whar;
        double *wh;

        if (nlhs != 1 || nrhs != 1)
            mexErrMsgTxt("Usage: WH=GLCALL(glc.getwindowsize)");

        whar = mxCreateNumericMatrix(1,2, mxDOUBLE_CLASS, mxREAL);
        wh = mxGetPr(whar);

        wh[0] = glutGet(GLUT_WINDOW_WIDTH);
        wh[1] = glutGet(GLUT_WINDOW_HEIGHT);

        GETWINDOWSIZE_OUT_WH = whar;
    }
    return;

    case GLC_RENDERTEXT:
    {
        const double *pos;
        double height;
        char *text;

        mwSize slen, i;

        if (nlhs != 0 || nrhs != 4)
            mexErrMsgTxt("Usage: GLCALL(glc.rendertext, [x y z], height, text)");

        verifyparam(RENDERTEXT_IN_POS, "GLC: rendertext: POS", VP_VECTOR|VP_DOUBLE|(3<<VP_VECLEN_SHIFT));
        pos = mxGetPr(RENDERTEXT_IN_POS);

        verifyparam(RENDERTEXT_IN_HEIGHT, "GLC: rendertext: HEIGHT", VP_SCALAR|VP_DOUBLE);
        height = *mxGetPr(RENDERTEXT_IN_HEIGHT);

        verifyparam(RENDERTEXT_IN_TEXT, "GLC: rendertext: TEXT", VP_VECTOR|VP_CHAR);
        text = mxArrayToString(RENDERTEXT_IN_TEXT);

        if (!text)
            mexErrMsgTxt("GLCALL: rendertext: Out of memory!");

        {
            glMatrixMode(GL_MODELVIEW);
            glPushMatrix();
            glPushAttrib(GL_CURRENT_BIT|GL_ENABLE_BIT);

            glColor3d(0.2, 0.2, 0.2);  /* XXX */
            glDisable(GL_TEXTURE_2D);
/*            glEnable(GL_LINE_SMOOTH); */

            glLoadIdentity();
            glTranslated(pos[0], pos[1], pos[2]);

            glScaled(height/119.05, height/119.05, height/119.05);

            slen = strlen(text);
            for (i=0; i<slen; i++)
            {
                glutStrokeCharacter(GLUT_STROKE_ROMAN, text[i]);
                glTranslated(119.05/10.0, 0, 0);  /* a bit of spacing... */
            }

            glPopMatrix();
            glPopAttrib();
        }

        mxFree(text);
    }
    return;

    case GLC_TOGGLE:
    {
        mwSize numkeys, i;
        const int32_t *kv;

        if (nlhs != 0 || nrhs != 2)
            mexErrMsgTxt("Usage: GLCALL(glc.toggle, [GL.<WHAT1> <state1> [, GL.<WHAT2> <state2>, ...]]))");

        verifyparam(TOGGLE_IN_KV, "GLCALL: toggle: KV", VP_VECTOR|VP_INT32);
        numkeys = mxGetNumberOfElements(TOGGLE_IN_KV);
        if (numkeys&1)
            mexErrMsgTxt("GLCALL: toggle: key-value vector must have even length");
        numkeys /= 2;

        kv = mxGetData(TOGGLE_IN_KV);

        /* validation */
        for (i=0; i<numkeys; i++)
            if (kv[2*i] != GL_DEPTH_TEST && kv[2*i] != GL_SCISSOR_TEST)
                mexErrMsgTxt("GLCALL: toggle: currently only supported: DEPTH_TEST and SCISSOR_TEST");

        for (i=0; i<numkeys; i++)
        {
            int32_t val = kv[2*i+1];

            if (val < 0)  /* flip */
                val = !glIsEnabled(kv[2*i]);

            if (val)
                glEnable(kv[2*i]);
            else
                glDisable(kv[2*i]);
        }
    }
    return;

    case GLC_SCISSOR:
    {
        int32_t *xywh;

        if (nlhs != 0 || nrhs != 2)
            mexErrMsgTxt("Usage: GLCALL(glc.scissor, int32([x y w h]))");

        verifyparam(SCISSOR_IN_XYWH, "GLCALL: scissor: XYWH", VP_VECTOR|VP_INT32|(4<<VP_VECLEN_SHIFT));

        xywh = mxGetData(SCISSOR_IN_XYWH);
        if (xywh[2]<0 || xywh[3]<0)
            mexErrMsgTxt("GLCALL: scissor: XYWH(3) and XYWH(4) must be non-negative");

        glScissor(xywh[0], xywh[1], xywh[2], xywh[3]);
    }
    return;

    case GLC_DELTEXTURES:
    {
        mwSize numtexs;

        if (nlhs != 0 || nrhs != 2)
            mexErrMsgTxt("Usage: GLCALL(glc.deltextures, [texname1 texname2 ...])");

        verifyparam(DELTEXTURES_IN_TEXNAMES, "GLCALL: deltextures: TEXNAMES", VP_VECTOR|VP_UINT32);
        numtexs = mxGetNumberOfElements(DELTEXTURES_IN_TEXNAMES);

        glDeleteTextures(numtexs, (uint32_t *)mxGetData(DELTEXTURES_IN_TEXNAMES));
    }
    return;

    case GLC_PUSH:
    case GLC_POP:
    {
        mwSize i, numwhats;
        const uint32_t *what;

        if (nlhs != 0 || nrhs != 2)
            mexErrMsgTxt("Usage: GLCALL(glcall.push, [WHAT1 WHAT2 ...]);  WHAT* can be either\n"
                         " GL.PROJECTION, GL.MODELVIEW, GL.TEXTURE to push the respective matrices,\n"
                         " or a bit combination of GL.*_BIT that is passed to glPushAttrib()\n"
                         "(see getglconsts.m)");

        verifyparam(PUSH_IN_WHATAR, "GLCALL: push: WHATAR", VP_VECTOR|VP_UINT32);
        what = (uint32_t *)mxGetData(PUSH_IN_WHATAR);

        numwhats = mxGetNumberOfElements(PUSH_IN_WHATAR);

        for (i=0; i<numwhats; i++)
        {
            if (what[i]==GL_PROJECTION || what[i]==GL_MODELVIEW || what[i]==GL_TEXTURE)
            {
                glMatrixMode(what[i]);
                if (cmd==GLC_PUSH)
                    glPushMatrix();
                else
                    glPopMatrix();
            }
            else
            {
                if (cmd==GLC_PUSH)
                    glPushAttrib(what[i]);
                else
                    glPopAttrib();
            }
        }

        if (glGetError() == (cmd==GLC_PUSH ? GL_STACK_OVERFLOW : GL_STACK_UNDERFLOW))
        {
            mexErrMsgTxt("GLCALL: push/pop: over/underflow");
        }
    }
    return;

    }  /* end switch(cmd) */
}
