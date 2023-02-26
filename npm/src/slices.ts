type Orientation = "Top" | "Bottom" | "Left" | "Right";
type EdgeType = "Solid" | "SolidStiched" | "Dashed" | "DashedStiched" | "AncillaJoin";

type VisualArrayCell = {
    edges: {
        [key in Orientation]: EdgeType;
    };
    patch_type: "Qubit" | "DistillationQubit" | "Ancilla";
    test: string;
    activity: {
        op: "I" | "X" | "Y" | "Z";
        activity_type: "Unitary" | "Measurement";
    };
};

type Slice = Array<Array<VisualArrayCell>>;
type Slices = Array<Slice>;

export type { Slices, Slice, VisualArrayCell };
