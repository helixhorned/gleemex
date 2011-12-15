
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include <GL/glew.h>
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
#define MULMATRIX_IN_MATRIX (nrhs==3 ? prhs[2] : prhs[1])

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
#define NEWTEXTURE_IN_OPTS (prhs[3-nlhs])
#define NEWTEXTURE_OUT_TEXNAME (plhs[0])

/* rendertext */
#define RENDERTEXT_IN_POS (prhs[1])
#define RENDERTEXT_IN_HEIGHT (prhs[2])
#define RENDERTEXT_IN_TEXT (prhs[3])
#define RENDERTEXT_IN_XYALIGN (prhs[4])

/* toggle */
#define TOGGLE_IN_KV (prhs[1])

/* scissor */
#define SCISSOR_IN_XYWH (prhs[1])

/* deltextures */
#define DELTEXTURES_IN_TEXNAMES (prhs[1])

/* push/pop */
#define PUSH_IN_WHATAR (prhs[1])

/* get */
#define GET_IN_WHAT (prhs[1])
#define GET_OUT_VALUE (plhs[0])

/* set */
#define SET_IN_WHAT (prhs[1])
#define SET_IN_VALUE (prhs[2])

/* colormap */
#define COLORMAP_IN_COLORMAP (prhs[1])

/* newfragprog */
#define NEWFRAGPROG_IN_SHADERSRC (prhs[1])
#define NEWFRAGPROG_OUT_PROGID (plhs[0])
#define NEWFRAGPROG_OUT_UNIFORMS (plhs[1])

/* usefragprog */
#define USEFRAGPROG_IN_PROGID (prhs[1])

/* setuniform */
#define SETUNIFORM_IN_UNIFORMHANDLE (prhs[1])
#define SETUNIFORM_IN_VAL (prhs[2])

/* closewindow */
#define CLOSEWINDOW_IN_OURWINID (prhs[1])

/* readpixels */
#define READPIXELS_IN_XYWH (prhs[1])
#define READPIXELS_OUT_PIXELS (plhs[0])


/**** GET tokens ****/
/* Use negative values for GLC tokens since we might want to allow GL ones
 * later. This way there will be no collisions. */
#define GLC_GET_WINDOW_ID (-100)
#define GLC_GET_WINDOW_SIZE (-101)


enum glcalls_setcallback_
{
    CB_DISPLAY = 0,
    CB_RESHAPE,
    CB_KEYBOARD,
/*    CB_SPECIAL, */
    CB_MOUSE,
    CB_MOTION,
/*    CB_PASSIVEMOTION, */
    NUM_CALLBACKS,  /* must be last */
};

const char *glcall_callback_names[] = 
{
    "cb_display",
    "cb_reshape",
    "cb_keyboard",
/*    "cb_special", */
    "cb_mouse",
    "cb_motion",
/*    "cb_passivemotion", */
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
    GLC_RENDERTEXT,
    GLC_TOGGLE,
    GLC_SCISSOR,
    GLC_DELTEXTURES,
    GLC_PUSH,
    GLC_POP,
    GLC_GET,
    GLC_SET,
    GLC_COLORMAP,
    GLC_NEWFRAGPROG,
    GLC_USEFRAGPROG,
    GLC_SETUNIFORM,
    GLC_LEAVEMAINLOOP,
    GLC_CLOSEWINDOW,
    GLC_READPIXELS,
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
    "rendertext",
    "toggle",
    "scissor",
    "deltextures",
    "push",
    "pop",
    "get",
    "set",
    "colormap",
    "newfragprog",
    "usefragprog",
    "setuniform",
    "leavemainloop",
    "closewindow",
    "readpixels",
};


/*//////// DATA //////////*/

#define MAXACTIVEWINDOWS 16
#define MAXLIFETIMEWINDOWS 32768
#define MAXCBNAMELEN 63
static int curglutwinidx, curourwinidx=-1;
/* our window index (start at 0) --> GLUT window index (start at 1) */
static uint16_t glutwinidx[MAXACTIVEWINDOWS];
/* the reverse mapping, MAXLIFETIMEWINDOWS is practically Inf for that purpose */
static int8_t ourwinidx[MAXLIFETIMEWINDOWS];
static char callback_funcname[MAXACTIVEWINDOWS][NUM_CALLBACKS][MAXCBNAMELEN+1];
static int numentered = 0;  /* entermainloop entered? */

static struct windata_
{
    int32_t height, buttons;
} win[MAXACTIVEWINDOWS];

static GLuint cmaptexname, proginuse;
static GLfloat g_strokefontheight;

/*//////// UTIL //////////*/
static char errstr[256];

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
            mexErrMsgTxt(msg);                  \
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

static void cleanup_after_mainloop(void)
{
    memset(callback_funcname, 0, sizeof(callback_funcname));
    memset(ourwinidx, 0xff, sizeof(ourwinidx));
    memset(glutwinidx, 0, sizeof(glutwinidx));

    curglutwinidx = 0;
    curourwinidx = -1;

    memset(win, 0, sizeof(win));
}

#define MAX_CB_ARGS 5  /* REMEMBER */
static int check_callback(int CallbackID)
{
    return (curglutwinidx=glutGetWindow()) &&
        (curourwinidx=ourwinidx[curglutwinidx],  /* assumed >= 0 */
         callback_funcname[curourwinidx][CallbackID][0]!='\0');
}

static int call_mfile_callback(int callbackid, int numargs, const int *args)
{
    int err, i;
    mxArray *mxargs[MAX_CB_ARGS];

    mxAssert(numargs <= MAX_CB_ARGS, "numargs > MAX_CB_ARGS, update MAX_CB_ARGS macro!");

    for (i=0; i<numargs; i++)
        mxargs[i] = mxCreateDoubleScalar((double)args[i]);

#ifdef HAVE_OCTAVE
    mexSetTrapFlag(1);
    err = mexCallMATLAB(0,NULL, numargs,mxargs, callback_funcname[curourwinidx][callbackid]);
#else
    {
        const mxArray *ex;

        ex = mexCallMATLABWithTrap(0,NULL, numargs,mxargs, callback_funcname[curourwinidx][callbackid]);
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
                 callback_funcname[curourwinidx][callbackid], err);
        mexErrMsgTxt(buf);
    }

