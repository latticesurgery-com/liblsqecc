









RequestMagicState 25 0;MultiBodyMeasure 0:Z,25:Z;
MeasureSinglePatch 25 X;RequestYState 26 0;BellPairInit 27 28 0:Z,26:X;
BusyRegion (5,4),(5,5),(6,5),StepsToClear(1);
BusyRegion (5,4),(5,5),(6,5),StepsToClear(0);MultiBodyMeasure 0:Z,27:Z;MultiBodyMeasure 28:X,26:X;
MeasureSinglePatch 27 X;MeasureSinglePatch 28 Z;HGate 26;RotateSingleCellPatch 26;
BusyRegion (4,4),(4,5),StepsToClear(1);
BusyRegion (4,4),(4,5),StepsToClear(0);BellPairInit 29 30 0:Z,26:X;
BusyRegion (5,4),(5,5),(6,5),StepsToClear(1);
BusyRegion (5,4),(5,5),(6,5),StepsToClear(0);MultiBodyMeasure 0:Z,29:Z;MultiBodyMeasure 30:X,26:X;
MeasureSinglePatch 29 X;MeasureSinglePatch 30 Z;HGate 26;RotateSingleCellPatch 26;
BusyRegion (4,4),(4,5),StepsToClear(1);
BusyRegion (4,4),(4,5),StepsToClear(0);RequestYState 26 0;

