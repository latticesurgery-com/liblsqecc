#!/usr/bin/env python3
"""Plot random circuit slicer benchmark results without requiring a display."""

from __future__ import annotations

import argparse
from pathlib import Path

import matplotlib

matplotlib.use("Agg")

import matplotlib.pyplot as plt
import numpy as np
import pandas as pd


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--root", type=Path, default=Path.cwd(), help="Project root directory.")
    parser.add_argument("--artifact-dir", type=Path, default=Path("benchmark_artifacts"), help="Artifact directory.")
    parser.add_argument(
        "--csv-path",
        type=Path,
        default=None,
        help="Benchmark CSV path. Defaults to <artifact-dir>/random_circuit_slicer_benchmark.csv",
    )
    parser.add_argument(
        "--plot-path",
        type=Path,
        default=None,
        help="Output plot PNG path. Defaults to <artifact-dir>/plot.png",
    )
    return parser.parse_args()


def resolve_under_root(root: Path, maybe_relative: Path) -> Path:
    return maybe_relative if maybe_relative.is_absolute() else root / maybe_relative


def build_summary(df: pd.DataFrame) -> pd.DataFrame:
    return (
        df.groupby("num_qubits")
        .agg(
            wall_time_mean=("wall_time_s", "mean"),
            wall_time_min=("wall_time_s", "min"),
            wall_time_max=("wall_time_s", "max"),
            patch_time_mean=("patch_compute_time_s", "mean"),
            patch_time_min=("patch_compute_time_s", "min"),
            patch_time_max=("patch_compute_time_s", "max"),
            total_volume_mean=("total_volume", "mean"),
            total_volume_min=("total_volume", "min"),
            total_volume_max=("total_volume", "max"),
            distillation_volume_mean=("distillation_volume", "mean"),
            distillation_volume_min=("distillation_volume", "min"),
            distillation_volume_max=("distillation_volume", "max"),
            unused_routing_volume_mean=("unused_routing_volume", "mean"),
            unused_routing_volume_min=("unused_routing_volume", "min"),
            unused_routing_volume_max=("unused_routing_volume", "max"),
            dead_volume_mean=("dead_volume", "mean"),
            dead_volume_min=("dead_volume", "min"),
            dead_volume_max=("dead_volume", "max"),
            other_active_volume_mean=("other_active_volume", "mean"),
            other_active_volume_min=("other_active_volume", "min"),
            other_active_volume_max=("other_active_volume", "max"),
            ls_instructions_read_mean=("ls_instructions_read", "mean"),
            ls_instructions_read_min=("ls_instructions_read", "min"),
            ls_instructions_read_max=("ls_instructions_read", "max"),
            slices_count_mean=("slices_count", "mean"),
            slices_count_min=("slices_count", "min"),
            slices_count_max=("slices_count", "max"),
        )
        .reset_index()
    )


def create_plot(summary_df: pd.DataFrame, plot_path: Path) -> None:
    fig, axes = plt.subplots(1, 2, figsize=(14, 5))

    wall_time_yerr = np.vstack(
        [
            summary_df["wall_time_mean"] - summary_df["wall_time_min"],
            summary_df["wall_time_max"] - summary_df["wall_time_mean"],
        ]
    )
    line_runtime = axes[0].errorbar(
        summary_df["num_qubits"],
        summary_df["wall_time_mean"],
        yerr=wall_time_yerr,
        marker="o",
        capsize=4,
        label="Wall time mean +/- range",
    )

    qubit_step_vals = summary_df["num_qubits"].diff().dropna()
    bar_width = 0.65 * (float(qubit_step_vals.min()) if not qubit_step_vals.empty else 1.0)
    ax0_right = axes[0].twinx()
    bars_total = ax0_right.bar(
        summary_df["num_qubits"],
        summary_df["total_volume_mean"],
        width=bar_width,
        alpha=0.28,
        color="tab:orange",
        label="Total volume mean",
    )

    axes[0].set_title("Runtime and Total Volume vs Number of Qubits")
    axes[0].set_xlabel("num_qubits")
    axes[0].set_ylabel("seconds")
    ax0_right.set_ylabel("total volume")
    axes[0].grid(True, alpha=0.3)

    handles = [line_runtime.lines[0], bars_total]
    labels = ["Wall time mean +/- range", "Total volume mean"]
    axes[0].legend(handles, labels, loc="upper left")

    volume_series = [
        ("distillation_volume", "Distillation volume"),
        ("unused_routing_volume", "Unused routing volume"),
        ("other_active_volume", "Other active volume"),
        ("ls_instructions_read", "LS Instructions read"),
        ("slices_count", "Slices count"),
    ]
    for col_name, label in volume_series:
        yerr = np.vstack(
            [
                summary_df[f"{col_name}_mean"] - summary_df[f"{col_name}_min"],
                summary_df[f"{col_name}_max"] - summary_df[f"{col_name}_mean"],
            ]
        )
        axes[1].errorbar(
            summary_df["num_qubits"],
            summary_df[f"{col_name}_mean"],
            yerr=yerr,
            marker="o",
            capsize=4,
            label=label,
        )

    axes[1].set_title("Circuit Statistics vs Number of Qubits")
    axes[1].set_xlabel("num_qubits")
    axes[1].set_ylabel("value")
    axes[1].grid(True, alpha=0.3)
    axes[1].legend(loc="upper left")

    plot_path.parent.mkdir(parents=True, exist_ok=True)
    plt.tight_layout()
    plt.savefig(plot_path)
    plt.close(fig)


def main() -> int:
    args = parse_args()

    root = args.root.resolve()
    artifact_dir = resolve_under_root(root, args.artifact_dir)

    csv_path = resolve_under_root(root, args.csv_path) if args.csv_path else artifact_dir / "random_circuit_slicer_benchmark.csv"
    plot_path = resolve_under_root(root, args.plot_path) if args.plot_path else artifact_dir / "plot.png"

    if not csv_path.exists():
        raise FileNotFoundError(f"Benchmark CSV not found: {csv_path}")

    df = pd.read_csv(csv_path)
    plot_df = df[df["returncode"] == 0].copy()
    if plot_df.empty:
        print("No successful runs to plot.")
        return 0

    summary_df = build_summary(plot_df)
    create_plot(summary_df, plot_path)

    print(f"Loaded benchmark CSV: {csv_path}")
    print(f"Saved plot to: {plot_path}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