#endif
    if (err)
        glutLeaveMainLoop();

    return err;
}

static int getModifiers()
{
    return glutGetModifiers();
}

static void mouse_cb(int button, int state, int x, int y)
{
    int havecb = check_callback(CB_MOUSE);

    if (curglutwinidx && curourwinidx>=0)
    {
#if 0
        static const char *btns[3] = {"left","middle","right"};
        if (button>=0 && button<=2)
            printf("window %d: %s button %s\n", curourwinidx, btns[button], state==GLUT_DOWN?"down":"up");
#endif
        if (state==GLUT_DOWN)
            win[curourwinidx].buttons |= (1<<button);
        else
            win[curourwinidx].buttons &= ~(1<<button);
    }

    if (havecb)
    {
        int args[MAX_CB_ARGS] = {1<<button, (state==GLUT_DOWN), x, y, 0};
        args[4] = getModifiers();
        call_mfile_callback(CB_MOUSE, 5, args);
    }
}

static void motion_cb(int x, int y)
{
    if (check_callback(CB_MOTION))
    {
        int args[MAX_CB_ARGS] = {win[curourwinidx].buttons, x, y};

        if (win[curourwinidx].buttons==0)
        {
            printf("window %d motion_cb: win[%d].buttons==0\n", curourwinidx, curourwinidx);
            args[0] = 1<<GLUT_LEFT_BUTTON;  /* guess, ugh */
        }

        call_mfile_callback(CB_MOTION, 3, args);
    }
}

static void passivemotion_cb(int x, int y)
{
    if (check_callback(CB_MOTION))
    {
        int args[MAX_CB_ARGS] = {0, x, y};

        if (win[curourwinidx].buttons!=0)
        {
            printf("window %d passivemotion_cb: win[%d].buttons==%d\n",
                   curourwinidx, curourwinidx, win[curourwinidx].buttons);
            win[curourwinidx].buttons = 0;
        }

        call_mfile_callback(CB_MOTION, 3, args);
    }
}

static void keyboard_cb(unsigned char key, int x, int y)
{
    if (check_callback(CB_KEYBOARD))
    {
        int args[MAX_CB_ARGS] = {key, x, y, 0};
        args[3] = getModifiers();
        call_mfile_callback(CB_KEYBOARD, 4, args);
    }
}

static void special_cb(int key, int x, int y)
{
    if (check_callback(CB_KEYBOARD))
    {
        int args[MAX_CB_ARGS] = {key+65536, x, y, 0};
        args[3] = getModifiers();
        call_mfile_callback(CB_KEYBOARD, 4, args);
    }
}

static void display_cb(void)
{
    if (check_callback(CB_DISPLAY))
    {
        call_mfile_callback(CB_DISPLAY, 0, NULL);
    }
    else
    {
        if (glutGetWindow())
            glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    }

    if (glutGetWindow())
        glutSwapBuffers();
}

