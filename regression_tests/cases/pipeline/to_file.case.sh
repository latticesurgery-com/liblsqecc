INPUT="
DeclareLogicalQubitPatches 0,1
LogicalPauli 1 X
"
echo "$INPUT" | lsqecc_slicer --compactlayout -o out.json > stdout_save
echo "stdout_save: (should show some stats)"
cat stdout_save | sed "s/Made patch computation. Took [0-9]*.[0-9e\-]*s./Made patch computation. Took <time_removed_by_case_script>/"
echo "out.json: (should have content)"
cat out.json
rm stdout_save
rm out.json

