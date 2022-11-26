INPUT="
DeclareLogicalQubitPatches 0,1,2,3,4,5,6
HGate 1
MultiBodyMeasure 1:X,4:Z
HGate 2
MultiBodyMeasure 2:Z,4:Z
"
echo "$INPUT" | ../build/lsqecc_slicer -l --compactlayout
