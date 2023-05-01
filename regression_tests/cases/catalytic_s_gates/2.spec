









RequestMagicState 25 0;MultiBodyMeasure 0:Z,25:Z;MeasureSinglePatch 25 X;RequestYState 26 0;Init 27 |+> 0:Z;
MultiBodyMeasure 0:Z,27:Z;
MultiBodyMeasure 27:X,26:X;MeasureSinglePatch 27 X;
HGate 26;RotateSingleCellPatch 26;
BusyRegion (4,4),(4,5),StepsToClear(2);
BusyRegion (4,4),(4,5),StepsToClear(1);
BusyRegion (4,4),(4,5),StepsToClear(0);Init 28 |+> 0:Z;MultiBodyMeasure 0:Z,28:Z;
MultiBodyMeasure 28:X,26:X;MeasureSinglePatch 28 X;
HGate 26;RotateSingleCellPatch 26;
BusyRegion (4,4),(4,5),StepsToClear(2);
BusyRegion (4,4),(4,5),StepsToClear(1);
BusyRegion (4,4),(4,5),StepsToClear(0);RequestYState 26 0;

