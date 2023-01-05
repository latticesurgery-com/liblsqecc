INPUT="
OPENQASM 2.0;
include "qelib1.inc";

qreg q[25];
"
echo "$INPUT" | lsqecc_slicer -q --edpclayout --graceful
#echo "$INPUT" | lsqecc_slicer -q --edpclayout --graceful -o ../tests_tyler/7.json


# s q[0];
# s q[1];
# s q[2];
# s q[3];
# s q[4];
# s q[5];
# s q[6];
# s q[7];
# s q[8];
# s q[9];
# s q[10];
# s q[11];
# s q[12];
# s q[13];
# s q[14];
# s q[15];