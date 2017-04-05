#!/bin/bash

cmds="HS skKpPfFiItTgGbB"

while true;
do
ref=`cat ./REF.dat`
cmd=`echo ${cmds:$ref:1}`
echo $cmd
#./ai mode real ref $cmd
#killall ai
done;
