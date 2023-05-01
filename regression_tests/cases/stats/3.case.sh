INPUT="
OPENQASM 2.0;
include \"qelib1.inc\";

qreg q[25];
"
echo "$INPUT" | lsqecc_slicer -o test.json -q -L edpc -f stats | \
  sed "s/Made patch computation. Took [0-9]*.[0-9e\-]*s./Made patch computation. Took <time_removed_by_case_script>/"