i=0
while [ $i -le 7 ]
do
    ../../build/lsqecc_slicer -l crossings.layout -i crossings_$(($i)).lli -P 3d -o crossings_$(($i)).json
    i=$(($i+1))
done