/*
 * Copyright (C) 2013 Robotics, Brain and Cognitive Sciences - Istituto Italiano di Tecnologia
 * Author: Jorhabib Eljaik Gomez
 * email: jorhabib.eljaik@iit.it
 *
 * The development of this software was supported by the FP7 EU project
 * CoDyCo (No. 600716 ICT 2011.2.1 Cognitive Systems and Robotics (b))
 * http://www.codyco.eu
 *
 * Permission is granted to copy, distribute, and/or modify this program
 * under the terms of the GNU General Public License, version 2 or any
 * later version published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
 * Public License for more details
 */

#include "yarpRead.h"

#define S_FUNCTION_LEVEL 2
#define S_FUNCTION_NAME yRead

// PARAMETERS MASK
#define NPARAMS 6                              // Number of input parameters

#define PARAM_IDX_1 0                           // FROM port name
#define PARAM_IDX_2 1                           // TO port name
#define PARAM_IDX_3 2                           // Size of the port you're reading
#define PARAM_IDX_4 3                           // boolean for blocking reading
#define PARAM_IDX_5 4                           // boolean to stream timestamp
#define PARAM_IDX_6 5                           // Autoconnect boolean
#define SIZE_READING_PORT mxGetScalar(ssGetSFcnParam(S,PARAM_IDX_3))    // Get first input parameter from mask
#define BLOCKING  mxGetScalar(ssGetSFcnParam(S,PARAM_IDX_4))
#define TIMESTAMP mxGetScalar(ssGetSFcnParam(S,PARAM_IDX_5))
#define AUTOCONNECT mxGetScalar(ssGetSFcnParam(S,PARAM_IDX_6))

// Need to include simstruc.h for the definition of the SimStruct and
// its associated macro definitions.
#include "simstruc.h"

#define IS_PARAM_DOUBLE(pVal) (mxIsNumeric(pVal) && !mxIsLogical(pVal) &&\
!mxIsEmpty(pVal) && !mxIsSparse(pVal) && !mxIsComplex(pVal) && mxIsDouble(pVal))


// Function: MDL_CHECK_PARAMETERS
// Input parameters for locomotionController in Simulink

#define MDL_CHECK_PARAMETERS
#if defined(MDL_CHECK_PARAMETERS) && defined(MATLAB_MEX_FILE)
static void mdlCheckParameters(SimStruct *S)
{
}
#endif  /*MDL_CHECK_PARAMETERS*/

// Function: mdlInitializeSizes ===============================================
// Abstract:
//    The sizes information is used by Simulink to determine the S-function
//    block's characteristics (number of inputs, s, states, etc.).
static void mdlInitializeSizes(SimStruct *S)
{
    ssSetNumSFcnParams(S, NPARAMS);
#if defined(MATLAB_MEX_FILE)
    if(ssGetNumSFcnParams(S) == ssGetSFcnParamsCount(S)){
        mdlCheckParameters(S);
        if(ssGetErrorStatus(S)!=NULL){
            return;
        }
        else{
            cout<<"All parameters have been checked and passed correctly"<<endl;
        }
    } else{
        return; // Parameter mismatch reported by Simulink
    }
#endif

    // Parameter mismatch will be reported by Simulink
    if (ssGetNumSFcnParams(S) != ssGetSFcnParamsCount(S)) {
        cout << "Number of paramaeters different from those defined" << endl;
        return;
    }

    // Specify I/O
    // INPUTS
    if(!ssSetNumInputPorts(S,0)) return;
    // OUTPUTS
    int timestamp = (int) TIMESTAMP;
    int size_reading_port = (int) SIZE_READING_PORT;
    int autoconnect = (int) AUTOCONNECT;
    if(!timestamp){
        cout<<"Yarp read block will have: " << SIZE_READING_PORT << "outputs" << endl;
        if (!ssSetNumOutputPorts(S,SIZE_READING_PORT)) return;
    } else {
        size_reading_port = size_reading_port + 1;
        cout<<"yarp read block will have: " << size_reading_port << "outputs" << endl;
        if (!ssSetNumOutputPorts(S,size_reading_port)) return;
        ssSetOutputPortWidth (S, size_reading_port-1, 2);
        ssSetOutputPortDataType (S, size_reading_port - 1, 0);
    }

    for (int i = 0; i < SIZE_READING_PORT; i++)
    {
        ssSetOutputPortWidth   (S, i, 1);
        ssSetOutputPortDataType(S, i, SS_DOUBLE);
    }


    ssSetNumSampleTimes(S, 1);

    // Reserve place for C++ object
    ssSetNumPWork(S, 1);

    // IWork vector for booleans
    ssSetNumIWork(S, 3);

    ssSetSimStateCompliance(S, USE_CUSTOM_SIM_STATE);

    ssSetOptions(S,
                 SS_OPTION_WORKS_WITH_CODE_REUSE |
                 SS_OPTION_EXCEPTION_FREE_CODE |
                 SS_OPTION_ALLOW_INPUT_SCALAR_EXPANSION |
                 SS_OPTION_USE_TLC_WITH_ACCELERATOR |
                 SS_OPTION_CALL_TERMINATE_ON_EXIT);

}

