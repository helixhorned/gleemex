
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include <GL/glew.h>
#include <GL/freeglut.h>

/********/
#include "mex.h"
#ifndef HAVE_OCTAVE
# include "matrix.h"
#endif

#ifdef DONT_HAVE_MWSIZE
# define mwSize unsigned long
#endif

#if _MSC_VER
#define snprintf _snprintf
#endif
/********/

/* Static assertions, based on source found in LuaJIT's src/lj_def.h. */
#define GLC_ASSERT_NAME2(name, line) name ## line
#define GLC_ASSERT_NAME(line) GLC_ASSERT_NAME2(eduke32_assert_, line)
#ifdef __COUNTER__
# define GLC_STATIC_ASSERT(cond) \
    extern void GLC_ASSERT_NAME(__COUNTER__)(int STATIC_ASSERTION_FAILED[(cond)?1:-1])
#else
# define GLC_STATIC_ASSERT(cond) \
    extern void GLC_ASSERT_NAME(__LINE__)(int STATIC_ASSERTION_FAILED[(cond)?1:-1])
#endif

#define ARRAY_SIZE(a) (sizeof(a)/sizeof((a)[0]))

/* 1 lhs, 0 rhs */
#define OUT_GLCSTRUCT (plhs[0])

/* > 0 rhs */
#define IN_COMMAND (prhs[0])

/* init */
#define NEWWIN_IN_POS (prhs[1])
#define NEWWIN_IN_EXTENT (prhs[2])
#define NEWWIN_IN_NAME (prhs[3])
#define NEWWIN_IN_OPTSTRUCT (prhs[4])
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

/* redisplay */
#define REDISPLAY_IN_NOWP (prhs[1])

/* newtexture */
#define NEWTEXTURE_IN_TEXAR (prhs[1])
#define NEWTEXTURE_IN_TEXNAME (prhs[2])
#define NEWTEXTURE_IN_OPTS (prhs[3-nlhs])
#define NEWTEXTURE_OUT_TEXNAME (plhs[0])

/* text */
#define TEXT_IN_POS (prhs[1])
#define TEXT_IN_HEIGHT (prhs[2])
#define TEXT_IN_TEXT (prhs[3])
#define TEXT_IN_XYALIGN (prhs[4])
#define TEXT_IN_OPTS (prhs[5])
#define TEXT_OUT_LENGTH (plhs[0])

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

/* fog */
#define FOG_IN_MODE (prhs[1])
#define FOG_IN_PARAM (prhs[2])
#define FOG_IN_COLOR (prhs[3])


/**** GET tokens ****/
/* Use negative values for GLC tokens since we might want to allow GL ones
 * later. This way there will be no collisions. KEEPINSYNC GLC_GET_TOKENS
 * in glconstants.m. */
#define GLC__WINDOW_ID (-100)
#define GLC__WINDOW_SIZE (-101)
#define GLC__MOUSE_POS (-102)  /* SET only */
#define GLC__MENU_ENABLE (-103)  /* SET only */
#define GLC__WINDOW_POS (-104)


enum glcalls_setcallback_
{
    CB_DISPLAY = 0,
    CB_RESHAPE,
    CB_KEYBOARD, /* for us, subsumes normal and special keyboard input */
    CB_MOUSE,
    CB_MOTION,  /* for us, subsumes both motion and passivemotion */
    CB_POSITION,
    NUM_CALLBACKS,  /* must be last */
};

const char *glcall_callback_names[] = 
{
    "cb_display",
    "cb_reshape",
    "cb_keyboard",
    "cb_mouse",
    "cb_motion",
    "cb_position",
};

GLC_STATIC_ASSERT(ARRAY_SIZE(glcall_callback_names) == NUM_CALLBACKS);


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
    GLC_REDISPLAY,
    GLC_GETERRSTR,
    GLC_NEWTEXTURE,
    GLC_TEXT,
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
    GLC_FOG,
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
    "redisplay",
    "geterrstr",
    "newtexture",
    "text",
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
    "fog",
};

GLC_STATIC_ASSERT(ARRAY_SIZE(glcall_names) == NUM_GLCALLS);


/******** DATA ********/

#define MAXACTIVEWINDOWS 32  /* max gleemex windows open at the same time */
#define MAXLIFETIMEWINDOWS 32768  /* max gleemex windows while top-level glcall is active */
#define MAXCBNAMELEN 63  /* max strlen of callback name */

static int g_curglutwidx=0, g_curourwidx=-1;
/* The mapping of our window indices (starting at 0) to GLUT window indices
 * (starting at 1). The value 0 is used as 'none'. */
static uint16_t g_glutwinidx[MAXACTIVEWINDOWS];
/* The reverse mapping. MAXLIFETIMEWINDOWS is practically infinite for that
 * purpose. The value -1 is used as 'none'. */
static int8_t g_ourwinidx[MAXLIFETIMEWINDOWS];
static char callback_funcname[MAXACTIVEWINDOWS][NUM_CALLBACKS][MAXCBNAMELEN+1];
static int numentered = 0;  /* entermainloop entered? */

static struct windata_
{
    int32_t height, buttons;
    mxArray *menus;  /* persistent */
    int32_t menubutton, menuid;
} win[MAXACTIVEWINDOWS];

/* The GL texture name for the color map 1D texture. */
static GLuint cmaptexname /*, proginuse */;
static GLfloat g_strokefontheight;

/******** UTIL ********/
static char errstr[256];

#ifndef HAVE_OCTAVE
static mxArray *exceptionar;
static char *errstrptr;

/* In MATLAB, we never jump out of the mex file via mexErrMsgTxt() because that
 * leaves windows hanging around or causes crashes. */
# define ourErrMsgTxt_(msg, retwhat) do { \
        if (numentered)                         \
        {                                       \
            if (errstrptr)                      \
                free(errstrptr);                \
            errstrptr = strdup(msg);            \
            glutLeaveMainLoop();                \
            /*mexErrMsgTxt(msg);*/              \
            return (retwhat);                   \
        }                                       \
        else                                    \
            return (retwhat);                   \
            /*mexErrMsgTxt(msg);*/              \
    } while (0);
# define ourErrMsgTxt(msg) ourErrMsgTxt_(msg, (void)0)
#else
# define ourErrMsgTxt_(msg, retwhat) mexErrMsgTxt(msg)
# define ourErrMsgTxt(msg) mexErrMsgTxt(msg)
#endif

#define GLC_MEX_ERROR_(retwhat, Text, ...) do { \
        snprintf(errstr, sizeof(errstr), Text, ## __VA_ARGS__); \
        ourErrMsgTxt_(errstr, retwhat); \
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
    VP_EMPTY_OK = 0x00000800,
    /* NOTE: May use flags up to
     *        0x00008000
     * (because of VP_VECLEN_MASK) */
    VP_SVM_MASK = 0x00000300,
    VP_DIMN_MASK = 0x0000f000,
    VP_DIMN_SHIFT = 12,

    VP_VECLEN_SHIFT = 16,
    VP_VECLEN_MASK = 0x00ff0000,  /* shift down 16 bits to get length */
};

static const mxClassID class_ids[] = {
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

#define GLC_MEX_ERROR_VP_(Text, ...) GLC_MEX_ERROR_(GL_TRUE, Text, ## __VA_ARGS__)

static int32_t arIsVector(const mxArray *ar, int32_t emptyok)
{
    /* XXX: It can be argued whether a 0x0 array is a vector or not.
     * Since we check for strings by seeing whether it's a "vector of chars",
     * this is practically relevant... */
    if (mxGetNumberOfDimensions(ar) == 2)
    {
        return !emptyok ?
            (mxGetN(ar)==1 || mxGetM(ar)==1) :
            (mxGetN(ar)<=1 || mxGetM(ar)<=1);
    }

    return 0;
}

/* return value: if vpflags contains VP_FP_TYPE or VP_INDEX_TYPE, either
 *   GL_FLOAT/GL_DOUBLE, or GL_UNSIGNED_BYTE/GL_UNSIGNED_INT, respectively
 * GL_TRUE if running on MATLAB and we didn't validate (on Octave, mexErrMsgTxt is called)
 * GL_FALSE if everything is OK else */
/* verifyparam_ret used only from mexFunction! XXX: not any more. */
static GLenum verifyparam_ret(const mxArray *ar, const char *arname, uint32_t vpflags)
{
    uint32_t vpclassidx = vpflags&VP_CLASS_MASK;

    if (!ar)
    {
        /* mainly for checking existence of struct fields */
        GLC_MEX_ERROR_VP_("%s must exist", arname);
    }

    /* check dimensionality requirements first */
    switch (vpflags & VP_SVM_MASK)
    {
    case VP_SCALAR:
        if (mxGetNumberOfElements(ar) != 1)
            GLC_MEX_ERROR_VP_("%s must be scalar", arname);
        break;

    case VP_VECTOR:
    {
        int bad = 1, wronglength=0;
        uint32_t reqdveclen;

        if (arIsVector(ar, vpflags&VP_EMPTY_OK))
        {
            if (vpflags&VP_VECLEN_MASK)
            {
                reqdveclen = (vpflags&VP_VECLEN_MASK)>>VP_VECLEN_SHIFT;

                if (mxGetNumberOfElements(ar) == reqdveclen)
                    bad = 0;
                else
                    wronglength = 1;
            }
            else
            {
                bad = 0;
            }
        }

        if (bad)
        {
            if (wronglength)
                GLC_MEX_ERROR_VP_("%s must be a length-%u vector", arname, reqdveclen);
            else
                GLC_MEX_ERROR_VP_("%s must be a vector", arname);
        }

        break;
    }

    case VP_MATRIX:
        if (mxGetNumberOfDimensions(ar) != 2)
            GLC_MEX_ERROR_VP_("%s must be a matrix", arname);
        break;

    case VP_DIMN:
    {
        mwSize reqddim = (vpflags&VP_DIMN_MASK)>>VP_DIMN_SHIFT;

        if (mxGetNumberOfDimensions(ar) != reqddim)
            GLC_MEX_ERROR_VP_("%s must have dimension %d", arname, reqddim);
        break;
    }

    }  /* switch */

    /* next, check data 'class', i.e. the base data type */
    if (vpclassidx)
    {
        if (vpclassidx < VP_FP_TYPE && class_ids[vpclassidx] != mxGetClassID(ar))
        {
            GLC_MEX_ERROR_VP_("%s must have class %s", arname, class_names[vpclassidx]);
        }
        else if (vpclassidx == VP_FP_TYPE)
        {
            if (mxIsDouble(ar))
                return GL_DOUBLE;
            else if (mxIsSingle(ar))
                return GL_FLOAT;
            GLC_MEX_ERROR_VP_("%s must be of floating-point type", arname);
        }
        else if (vpclassidx == VP_INDEX_TYPE)
        {
            if (mxIsUint32(ar))
                return GL_UNSIGNED_INT;
            else if (mxIsUint8(ar))
                return GL_UNSIGNED_BYTE;
            GLC_MEX_ERROR_VP_("%s must be of index type (uint8 or uint32)", arname);
        }
    }

    return GL_FALSE;
}

/* verifyparam used only from mexFunction! */
#ifdef HAVE_OCTAVE
static void verifyparam(const mxArray *ar, const char *arname, uint32_t vpflags)
{
    verifyparam_ret(ar, arname, vpflags);
}
#else
# define verifyparam(ar, arname, vpflags) do { \
        if (verifyparam_ret(ar, arname, vpflags)==GL_TRUE) \
            return; \
    } while (0)
