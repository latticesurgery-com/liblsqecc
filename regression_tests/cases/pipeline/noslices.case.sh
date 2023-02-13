INPUT="
DeclareLogicalQubitPatches 0,1
LogicalPauli 1 X
"
echo "$INPUT" | lsqecc_slicer -L compact --noslices | \
  sed "s/Made patch computation. Took [0-9]*.[0-9e\-]*s./Made patch computation. Took <time_removed_by_case_script>/"