#if defined(MATLAB_MEX_FILE)
# define MDL_SET_INPUT_PORT_WIDTH
static void mdlSetInputPortWidth(SimStruct *S, int_T port,
                                 int_T inputPortWidth)
{
    ssSetInputPortWidth(S,port,inputPortWidth);
}
# define MDL_SET_OUTPUT_PORT_WIDTH
static void mdlSetOutputPortWidth(SimStruct *S, int_T port,
                                  int_T outputPortWidth)
{
    ssSetInputPortWidth(S,port,outputPortWidth);
}


# define MDL_SET_DEFAULT_PORT_DIMENSION_INFO
/* Function: mdlSetDefaultPortDimensionInfo ===========================================
 * Abstract:
 *   In case no ports were specified, the default is an input port of width 2
 *   and an output port of width 1.
 */
static void mdlSetDefaultPortDimensionInfo(SimStruct        *S)
{
    ssSetInputPortWidth(S, 0, 1);
}
#endif

// Function: mdlInitializeSampleTimes =========================================
// Abstract:
//   This function is used to specify the sample time(s) for your
//   S-function. You must register the same number of sample times as
//   specified in ssSetNumSampleTimes.
static void mdlInitializeSampleTimes(SimStruct *S)
{
    // The sampling time of this SFunction must be inherited so that the Soft Real Time sblock can be used.
    ssSetSampleTime(S, 0, INHERITED_SAMPLE_TIME);
    // ssSetSampleTime(S, 0, 10.0);
    ssSetOffsetTime(S, 0, 0.0);
    ssSetModelReferenceSampleTimeDefaultInheritance(S);
}