#endif

static int32_t verify_callback_name(const mxArray *strmxAr, int32_t slen,
                                    char *tmpbuf)
{
    int32_t i;

    /* tmpbuf must have sizeof >= MAXCBNAMELEN+1 */

    mxGetString(strmxAr, tmpbuf, MAXCBNAMELEN+1);
    for (i=0; i<slen; i++)
    {
        char c = tmpbuf[i];

        if ((i==0 && (c=='_' || (c>='0' && c<='9'))) ||
            (c!='_' && !(c>='0' && c<='9') && !(c>='a' && 'c'<='z') && !(c>='A' && c<='Z')))
            return 1;
    }

    return 0;
}

static int32_t util_dtoi(double d, double minnum, double maxnum, const char *arname)
{
    if (!(d >= minnum && d <= maxnum))
        GLC_MEX_ERROR_(INT32_MIN, "%s must be between %d and %d", arname, (int)minnum, (int)maxnum);
    return (int32_t)d;
}

static mxArray *createScalar(mxClassID cid, const void *ptr)
{
    mxArray *tmpar = mxCreateNumericMatrix(1,1, cid, mxREAL);

    GLC_STATIC_ASSERT(sizeof(mxLogical)==1);

    switch (cid)
    {
    case mxLOGICAL_CLASS:
        *(int8_t *)mxGetData(tmpar) = !!*(int8_t *)ptr;
        break;
    /* case mxCHAR_CLASS: */
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
#ifndef HAVE_OCTAVE
        mxAssert(0, "INTRNAL ERROR");
#else
        ourErrMsgTxt("INTERNAL ERROR in createScalar: invalid cid!");
#endif
    }

    return tmpar;
}


/******** FUNCTIONS ********/

static void destroy_menu_struct(int32_t ourwidx)
{
    if (win[ourwidx].menus)
        mxDestroyArray(win[ourwidx].menus);
    win[ourwidx].menus = NULL;
}

static void cleanup_window(int32_t ourwidx)
{
    destroy_menu_struct(ourwidx);
    memset(&win[ourwidx], 0, sizeof(win[0]));
}

static void clear_window_indices(int32_t ourwidx, int32_t glutwidx)
{
    g_ourwinidx[glutwidx] = -1;
    if (ourwidx >= 0)
        g_glutwinidx[ourwidx] = 0;
}

static void cleanup_after_mainloop(void)
{
    int32_t i;

    memset(callback_funcname, 0, sizeof(callback_funcname));
    memset(g_ourwinidx, 0xff, sizeof(g_ourwinidx));
    memset(g_glutwinidx, 0, sizeof(g_glutwinidx));

    if (cmaptexname)
    {
        glDeleteTextures(1, &cmaptexname);
        cmaptexname = 0;
    }

    g_curglutwidx = 0;
    g_curourwidx = -1;

    for (i=0; i<MAXACTIVEWINDOWS; i++)
        cleanup_window(i);
}

/* GLUT->MATLAB callback functionality */
#define MAX_CB_ARGS 5  /* REMEMBER */
static int check_callback(int CallbackID)
{
    return (g_curglutwidx=glutGetWindow()) &&
        (g_curourwidx=g_ourwinidx[g_curglutwidx],  /* assumed >= 0 */
         callback_funcname[g_curourwidx][CallbackID][0]!='\0');
}

static int do_callback(int numargs, mxArray **mxargs, const char *cbfuncname)
{
    int err;

#ifdef HAVE_OCTAVE
    mexSetTrapFlag(1);
    err = mexCallMATLAB(0,NULL, numargs,mxargs, cbfuncname);
    if (err)
        glutLeaveMainLoop();
#else
    {
        mxArray *ex;

        ex = mexCallMATLABWithTrap(0,NULL, numargs,mxargs, cbfuncname);
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
        static char buf[128];
        snprintf(buf, sizeof(buf), "left main loop: error in callback %s: %d",
                 cbfuncname, err);

        ourErrMsgTxt_(buf, err);
    }
#endif

    return err;
}

static int call_mfile_callback(int callbackid, int numargs, const int *args)
{
    int i;
    mxArray *mxargs[MAX_CB_ARGS];

    mxAssert(numargs <= MAX_CB_ARGS, "numargs > MAX_CB_ARGS, update MAX_CB_ARGS macro!");

    for (i=0; i<numargs; i++)
        mxargs[i] = mxCreateDoubleScalar((double)args[i]);

    return do_callback(numargs, mxargs, callback_funcname[g_curourwidx][callbackid]);
}

/* callbacks for specific events */
static int getModifiers()
{
    return glutGetModifiers();
}

static void mouse_cb(int button, int state, int x, int y)
{
    int havecb = check_callback(CB_MOUSE);

    if (g_curglutwidx && g_curourwidx>=0)
    {
#if 0
        static const char *btns[3] = {"left","middle","right"};
        if (button>=0 && button<=2)
            printf("window %d: %s button %s\n", g_curourwidx, btns[button], state==GLUT_DOWN?"down":"up");
#endif
        /* Save which buttons are pressed or released for the mouse motion events. */
        if (state==GLUT_DOWN)
            win[g_curourwidx].buttons |= (1<<button);
        else
            win[g_curourwidx].buttons &= ~(1<<button);
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
        int args[MAX_CB_ARGS] = {win[g_curourwidx].buttons, x, y};

        if (win[g_curourwidx].buttons==0)
        {
            /* We hit an inconsistency: we're in the motion event whereas
             * win[].buttons tells us that none is pressed. */
            printf("window %d motion_cb: win[].buttons==0\n", g_curourwidx);
            args[0] = 1<<GLUT_LEFT_BUTTON;  /* guess that it's the left button, ugh */
        }

        call_mfile_callback(CB_MOTION, 3, args);
    }
}

