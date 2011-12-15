
#include <stdio.h>
#include <stdlib.h>

#include <GL/gl.h>
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
#define IN_INIT_POS (prhs[1])
#define IN_INIT_EXTENT (prhs[2])
#define IN_INIT_WINDOWNAME (prhs[3])


enum glcalls_
{
    GLC_INIT,
    GLC_LINES,
    NUM_GLCALLS,  // must be last
};

const char *glcall_names[] =
{
    "init",
    "lines"
};

////////// UTIL //////////
static char errstr[128];

#define GLC_MEX_ERROR(Text, ...) do {           \
        sprintf(errstr, Text, ## __VA_ARGS__);  \
        mexErrMsgTxt(errstr);                  \
    } while (0)

void verifyparam(const mxArray *ar, const char *arname, const char *typedesc)
{
    switch (typedesc[0])
    {
    case 's':  // string
        if (mxGetClassID(ar) != mxCHAR_CLASS)
            GLC_MEX_ERROR("%s must be a character array", arname);
        break;
    case '1':  // double scalar
        if (mxGetNumberOfElements(ar) != 1 || !mxIsDouble(ar))
            GLC_MEX_ERROR("%s must be a double scalar", arname);
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
            int i;

            for (i=0; i<NUM_GLCALLS; i++)
                mxSetFieldByNumber(glcstruct, 0, i, mxCreateDoubleScalar(i));
            OUT_GLCSTRUCT = glcstruct;

            return;
        }
        else
            mexErrMsgTxt("Usage: GLCALLINFO_STRUCT = GLCALL()");
    }

    // nrhs > 0

    verifyparam(IN_COMMAND, "COMMAND", "1");
    cmd = util_dtoi(*mxGetPr(IN_COMMAND), 0, NUM_GLCALLS, "COMMAND");

    //////////
    switch (cmd)
    {
    case GLC_INIT:
    {
        double *posptr, *extentptr;

        int pos[2], extent[2];
        char windowname[80];

        if (nrhs < 4)
            mexErrMsgTxt("Usage: GLCALL(glc.init, POS, EXTENT, WINDOWNAME)");
        verifyparam(IN_INIT_POS, "POS", "V2");
        verifyparam(IN_INIT_EXTENT, "EXTENT", "V2");
        verifyparam(IN_INIT_WINDOWNAME, "WINDOWNAME", "s");

        posptr = mxGetPr(IN_INIT_POS);
        extentptr = mxGetPr(IN_INIT_EXTENT);

        pos[0] = util_dtoi(posptr[0], 0, 1680, "POS(1)");
        pos[1] = util_dtoi(posptr[1], 0, 1050, "POS(2)");

        extent[0] = util_dtoi(extentptr[0], 320, 1680, "EXTENT(1)");
        extent[1] = util_dtoi(extentptr[1], 200, 1050, "EXTENT(2)");

        mxGetString(IN_INIT_WINDOWNAME, windowname, sizeof(windowname)-1);

        // init!
        if (!inited)
        {
            char *argvdummy[1] = {"QWEASDproggy"};
            int argcdummy = 1;

            glutInit(&argcdummy, argvdummy);  // XXX
            inited = 1;
        }

        glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH | GLUT_RGBA);
        glutInitWindowPosition(pos[0], pos[1]);
        glutInitWindowSize(extent[0], extent[1]);
        glutCreateWindow(windowname);
    }
    break;

    case GLC_LINES:
        mexErrMsgTxt("LINES command not implemented");
    }
}
