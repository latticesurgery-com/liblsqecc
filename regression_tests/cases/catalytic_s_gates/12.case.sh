INPUT="
OPENQASM 2.0;
include "qelib1.inc";

qreg q[25];

t q[0];
t q[1];
t q[2];
t q[3];
t q[4];
t q[5];
t q[6];
t q[7];
t q[8];
t q[9];
t q[10];
t q[11];
t q[12];
t q[13];
t q[14];
t q[15];
"
echo "$INPUT" | lsqecc_slicer -q -L edpc --nostagger --notwists -P wave --printlli sliced




