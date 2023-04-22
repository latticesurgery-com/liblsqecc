









RequestMagicState 25;BellPairInit 26 27 0:Z,25:X [BellPrepare (5,6),(5,5)];
BellPairInit 26 27 0:Z,25:X [Move (5,5),(6,5)];
MultiBodyMeasure 0:Z,26:Z;MultiBodyMeasure 27:X,25:X;
MeasureSinglePatch 26 X;MeasureSinglePatch 27 Z;MeasureSinglePatch 25 Z;RequestYState 28 0;
BellPairInit 29 30 0:Z,28:X [BellPrepare (5,4),(5,5)];
BellPairInit 29 30 0:Z,28:X [Move (5,5),(6,5)];
MultiBodyMeasure 0:Z,29:Z;MultiBodyMeasure 30:X,28:X;
MeasureSinglePatch 29 X;MeasureSinglePatch 30 Z;HGate 28;RotateSingleCellPatch 28;
BusyRegion (4,4),(4,5),StepsToClear(1);
BusyRegion (4,4),(4,5),StepsToClear(0);BellPairInit 31 32 0:Z,28:X [BellPrepare (5,4),(5,5)];
BellPairInit 31 32 0:Z,28:X [Move (5,5),(6,5)];
MultiBodyMeasure 0:Z,31:Z;MultiBodyMeasure 32:X,28:X;
MeasureSinglePatch 31 X;MeasureSinglePatch 32 Z;HGate 28;RotateSingleCellPatch 28;
BusyRegion (4,4),(4,5),StepsToClear(1);
BusyRegion (4,4),(4,5),StepsToClear(0);RequestYState 28 0;

