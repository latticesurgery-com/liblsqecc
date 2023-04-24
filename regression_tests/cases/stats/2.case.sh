INPUT="
OPENQASM 2.0;
include \"qelib1.inc\";

qreg q[15];

cx q[0],q[10];
s q[1];
cx q[1],q[3];
cx q[5],q[7];
t q[2];
"
echo "$INPUT" | lsqecc_slicer --noslices -q -L compact -f stats | \
  sed "s/Made patch computation. Took [0-9]*.[0-9e\-]*s./Made patch computation. Took <time_removed_by_case_script>/"