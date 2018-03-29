#!/bin/bash
# killbackupchannel.sh
typeset i=0;
for((i=1;i<201;i++)); do
start=$(date +%s)

	time ./testcontainer
end=$(date +%s)
time=$(($end-$start))
echo "=======================$time=============";

#	sleep 3
done

