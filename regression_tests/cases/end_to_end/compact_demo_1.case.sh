INPUT="
OPENQASM 2.0;
include \"qelib1.inc\";

qreg q[15];

cx q[0],q[10];
s q[10];
cx q[1],q[3];
cx q[5],q[7];
t q[2];
"
echo "$INPUT" | lsqecc_slicer -q -L compact