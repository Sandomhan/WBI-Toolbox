function controllerQPsolver(block)

setup(block);

function setup(block)
    
block.NumInputPorts  = 5; 
block.NumOutputPorts = 1; 

% Setup port properties to be inherited or dynamic
block.SetPreCompInpPortInfoToDynamic;
block.SetPreCompOutPortInfoToDynamic;
% block.SetPreCompInpPortInfoToInherited;
% block.SetPreCompOutPortInfoToInherited;

% %1 % [quadTerm;linTerm] (12x12;1x12):(13x12)               
% %2 % [Aineq,bineq]             
% %3 % [Aeq,beq]                   
% %4 % [x0;lb;ub]                  

% for now based on both feet (needs to be defined dynamically)
% Override input port properties
% block.InputPort(1).Dimensions        = 13*12;      % %1 % [quadTerm,linTerm]               
% block.InputPort(2).Dimensions        = 28*13;      % %2 % [Aineq,bineq]
% block.InputPort(3).Dimensions        = 13;      % %3 % [Aeq,beq] 
% block.InputPort(4).Dimensions        = 6*2*3;   % %4 % [x0;lb;ub] 
% block.InputPort(5).Dimensions        = 1;   % %5 % n_constraint 
% % Override output port properties
% block.OutputPort(1).Dimensions       = 6*2;

% % use this if on left foot only 
% % Override input port properties
block.InputPort(1).Dimensions        = 7*6;      % %1 % [quadTerm,linTerm]               
block.InputPort(2).Dimensions        = 14*7;      % %2 % [Aineq,bineq]
block.InputPort(3).Dimensions        = 7;      % %3 % [Aeq,beq] 
block.InputPort(4).Dimensions        = 6*3;   % %4 % [x0;lb;ub] 
block.InputPort(5).Dimensions        = 1;   % %5 % n_constraint 
% Override output port properties
block.OutputPort(1).Dimensions       = 6;

% Override input and output port properties
block.InputPort(1).DatatypeID  = 0;  % double
block.InputPort(2).DatatypeID  = 0;  % double
block.InputPort(3).DatatypeID  = 0;  % double
block.InputPort(4).DatatypeID  = 0;  % double
block.InputPort(5).DatatypeID  = 0;  % double

block.InputPort(1).Complexity  = 'Real';
block.InputPort(2).Complexity  = 'Real';
block.InputPort(3).Complexity  = 'Real';
block.InputPort(4).Complexity  = 'Real';
block.InputPort(5).Complexity  = 'Real';

block.InputPort(1).DirectFeedthrough = true;
block.InputPort(2).DirectFeedthrough = true;
block.InputPort(3).DirectFeedthrough = true;
block.InputPort(4).DirectFeedthrough = true;
block.InputPort(5).DirectFeedthrough = true;

block.OutputPort(1).DatatypeID  = 0; % double
block.OutputPort(1).Complexity  = 'Real';

% Register parameters
block.NumDialogPrms     = 0;

% Register sample times
%  [0 offset]            : Continuous sample time
%  [positive_num offset] : Discrete sample time
%
%  [-1, 0]               : Inherited sample time
%  [-2, 0]               : Variable sample time
block.SampleTimes = [-1 0];

% Specify the block simStateCompliance. The allowed values are:
%    'UnknownSimState', < The default setting; warn and assume DefaultSimState
%    'DefaultSimState', < Same sim state as a built-in block
%    'HasNoSimState',   < No sim state
%    'CustomSimState',  < Has GetSimState and SetSimState methods
%    'DisallowSimState' < Error out when saving or restoring the model sim state
block.SimStateCompliance = 'DefaultSimState';

%% -----------------------------------------------------------------
%% The MATLAB S-function uses an internal registry for all
%% block methods. You should register all relevant methods
%% (optional and required) as illustrated below. You may choose
%% any suitable name for the methods and implement these methods
%% as local functions within the same file. See comments
%% provided for each function for more information.
%% -----------------------------------------------------------------

% block.RegBlockMethod('PostPropagationSetup',    @DoPostPropSetup);
% block.RegBlockMethod('InitializeConditions', @InitializeConditions);
% block.RegBlockMethod('Start', @Start);
block.RegBlockMethod('Outputs', @Outputs);     % Required
% block.RegBlockMethod('Update', @Update);
% block.RegBlockMethod('Derivatives', @Derivatives);
block.RegBlockMethod('Terminate', @Terminate); % Required    

%end setup

%%
%% PostPropagationSetup:
%%   Functionality    : Setup work areas and state variables. Can
%%                      also register run-time methods here
%%   Required         : No
%%   C-Mex counterpart: mdlSetWorkWidths
%%
% function DoPostPropSetup(block)
% 
% numberOfPoints = 2; %number of points in a quadrant
% block.NumDworks = 1;
%   
%   block.Dwork(1).Name            = 'A';
%   block.Dwork(1).Dimensions      = 2 * 12 * (4 * (numberOfPoints - 2) + 4);
%   block.Dwork(1).DatatypeID      = 0;      % double
%   block.Dwork(1).Complexity      = 'Real'; % real
%   block.Dwork(1).UsedAsDiscState = false;
  

%%
%% InitializeConditions:
%%   Functionality    : Called at the start of simulation and if it is 
%%                      present in an enabled subsystem configured to reset 
%%                      states, it will be called when the enabled subsystem
%%                      restarts execution to reset the states.
%%   Required         : No
%%   C-MEX counterpart: mdlInitializeConditions
%%
% function InitializeConditions(block)

