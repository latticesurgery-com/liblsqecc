i=0
while [ $i -le 8 ]
do
    ../../build/lsqecc_slicer -l crossings.layout -i crossings_$(($i)).lli -P 3d -o crossings_3d_$(($i)).json
    ../../build/lsqecc_slicer -l crossings.layout -i crossings_$(($i)).lli -P dag -o crossings_dag_$(($i)).json
    i=$(($i+1))
done