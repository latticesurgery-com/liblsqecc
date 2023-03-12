INPUT="
DeclareLogicalQubitPatches 0,1,2,3,4,5,6
Init 100 +
MultiBodyMeasure 0:X,4:X
MultiBodyMeasure 5:Z,6:Z
MultiBodyMeasure 2:Z,3:Z
RotateSingleCellPatch 100
HGate 1
HGate 1
RotateSingleCellPatch 1
"
echo "$INPUT" | lsqecc_slicer -l -L compact