% %compute friction cones contraints
% staticFrictionCoefficient = 0.45;
% %approximation with straight lines
% numberOfPoints = 2; %number of points in a quadrant
% 
% %split the pi/2 angle into numberOfPoints - 1;
% segmentAngle = pi/2 / (numberOfPoints - 1);
% 
% %define angle
% angle = 0 : segmentAngle : (2 * pi - segmentAngle);
% points = [cos(angle); sin(angle)];
% numberOfEquations = size(points, 2);
% assert(size(points, 2) == (4 * (numberOfPoints - 2) + 4));
% 
% %A*x <= b, with b is all zeros.
% A = zeros(numberOfEquations, 6);
% 
% %define equations
% for i = 1 : numberOfEquations
%    firstPoint = points(:, i);
%    secondPoint = points(:, rem(i, numberOfEquations) + 1);
%    
%    %define line passing through the above points
%    angularCoefficients = (secondPoint(2) - firstPoint(2)) / (secondPoint(1) - firstPoint(1));
%    offsets = firstPoint(2) - angularCoefficients * firstPoint(1);
% 
%    inequalityFactor = +1;
%    %if any of the two points are between pi and 2pi, then the inequality is
%    %in the form of y >= m*x + q, and I need to change the sign of it.
%    if (angle(i) > pi || angle(rem(i, numberOfEquations) + 1) > pi)
%        inequalityFactor = -1;
%    end
%    
%    %a force is 6 dimensional f = [fx, fy, fz, mux, muy, muz]'
%    %I have constraints on fx and fy, and the offset will be multiplied by
%    %mu * fz
%    
%    A(i,:) = inequalityFactor .* [-angularCoefficients, 1, -offsets * staticFrictionCoefficient, 0, 0, 0];
%    
% end
% 
% %I have to duplicate the matrices and vector for the two feet
% A = [A, zeros(size(A));
%     zeros(size(A)), A];
% 
% %reshape matrix into single vector
% A = reshape(A, 12 * 2 * numberOfEquations, 1);
% block.Dwork(1).Data = A;


% end InitializeConditions


%%
%% Start:
%%   Functionality    : Called once at start of model execution. If you
%%                      have states that should be initialized once, this 
%%                      is the place to do it.
%%   Required         : No
%%   C-MEX counterpart: mdlStart
%%
% function Start(block)
% 
% block.Dwork(1).Data = 0;

%endfunction

%%
%% Outputs:
%%   Functionality    : Called to generate block outputs in
%%                      simulation step
%%   Required         : Yes
%%   C-MEX counterpart: mdlOutputs
%%

function Outputs(block)

n_constraint = block.InputPort(5).Data;
fdim = n_constraint*6; 
QPterms             = block.InputPort(1).Data;
const_ineq          = block.InputPort(2).Data;
const_eq            = block.InputPort(3).Data;
x0_lb_ub            = block.InputPort(4).Data;

QPterms    = reshape(QPterms,fdim+1,fdim);
  quadTerm = QPterms(1:fdim,:);
   linTerm = QPterms(fdim+1,:);
   
% const_ineq = reshape(const_ineq,28,fdim+1);   
const_ineq = reshape(const_ineq,14,fdim+1);

     Aineq = const_ineq(:,1:fdim);
     bineq = const_ineq(:,fdim+1);
const_eq   = reshape(const_eq,1,fdim+1);
       Aeq = const_eq(:,1:fdim);
       beq = const_eq(:,fdim+1);
x0         = x0_lb_ub(1:fdim);               
lb         = x0_lb_ub(fdim+1:2*fdim);
ub         = x0_lb_ub(2*fdim+1:3*fdim);

% block.InputPort(1).Dimensions        = 12*13;      % %1 % [quadTerm,linTerm]               
% block.InputPort(2).Dimensions        = 24*13;      % %2 % [Aineq,bineq]
% block.InputPort(3).Dimensions        = 13;      % %3 % [Aeq,beq] 
% block.InputPort(4).Dimensions        = 6*2*3;   % %4 % [x0;lb;ub] 

         
% QP FOR TORQUE MINIMIZATION BASED ON arbitrary vector F0

% options = optimset('Algorithm','active-set','Display','off');
    
% [desiredf0, ~, exitFlag, ~, ~] = quadprog( quadTerm, linTerm, ...
%                                               Aineq,   bineq, ... %inequalities
%                                                 Aeq,     beq, ... %equalities
%                                                  lb,      ub, ... %bounds
%                                                  x0,          ... %initial solution
%                                                  options);


% [desiredf0, ~, exitFlag, ~, ~] = quadprog( quadTerm, linTerm, ...
%                                               Aineq, bineq, ...   % Inequalities
%                                                  [],   [], ...    % Equalities
%                                                  lb,      ub, ... % Bounds
%                                                  x0,          ... % Initial solution
%                                                  options);
                                      
 desiredf0 = -quadTerm\linTerm';                                       
 exitFlag  = 1;               
 
if exitFlag~=1
    disp('qp_tau_f0 failed');
    desiredf0 = zeros(6*n_constraint,1);
end

block.OutputPort(1).Data = desiredf0;

%end Outputs


function Terminate(block)

%end Terminate





