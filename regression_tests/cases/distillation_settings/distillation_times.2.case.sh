INPUT="
OPENQASM 2.0;
include "qelib1.inc";

qreg q[2];

cx q[0],q[1];
t q[1];
h q[1];
h q[1];
h q[1];
h q[1];
h q[1];
h q[1];
h q[1];
t q[1];
t q[1];
"
LAYOUT="
11rrQQrr11
11rrrrrr11
22rrQQrr22
22rrrrrr22
rrrrrrrrrr
rr334455rr
rr334455rr
"

echo "$LAYOUT" > tmp.layout
echo "$INPUT" | lsqecc_slicer -l tmp.layout -q --nostagger --disttime 5
rm tmp.layout