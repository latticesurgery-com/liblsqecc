i=4
while [ $i -le 10 ]
do
    echo -e "OPENQASM 2.0;\ninclude \"qelib1.inc\";\n" > cases/edpc_t_layers/t_layer_$(($i*$i))qubits.qasm
    echo -e "qreg q[$(($i*$i))];\n" >> cases/edpc_t_layers/t_layer_$(($i*$i))qubits.qasm
    j=0
    while [ $j -lt $(($i*$i)) ]
    do 
        echo "t q[$j];" >> cases/edpc_t_layers/t_layer_$(($i*$i))qubits.qasm
        j=$(($j+1))
    done
    ./../build/lsqecc_slicer -q -i "cases/edpc_t_layers/t_layer_$(($i*$i))qubits.qasm" -L edpc --nostagger -P wave --noslices -f stats
    i=$(($i+1))
done