static void reshape_cb(int w, int h)
{
    int havecb = check_callback(CB_RESHAPE);

    if (curglutwinidx && curourwinidx>=0)
        win[curourwinidx].height = h;

    if (havecb)
    {
        int args[MAX_CB_ARGS] = {w, h};
        call_mfile_callback(CB_RESHAPE, 2, args);
    }
    else
    {
        if (glutGetWindow())
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
            int32_t i, fieldnum;

            for (i=0; i<NUM_GLCALLS; i++)
            {
                mxSetFieldByNumber(glcstruct, 0, i, createScalar(mxINT32_CLASS, &i));
            }

            for (i=0; i<NUM_CALLBACKS; i++)
            {
                fieldnum = mxAddField(glcstruct, glcall_callback_names[i]);
                mxSetFieldByNumber(glcstruct, 0, fieldnum, createScalar(mxINT32_CLASS, &i));
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

        int32_t i, pos[2], extent[2], winid, multisamplep=0, oglutwinidx;
        char windowname[80];

        if (nlhs > 1 || (nrhs != 4 && nrhs != 5))
            mexErrMsgTxt("Usage: [WINID =] GLCALL(glc.newwindow, POS, EXTENT, WINDOWNAME [, MULTISAMPLEP]),"
                         " create new window.");

        verifyparam(NEWWIN_IN_POS, "GLCALL: newwindow: POS", VP_VECTOR|VP_DOUBLE|(2<<VP_VECLEN_SHIFT));
        verifyparam(NEWWIN_IN_EXTENT, "GLCALL: newwindow: EXTENT", VP_VECTOR|VP_DOUBLE|(2<<VP_VECLEN_SHIFT));
        verifyparam(NEWWIN_IN_NAME, "GLCALL: newwindow: WINDOWNAME", VP_VECTOR|VP_CHAR);
        if (nrhs >= 5)
        {
            verifyparam(NEWWIN_IN_MULTISAMPLEP, "GLCALL: newwindow: MULTISAMPLEP", VP_SCALAR|VP_LOGICAL);
            multisamplep = *(uint8_t *)mxGetData(NEWWIN_IN_MULTISAMPLEP);
        }

        posptr = mxGetPr(NEWWIN_IN_POS);
        extentptr = mxGetPr(NEWWIN_IN_EXTENT);

        /* XXX: the magic constants here are pretty bad! */
        pos[0] = util_dtoi(posptr[0], 0, 1680, "GLCALL: newwindow: POS(1)");
        pos[1] = util_dtoi(posptr[1], 0, 1050, "GLCALL: newwindow: POS(2)");

        extent[0] = util_dtoi(extentptr[0], 1, 1680, "GLCALL: newwindow: EXTENT(1)");
        extent[1] = util_dtoi(extentptr[1], 1, 1050, "GLCALL: newwindow: EXTENT(2)");

        mxGetString(NEWWIN_IN_NAME, windowname, sizeof(windowname)-1);

        /* init!*/
        if (!inited)
        {
            char *argvdummy[1] = {"dummy_program_parameter"};
            int argcdummy = 1;

            glutInit(&argcdummy, argvdummy);  /* XXX */
            glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_CONTINUE_EXECUTION);

            /* only set initial display mode options when creating the first window
             * (glewInit() paranoia in Windows -- necessary?) */
            glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH | multisamplep*GLUT_MULTISAMPLE | GLUT_RGBA);

            g_strokefontheight = glutStrokeHeight(GLUT_STROKE_ROMAN);

            inited = 1;
        }
        /* else creating second+ window OR got here because of an error before having
         * entered the mainloop (when running the prog another time) */

        glutInitWindowPosition(pos[0], pos[1]);
        glutInitWindowSize(extent[0], extent[1]);

        oglutwinidx = glutGetWindow();

        /* look for a free ourwinidx slot and clean up window slots for windows
         * that got destroyed by clicking [x] instead of calling glcall(glc.closewindow) */
        for (i=0; i<MAXACTIVEWINDOWS; i++)
        {
            if (glutwinidx[i]==0)
                break;

            glutSetWindow(glutwinidx[i]);
            if (glutGetWindow() != glutwinidx[i])  /* window nonexistent! */
            {
                ourwinidx[glutwinidx[i]] = -1;
                glutwinidx[i] = 0;
            }
        }

        /* Reset the old current window before the potential error. This
         * doesn't matter normally, but users might protect this NEWWINDOW
         * call with a try/catch, where it does. */
        if (oglutwinidx > 0)
            glutSetWindow(oglutwinidx);

        if (i==MAXACTIVEWINDOWS)
            GLC_MEX_ERROR("GLCALL: newwindow: exceeded maximum active window count (%d)", MAXACTIVEWINDOWS);

        winid = glutCreateWindow(windowname);
        if (winid <= 0)
            mexErrMsgTxt("GLCALL: newwindow: failed creating window!");

        if (winid >= MAXLIFETIMEWINDOWS)
            GLC_MEX_ERROR("GLCALL: newwindow: exceeded maximum lifetime window count (%d)", MAXLIFETIMEWINDOWS);

        glutwinidx[i] = winid;
        ourwinidx[winid] = i;

        win[i].height = extent[1];

        /** set callbacks for the newly created window **/
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

        {
            GLenum err = glewInit();
            if (err != GLEW_OK)
            {
                /* glewInit failed, something is seriously wrong */
                GLC_MEX_ERROR("GLEW init failed: %s", glewGetErrorString(err));
            }
        }

        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
        glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);

        if (nlhs > 0)
        {
            /* +1 is a convenience for the Octave coder */
            int32_t ret_ourwidx = ourwinidx[winid]+1;
            NEWWIN_OUT_WINID = createScalar(mxINT32_CLASS, &ret_ourwidx);
        }
    }
    break;

    case GLC_DRAW:
    {
        /* TODO: pass color array */

        unsigned int primitivetype, doline;
        mwSize i, numdims, numtotalverts, numverts;

        mwSize colorsz;

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
        doline = primitivetype&16;
        primitivetype &= ~16;

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

        if (doline)
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        else
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        if (!texname)
        {
            glDisableClientState(GL_TEXTURE_COORD_ARRAY);
            glDisable(GL_TEXTURE_2D);
        }

        colorsz = 0;

        if (colorsar)
        {
            mwSize sz[2], szprod;

            verifyparam(colorsar, "GLCALL: draw: OPTSTRUCT.colors", VP_MATRIX|VP_DOUBLE);

            sz[0] = mxGetM(colorsar);
            sz[1] = mxGetN(colorsar);
            szprod = sz[0]*sz[1];

            if (szprod == 3 || szprod==4)
                colorsz = szprod;
            else
            {
                if ((sz[0] != 3 && sz[1] != 4) || sz[1] != numtotalverts)
                    mexErrMsgTxt("GLCALL: draw: OPTSTRUCT.colors must either have length 3 or 4,\n"
                                 "              or have 3 or 4 rows and size(VERTEXDATA,2) columns");
                colorsz = -sz[0];
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

        if (!colors || colorsz>0)
        {
            glDisableClientState(GL_COLOR_ARRAY);
            if (colorsz==3)
                glColor3dv(colors);
            else if (colorsz==4)
                glColor4dv(colors);
            else
                glColor3f(0.5f, 0.5f, 0.5f);
        }
        else
        {
            glEnableClientState(GL_COLOR_ARRAY);
            glColorPointer(-colorsz, GL_DOUBLE, 0, colors);
        }

        glEnableClientState(GL_VERTEX_ARRAY);
        glVertexPointer(numdims, vertdatatype, 0, mxGetPr(DRAW_IN_VERTEXDATA));

        if (!indicesar)
            glDrawArrays(primitivetype, 0, numverts);
        else
            glDrawElements(primitivetype, numverts, indicestype, indices);
    }
    break;

    case GLC_ENTERMAINLOOP:
    {
        if (nlhs != 0 || nrhs != 1)
            mexErrMsgTxt("Usage: GLCALL(glc.entermainloop), enter main loop.");

        if (numentered)
            return;
/*            mexErrMsgTxt("GLCALL: entermainloop: entered recursively!"); */
        numentered++;

        glutMainLoop();

        numentered--;
        inited = 0;

        cleanup_after_mainloop();
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
    break;

    case GLC_MULMATRIX:
    {
        uint32_t matrixmode;
        mwSize numel;

        static const char *usageText =
            "Usage: GLCALL(glc.mulmatrix [, GL.<MATRIX_MODE>], X), where X can be\n"
            "  * a 4x4 double matrix\n"
            "  * a length-3 vector, whose elements are passed to glTranslate, or\n"
            "  * a length-4 vector, whose elements are passed to glRotate."
            " Omitting the matrix mode means to multiply the modelview matrix.";

        if (nlhs != 0 || (nrhs != 2 && nrhs != 3))
            mexErrMsgTxt(usageText);

        if (nrhs == 2)
        {
            matrixmode = GL_MODELVIEW;
        }
        else
        {
            verifyparam(MULMATRIX_IN_MODE, "GLCALL: mulmatrix: MATRIX_MODE", VP_SCALAR|VP_UINT32);
            matrixmode = *(uint32_t *)mxGetData(MULMATRIX_IN_MODE);

            if (matrixmode != GL_MODELVIEW && matrixmode != GL_PROJECTION && matrixmode != GL_TEXTURE)
                mexErrMsgTxt("GLCALL: mulmatrix: MATRIX_MODE must be one of GL_MODELVIEW, "
                             "GL_PROJECTION or GL_TEXTURE");
        }

        if (!mxIsDouble(MULMATRIX_IN_MATRIX))
            mexErrMsgTxt("GLCALL: mulmatrix: X must have class double");

        numel = mxGetNumberOfElements(MULMATRIX_IN_MATRIX);
        /* XXX: no dim check this way, but also simpler */

        glMatrixMode(matrixmode);

        if (numel == 16)
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
    break;

    case GLC_SETCALLBACK:
    {
        int32_t callbackid, i, slen, glutwinidx;
        char tmpbuf[MAXCBNAMELEN+1], c;

        if (nlhs != 0 || nrhs != 3)
            mexErrMsgTxt("Usage: GLCALL(glc.setcallback, glc.cb_<callbacktype>, 'mfuncname'), pass '' to reset");

        glutwinidx = glutGetWindow();
        if (glutwinidx==0)
            mexErrMsgTxt("GLCALL: glc.setcallback: no current window!");

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

        memcpy(&callback_funcname[ourwinidx[glutwinidx]][callbackid], tmpbuf, sizeof(tmpbuf));
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
    break;

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
    break;

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

        if (numentered)
            mexErrMsgTxt("GLCALL.geterrstr should only be called after an error from the prompt");

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

        int32_t newtex = (nlhs == 1 && (nrhs == 2 || nrhs == 3));
        int32_t repltex = (nlhs == 0 && (nrhs == 3 || nrhs == 4));
        int32_t haveopts = (nlhs + nrhs == 4);

        GLint minfilt=GL_LINEAR, magfilt=GL_LINEAR;

        GLint internalFormat;
        GLenum format;
        GLenum type;

        mwSize width, height;

        if (!newtex && !repltex)
            mexErrMsgTxt("Usage: texname = GLCALL(glc.newtexture, texar [, opts]), texar must be 3xNxM uint8 or NxM single\n"
                         "   or: GLCALL(glc.newtexture, texar, texname [, opts])  to replace an earlier texture");

        if (mxIsUint8(NEWTEXTURE_IN_TEXAR))
        {
            /* RBG uint8 */
            verifyparam(NEWTEXTURE_IN_TEXAR, "GLCALL: newtexture: TEXAR", VP_DIMN|(3<<VP_DIMN_SHIFT));

            dimsizes = mxGetDimensions(NEWTEXTURE_IN_TEXAR);
            if (dimsizes[0] != 3 && dimsizes[0] != 4)
                mexErrMsgTxt("GLCALL: newtexture: TEXAR's first dimension must have length 3 (RGB) or 4 (RGBA)");

            width = dimsizes[1];
            height = dimsizes[2];

            if (dimsizes[0] == 3)
            {
                internalFormat = GL_RGB;
                format = GL_RGB;
            }
            else
            {
                internalFormat = GL_RGBA;
                format = GL_RGBA;
            }

            type = GL_UNSIGNED_BYTE;
        }
        else
        {
            int32_t numdims;

            /* grayscale float */
            verifyparam(NEWTEXTURE_IN_TEXAR, "GLCALL: newtexture: TEXAR", VP_SINGLE);

            numdims = mxGetNumberOfDimensions(NEWTEXTURE_IN_TEXAR);
            if (numdims != 2 && numdims != 3)
                mexErrMsgTxt("GLCALL: newtexture: TEXAR must have 2 or 3 dimensions with 'single' type");

            dimsizes = mxGetDimensions(NEWTEXTURE_IN_TEXAR);
            if (numdims == 2)
            {
                width = dimsizes[0];
                height = dimsizes[1];

                internalFormat = GL_LUMINANCE16;
                format = GL_LUMINANCE;
            }
            else
            {
                if (dimsizes[0] != 2)
                    mexErrMsgTxt("GLCALL: newtexture: TEXAR's first dimension must have length 2 (LUMINANCE_ALPHA)");

                width = dimsizes[1];
                height = dimsizes[2];

                internalFormat = GL_LUMINANCE16_ALPHA16;
                format = GL_LUMINANCE_ALPHA;
            }

            type = GL_FLOAT;
        }

        if (haveopts)
        {
            const mxArray *minmag_mxar;

            verifyparam(NEWTEXTURE_IN_OPTS, "GLCALL: newtexture: OPTS", VP_SCALAR|VP_STRUCT);

            minmag_mxar = mxGetField(NEWTEXTURE_IN_OPTS, 0, "minmag");
            if (minmag_mxar)
            {
                const int32_t *minmagptr;

                verifyparam(minmag_mxar, "GLCALL: newtexture: OPTS.minmag", VP_VECTOR|VP_UINT32|(2<<VP_VECLEN_SHIFT));
                minmagptr = mxGetData(minmag_mxar);

                if ((minmagptr[0]!=GL_NEAREST && minmagptr[0]!=GL_LINEAR) ||
                    (minmagptr[1]!=GL_NEAREST && minmagptr[1]!=GL_LINEAR))
                {
                    mexErrMsgTxt("GLCALL: newtexture: OPTS.minmag elements must be either GL.NEAREST or GL.LINEAR");
                }

                minfilt = minmagptr[0];
                magfilt = minmagptr[1];
            }
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
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minfilt);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magfilt);

        /* NOTE: CLAMP_TO_EDGE available if GL >= 1.2 */
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

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
    break;

    case GLC_RENDERTEXT:
    {
        const double *pos;
        double height;
        char *text;

        mwSize slen, i, vlen;
        double xyalign[2] = { 0.0, 0.0 };

        if (nlhs != 0 || (nrhs != 4 && nrhs != 5))
            mexErrMsgTxt("Usage: GLCALL(glc.rendertext, [x y [z]], height, text [, xyalign])");

        verifyparam(RENDERTEXT_IN_POS, "GLC: rendertext: POS", VP_VECTOR|VP_DOUBLE);
        vlen = mxGetNumberOfElements(RENDERTEXT_IN_POS);
        if (vlen != 2 && vlen != 3)
            mexErrMsgTxt("GLC: rendertext: POS must have length 2 or 3");
        pos = mxGetPr(RENDERTEXT_IN_POS);

        verifyparam(RENDERTEXT_IN_HEIGHT, "GLC: rendertext: HEIGHT", VP_SCALAR|VP_DOUBLE);
        height = *mxGetPr(RENDERTEXT_IN_HEIGHT);

        if (nrhs > 4)
        {
            const double *tmpxyalign;

            /* xalign: -1: align on left (default), 0: align centered, 1: align on right
             * yalign: -1: align on bottom (default), 0: align centered, 1: align on top
             * mapped to 0.0, 0.5 and 1.0 in the internal representation */
            verifyparam(RENDERTEXT_IN_XYALIGN, "GLC: rendertext: XYALIGN",
                        VP_VECTOR|VP_DOUBLE|(2<<VP_VECLEN_SHIFT));
            tmpxyalign = mxGetData(RENDERTEXT_IN_XYALIGN);
            if (tmpxyalign[0] >= 0.0)
                xyalign[0] = 0.5 + 0.5*(tmpxyalign[0] > 0.0);
            if (tmpxyalign[1] >= 0.0)
                xyalign[1] = 0.5 + 0.5*(tmpxyalign[1] > 0.0);
        }

        verifyparam(RENDERTEXT_IN_TEXT, "GLC: rendertext: TEXT", VP_VECTOR|VP_CHAR);
        text = mxArrayToString(RENDERTEXT_IN_TEXT);

        if (!text)
            mexErrMsgTxt("GLCALL: rendertext: Out of memory!");

        {
            glMatrixMode(GL_MODELVIEW);
            glPushMatrix();
            glPushAttrib(GL_CURRENT_BIT|GL_ENABLE_BIT|GL_COLOR_BUFFER_BIT);

            glColor3d(0.2, 0.2, 0.2);  /* XXX */
            glDisable(GL_TEXTURE_2D);

            glEnable(GL_LINE_SMOOTH);
            glEnable(GL_BLEND);
            glBlendEquation(GL_FUNC_ADD);

/*            glLoadIdentity(); */
            glTranslated(pos[0], pos[1], vlen==2 ? 0.0 : pos[2]);

            glScaled(height/119.05, height/119.05, height/119.05);

            if (xyalign[1] != 0.0)  /* y-align */
                glTranslated(0, -xyalign[1]*119.05, 0);

            slen = strlen(text);
            if (xyalign[0] != 0.0)
            {
                double strokeslen = (double)glutStrokeLength(GLUT_STROKE_ROMAN, (unsigned char *)text);

                /* TODO: proper newline handling */
                glTranslated(-xyalign[0]*(strokeslen + (slen-1)*(119.05/10.0)), 0, 0);
            }

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
    break;

    case GLC_TOGGLE:
    {
        int32_t numkeys, i, j;
        const int32_t *kv;

        static const GLenum accessible_enables[] = {
            GL_DEPTH_TEST, GL_SCISSOR_TEST, GL_BLEND, GL_POINT_SMOOTH, GL_LINE_SMOOTH,
            GL_LINE_STIPPLE, GL_POLYGON_SMOOTH,
        };

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
        {
            for (j=sizeof(accessible_enables)/sizeof(accessible_enables[0]) - 1; j>=0; j--)
                if ((GLenum)kv[2*i] == accessible_enables[j])
                    break;
            if (j < 0)
                GLC_MEX_ERROR("GLCALL: toggle: unsupported enable at position 2*%d", i);
        }

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
    break;

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
    break;

    case GLC_DELTEXTURES:
    {
        mwSize numtexs;

        if (nlhs != 0 || nrhs != 2)
            mexErrMsgTxt("Usage: GLCALL(glc.deltextures, [texname1 texname2 ...])");

        verifyparam(DELTEXTURES_IN_TEXNAMES, "GLCALL: deltextures: TEXNAMES", VP_VECTOR|VP_UINT32);
        numtexs = mxGetNumberOfElements(DELTEXTURES_IN_TEXNAMES);

        glDeleteTextures(numtexs, (uint32_t *)mxGetData(DELTEXTURES_IN_TEXNAMES));
    }
    break;

    case GLC_PUSH:
    case GLC_POP:
    {
        mwSize i, numwhats;
        const uint32_t *what;

        if (nlhs != 0 || nrhs != 2)
            mexErrMsgTxt("Usage: GLCALL(glc.push, [WHAT1 WHAT2 ...]);  WHAT* can be either\n"
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
    break;

    case GLC_GET:
    {
        int32_t what;

        if (nlhs != 1 || nrhs != 2)
            mexErrMsgTxt("Usage: VALUE = GLCALL(glc.get, WHAT)");

        verifyparam(GET_IN_WHAT, "GLCALL: set: WHAT", VP_SCALAR|VP_INT32);
        what = *(int32_t *)mxGetData(GET_IN_WHAT);

        switch (what)
        {
        case GLC_GET_WINDOW_ID:
        {
            int32_t glutwidx = glutGetWindow(), ret_ourwidx;

            if (glutwidx==0)
                ret_ourwidx = -1;
            else
                ret_ourwidx = ourwinidx[glutwidx]+1;

            GET_OUT_VALUE = createScalar(mxINT32_CLASS, &ret_ourwidx);
            return;
        }

        case GLC_GET_WINDOW_SIZE:
        {
            mxArray *whar;
            double *wh;
            int32_t glutwidx;

            glutwidx = glutGetWindow();
            if (glutwidx==0)
                mexErrMsgTxt("GLCALL: get WINDOW_SIZE: no active window!");

            whar = mxCreateNumericMatrix(1,2, mxDOUBLE_CLASS, mxREAL);
            wh = mxGetPr(whar);

            wh[0] = glutGet(GLUT_WINDOW_WIDTH);
            wh[1] = glutGet(GLUT_WINDOW_HEIGHT);

            GET_OUT_VALUE = whar;
            return;
        }

        default:
            mexErrMsgTxt("GLCALL: get: WHAT token unknown");
        }
    }
    break; /* not reached */

    case GLC_SET:
    {
        int32_t what;

        if (nlhs != 0 || nrhs != 3)
            mexErrMsgTxt("Usage: GLCALL(glc.set, WHAT, VALUE)");

        verifyparam(SET_IN_WHAT, "GLCALL: set: WHAT", VP_SCALAR|VP_INT32);
        what = *(int32_t *)mxGetData(SET_IN_WHAT);

        switch (what)
        {
        case GL_POINT_SIZE:
        {
            double value_d;

            verifyparam(SET_IN_VALUE, "GLCALL: set GL.POINT_SIZE: SIZE", VP_SCALAR|VP_DOUBLE);
            value_d = *mxGetPr(SET_IN_VALUE);

            /* potential undefined behavoiur when downcasting */
            glPointSize((GLfloat)value_d);
            break;
        }

        case GLC_GET_WINDOW_SIZE:
        {
            const double *wh_d;
            int32_t wh[2];

            verifyparam(SET_IN_VALUE, "GLC: set GL.WINDOW_SIZE: WH",
                        VP_VECTOR|VP_DOUBLE|(2<<VP_VECLEN_SHIFT));
            wh_d = mxGetPr(SET_IN_VALUE);

            wh[0] = util_dtoi(wh_d[0], 1, 16384, "GLC: setwindowsize: WH(1)");
            wh[1] = util_dtoi(wh_d[1], 1, 16384, "GLC: setwindowsize: WH(2)");

            glutReshapeWindow(wh[0], wh[1]);
            return;
        }

        case GL_BLEND_EQUATION:
        {
            verifyparam(SET_IN_VALUE, "GLC: set GL.BLEND_EQUATION: EQN", VP_SCALAR|VP_INT32);
            glBlendEquation(*(int32_t *)mxGetData(SET_IN_VALUE));
            break;  /* glGetError handles invalid enum vals */
        }

        case GL_LINE_STIPPLE_PATTERN:
        {
            verifyparam(SET_IN_VALUE, "GLC: set GL.LINE_STIPPLE_PATTERN: PATTERN", VP_SCALAR|VP_UINT16);
            glLineStipple(1, *(uint16_t *)mxGetData(SET_IN_VALUE));
            break;
        }

        default:
            mexErrMsgTxt("GLCALL: set: WHAT token unknown");
        }
    }
    break;

    case GLC_COLORMAP:
    {
        const mwSize *dimsizes;

        if (!GLEW_VERSION_1_3 && !GLEW_ARB_multitexture)
            mexErrMsgTxt("GLCALL: colormap needs OpenGL 1.3 or GL_ARB_multitexture");

        if (nlhs != 0 || nrhs != 2)
            mexErrMsgTxt("Usage: GLCALL(glc.colormap, COLORMAP), COLORMAP should be 3x256 uint8");

        verifyparam(COLORMAP_IN_COLORMAP, "GLCALL: colormap: COLORMAP", VP_UINT8|VP_DIMN|(2<<VP_DIMN_SHIFT));
        dimsizes = mxGetDimensions(COLORMAP_IN_COLORMAP);
        if (dimsizes[0] != 3 || dimsizes[1] != 256)
            mexErrMsgTxt("GLCALL: colormap: COLORMAP must be a 3x256 matrix");

        if (cmaptexname==0)
            glGenTextures(1, &cmaptexname);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_1D, cmaptexname);
        glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

        glTexImage1D(GL_TEXTURE_1D, 0, GL_RGB, 256,
                     0, GL_RGB, GL_UNSIGNED_BYTE, mxGetData(COLORMAP_IN_COLORMAP));

        glActiveTexture(GL_TEXTURE0);
    }
    break;

    case GLC_NEWFRAGPROG:
    {
        GLuint progId, fragshaderId;
        const char *fragshaderSrc;
        GLint status;

        if ((nlhs != 1 && nlhs != 2) || nrhs != 2)
            mexErrMsgTxt("Usage: PROGID [, UNIFORMS] = GLCALL(glc.newfragprog, FRAGSHADERSRC)");

        if (!GLEW_VERSION_2_0)
            mexErrMsgTxt("GLCALL: newfragprog: OpenGL 2.0 required!");

        verifyparam(NEWFRAGPROG_IN_SHADERSRC, "GLCALL: newfragprog: FRAGSHADERSRC", VP_VECTOR|VP_CHAR);

        progId = glCreateProgram();
        if (!progId)
            mexErrMsgTxt("GLCALL: newfragprog: Error creating program object");

        fragshaderId = glCreateShader(GL_FRAGMENT_SHADER);
        if (!fragshaderId)
            mexErrMsgTxt("GLCALL: newfragprog: Error creating fragment shader object");

        fragshaderSrc = mxArrayToString(NEWFRAGPROG_IN_SHADERSRC);
        glShaderSource(fragshaderId, 1, &fragshaderSrc, NULL);
        mxFree((char *)fragshaderSrc);
        fragshaderSrc = NULL;

        glCompileShader(fragshaderId);
        glGetShaderiv(fragshaderId, GL_COMPILE_STATUS, &status);
        if (status != GL_TRUE)
        {
            /* shader compilation failed */
            memcpy(errstr, "SH: ", 4);
            glGetShaderInfoLog(fragshaderId, sizeof(errstr)-4, NULL, errstr+4);

            mexErrMsgTxt(errstr);
        }

        glAttachShader(progId, fragshaderId);

        glLinkProgram(progId);
        glGetProgramiv(progId, GL_LINK_STATUS, &status);
        if (status != GL_TRUE)
        {
            /* program linking failed */
            memcpy(errstr, "PR: ", 4);
            glGetProgramInfoLog(progId, sizeof(errstr)-4, NULL, errstr+4);

            mexErrMsgTxt(errstr);
        }

        NEWFRAGPROG_OUT_PROGID = createScalar(mxUINT32_CLASS, &progId);

        if (nlhs > 1)
        {
            GLint i, numActiveUniforms, uniformLocation;
            char name[64], c;
            mxArray *uniformStructAr;
            int32_t j, k, fieldnum;
            uint32_t floatp, vecn, uniformHandle;

            GLint size;
            GLenum type;

            static const GLenum accepted_types[] = {
                GL_FLOAT, GL_FLOAT_VEC2, GL_FLOAT_VEC3, GL_FLOAT_VEC4,
                GL_INT, GL_INT_VEC2, GL_INT_VEC3, GL_INT_VEC4,
                GL_BOOL, GL_BOOL_VEC2, GL_BOOL_VEC3, GL_BOOL_VEC4,
            };

            glGetProgramiv(progId, GL_ACTIVE_UNIFORMS, &numActiveUniforms);

            uniformStructAr = mxCreateStructMatrix(1,1, 0,NULL);

            for (i=0; i<numActiveUniforms; i++)
            {
                glGetActiveUniform(progId, i, sizeof(name), NULL, &size, &type, name);

                if (!strncmp(name, "gl_", 3))
                    continue;

                for (j=sizeof(accepted_types)/sizeof(accepted_types[0])-1; j>=0; j--)
                    if (type == accepted_types[j])
                        break;
                if (j < 0)
                    continue;  /* not an accepted uniform type */
                floatp = (j <= 3);
                vecn = j&3;

                c = name[0];
                if (!(c>='A' && c<='Z') && !(c>='a' && c<='z'))
                    continue;  /* not a valid MATLAB variable/field name */

                for (j=1; (c=name[j]); j++)
                    if (!(c>='A' && c<='Z') && !(c>='a' && c<='z') && c!='_' && !(c>='0' && c<='9'))
                    {
                        if (c=='[')
                        {
                            k = j;
                            for (k=j+1; name[k]; k++)
                            {
                                if (!(name[k]>='0' && name[k]<='9'))
                                    break;
                            }

                            if (name[k]==']' && name[k+1]==0)
                                name[j] = 0;  /* only use the part without the [<number>] as the name */
                        }

                        break;
                    }
                if (name[j])
                    continue;  /* not a valid MATLAB variable/field name */

                fieldnum = mxAddField(uniformStructAr, name);
                if (fieldnum == -1)
                    mexErrMsgTxt("GLCALL: INTERNAL ERROR calling mxAddField");
                /* uniform IDs != uniform locations !! */
                uniformLocation = glGetUniformLocation(progId, name);
                if (uniformLocation & (0xe0000000u))
                    mexErrMsgTxt("GLCALL: INTERNAL ERROR: (uniformLocation & 0xe0000000u) != 0");

                uniformHandle = ((uint32_t)uniformLocation) | (floatp<<31) | (vecn<<29);
                mxSetFieldByNumber(uniformStructAr, 0, fieldnum,
                                   createScalar(mxUINT32_CLASS, &uniformHandle));

                NEWFRAGPROG_OUT_UNIFORMS = uniformStructAr;
            }
        }
    }
    break;

    case GLC_USEFRAGPROG:
    {
        GLuint progId;
        GLint cmap_uniform;

        if (nlhs != 0 || (nrhs != 2 && nrhs != 1))
            mexErrMsgTxt("Usage: GLCALL(glc.usefragprog [, PROGID])");

        if (nrhs == 1)
        {
            glUseProgram(0);
            proginuse = 0;
            return;
        }

        verifyparam(USEFRAGPROG_IN_PROGID, "GLCALL: usefragprog: PROGID", VP_SCALAR|VP_UINT32);

        progId = *(uint32_t *)mxGetData(USEFRAGPROG_IN_PROGID);

        glUseProgram(progId);
        proginuse = progId;

        cmap_uniform = glGetUniformLocation(progId, "cmap");
        if (cmap_uniform != -1)
            glUniform1i(cmap_uniform, 1);  /* texture unit 1: color map 1D texture */
    }
    break;

    case GLC_SETUNIFORM:
    {
        GLint uniformLocation, progId, size;
        int32_t sz;
        const void *data;
        size_t nelts;

        uint32_t uniformHandle;
        int32_t vptype;

        if (nlhs != 0 || nrhs != 3)
            mexErrMsgTxt("Usage: GLCALL(glc.setuniform, UNIFORMHANDLE, VAL)");

        glGetIntegerv(GL_CURRENT_PROGRAM, &progId);

        if (progId==0)
            mexErrMsgTxt("GLCALL: setuniform: a fragment program must be active!");

        verifyparam(SETUNIFORM_IN_UNIFORMHANDLE, "GLCALL: setuniform: UNIFORMHANDLE", VP_SCALAR|VP_UINT32);
        uniformHandle = *(uint32_t *)mxGetData(SETUNIFORM_IN_UNIFORMHANDLE);

        sz = ((uniformHandle>>29)&3)+1;
        vptype = (uniformHandle & 0x80000000u) ? VP_SINGLE : VP_INT32;
        uniformLocation = uniformHandle & 0x1fffffffu;

        verifyparam(SETUNIFORM_IN_VAL, "GLCALL: setuniform: VAL", VP_VECTOR|vptype);

        nelts = mxGetNumberOfElements(SETUNIFORM_IN_VAL);
        if (nelts % sz)
            GLC_MEX_ERROR("Uniform has %d-vector base elements, length"
                          " of VAL must be evenly divisible by %d", sz, sz);
        size = nelts/sz;

        data = mxGetData(SETUNIFORM_IN_VAL);

        /* setting the values if all OK */
        if (vptype == VP_SINGLE)
        {
            switch (sz)
            {
            case 1:
                glUniform1fv(uniformLocation, size, data);
                break;
            case 2:
                glUniform2fv(uniformLocation, size, data);
                break;
            case 3:
                glUniform3fv(uniformLocation, size, data);
                break;
            case 4:
                glUniform4fv(uniformLocation, size, data);
                break;
            }
        }
        else
        {
            switch (sz)
            {
            case 1:
                glUniform1iv(uniformLocation, size, data);
                break;
            case 2:
                glUniform2iv(uniformLocation, size, data);
                break;
            case 3:
                glUniform3iv(uniformLocation, size, data);
                break;
            case 4:
                glUniform4iv(uniformLocation, size, data);
                break;
            }
        }
    }
    break;

    case GLC_LEAVEMAINLOOP:
    {
        /* don't leave out the opportunity to mock the user even if the effect is the same :P */
        if (nlhs!=0 || nrhs!=1)
            mexErrMsgTxt("Usage: GLCALL(glc.leavemainloop)");

        glutLeaveMainLoop();
    }
    return;

    case GLC_CLOSEWINDOW:
    {
        int32_t ourwidx, glutwidx;

        if (nlhs!=0 || (nrhs!=1 && nrhs!=2))
            mexErrMsgTxt("Usage: GLCALL(glc.closewindow [, OURWINID])");

        if (nrhs == 2)
        {
            verifyparam(CLOSEWINDOW_IN_OURWINID, "GLCALL: closewindow: OURWINID", VP_SCALAR|VP_INT32);
            ourwidx = *(int32_t *)mxGetData(CLOSEWINDOW_IN_OURWINID);

            if (ourwidx <= 0 || ourwidx > MAXACTIVEWINDOWS)
                mexErrMsgTxt("GLCALL: closewindow: passed invalid window identifier");
            ourwidx--;

            glutwidx = glutwinidx[ourwidx];
        }
        else
        {
            glutwidx = glutGetWindow();
            if (glutwidx <= 0)
                GLC_MEX_ERROR("GLCALL: closewindow: glutGetWindow returned %d!", glutwidx);

            ourwidx = ourwinidx[glutwidx];
        }

        if (glutwidx > 0)  /* XXX: still might be nonexistent because closed by clicking [x] */
            glutDestroyWindow(glutwidx);

        ourwinidx[glutwidx] = -1;
        if (ourwidx < 0)
            mexErrMsgTxt("GLCALL: closewindow: INTERNAL ERROR: ourwidx < 0!");
        glutwinidx[ourwidx] = 0;
    }
    return;

    case GLC_READPIXELS:
    {
        int32_t winw, winh, x, y, w, h;
        mwSize thedims[3]={3, 0, 0};
        const double *xywh_d;

        mxArray *pixels_mxar;

        if (nlhs != 1 || (nrhs != 1 && nrhs != 2))
            mexErrMsgTxt("Usage: PIXELS = GLCALL(glc.readpixels [, XYWH]), PIXELS is 3xWxH");

        winw = glutGet(GLUT_WINDOW_WIDTH);
        winh = glutGet(GLUT_WINDOW_HEIGHT);

        if (nrhs == 1)
        {
            /* grab whole screen */
            x = 0;
            y = 0;
            w = winw;
            h = winh;
        }
        else
        {
            verifyparam(READPIXELS_IN_XYWH, "GLCALL: readpixels: XYWH", VP_VECTOR|VP_DOUBLE|(4<<VP_VECLEN_SHIFT));
            xywh_d = mxGetPr(READPIXELS_IN_XYWH);

            x = util_dtoi(xywh_d[0], 0, winw-1, "GLCALL: readpixels: XYWH(1)");
            y = util_dtoi(xywh_d[1], 0, winh-1, "GLCALL: readpixels: XYWH(2)");
            w = util_dtoi(xywh_d[2], 1, winw-x, "GLCALL: readpixels: XYWH(3)");
            h = util_dtoi(xywh_d[3], 1, winh-y, "GLCALL: readpixels: XYWH(4)");
        }

        thedims[1] = w;
        thedims[2] = h;

        pixels_mxar = mxCreateNumericArray(3, thedims, mxUINT8_CLASS, mxREAL);
        /* alloc'd OK or out of here */

        glPixelStorei(GL_PACK_ALIGNMENT, 1);
        glReadPixels(x, y, w, h, GL_RGB, GL_UNSIGNED_BYTE, mxGetData(pixels_mxar));

        READPIXELS_OUT_PIXELS = pixels_mxar;
    }
    break;

    }  /* end switch(cmd) */

    {
        static const char *errmacronames[] = {
            "INVALID_ENUM",  /* 0x500 */
            "INVALID_VALUE",
            "INVALID_OPERATION",
            "STACK_OVERFLOW",
            "STACK_UNDERFLOW",
            "OUT_OF_MEMORY",  /* 0x505 */
            "TABLE_TOO_LARGE",  /* 0x8031 */
        };

        int32_t err = glGetError();

        if (err != GL_NO_ERROR)
        {
            if (err == GL_TABLE_TOO_LARGE)  /* GL 1.2 */
                err = 0x506;
            if (err >= 0x500 && err <= 0x506)
                GLC_MEX_ERROR("glGetError returned GL_%s", errmacronames[err-0x500]);
            GLC_MEX_ERROR("glGetError returned 0x%x", err);
        }
    }
}