static void passivemotion_cb(int x, int y)
{
    if (check_callback(CB_MOTION))
    {
        int args[MAX_CB_ARGS] = {0, x, y};

        if (win[g_curourwidx].buttons!=0)
        {
            /* We hit an inconsistency: we're in the passive motion event
             * whereas win[].buttons tells us that one is pressed. */
            printf("window %d passivemotion_cb: win[].buttons==%d\n",
                   g_curourwidx, win[g_curourwidx].buttons);
            win[g_curourwidx].buttons = 0;
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
    /* TODO: make the M-file display function need to define an output arg,
     *  checking it will determine whether to swap buffers ? */

    if (check_callback(CB_DISPLAY))
    {
        if (call_mfile_callback(CB_DISPLAY, 0, NULL))
            return;
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

    if (g_curglutwidx && g_curourwidx>=0)
        win[g_curourwidx].height = h;

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

static void position_cb(int x, int y)
{
    if (check_callback(CB_POSITION))
    {
        int args[MAX_CB_ARGS] = {x, y};
        call_mfile_callback(CB_POSITION, 2, args);
    }
}

/* Window closure/destruction callback. Also runs when closing a window using
 * its [x] button. */
static void close_cb()
{
    /* This is the soon to be destroyed window's index, see
     * http://sourceforge.net/mailarchive/message.php?msg_id=30131865 */
    int32_t glutwidx=glutGetWindow(), ourwidx;

    mxAssert((unsigned)glutwidx < MAXLIFETIMEWINDOWS, "XXX");
    ourwidx = g_ourwinidx[glutwidx];

/*    mxAssert((unsigned)ourwidx < MAXACTIVEWINDOWS, "XXX"); */
    if (ourwidx >= 0)
        cleanup_window(ourwidx);
    clear_window_indices(ourwidx, glutwidx);
/*    printf("Closed window GLUT %d, our %d\n", glutwidx, ourwidx); */
}

#ifndef HAVE_OCTAVE
# define RETIFERR(x) do { if (x==INT32_MIN) return; } while (0)
#else
# define RETIFERR(x) ((void)(0))
#endif

#define GLC_MEX_ERROR(Text, ...) GLC_MEX_ERROR_((void)0, Text, ## __VA_ARGS__)

/******** MENUS ********/
/* menu struct walker */
static GLenum walk_menu_struct(const mxArray *menuar, int32_t *numleaves,
                               int (begin_func(const mxArray *cbf)),
                               GLenum (perentry_func(const mxArray *labelar, int32_t leafp,
                                                     int32_t k, const mxArray *cbf)),
                               int32_t nesting_depth,
                               const mxArray *cbfunc)
{
    const mxArray *entries;

    if (verifyparam_ret(menuar, "menu", VP_SCALAR|VP_STRUCT))
        return GL_TRUE;

    if (nesting_depth > 0)
    {
        /* SUBMENU_LABEL */
        const mxArray *sublabel = mxGetField(menuar, 0, "label");

        /* er, shoehorn the label of the submenu into the submenu's struct... */
        if (verifyparam_ret(sublabel, "menu.entries(i).label, for entries(i) a submenu,",
                            VP_VECTOR|VP_CHAR))
            return GL_TRUE;
    }

    {
        const mxArray *tmpcbfunc = mxGetField(menuar, 0, "cbfunc");

        /* the menu root must have a 'cbfunc' field, the children may have one */
        if (nesting_depth == 0 || tmpcbfunc!=NULL)
        {
            if (verifyparam_ret(tmpcbfunc, "menu.cbfunc", VP_VECTOR|VP_CHAR))
                return GL_TRUE;

            /* This walk_menu_struct() arg will hold the callback function name
             * at the deepest possible level, and always be non-NULL because of
             * the requirement that the root menu must have a 'cbfunc' field. */
            cbfunc = tmpcbfunc;
        }
    }

    entries = mxGetField(menuar, 0, "entries");
    if (verifyparam_ret(entries, "menu.entries", VP_VECTOR|VP_CELL))
        return GL_TRUE;

    {
        const int32_t numentries = mxGetNumberOfElements(entries);
        int32_t i, newmenu=0;

        if (numentries==0)
            GLC_MEX_ERROR_(GL_TRUE, "menu.entries must not be empty");

        if (begin_func)
        {
            newmenu = begin_func(cbfunc);

            if (newmenu <= 0)
                GLC_MEX_ERROR_(GL_TRUE, "couldn't create menu (begin_func returned <=0)");
        }

        for (i=0; i<numentries; i++)
        {
            const mxArray *label_or_submenu = mxGetCell(entries, i);
            int leafp;

            if (mxIsStruct(label_or_submenu))
            {
                /* recurse! */
                if (walk_menu_struct(label_or_submenu, numleaves, begin_func,
                                     perentry_func, nesting_depth+1, cbfunc))
                    return GL_TRUE;

                leafp = 0;
            }
            else
            {
                if (verifyparam_ret(label_or_submenu, "menu.entries(i)", VP_VECTOR|VP_CHAR))
                    return GL_TRUE;

                leafp = 1;
                (*numleaves)++;
            }

            if (perentry_func)
                perentry_func(label_or_submenu, leafp, leafp ? *numleaves : newmenu, cbfunc);
        }
    }

    /* Everything is OK, we can use that menu */
    return GL_FALSE;
}

/* calling back from GLUT to the M-function */
static const mxArray *menulabelar;
static char menucbfname[MAXCBNAMELEN+1];

static GLenum callmenu_perentry(const mxArray *labelar, int32_t leafp, int32_t k, const mxArray *cbfunc)
{
    if (leafp)
    {
        if (k == 0)
        {
            /* found matching menu entry */
            mxGetString(cbfunc, menucbfname, sizeof(menucbfname));
            menulabelar = labelar;  /* label comes from a persistent variable */
        }
    }

    return GL_FALSE;
}

static void menu_callback(int cbval)
{
    int32_t glutwidx = glutGetWindow();

    if (glutwidx > 0)
    {
        const mxArray *menu;
        int32_t ourwidx = g_ourwinidx[glutwidx], ocbval=cbval;

        mxAssert(ourwidx >= 0, "menu callback: g_ourwinidx[glutwidx] < 0!");

        menu = win[ourwidx].menus;
        mxAssert(menu, "menu callback: win[ourwidx].menus == NULL!");

        cbval = -cbval;
        menulabelar = NULL;

        if (walk_menu_struct(menu, &cbval, NULL, &callmenu_perentry, 0, NULL))
            return;

        if (menulabelar)
        {
            mxArray *ocbvalar = mxCreateDoubleScalar(ocbval);
            mxArray *args[2] = { (mxArray *)menulabelar, ocbvalar };

            /* call the menu callback with the menu label as arg */
            do_callback(2, args, menucbfname);
        }
    }
}

/* menu creation */
static int createmenu_begin(const mxArray *cbfunc)
{
    (void)cbfunc;
    return glutCreateMenu(menu_callback);
}

static GLenum createmenu_perentry(const mxArray *labelar, int32_t leafp, int32_t k, const mxArray *cbfunc)
{
    char *label;
    (void)cbfunc;

    if (leafp)
    {
        label = mxArrayToString(labelar);
    }
    else
    {
        /* existence verified in SUBMENU_LABEL */
        const mxArray *sublabelar = mxGetField(labelar, 0, "label");
        label = mxArrayToString(sublabelar);
    }

    if (!label)
        GLC_MEX_ERROR_(GL_TRUE, "Out of memory in menu creation!");

    if (leafp)
    {
        glutAddMenuEntry(label, k);
    }
    else
    {
        /* Submenu creation. Here, the children have been handled.
         * Current menu is the child, k is the parent. */

        int childmenu = glutGetMenu();
        glutSetMenu(k);
        glutAddSubMenu(label, childmenu);
    }

    mxFree(label);

    return GL_FALSE;
}


/**************** THE MAIN MEX FUNCTION ****************/
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
            ourErrMsgTxt("Usage: GLCALLINFO_STRUCT = GLCALL(), query available subcommands.");
    }

    /* nrhs > 0 */

    verifyparam(IN_COMMAND, "COMMAND", VP_SCALAR|VP_INT32);
    cmd = *(int32_t *)mxGetData(IN_COMMAND);

    /* XXX: GLUT functions may be called with uninited freeglut when passing GLC_GET. */
    if (!inited && !(cmd == GLC_NEWWINDOW || cmd == GLC_GET
#ifndef HAVE_OCTAVE
                     || cmd == GLC_GETERRSTR
#endif
            ))
        ourErrMsgTxt("GLCALL: Must call 'newwindow' subcommand to initialize GLUT before any other command!");

    /*********/
    switch (cmd)
    {
    case GLC_NEWWINDOW:
    {
        double *posptr, *extentptr;

        int32_t i, pos[2], extent[2], winid, oglutwidx;
        int32_t multisamplep=0, subwindowp=0;
        char windowname[80];

        mxArray *menus = NULL;

        if (nlhs > 1 || (nrhs != 4 && nrhs != 5))
            ourErrMsgTxt("Usage: [WINID =] GLCALL(glc.newwindow, POS, EXTENT, WINDOWNAME [, OPTSTRUCT]),"
                         " create new window.");

        verifyparam(NEWWIN_IN_POS, "GLCALL: newwindow: POS", VP_VECTOR|VP_DOUBLE|(2<<VP_VECLEN_SHIFT));
        verifyparam(NEWWIN_IN_EXTENT, "GLCALL: newwindow: EXTENT", VP_VECTOR|VP_DOUBLE|(2<<VP_VECLEN_SHIFT));
        verifyparam(NEWWIN_IN_NAME, "GLCALL: newwindow: WINDOWNAME", VP_VECTOR|VP_CHAR);
        if (nrhs >= 5)
        {
            mxArray *tmpar;

            verifyparam(NEWWIN_IN_OPTSTRUCT, "GLCALL: newwindow: OPTSTRUCT", VP_SCALAR|VP_STRUCT);

            tmpar = mxGetField(NEWWIN_IN_OPTSTRUCT, 0, "multisample");
            if (tmpar)
            {
                verifyparam(tmpar, "GLCALL: newwindow: OPTSTRUCT.multisample", VP_SCALAR|VP_LOGICAL);
                multisamplep = !!*(int8_t *)mxGetData(tmpar);
            }

            tmpar = mxGetField(NEWWIN_IN_OPTSTRUCT, 0, "subwindow");
            if (tmpar)
            {
                verifyparam(tmpar, "GLCALL: newwindow: OPTSTRUCT.subwindow", VP_SCALAR|VP_LOGICAL);
                subwindowp = !!*(int8_t *)mxGetData(tmpar);
            }

            /* Menu creation */
            tmpar = mxGetField(NEWWIN_IN_OPTSTRUCT, 0, "menus");
            if (tmpar)
            {
                int32_t numleaves=0;

                if (walk_menu_struct(tmpar, &numleaves, NULL, NULL, 0, NULL))
                    return;

                /* Everything is OK, we can use that menu */
                menus = tmpar;
            }
        }

        posptr = mxGetPr(NEWWIN_IN_POS);
        extentptr = mxGetPr(NEWWIN_IN_EXTENT);

        /* XXX: the magic constants here are still pretty bad! */
        pos[0] = util_dtoi(posptr[0], 0, 8000, "GLCALL: newwindow: POS(1)");
        pos[1] = util_dtoi(posptr[1], 0, 4000, "GLCALL: newwindow: POS(2)");

        extent[0] = util_dtoi(extentptr[0], 1, 8000, "GLCALL: newwindow: EXTENT(1)");
        extent[1] = util_dtoi(extentptr[1], 1, 4000, "GLCALL: newwindow: EXTENT(2)");

        RETIFERR(pos[0]);
        RETIFERR(pos[1]);
        RETIFERR(extent[0]);
        RETIFERR(extent[1]);

        mxGetString(NEWWIN_IN_NAME, windowname, sizeof(windowname)-1);

        if (subwindowp && !inited)
            ourErrMsgTxt("Must have initialized GLCALL (i.e. created a"
                         " top-level window) before creating subwindow");

        /* init!*/
        if (!inited)
        {
            /* Pass a dummy arg vector to glutInit(). Trailing NULL is so that
             * it's consistent with how C99 mandates main()'s argv[]. */
            char *argvdummy[1] = { NULL };
            int argcdummy = 0;

            glutInit(&argcdummy, argvdummy);
            glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_CONTINUE_EXECUTION);

            /* only set initial display mode options when creating the first window
             * (glewInit() paranoia in Windows -- necessary?) */
            glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH | (multisamplep*GLUT_MULTISAMPLE) | GLUT_RGBA);

            g_strokefontheight = glutStrokeHeight(GLUT_STROKE_ROMAN);

            inited = 1;
        }
        /* else creating second+ window OR got here because of an error before having
         * entered the mainloop (when running the prog another time) */

        if (!subwindowp)
        {
            glutInitWindowPosition(pos[0], pos[1]);
            glutInitWindowSize(extent[0], extent[1]);
        }

        oglutwidx = glutGetWindow();

        /* Look for a free g_ourwinidx slot and clean up window slots for windows
         * that aren't alive any more, for example because we bailed out due to
         * an error in the M code earlier. */
        for (i=0; i<MAXACTIVEWINDOWS; i++)
        {
            if (g_glutwinidx[i]==0)
                break;

            glutSetWindow(g_glutwinidx[i]);
            if (glutGetWindow() != g_glutwinidx[i])  /* window nonexistent! */
            {
                cleanup_window(i);
                clear_window_indices(i, g_glutwinidx[i]);
                break;
            }
        }

        /* Reset the old current window before the potential error. This
         * doesn't matter normally, but users might protect this NEWWINDOW
         * call with a try/catch, where it does. XXX: really?*/
        if (oglutwidx > 0)
            glutSetWindow(oglutwidx);

        if (i==MAXACTIVEWINDOWS)
            GLC_MEX_ERROR("GLCALL: newwindow: exceeded maximum active window count (%d)", MAXACTIVEWINDOWS);

        if (subwindowp)
            winid = glutCreateSubWindow(oglutwidx, pos[0],pos[1], extent[0],extent[1]);
        else
            winid = glutCreateWindow(windowname);

        if (winid <= 0)
            ourErrMsgTxt("GLCALL: newwindow: failed creating window!");

        if (winid >= MAXLIFETIMEWINDOWS)
            GLC_MEX_ERROR("GLCALL: newwindow: exceeded maximum lifetime window count (%d)", MAXLIFETIMEWINDOWS);

        g_glutwinidx[i] = winid;
        g_ourwinidx[winid] = i;

        win[i].height = extent[1];

        if (menus)
        {
            int32_t numleaves=0;
            int32_t button = GLUT_RIGHT_BUTTON;
            const mxArray *buttonar;

            /* Save off the 'menus' struct passed from above, clearing a
             * potential old one first. Note that this is not problematic with
             * aliasing (same menu struct passed to two different windows),
             * because we duplicate the mxArrays below. */
            destroy_menu_struct(i);

            /* We must not make the original, input argument mxArray persistent
             * (e.g. on Octave: crash) */
            win[i].menus = mxDuplicateArray(menus);
            mexMakeArrayPersistent(win[i].menus);

            /* XXX: currently, neither the mxArrays nor the GLUT menus are
             * freed anywhere. (For the mxArrays: except above) */

            if (walk_menu_struct(menus, &numleaves, &createmenu_begin,
                                 &createmenu_perentry, 0, NULL))
                return;

            buttonar = mxGetField(menus, 0, "button");
            if (buttonar)
            {
                double buttond;

                verifyparam(buttonar, "GLCALL: newwindow: MENUS.button", VP_SCALAR|VP_DOUBLE);
                buttond = *(double *)mxGetData(buttonar);

                if (buttond==1)
                    button = GLUT_LEFT_BUTTON;
                else if (buttond==2)
                    button = GLUT_MIDDLE_BUTTON;
                else if (buttond==4)
                    button = GLUT_RIGHT_BUTTON;
            }

            win[i].menubutton = button;
            win[i].menuid = glutGetMenu();
            if (win[i].menuid <= 0)
                ourErrMsgTxt("GLCALL: newwindow: INTERNAL ERROR, menu ID is <= 0!");

            glutAttachMenu(button);
        }

        /** set callbacks for the newly created window **/
        /* these two are always there */
        glutDisplayFunc(display_cb);
        glutReshapeFunc(reshape_cb);
        glutPositionFunc(position_cb);
        glutCloseFunc(close_cb);

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
        /* probably default anyway, but not mentioned in glFog doc: */
        if (GLEW_VERSION_1_4)
            glFogi(GL_FOG_COORD_SRC, GL_FRAGMENT_DEPTH);

        if (nlhs > 0)
        {
            /* +1 is a convenience for the Octave/MATLAB coder */
            int32_t ret_ourwidx = g_ourwinidx[winid]+1;
            NEWWIN_OUT_WINID = createScalar(mxINT32_CLASS, &ret_ourwidx);
        }
    }
    break;

    case GLC_DRAW:
    {
        /* QUESTIONS/TODO:
         *  When passing color array with 4 components, enable blending temporarily?
         *  When passing a texture, make the default color white?
         *  Some easy means of specifying a rect? (using glRect)
         *  When a lot of data will need to be drawn: Buffer Objects...  */

        uint32_t primitivetype, doline;
        mwSize i, numdims, numtotalverts, numverts;

        mwSize colorsz;
        const mxArray *colorsar=NULL, *indicesar=NULL;
        const double *colors=NULL;

        const void *indices=NULL;  /* uint8_t or uint32_t */
        GLuint texname = 0;
        GLenum vertdatatype, indicestype=0 /* compiler-happy */;

        if (nlhs != 0 || (nrhs != 3 && nrhs != 4))
            ourErrMsgTxt("Usage: GLCALL(glc.draw, GL.<PRIMITIVE_TYPE>, VERTEXDATA [, OPTSTRUCT])");

        if (!mxIsUint32(DRAW_IN_PRIMITIVETYPE) || mxGetNumberOfElements(DRAW_IN_PRIMITIVETYPE)!=1)
            ourErrMsgTxt("GLCALL: draw: PRIMITIVE_TYPE must be a uint32 scalar");
        primitivetype = *(uint32_t *)mxGetData(DRAW_IN_PRIMITIVETYPE);

        /* XXX: This seems like an ugly hack. */
        doline = primitivetype&16;
        primitivetype &= ~16;

        if (!(/*primitivetype >= GL_POINTS &&*/ primitivetype <= GL_POLYGON))
            ourErrMsgTxt("GLCALL: draw: invalid GL primitive type");

        vertdatatype = verifyparam_ret(DRAW_IN_VERTEXDATA, "GLCALL: draw: VERTEXDATA", VP_MATRIX|VP_FP_TYPE);
#ifndef HAVE_OCTAVE
        if (vertdatatype == GL_TRUE)
            return;
#endif
        numdims = mxGetM(DRAW_IN_VERTEXDATA);
        numtotalverts = mxGetN(DRAW_IN_VERTEXDATA);

        if (!(numdims >=2 && numdims <= 4))
            ourErrMsgTxt("GLCALL: draw: VERTEXDATA must have between 2 and 4 rows (number of coordinates)");

        if (nrhs > 3)
        {
            const mxArray *texar, *texcoordsar;

            verifyparam(DRAW_IN_OPTSTRUCT, "GLCALL: draw: OPTSTRUCT", VP_SCALAR|VP_STRUCT);

            colorsar = mxGetField(DRAW_IN_OPTSTRUCT, 0, "colors");
            indicesar = mxGetField(DRAW_IN_OPTSTRUCT, 0, "indices");
            texar = mxGetField(DRAW_IN_OPTSTRUCT, 0, "tex");
            if (texar)
            {
                GLenum tcdatatype;

                verifyparam(texar, "GLCALL: draw: OPTSTRUCT.tex", VP_SCALAR|VP_UINT32);

                texname = *(uint32_t *)mxGetData(texar);
                if (texname == 0)
                    ourErrMsgTxt("GLCALL: draw: OPTSTRUCT.tex must be greater zero");

                texcoordsar = mxGetField(DRAW_IN_OPTSTRUCT, 0, "texcoords");
                if (!texcoordsar)
                    ourErrMsgTxt("GLCALL: draw: When passing OPTSTRUCT.tex, must also have OPTSTRUCT.texcoords");

                tcdatatype = verifyparam_ret(texcoordsar, "GLCALL: draw: OPTSTRUCT.texcoords", VP_MATRIX|VP_FP_TYPE);
#ifndef HAVE_OCTAVE
                if (tcdatatype == GL_TRUE)
                    return;
#endif
                if (mxGetM(texcoordsar) != 2 || mxGetN(texcoordsar) != (unsigned long)numtotalverts)
                    ourErrMsgTxt("GLCALL: draw: OPTSTRUCT.texcoords must have "
                                 "2 rows and size(VERTEXDATA,2) columns");

                glEnable(GL_TEXTURE_2D);
                glBindTexture(GL_TEXTURE_2D, texname);

                glEnableClientState(GL_TEXTURE_COORD_ARRAY);
                glTexCoordPointer(2, tcdatatype, 0, mxGetData(texcoordsar));
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
                    ourErrMsgTxt("GLCALL: draw: OPTSTRUCT.colors must either have length 3 or 4,\n"
                                 "              or have 3 or 4 rows and size(VERTEXDATA,2) columns");
                colorsz = -sz[0];
            }

            colors = (const double *)mxGetData(colorsar);
        }

        if (indicesar)
        {
            indicestype = verifyparam_ret(indicesar, "GLCALL: draw: OPTSTRUCT.indices", VP_VECTOR|VP_INDEX_TYPE);

            numverts = mxGetNumberOfElements(indicesar);
            indices = mxGetData(indicesar);

            if (numverts==0)
                return;

            /* bounds check! */
            for (i=0; i<numverts; i++)
            {
                if (((indicestype==GL_UNSIGNED_INT) ?
                     ((uint32_t *)indices)[i] : ((uint8_t *)indices)[i]) >= (unsigned)numverts)
                    ourErrMsgTxt("GLCALL: draw: INDICES must contain values between 0 and size(INDICES,2)-1");
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
            ourErrMsgTxt("Usage: GLCALL(glc.entermainloop), enter main loop.");

        if (numentered)
            return;
/*            ourErrMsgTxt("GLCALL: entermainloop: entered recursively!"); */
        numentered++;

        glutMainLoop();

        if (numentered > 0)
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
            ourErrMsgTxt(usageText);

        verifyparam(SETMATRIX_IN_MODE, "GLCALL: setmatrix: MATRIX_MODE", VP_SCALAR|VP_UINT32);
        matrixmode = *(uint32_t *)mxGetData(SETMATRIX_IN_MODE);

        if (matrixmode != GL_MODELVIEW && matrixmode != GL_PROJECTION)
            ourErrMsgTxt("GLCALL: setmatrix: MATRIX_MODE must be one of GL_MODELVIEW or GL_PROJECTION");

        if (!mxIsDouble(SETMATRIX_IN_MATRIX))
            ourErrMsgTxt("GLCALL: setmatrix: X must have class double");

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
                ourErrMsgTxt("GLCALL: setmatrix: invalid call, passing a length-6 (or 4) vector as X is"
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
            ourErrMsgTxt("GLCALL: setmatrix: invalid call, see GLCALL(glc.setmatrix) for usage.");
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
            ourErrMsgTxt(usageText);

        if (nrhs == 2)
        {
            matrixmode = GL_MODELVIEW;
        }
        else
        {
            verifyparam(MULMATRIX_IN_MODE, "GLCALL: mulmatrix: MATRIX_MODE", VP_SCALAR|VP_UINT32);
            matrixmode = *(uint32_t *)mxGetData(MULMATRIX_IN_MODE);

            if (matrixmode != GL_MODELVIEW && matrixmode != GL_PROJECTION && matrixmode != GL_TEXTURE)
                ourErrMsgTxt("GLCALL: mulmatrix: MATRIX_MODE must be one of GL_MODELVIEW, "
                             "GL_PROJECTION or GL_TEXTURE");
        }

        if (!mxIsDouble(MULMATRIX_IN_MATRIX))
            ourErrMsgTxt("GLCALL: mulmatrix: X must have class double");

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
            ourErrMsgTxt("GLCALL: mulmatrix: invalid call, see GLCALL(glc.mulmatrix) for usage.");
        }
    }
    break;

    case GLC_SETCALLBACK:
    {
        int32_t callbackid, slen, glutwidx;
        char tmpbuf[MAXCBNAMELEN+1];

        if (nlhs != 0 || nrhs != 3)
            ourErrMsgTxt("Usage: GLCALL(glc.setcallback, glc.cb_<callbacktype>, 'mfuncname'), pass '' to reset");

        glutwidx = glutGetWindow();
        if (glutwidx==0)
            ourErrMsgTxt("GLCALL: glc.setcallback: no current window!");

        verifyparam(SETCALLBACK_IN_TYPE, "GLCALL: setcallback: CALLBACKTYPE", VP_SCALAR|VP_INT32);
        callbackid = *(int32_t *)mxGetData(SETCALLBACK_IN_TYPE);
        if ((unsigned)callbackid >= NUM_CALLBACKS)
            ourErrMsgTxt("GLCALL: setcallback: CALLBACKTYPE must be a valid callback ID");

        verifyparam(SETCALLBACK_IN_FUNCNAME, "GLCALL: setcallback: FUNCNAME", VP_VECTOR|VP_CHAR);
        /* XXX: the string might be multibyte in MATLAB */
        slen = mxGetNumberOfElements(SETCALLBACK_IN_FUNCNAME);
        if (slen > MAXCBNAMELEN)
            GLC_MEX_ERROR("GLCALL: setcallback: FUNCNAME must not exceed %d chars", MAXCBNAMELEN);

        if (verify_callback_name(SETCALLBACK_IN_FUNCNAME, slen, tmpbuf))
            ourErrMsgTxt("GLCALL: setcallback: FUNCNAME must be a valid "
                         "MATLAB identifier ([A-Za-z][A-Za-z0-9_]+)");

        memcpy(&callback_funcname[g_ourwinidx[glutwidx]][callbackid], tmpbuf, sizeof(tmpbuf));
    }
    return;

    /* TODO: merge with GLCALL(glc.scissor) ... */
    case GLC_VIEWPORT:
    {
        const double *xywh_d;
        int32_t xywh[4];

        if (nlhs != 0 || nrhs != 2)
            ourErrMsgTxt("Usage: GLCALL(glc.viewport, [x y width height])");

        verifyparam(VIEWPORT_IN_XYWH, "GLCALL: viewport: XYWH", VP_VECTOR|VP_DOUBLE|(4<<VP_VECLEN_SHIFT));
        xywh_d = mxGetPr(VIEWPORT_IN_XYWH);

        /* XXX: magic constants bad! */
        xywh[0] = util_dtoi(xywh_d[0], -16384, 16384, "GLCALL: viewport: XYWH(1)");
        xywh[1] = util_dtoi(xywh_d[1], -16384, 16384, "GLCALL: viewport: XYWH(2)");
        xywh[2] = util_dtoi(xywh_d[2], 0, 16384, "GLCALL: viewport: XYWH(3)");
        xywh[3] = util_dtoi(xywh_d[3], 0, 16384, "GLCALL: viewport: XYWH(4)");

        RETIFERR(xywh[0]);
        RETIFERR(xywh[1]);
        RETIFERR(xywh[2]);
        RETIFERR(xywh[3]);

        glViewport(xywh[0], xywh[1], xywh[2], xywh[3]);
    }
    break;

    case GLC_CLEAR:
    {
        /* TODO: might want to clear the depth buffer with a different value */

        mwSize numel;
        const double *color;

        if (nlhs != 0 || (nrhs != 1 && nrhs != 2))
            ourErrMsgTxt("Usage: GLCALL(glc.clear, [r g b [a]]), color must have 'double' type");

        if (nrhs == 1)
        {
            glClear(GL_DEPTH_BUFFER_BIT);
        }
        else
        {
            verifyparam(CLEAR_IN_COLOR, "GLCALL: clear: COLOR", VP_VECTOR|VP_DOUBLE);
            numel = mxGetNumberOfElements(CLEAR_IN_COLOR);
            if (numel != 3 && numel != 4)
                ourErrMsgTxt("GLCALL: clear: COLOR must have length 3 or 4");

            color = mxGetPr(CLEAR_IN_COLOR);

            glClearColor(color[0], color[1], color[2], numel==4?color[3]:0);
            glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
        }
    }
    break;

    case GLC_REDISPLAY:
    {
        int32_t now_p = 0;

        if (nlhs != 0 || (nrhs != 1 && nrhs != 2))
            ourErrMsgTxt("Usage: GLCALL(glc.redisplay [, NOW_P])");

        if (glutGetWindow() == 0)
            ourErrMsgTxt("GLCALL: redisplay: called with no current window!");

        if (nrhs==2)
        {
            verifyparam(REDISPLAY_IN_NOWP, "GLCALL: redisplay: NOW_P",
                        VP_SCALAR|VP_LOGICAL);
            now_p = !!*(int8_t *)mxGetData(REDISPLAY_IN_NOWP);
        }

        if (now_p)
            glutSwapBuffers();
        else
            glutPostRedisplay();
    }
    return;

    case GLC_GETERRSTR:
    {
#ifdef HAVE_OCTAVE
        ourErrMsgTxt("CLGALL.geterrstr only available on MATLAB");
#else
        if (numentered)
            ourErrMsgTxt("GLCALL.geterrstr should only be called after an error from the prompt");

        if (nlhs != 3 || nrhs != 1)
            return;

        plhs[0] = mxCreateString(errstrptr ? errstrptr : "<empty>");
        plhs[1] = mxCreateString(errstr);
        plhs[2] = exceptionar ? exceptionar : mxCreateDoubleScalar(0);
#endif
    }
    return;

    case GLC_NEWTEXTURE:
    {
        /* TODO: generalize
         -  OK: 1, 2, 3, 4 components: LUM, LUM_ALPHA, RGB, RGBA
         -  OK: different base data type components
         *  1D and 3D textures
         *  an option struct to specify deviations from these 'default convenience settings'
         *  allow specifying different s,t wrap modes
         *  allow specifying borders */

        GLuint texname;
        const mwSize *dimsizes;  /* XXX: older versions w/o mwSize? */
        GLint tmpwidth;

        int32_t newtex = (nlhs == 1 && (nrhs == 2 || nrhs == 3));
        int32_t repltex = (nlhs == 0 && (nrhs == 3 || nrhs == 4));
        int32_t haveopts = (nlhs + nrhs == 4);
        int32_t numdims, alignment=0; /* compiler-happy */

        const mxArray *minmag_mxar = NULL;
        GLint minfilt=GL_LINEAR, magfilt=GL_LINEAR;

        GLenum format;
        GLenum type=0;  /* compiler-happy */

        mwSize width, height;
        mxClassID classid;

        if (!newtex && !repltex)
            ourErrMsgTxt("Usage: texname = GLCALL(glc.newtexture, texar [, opts]), texar must be 3xNxM uint8 or NxM single\n"
                         "   or: GLCALL(glc.newtexture, texar, texname [, opts])  to replace an earlier texture");

        numdims = mxGetNumberOfDimensions(NEWTEXTURE_IN_TEXAR);

        if (numdims != 2 && numdims != 3)
            ourErrMsgTxt("GLCALL: newtexture: TEXAR must have either 2 or 3 dimensions");

        /* first, get the texture width, height and format */
        dimsizes = mxGetDimensions(NEWTEXTURE_IN_TEXAR);
        if (numdims == 2)
        {
            width = dimsizes[0];
            height = dimsizes[1];

            format = GL_LUMINANCE;
        }
        else
        {
            static const GLenum formats[3] = { GL_LUMINANCE_ALPHA, GL_RGB, GL_RGBA };

            width = dimsizes[1];
            height = dimsizes[2];

            if (dimsizes[0] < 2 || dimsizes[0] > 4)
                ourErrMsgTxt("GLCALL: newtexture: TEXAR's 1st dimension must have length 2, 3, or 4");

            format = formats[dimsizes[0]-2];
        }

        if (width == 0 || height == 0)
            ourErrMsgTxt("GLCALL: newtexture: TEXAR's width and height must not be 0");

        /* next, determine the data type and alignment... */
        classid = mxGetClassID(NEWTEXTURE_IN_TEXAR);
        switch (classid)
        {
        case mxSINGLE_CLASS:
            type = GL_FLOAT; alignment = 4; break;
        case mxINT8_CLASS:
            type = GL_BYTE; alignment = 1; break;
        case mxUINT8_CLASS:
            type = GL_UNSIGNED_BYTE; alignment = 1; break;
        case mxINT16_CLASS:
            type = GL_SHORT; alignment = 2; break;
        case mxUINT16_CLASS:
            type = GL_UNSIGNED_SHORT; alignment = 2; break;
        case mxINT32_CLASS:
            type = GL_INT; alignment = 4; break;
        case mxUINT32_CLASS:
            type = GL_UNSIGNED_INT; alignment = 4; break;
        default:
            ourErrMsgTxt("GLCALL: newtexture: TEXAR must have 'single' or [u]int{8,16,32} numeric class");
        }

        /* we could tweak the internal format for more precision, but the internalFormat value
         * passed to glTexImage2D is taken as a wish, not a request, anyway... */

        if (haveopts)
        {
            verifyparam(NEWTEXTURE_IN_OPTS, "GLCALL: newtexture: OPTS", VP_SCALAR|VP_STRUCT);

            minmag_mxar = mxGetField(NEWTEXTURE_IN_OPTS, 0, "minmag");
            if (minmag_mxar)
            {
                const int32_t *minmagptr;
                mwSize nel;

                verifyparam(minmag_mxar, "GLCALL: newtexture: OPTS.minmag", VP_VECTOR|VP_INT32);
                nel = mxGetNumberOfElements(minmag_mxar);
                if (nel != 1 && nel != 2)
                    ourErrMsgTxt("GLCALL: newtexture: OPTS.minmag must have length 1 or 2");
                minmagptr = mxGetData(minmag_mxar);

                if ((minmagptr[0]!=GL_NEAREST && minmagptr[0]!=GL_LINEAR) ||
                    (nel > 1 && minmagptr[1]!=GL_NEAREST && minmagptr[1]!=GL_LINEAR))
                {
                    ourErrMsgTxt("GLCALL: newtexture: OPTS.minmag elements must be either GL.NEAREST or GL.LINEAR");
                }

                minfilt = minmagptr[0];
                magfilt = (nel==1) ? minfilt : minmagptr[1];
            }
        }

        if (repltex)
        {
            /* we're replacing an existing texture */
            verifyparam(NEWTEXTURE_IN_TEXNAME, "GLCALL: newtexture: TEXNAME", VP_SCALAR|VP_UINT32);
            texname = *(uint32_t *)mxGetData(NEWTEXTURE_IN_TEXNAME);
            if (!glIsTexture(texname))
                ourErrMsgTxt("GLCALL: newtexture: TEXNAME must specify an existing texture");
        }
        else
        {
            glGenTextures(1, &texname);
        }

        glBindTexture(GL_TEXTURE_2D, texname);

        if (newtex || minmag_mxar)  /* keep the min/mag filters if reuploading and not explicitly given */
        {
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minfilt);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magfilt);
        }

        /* NOTE: CLAMP_TO_EDGE available if GL >= 1.2 */
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        glPixelStorei(GL_UNPACK_ALIGNMENT, format==GL_RGB ? 1 : alignment);

        /* proxy check first; params:
         * target, level, internalFormat, width, height, border, format, type, data */
        glTexImage2D(GL_PROXY_TEXTURE_2D, 0, format,  width, height,
                     0, format, type, NULL);
        glGetTexLevelParameteriv(GL_PROXY_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &tmpwidth);
        if (tmpwidth==0)
        {
            glDeleteTextures(1, &texname);
            ourErrMsgTxt("GLCALL: newtexture: cannot accomodate texture");
        }
        glTexImage2D(GL_TEXTURE_2D, 0, format,  width, height,
                     0, format, type, mxGetData(NEWTEXTURE_IN_TEXAR));

        if (newtex)
            NEWTEXTURE_OUT_TEXNAME = createScalar(mxUINT32_CLASS, &texname);
    }
    break;

/* FreeGLUT's Roman font characteristics */
#define SPCWIDTH (104.762)  /* The width of the space and all characters for the monospaced font. */
#define FONTHEIGHT (119.05)

    case GLC_TEXT:
    {
        /* TODO:
         *  Make TAB have other widths than GLUT default (used as 'small space')?
         *  <forgot one?> */

        const double *pos;
        double height;
        char *text;

        mwSize i, vlen;
        double xyalign[2] = { 0.0, 0.0 };  /* corresponds to passed [-1 -1] */
        double color[3] = { 0.2, 0.2, 0.2 };

        /* This value was determined to look good by trial/error. */
        double xspacing = (FONTHEIGHT/10.0);
        void *font = GLUT_STROKE_ROMAN;

        const mxArray *xyalignAr=NULL, *optsAr=NULL;

        if (nlhs > 1 || !(nrhs >= 4 && nrhs <= 6))
            ourErrMsgTxt("Usage: [textlen] = GLCALL(glc.text, [x y [z]], height, text [, xyalign [, opts]])");

        verifyparam(TEXT_IN_POS, "GLC: text: POS", VP_VECTOR|VP_DOUBLE);
        /* Not using VP_VECLEN_SHIFT above because we need vector length later. */
        vlen = mxGetNumberOfElements(TEXT_IN_POS);
        if (vlen != 2 && vlen != 3)
            ourErrMsgTxt("GLC: text: POS must have length 2 or 3");
        pos = mxGetPr(TEXT_IN_POS);

        verifyparam(TEXT_IN_HEIGHT, "GLC: text: HEIGHT", VP_SCALAR|VP_DOUBLE);
        height = *mxGetPr(TEXT_IN_HEIGHT);

        if (nrhs >= 5)
        {
            /* Either
             *  glcall(glc.text, pos, height, str, XYALIGN), 
             *  glcall(glc.text, pos, height, str, OPTS), or 
             *  glcall(glc.text, pos, height, str, XYALIGN, OPTS). */
            if (nrhs==5)
            {
                if (mxIsStruct(TEXT_IN_XYALIGN))
                    optsAr = TEXT_IN_XYALIGN;
                else
                    xyalignAr = TEXT_IN_XYALIGN;
            }
            else
            {
                xyalignAr = TEXT_IN_XYALIGN;
                optsAr = TEXT_IN_OPTS;
            }
        }

        if (xyalignAr)
        {
            const double *tmpxyalign;

            /* xalign: -1: align on left (default), 0: align centered, 1: align on right
             * yalign: -1: align on bottom (default), 0: align centered, 1: align on top
             * mapped to 0.0, 0.5 and 1.0 in the internal representation */
            verifyparam(xyalignAr, "GLC: text: XYALIGN",
                        VP_VECTOR|VP_DOUBLE|(2<<VP_VECLEN_SHIFT));
            tmpxyalign = mxGetData(xyalignAr);
            if (tmpxyalign[0] >= 0.0)
                xyalign[0] = 0.5 + 0.5*(tmpxyalign[0] > 0.0);
            if (tmpxyalign[1] >= 0.0)
                xyalign[1] = 0.5 + 0.5*(tmpxyalign[1] > 0.0);
        }

        if (optsAr)
        {
            mxArray *tmpar;

            verifyparam(optsAr, "GLC: text: OPTS", VP_SCALAR|VP_STRUCT);

            tmpar = mxGetField(optsAr, 0, "colors");
            if (tmpar)
            {
                verifyparam(tmpar, "GLC: text: OPTS.colors", VP_VECTOR|VP_DOUBLE|(3<<VP_VECLEN_SHIFT));
                memcpy(color, mxGetData(tmpar), 3*sizeof(double));
            }

            tmpar = mxGetField(optsAr, 0, "xgap");
            if (tmpar)
            {
                verifyparam(tmpar, "GLC: text: OPTS.xgap", VP_SCALAR|VP_DOUBLE);
                xspacing = *(double *)mxGetData(tmpar) * SPCWIDTH;
            }

            tmpar = mxGetField(optsAr, 0, "mono");
            if (tmpar)
            {
                verifyparam(tmpar, "GLC: text: OPTS.mono", VP_SCALAR|VP_LOGICAL);
                if (*(int8_t *)mxGetData(tmpar))
                    font = GLUT_STROKE_MONO_ROMAN;
            }
        }

        verifyparam(TEXT_IN_TEXT, "GLC: text: TEXT", VP_VECTOR|VP_CHAR);
        text = mxArrayToString(TEXT_IN_TEXT);

        if (!text)
            ourErrMsgTxt("GLCALL: text: Out of memory!");

        {
            mwSize slen = strlen(text);
            double strokeslen, textlen=0.0;  /* compiler-happy */

            int32_t numspaces = 0;
            static int32_t *spaceidxs, maxslen;

            if (slen > UINT16_MAX)
                ourErrMsgTxt("GLCALL: text: TEXT must be 65535 characters or shorter");

            if (slen >= maxslen)
            {
                maxslen = slen;
                spaceidxs = realloc(spaceidxs, slen*sizeof(*spaceidxs));
            }

            for (i=0; i<slen; i++)
                if (text[i]==' ')
                {
                    text[i] = 't';
                    spaceidxs[numspaces++] = i;
                }

            if (xyalign[0] != 0.0 || nlhs >= 1)
            {
                /* XXX: if the string contains a newline, this will be wrong. */
                strokeslen = (double)glutStrokeLength(font, (unsigned char *)text);
                textlen = (strokeslen + (slen-1)*xspacing);

                if (nlhs >= 1)
                {
                    TEXT_OUT_LENGTH = mxCreateDoubleScalar(textlen*(height/FONTHEIGHT));
                    mxFree(text);
                    return;  /* don't draw text if length requested */
                }
            }

            for (i=0; i<numspaces; i++)
                text[spaceidxs[i]] |= 128;

            glMatrixMode(GL_MODELVIEW);
            glPushMatrix();
            glPushAttrib(GL_CURRENT_BIT|GL_ENABLE_BIT|GL_COLOR_BUFFER_BIT);

            glColor4d(color[0], color[1], color[2], 1);
            glDisable(GL_TEXTURE_2D);

            glEnable(GL_LINE_SMOOTH);
            glEnable(GL_BLEND);
            glBlendEquation(GL_FUNC_ADD);

            glTranslated(pos[0], pos[1], vlen==2 ? 0.0 : pos[2]);

            glScaled(height/FONTHEIGHT, height/FONTHEIGHT, height/FONTHEIGHT);

            if (xyalign[1] != 0.0)  /* y-align */
                glTranslated(0, -xyalign[1]*FONTHEIGHT, 0);

            if (xyalign[0] != 0.0)
            {
                /* TODO: proper newline handling (see above) */
                glTranslated(-xyalign[0]*textlen, 0, 0);
            }

            for (i=0; i<slen; i++)
            {
                char ch = text[i];

                if (ch&128)
                {
                    glColor4d(0, 0, 0, 0);
                    glutStrokeCharacter(font, ch&127);
                    glColor4d(color[0], color[1], color[2], 1);
                }
                else
                {
                    glutStrokeCharacter(font, ch);
                }

                /* Add a bit of spacing, since by default, GLUT's stroke text
                 * looks too cramped... */
                glTranslated(xspacing, 0, 0);
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
            GL_LINE_STIPPLE, GL_POLYGON_SMOOTH, GL_FOG,
        };

        if (nlhs != 0 || nrhs != 2)
            ourErrMsgTxt("Usage: GLCALL(glc.toggle, [GL.<WHAT1> <state1> [, GL.<WHAT2> <state2>, ...]]))");

        verifyparam(TOGGLE_IN_KV, "GLCALL: toggle: KV", VP_VECTOR|VP_INT32);
        numkeys = mxGetNumberOfElements(TOGGLE_IN_KV);
        if (numkeys&1)
            ourErrMsgTxt("GLCALL: toggle: key-value vector must have even length");
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
            ourErrMsgTxt("Usage: GLCALL(glc.scissor, int32([x y w h]))");

        verifyparam(SCISSOR_IN_XYWH, "GLCALL: scissor: XYWH", VP_VECTOR|VP_INT32|(4<<VP_VECLEN_SHIFT));

        xywh = mxGetData(SCISSOR_IN_XYWH);
        if (xywh[2]<0 || xywh[3]<0)
            ourErrMsgTxt("GLCALL: scissor: XYWH(3) and XYWH(4) must be non-negative");

        glScissor(xywh[0], xywh[1], xywh[2], xywh[3]);
    }
    break;

    case GLC_DELTEXTURES:
    {
        mwSize numtexs;

        if (nlhs != 0 || nrhs != 2)
            ourErrMsgTxt("Usage: GLCALL(glc.deltextures, [texname1 texname2 ...])");

        verifyparam(DELTEXTURES_IN_TEXNAMES, "GLCALL: deltextures: TEXNAMES",
                    VP_VECTOR|VP_EMPTY_OK|VP_UINT32);
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
            ourErrMsgTxt("Usage: GLCALL(glc.push, [WHAT1 WHAT2 ...]);  WHAT* can be either\n"
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
            ourErrMsgTxt("GLCALL: push/pop: over/underflow");
        }
    }
    break;

    case GLC_GET:
    {
        int32_t what;

        if (nlhs != 1 || nrhs != 2)
            ourErrMsgTxt("Usage: VALUE = GLCALL(glc.get, WHAT)");

        verifyparam(GET_IN_WHAT, "GLCALL: set: WHAT", VP_SCALAR|VP_INT32);
        what = *(int32_t *)mxGetData(GET_IN_WHAT);

        if (!inited && what != GLC__WINDOW_ID)
            ourErrMsgTxt("GLCALL: get: Only GL.WINDOW_ID supported when not initialized");

        switch (what)
        {
        case GLC__WINDOW_ID:
        {
            int32_t glutwidx = inited ? glutGetWindow() : 0, ret_ourwidx;

            if (glutwidx==0)
                ret_ourwidx = 0;
            else
                ret_ourwidx = g_ourwinidx[glutwidx]+1;

            GET_OUT_VALUE = createScalar(mxINT32_CLASS, &ret_ourwidx);
            return;
        }

        case GLC__WINDOW_SIZE:
        case GLC__WINDOW_POS:
        {
            mxArray *whar;
            double *wh;
            int32_t glutwidx;

            glutwidx = glutGetWindow();
            if (glutwidx==0)
                ourErrMsgTxt("GLCALL: get WINDOW_*: no active window!");

            whar = mxCreateNumericMatrix(1,2, mxDOUBLE_CLASS, mxREAL);
            wh = mxGetPr(whar);

            if (what == GLC__WINDOW_SIZE)
            {
                wh[0] = glutGet(GLUT_WINDOW_WIDTH);
                wh[1] = glutGet(GLUT_WINDOW_HEIGHT);
            }
            else
            {
                wh[0] = glutGet(GLUT_WINDOW_X);
                wh[1] = glutGet(GLUT_WINDOW_Y);
            }

            GET_OUT_VALUE = whar;
            return;
        }

        case GL_MODELVIEW_MATRIX:
        case GL_PROJECTION_MATRIX:
        {
            mxArray *matrixAr = mxCreateDoubleMatrix(4,4, mxREAL);
            double *matrixptr = mxGetPr(matrixAr);

            glGetDoublev(what, matrixptr);

            GET_OUT_VALUE = matrixAr;
            return;
        }

        default:
            ourErrMsgTxt("GLCALL: get: WHAT token unknown");
        }
    }
    break; /* not reached */

    case GLC_SET:
    {
        int32_t what;

        if (nlhs != 0 || nrhs != 3)
            ourErrMsgTxt("Usage: GLCALL(glc.set, WHAT, VALUE)");

        verifyparam(SET_IN_WHAT, "GLCALL: set: WHAT", VP_SCALAR|VP_INT32);
        what = *(int32_t *)mxGetData(SET_IN_WHAT);

        switch (what)
        {
        case GLC__MENU_ENABLE:
        {
            int32_t glutwidx = glutGetWindow(), ourwidx, menuid, yn;

            verifyparam(SET_IN_VALUE, "GLCALL: set menu enable: YN", VP_SCALAR|VP_LOGICAL);

            if (glutwidx < 1)
                ourErrMsgTxt("GLCALL: set menu enable: invalid current window");

            ourwidx = g_ourwinidx[glutwidx];
            mxAssert((unsigned)ourwidx < MAXACTIVEWINDOWS, "XXX");

            menuid = win[ourwidx].menuid;
            if (win[ourwidx].menus == NULL || menuid < 1)
                ourErrMsgTxt("GLCALL: set menu enable: window has no menu");

            yn = *(uint8_t *)mxGetData(SET_IN_VALUE);

            glutSetMenu(menuid);
            if (glutGetMenu() != menuid)
                ourErrMsgTxt("GLCALL: set menu enable: INTERNAL error, glutSetMenu() failed");

            if (yn)
                glutAttachMenu(win[ourwidx].menubutton);
            else
                glutDetachMenu(win[ourwidx].menubutton);

            return;
        }

        case GLC__MOUSE_POS:
        {
            int32_t wh[2], newposi[2];
            const double *newpos;

            verifyparam(SET_IN_VALUE, "GLCALL: set GL.MOUSE_POS: POS", VP_VECTOR|VP_DOUBLE|(2<<VP_VECLEN_SHIFT));

            wh[0] = glutGet(GLUT_WINDOW_WIDTH);
            wh[1] = glutGet(GLUT_WINDOW_HEIGHT);

            newpos = mxGetPr(SET_IN_VALUE);
            newposi[0] = util_dtoi(newpos[0], 0, wh[0], "GLCALL: set mouse pos: POS(1)");
            newposi[1] = util_dtoi(newpos[1], 0, wh[1], "GLCALL: set mouse pos: POS(2)");

            RETIFERR(newposi[0]);
            RETIFERR(newposi[1]);

            /* TODO: what if no current window? */
            glutWarpPointer(newposi[0], newposi[1]);

            return;
        }

        case GLC__WINDOW_ID:
        {
            int32_t ourwidx, glutwidx;

            verifyparam(SET_IN_VALUE, "GLCALL: set GL.WINDOW_ID: ID", VP_SCALAR|VP_INT32);
            ourwidx = *(int32_t *)mxGetData(SET_IN_VALUE);
            ourwidx--;

            if ((unsigned)ourwidx >= MAXACTIVEWINDOWS || (glutwidx=g_glutwinidx[ourwidx])==0)
                ourErrMsgTxt("GLCALL: set GL.WINDOW_ID: window index invalid or nonexistent window");

            glutSetWindow(glutwidx);
            /* glutSetWindow might still come up empty; in that case following commands
             * will probably bring the program to its fall. */

            return;
        }

        case GL_POINT_SIZE:
        {
            double value_d;

            verifyparam(SET_IN_VALUE, "GLCALL: set GL.POINT_SIZE: SIZE", VP_SCALAR|VP_DOUBLE);
            value_d = *mxGetPr(SET_IN_VALUE);

            /* XXX: potential undefined behavoiur when downcasting */
            glPointSize((GLfloat)value_d);
            break;
        }

        case GL_LINE_WIDTH:
        {
            double value_d;

            verifyparam(SET_IN_VALUE, "GLCALL: set GL.LINE_WIDTH: WIDTH", VP_SCALAR|VP_DOUBLE);
            value_d = *mxGetPr(SET_IN_VALUE);

            /* XXX: potential undefined behavoiur when downcasting */
            glLineWidth((GLfloat)value_d);
            break;
        }

        case GLC__WINDOW_SIZE:
        {
            const double *wh_d;
            int32_t wh[2];

            verifyparam(SET_IN_VALUE, "GLCALL: set window size: WH",
                        VP_VECTOR|VP_DOUBLE|(2<<VP_VECLEN_SHIFT));
            wh_d = mxGetPr(SET_IN_VALUE);

            wh[0] = util_dtoi(wh_d[0], 1, 16384, "GLCALL: set window size: WH(1)");
            wh[1] = util_dtoi(wh_d[1], 1, 16384, "GLCALL: set window size: WH(2)");

            RETIFERR(wh[0]);
            RETIFERR(wh[1]);

            glutReshapeWindow(wh[0], wh[1]);
            return;
        }

        case GL_BLEND_EQUATION:
        {
            verifyparam(SET_IN_VALUE, "GLCALL: set GL.BLEND_EQUATION: EQN", VP_SCALAR|VP_INT32);
            glBlendEquation(*(int32_t *)mxGetData(SET_IN_VALUE));
            break;  /* glGetError handles invalid enum vals */
        }

        case GL_DEPTH_FUNC:
        {
            verifyparam(SET_IN_VALUE, "GLCALL: set GL.DEPTH_FUNC: FUNC", VP_SCALAR|VP_INT32);
            glDepthFunc(*(int32_t *)mxGetData(SET_IN_VALUE));
            break;  /* glGetError handles invalid enum vals */
        }

        case GL_LINE_STIPPLE_PATTERN:
        {
            verifyparam(SET_IN_VALUE, "GLCALL: set GL.LINE_STIPPLE_PATTERN: PATTERN", VP_SCALAR|VP_UINT16);
            glLineStipple(1, *(uint16_t *)mxGetData(SET_IN_VALUE));
            break;
        }

        default:
            ourErrMsgTxt("GLCALL: set: WHAT token unknown");
        }
    }
    break;

    case GLC_COLORMAP:
    {
        const mwSize *dimsizes;

        if (!GLEW_VERSION_1_3 && !GLEW_ARB_multitexture)
            ourErrMsgTxt("GLCALL: colormap needs OpenGL 1.3 or GL_ARB_multitexture");

        if (nlhs != 0 || nrhs != 2)
            ourErrMsgTxt("Usage: GLCALL(glc.colormap, COLORMAP), COLORMAP should be 3x256 uint8");

        verifyparam(COLORMAP_IN_COLORMAP, "GLCALL: colormap: COLORMAP", VP_UINT8|VP_DIMN|(2<<VP_DIMN_SHIFT));
        dimsizes = mxGetDimensions(COLORMAP_IN_COLORMAP);
        if (dimsizes[0] != 3 || dimsizes[1] != 256)
            ourErrMsgTxt("GLCALL: colormap: COLORMAP must be a 3x256 matrix");

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
            ourErrMsgTxt("Usage: PROGID [, UNIFORMS] = GLCALL(glc.newfragprog, FRAGSHADERSRC)");

        if (!GLEW_VERSION_2_0)
            ourErrMsgTxt("GLCALL: newfragprog: OpenGL 2.0 required!");

        verifyparam(NEWFRAGPROG_IN_SHADERSRC, "GLCALL: newfragprog: FRAGSHADERSRC", VP_VECTOR|VP_CHAR);

        progId = glCreateProgram();
        if (!progId)
            ourErrMsgTxt("GLCALL: newfragprog: Error creating program object");

        fragshaderId = glCreateShader(GL_FRAGMENT_SHADER);
        if (!fragshaderId)
            ourErrMsgTxt("GLCALL: newfragprog: Error creating fragment shader object");

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

            ourErrMsgTxt(errstr);
        }

        glAttachShader(progId, fragshaderId);

        glLinkProgram(progId);
        glGetProgramiv(progId, GL_LINK_STATUS, &status);
        if (status != GL_TRUE)
        {
            /* program linking failed */
            memcpy(errstr, "PR: ", 4);
            glGetProgramInfoLog(progId, sizeof(errstr)-4, NULL, errstr+4);

            ourErrMsgTxt(errstr);
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
                    ourErrMsgTxt("GLCALL: INTERNAL ERROR calling mxAddField");
                /* uniform IDs != uniform locations !! */
                uniformLocation = glGetUniformLocation(progId, name);
                if (uniformLocation & (0xe0000000u))
                    ourErrMsgTxt("GLCALL: INTERNAL ERROR: (uniformLocation & 0xe0000000u) != 0");

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
            ourErrMsgTxt("Usage: GLCALL(glc.usefragprog [, PROGID])");

        if (nrhs == 1)
        {
            glUseProgram(0);
            /* proginuse = 0; */
            return;
        }

        verifyparam(USEFRAGPROG_IN_PROGID, "GLCALL: usefragprog: PROGID", VP_SCALAR|VP_UINT32);

        progId = *(uint32_t *)mxGetData(USEFRAGPROG_IN_PROGID);

        glUseProgram(progId);
        /* proginuse = progId; */

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
            ourErrMsgTxt("Usage: GLCALL(glc.setuniform, UNIFORMHANDLE, VAL)");

        glGetIntegerv(GL_CURRENT_PROGRAM, &progId);

        if (progId==0)
            ourErrMsgTxt("GLCALL: setuniform: a fragment program must be active!");

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
            ourErrMsgTxt("Usage: GLCALL(glc.leavemainloop)");

        glutLeaveMainLoop();
    }
    return;

    case GLC_CLOSEWINDOW:
    {
        int32_t ourwidx, glutwidx;

        if (nlhs!=0 || (nrhs!=1 && nrhs!=2))
            ourErrMsgTxt("Usage: GLCALL(glc.closewindow [, OURWINID])");

        if (nrhs == 2)
        {
            verifyparam(CLOSEWINDOW_IN_OURWINID, "GLCALL: closewindow: OURWINID", VP_SCALAR|VP_INT32);
            ourwidx = *(int32_t *)mxGetData(CLOSEWINDOW_IN_OURWINID);

            if (ourwidx < 1 || ourwidx > MAXACTIVEWINDOWS)
                ourErrMsgTxt("GLCALL: closewindow: passed invalid window identifier");
            ourwidx--;

            glutwidx = g_glutwinidx[ourwidx];
        }
        else
        {
            glutwidx = glutGetWindow();
            if (glutwidx <= 0)
                GLC_MEX_ERROR("GLCALL: closewindow: glutGetWindow returned %d!", glutwidx);

            ourwidx = g_ourwinidx[glutwidx];
        }

        if (glutwidx > 0)  /* XXX: still might be nonexistent? */
            glutDestroyWindow(glutwidx);

        /* use clear_window_indices()? */
        g_ourwinidx[glutwidx] = -1;
/*
        if (ourwidx < 0)
            ourErrMsgTxt("GLCALL: closewindow: INTERNAL ERROR: ourwidx < 0!");
*/
        if (ourwidx >= 0)
            g_glutwinidx[ourwidx] = 0;
/*        printf("Closewindow GLUT %d, our %d\n", glutwidx, ourwidx); */
    }
    return;

    case GLC_READPIXELS:
    {
        int32_t winw, winh, x, y, w, h;
        mwSize thedims[3]={3, 0, 0};
        const double *xywh_d;

        mxArray *pixels_mxar;

        if (nlhs != 1 || (nrhs != 1 && nrhs != 2))
            ourErrMsgTxt("Usage: PIXELS = GLCALL(glc.readpixels [, XYWH]), PIXELS is 3xWxH");

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

            RETIFERR(x);
            RETIFERR(y);
            RETIFERR(w);
            RETIFERR(h);
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

    case GLC_FOG:
    {
        int32_t fogmode;
        const float *fogparamptr;

        if (nlhs != 0 || nrhs != 4)
            ourErrMsgTxt("Usage: GLCALL(glc.fog, GL.<FOG_MODE>, FOG_PARAM, FOG_COLOR)\n"
                         "        - FOG_PARAM: 2-vector if mode is GL.LINEAR, scalar else\n"
                         "        - FOG_COLOR: 4-vector;  both must be of 'single' type\n");

        verifyparam(FOG_IN_MODE, "GLCALL: fog: MODE", VP_SCALAR|VP_INT32);
        fogmode = *(int32_t *)mxGetData(FOG_IN_MODE);

        if (fogmode != GL_LINEAR && fogmode != GL_EXP && fogmode != GL_EXP2)
            ourErrMsgTxt("GLCALL: fog: GL.<FOG_MODE> must be one of GL.LINEAR, GL.EXP or GL.EXP2");

        if (fogmode == GL_LINEAR)
            verifyparam(FOG_IN_PARAM, "GLCALL: fog: PARAM", VP_VECTOR|VP_SINGLE|(2<<VP_VECLEN_SHIFT));
        else
            verifyparam(FOG_IN_PARAM, "GLCALL: fog: PARAM", VP_SCALAR|VP_SINGLE);

        verifyparam(FOG_IN_COLOR, "GLCALL: fog: COLOR", VP_VECTOR|VP_SINGLE|(4<<VP_VECLEN_SHIFT));

        /* param verification OK */

        fogparamptr = mxGetData(FOG_IN_PARAM);

        glFogi(GL_FOG_MODE, fogmode);

        if (fogmode == GL_LINEAR)
        {
            glFogf(GL_FOG_START, fogparamptr[0]);
            glFogf(GL_FOG_END, fogparamptr[1]);
        }
        else
        {
            glFogf(GL_FOG_DENSITY, fogparamptr[0]);
        }

        glFogfv(GL_FOG_COLOR, (float *)mxGetData(FOG_IN_COLOR));
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
