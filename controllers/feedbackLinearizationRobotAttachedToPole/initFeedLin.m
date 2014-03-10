clear all;

% Add path to MATLAB
addpath(genpath('/home/daniele/MATLAB'))
addpath(genpath('/home/daniele/src/codyco/build'))
addpath(genpath('/home/daniele/src/codyco/src/simulink/controllers'))

robotName = 'icubGazeboSim';
localName = 'torqueControlTests';
%%              TORSO       LEFT ARM     RIGHT ARM    LEFT LEG     LEFT ANKLE   RIGHT LEG    RIGHT ANKLE
 kp = diag([   ones(1,3),   ones(1,5),   ones(1,5),   ones(1,4),   ones(1,2),   ones(1,4),   ones(1,2)]);
 ki = diag([   ones(1,3),   ones(1,5),   ones(1,5),   ones(1,4),   ones(1,2),   ones(1,4),   ones(1,2)]);
 kd = diag([   ones(1,3),   ones(1,5),   ones(1,5),   ones(1,4),   ones(1,2),   ones(1,4),   ones(1,2)]);
 
 gains = [50*kp,ki*10,kd*7];
 delete kp ki kd;
 
 Ts = 0.01;
 qDes = [ 1.1695	
          4.2710e-04	
         -0.0020	
         -0.5211	
          0.5208	
          3.7614e-05	
          0.7857	
         -1.1384e-04	
         -0.5214	
          0.5183	
          4.3882e-04	
          0.7852	
         -5.0742e-04	
         -0.0104	
          0.6844	
          0.0016	
         -0.9278	
          7.2468e-05	
         -3.9067e-05	
         -0.0106	
          0.5277	
          0.0018	
         -0.9622	
          6.1920e-05	
         -3.9608e-05];
     

     
