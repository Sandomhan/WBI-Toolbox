// Notes for new Simulink wbInterface

// 1. Change macro definitions from _ROBOT_STATE_CPP_ to _WBINTERFACE_
// 2. Methods and dependencies for the following two classes:
//  a. robotStatus class (which should also change name to something more appropriate)
//  b. counterClass
// 3. Remove all use namespace from header file.

// -----------------------------------------------------------------------------------

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

#ifndef _WBINTERFACENEW_INCLUDE_WBINTERFACE_H_
#define _WBINTERFACENEW_INCLUDE_WBINTERFACE_H_

#define S_FUNCTION_LEVEL 2
#define S_FUNCTION_NAME  robotState

#include <yarp/sig/all.h>
#include <yarp/os/all.h>
#include <yarp/dev/all.h>
#include <yarp/os/Time.h>

#include <stdio.h>              // For printing debugging messages
#include <string.h>
#include <iostream>

#include <Eigen/Core>                           // import most common Eigen types
#include <Eigen/SVD>
#include <wbi/wbi.h>
#include <../../../../codyco-superbuild/external/orocos_kdl/python_orocos_kdl/PyKDL/std_string.sip>

// Need to include simstruc.h for the definition of the SimStruct and its associated macro definitions.
#include "simstruc.h"

namespace wbi {
class wholeBodyInterface;
}

//This should somehow be provided by the user, but 25 will be the default
#define ROBOT_DOF 25

typedef Eigen::Matrix<double, 7, 1>  Vector7d;
const int Dynamic = -1;
// a Jacobian is 6 rows and N columns
typedef Eigen::Matrix<double, 6, Dynamic, Eigen::RowMajor>           JacobianMatrix;
// N+6 x N+6 mass matrix
typedef Eigen::Matrix < double, ROBOT_DOF + 6, ROBOT_DOF + 6, Eigen::RowMajor > MassMatrix;

static const Vector7d              DEFAULT_X_LINK_SIZE = Vector7d::Constant (0.0);
static const Eigen::Vector2d       DEFAULT_X_COM_SIZE  = Eigen::Vector2d::Constant (0.0);

class robotStatus {
private:
    /** This object is used to control reentrancy. Counts the times the class robotStatus has been created.*/
    static int              creationCounter;

    /** id of the COM that is different from the other parts of the robot body.*/
    int                     comLinkId;

    /** id of the controlled foot link.*/
    int                     footLinkId;

    /** Current active joints (not being updated at the moment 24/02/2014 12:44pm). */
    int                     actJnts;

    /** current number of active joints. */
    int                     _n;

    /** Prefix given to the ports that will be open by Simulink. */
    std::string             moduleName;

    /** name of the robot being used, e.g. 'icubSim' or 'icub'. */
    std::string             robotName;
    // This variable map an Eigen vector to a yarp vector. //
    // Eigen::Map<Eigen::VectorXd>  dqDesMap; //

    /** Link name used for parametric blocks, such as Link Forward Kinematics, Link Jacobian, etc */
    std::string             linkName;

    static yarp::os::ConstString   worldRefFrame;

    /** Joint velocities (size of vectors: n+6, n, 6). */
    Eigen::VectorXd         dq, dqJ;

    /** rotation matrix from world to base reference frame. */
    Eigen::Matrix4d         H_w2b;

    /** \todo This variable has a wrong name because when the library was born it was meant to send only velocity commands. With its scalation it started using this very same vector for references to the other control modes available. */
    Eigen::VectorXd         dqDes;

    /** General vector initialized depending on the link for which forwardKinematics is computed as well as dJdq. */
    yarp::sig::Vector       x_pose;

    /** Robot joint angles in radians later initialized for the entire body. */
    yarp::sig::Vector       qRad;

    /** Robot joint accelerations ROBOT_DOFS-dim */
    yarp::sig::Vector       ddqJ;

