#! /bin/bash

for l in `find . -type f` 
do
if [ ${l: -5} == ".json" ]
then
echo "$l : "
echo `../ssl-visual-planner/Visual_Planner update $l`
fi
done
rm \(copy\).* temppo.db 