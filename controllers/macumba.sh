#!/bin/bash

list="4 5"

for a in $list; do
	echo "set adi $a"  | yarp rpc /icub/right_leg/rpc:i
	echo "set adi $a"  | yarp rpc /icub/left_leg/rpc:i
done

sleep 1

for a in $list; do
	echo "set aen $a"  | yarp rpc /icub/right_leg/rpc:i
	echo "set aen $a"  | yarp rpc /icub/left_leg/rpc:i
done

sleep 1

for a in $list; do
	echo "set ena $a"  | yarp rpc /icub/right_leg/rpc:i
	echo "set ena $a"  | yarp rpc /icub/left_leg/rpc:i
done
for a in $list; do
	echo "set adi $a"  | yarp rpc /icub/right_leg/rpc:i
	echo "set adi $a"  | yarp rpc /icub/left_leg/rpc:i
done

sleep 1

for a in $list; do
	echo "set aen $a"  | yarp rpc /icub/right_leg/rpc:i
	echo "set aen $a"  | yarp rpc /icub/left_leg/rpc:i
done
for a in $list; do
	echo "set ena $a"  | yarp rpc /icub/right_leg/rpc:i
	echo "set ena $a"  | yarp rpc /icub/left_leg/rpc:i
done