// Function: mdlStart =======================================================
// Abstract:
//   This function is called once at start of model execution. If you
//   have states that should be initialized once, this is the place
//   to do it.
#define MDL_START
static void mdlStart(SimStruct *S)
{
    // ######### YARP INITIALIZATION STUFF ##################
    Network::init();
    fprintf(stderr,"YARP NETWORK INITIALIZED\n");

    if (!Network::checkNetwork() || !Network::initialized()){
        ssSetErrorStatus(S,"YARP server wasn't found active!! \n");
        return;
    }
    else
        cout<<"YARP is running!!\n"<<endl;

    int_T buflen, status;
    char *buffer;

    buflen = (1 + mxGetN(ssGetSFcnParam(S, PARAM_IDX_1))) * sizeof(mxChar);
    buffer = static_cast<char*>(mxMalloc(buflen));
    status = mxGetString((ssGetSFcnParam(S, PARAM_IDX_1)), buffer, buflen);
    if (status) {
        ssSetErrorStatus(S,"Cannot retrieve string from parameter 1 (from)");
        return;
    }

    char *port_name = buffer;				//FROM port name

    buflen = (1 + mxGetN(ssGetSFcnParam(S, PARAM_IDX_2))) * sizeof(mxChar);
    buffer = static_cast<char*>(mxMalloc(buflen));
    status = mxGetString((ssGetSFcnParam(S, PARAM_IDX_2)), buffer, buflen);
    if (status) {
        ssSetErrorStatus(S,"Cannot retrieve string from parameter 2 (to)");
        return;
    }

    char *toPort_name = buffer;

    // ######## CHECKING INPUT PARAMETERS ############

    BufferedPort<Vector> *toPort;
    //allocate memory using matlab memory management
    toPort = new BufferedPort<Vector>();
    ssGetPWork(S)[0] = toPort;

    if (!toPort || !toPort->open(toPort_name)) {
        ssSetErrorStatus(S,"Error while opening yarp port");
        return;
    }
    ConstString toPortName = toPort->getName();
    cout<<"[From] Port name will be: "<<port_name<<endl;
    cout<<"[To] Port name will be:   "<<toPortName<<endl;

    int_T blocking = mxGetScalar(ssGetSFcnParam(S,PARAM_IDX_4));
    cout << "Blocking? : " << blocking << endl;
    ssGetIWork(S)[0] = blocking;

    int_T timestamp = mxGetScalar(ssGetSFcnParam(S,PARAM_IDX_5));
    cout << "Timestamp? : " << timestamp << endl;
    ssGetIWork(S)[1] = timestamp;

    int_T autoconnect = mxGetScalar(ssGetSFcnParam(S,PARAM_IDX_6));
    cout << "Autoconnect? : " << autoconnect << endl;
    ssGetIWork(S)[2] = autoconnect;

    if (autoconnect) {
        fprintf(stderr, "Connecting '%s' to '%s'\n", port_name, toPortName.c_str());
        if(!Network::connect(port_name, toPortName)) {
            printf("WBI-Toolbox failed connecting %s to %s\n", port_name, toPortName.c_str());
            ssSetErrorStatus(S,"ERROR connecting ports!");
            return;
        }
    }
    mxFree(toPort_name);
    mxFree(port_name);

    //     fprintf(stderr,"Result %d\n", port_name, toPortName.c_str(), Network::connect(port_name,toPortName));
}

// Function: mdlOutputs =======================================================
// Abstract:
//   In this function, you compute the outputs of your S-function
//   block.
static void mdlOutputs(SimStruct *S, int_T tid)
{
    BufferedPort<Vector> *toPort = static_cast<BufferedPort<Vector>*>(ssGetPWork(S)[0]);
    int_T blocking = ssGetIWork(S)[0];
    Vector *v = toPort->read((boolean_T) blocking); // Read from the port.  Waits until data arrives.
    if (v!=NULL)
    {
        if (TIMESTAMP) {
            yarp::os::Stamp timestamp;
            toPort->getEnvelope(timestamp);

            int timestamp_index = SIZE_READING_PORT;
            real_T *pY1 = (real_T *) ssGetOutputPortSignal(S, timestamp_index);
            pY1[0] = (real_T)(timestamp.getCount());
            pY1[1] = (real_T)(timestamp.getTime());
        }

        for (int i = 0; i < SIZE_READING_PORT; i++)
        {
            real_T *pY = (real_T *)ssGetOutputPortSignal(S,i);
            int_T widthPort = ssGetOutputPortWidth(S,i);
            for(int_T j=0; j<widthPort; j++){
                if (i < (v->length()))
                    pY[j] = v->data()[i];
                else
                    pY[j] = 0;
            }
        }
    }
    //     else {
    //       fprintf(stderr,"Cannot read port %s\n", toPort->getName().c_str());
    //     }
}

static void mdlTerminate(SimStruct *S)
{
    // IF YOU FORGET TO DESTROY OBJECTS OR DEALLOCATE MEMORY, MATLAB WILL CRASH.
    // Retrieve and destroy C++ object
    if (ssGetPWork(S)) { //This is not created in compilation
        BufferedPort<Vector> *toPort = static_cast<BufferedPort<Vector>*>(ssGetPWork(S)[0]);
        if (toPort) {
            toPort->close();
            delete toPort; //remove port
        }
        Network::fini();
    }
    fprintf(stderr,"Everything was closed correctly\n");
}

// Required S-function trailer
#ifdef  MATLAB_MEX_FILE    /* Is this file being compiled as a MEX-file? */
#include "simulink.c"      /* MEX-file interface mechanism */
#else
#include "cg_sfun.h"       /* Code generation registration function */
#endif
