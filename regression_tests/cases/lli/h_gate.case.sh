INPUT="
DeclareLogicalQubitPatches 0,1
HGate 1
HGate 1
HGate 0
"
echo "$INPUT" | lsqecc_slicer -l --compactlayout
