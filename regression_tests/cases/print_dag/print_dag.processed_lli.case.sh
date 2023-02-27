INPUT="
DeclareLogicalQubitPatches 0,1,2,3,4,5,6
MultiBodyMeasure 0:X,4:X
MultiBodyMeasure 0:Z,4:Z
"
echo "$INPUT" | lsqecc_slicer --printdag -L compact --printdag processedlli