    /** Robot joint torques*/
    yarp::sig::Vector       tauJ;

    /** Robot joint torques computed by inverseDynamics*/
    yarp::sig::Vector       tauJ_out;

    /** General Jacobian matrix initialized depending on the link for which the Jacobian is then needed.*/
    JacobianMatrix          JfootR;

    /** rotation to align foot Z axis with gravity, Ha=[0 0 1 0; 0 -1 0 0; 1 0 0 0; 0 0 0 1] */
    wbi::Frame              Ha;

    /** rototranslation from robot base to left foot (i.e. world)*/
    wbi::Frame              H_base_wrfLink;

    /** Floating base 3D rototranslation from world ot base.*/
    wbi::Frame              xBase;

    /** Floating base velocity 6x1 */
    yarp::sig::Vector       dxB;

    /** Floating base acceleration 6x1*/
    yarp::sig::Vector       ddxB;

    /** Generalized bias forces N+6 dim vector */
    yarp::sig::Vector       hterm;

    /** Gravity vector for now constant */
    yarp::sig::Vector       grav;

    /** Variable to store the product dot{J}*dot{q} */
    yarp::sig::Vector       dJdq;

    /** Mass matrix N+6xN+6 */
    MassMatrix              massMatrix;

    /** End effector wrench */
    yarp::sig::Vector       EEWrench;

    /** Flag defining whether the robot is fixed on its pole (true) or standing on the ground (false)**/
    static bool             icub_fixed;




public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW
    wbi::wholeBodyInterface* wbInterface;
    // Temporal container to copy wbInterface object for other copies of this module
    static int*              tmpContainer;

    robotStatus();
    ~robotStatus();
    void                setmoduleName (std::string mn);
    void                setRobotName (std::string rn); //checked
    void                setParamLink (std::string lk);
    std::string         getParamLink ();
    int                 decreaseCounter();
    static void         resetCounter();
    bool                robotConfig();
    bool                robotInit (int btype, int link);
    void                getLinkId (const char* linkName, int& lid);
    //This is especifically for the COM
    int                 getLinkId (const char* linkName);
    bool                world2baseRototranslation (double* q);
    void                setWorldReferenceFrame(const char* wrf);
    bool                robotJntAngles (bool blockingRead);
    bool                robotJntVelocities (bool blockingRead);
    bool                robotJntAccelerations (bool blockingRead);
    bool                robotJntTorques (bool blockingRead);
    yarp::sig::Vector   forwardKinematics (int& linkId);
    JacobianMatrix      jacobian (int& lid);
    yarp::sig::Vector   getEncoders();
    Eigen::VectorXd     getJntVelocities();
    yarp::sig::Vector   getJntTorques();

    bool                setCtrlMode (wbi::ControlMode ctrl_mode);
    bool                setCtrlMode (wbi::ControlMode ctrl_mode, int dof, double constRefSpeed);
    void                setdqDes (yarp::sig::Vector dqD);

    bool                inverseDynamics (double* qrad_input, double* dq_input, double* ddq_input, double* tau_computed);
    bool                dynamicsMassMatrix (double* qrad_input);
    yarp::sig::Vector   dynamicsGenBiasForces (double* qrad_input, double* dq_input);
    bool                robotBaseVelocity();
    bool                dynamicsDJdq (int& linkId, double* qrad_input, double* dq_input);
    MassMatrix          getMassMatrix();
    yarp::sig::Vector   getDJdq();
    yarp::sig::Vector   getJntAccelerations();
    bool                getJointLimits (double* qminLims, double* qmaxLims, const int jnt);
    bool                centroidalMomentum (double* qrad_input, double* dq_input, double* h);
    bool                robotEEWrenches (wbi::LocalId LID);
    yarp::sig::Vector   getEEWrench();
    bool                addEstimate();
};

class counterClass {
private:
    static int count;
public:
    counterClass();
    int getCount();
};

#endif
