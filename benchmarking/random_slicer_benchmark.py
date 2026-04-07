#!/usr/bin/env python3
"""Run random Qiskit circuit sweeps against the LSQECC slicer on headless systems."""

from __future__ import annotations

import argparse
import re
import subprocess
import time
from pathlib import Path

import numpy as np
import pandas as pd
from qiskit import QuantumCircuit
from qiskit.circuit.exceptions import CircuitError
from qiskit.circuit.library import CXGate, HGate, TGate
from qiskit.qasm2 import dumps


STATS_PATTERNS = {
    "ls_instructions_read": re.compile(r"LS\s+Instructions\s+read\s+(\d+)"),
    "slices_count": re.compile(r"Slices\s+(\d+)"),
    "total_volume": re.compile(r"Total volume:\s*(\d+)"),
    "distillation_volume": re.compile(r"Distillation volume:\s*(\d+)"),
    "unused_routing_volume": re.compile(r"Unused routing volume:\s*(\d+)"),
    "dead_volume": re.compile(r"Dead volume:\s*(\d+)"),
    "other_active_volume": re.compile(r"Other active volume:\s*(\d+)"),
    "patch_compute_time_s": re.compile(r"Made patch computation\. Took\s*([0-9.eE+\-]+)s"),
}

RESULT_COLUMNS = [
    "num_qubits",
    "cycle",
    "depth",
    "seed",
    "qasm_path",
    "timeout_s",
    "benchmark_elapsed_s",
    "timed_out",
    "returncode",
    "wall_time_s",
    "patch_compute_time_s",
    "ls_instructions_read",
    "slices_count",
    "total_volume",
    "distillation_volume",
    "unused_routing_volume",
    "dead_volume",
    "other_active_volume",
    "stdout",
    "stderr",
]


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--root", type=Path, default=Path.cwd(), help="Project root directory.")
    parser.add_argument("--artifact-dir", type=Path, default=Path("benchmark_artifacts"), help="Artifact output directory.")
    parser.add_argument("--qasm-subdir", default="qasm", help="Subdirectory under artifact dir for QASM files.")
    parser.add_argument(
        "--csv-name",
        default="random_circuit_slicer_benchmark.csv",
        help="Filename for benchmark CSV inside artifact dir.",
    )
    parser.add_argument(
        "--layout-file",
        type=Path,
        default=Path("layout_80x80.txt"),
        help="Layout file path passed to the slicer.",
    )
    parser.add_argument(
        "--slicer-bin",
        type=Path,
        default=None,
        help="Explicit path to lsqecc_slicer binary. Auto-discovered when omitted.",
    )
    parser.add_argument("--qubit-start", type=int, default=5, help="Starting number of qubits.")
    parser.add_argument("--qubit-step", type=int, default=5, help="Qubit increment after each completed n.")
    parser.add_argument("--cycles-per-n", type=int, default=1, help="Number of random circuits per qubit count.")
    parser.add_argument("--max-benchmark-timeout-min", type=int, default=100, help="Total benchmark timeout in minutes.")
    parser.add_argument(
        "--max-cycle-timeout-s",
        type=int,
        default=None,
        help="Timeout in seconds per slicer invocation. Defaults to total benchmark timeout.",
    )
    parser.add_argument("--max-operands", type=int, default=2, choices=[1, 2], help="Maximum gate operands.")
    parser.add_argument("--seed-base", type=int, default=42, help="Base seed for deterministic random circuits.")
    parser.add_argument(
        "--depth-power",
        type=int,
        default=3,
        help="Circuit depth is computed as int(num_qubits ** depth_power).",
    )
    parser.add_argument(
        "--max-qubits",
        type=int,
        default=None,
        help="Optional hard stop after this qubit count is processed.",
    )
    return parser.parse_args()


def resolve_under_root(root: Path, maybe_relative: Path) -> Path:
    return maybe_relative if maybe_relative.is_absolute() else root / maybe_relative


def find_slicer_binary(root: Path, explicit_bin: Path | None) -> Path:
    if explicit_bin is not None:
        slicer = resolve_under_root(root, explicit_bin)
        if slicer.exists():
            return slicer
        raise FileNotFoundError(f"Provided slicer binary does not exist: {slicer}")

    candidates = [
        root / "build" / "lsqecc_slicer_main",
        root / "build" / "lsqecc_slicer",
        root / "build" / "bin" / "lsqecc_slicer_main",
        root / "build" / "bin" / "lsqecc_slicer",
        root / "lsqecc_slicer",
    ]
    slicer = next((p for p in candidates if p.exists()), None)
    if slicer is None:
        raise FileNotFoundError(
            "Could not find lsqecc slicer executable. Build project first (e.g., cmake --build build)."
        )
    return slicer


def qiskit_random_circuit(num_qubits: int, depth: int, max_operands: int = 2, seed: int | None = None) -> QuantumCircuit:
    if max_operands < 1 or max_operands > 2:
        raise CircuitError("Parameter max_operands must be between 1 and 2")

    one_q_ops = [HGate, TGate]
    two_q_ops = [CXGate]

    qc = QuantumCircuit(num_qubits)

    if seed is None:
        seed = np.random.randint(0, np.iinfo(np.int32).max)

    rng = np.random.default_rng(seed)

    for _ in range(depth):
        remaining_qubits = list(range(num_qubits))

        while remaining_qubits:
            max_possible_operands = min(len(remaining_qubits), max_operands)
            num_operands = rng.integers(1, max_possible_operands + 1)

            rng.shuffle(remaining_qubits)
            operands = remaining_qubits[:num_operands]
            remaining_qubits = [q for q in remaining_qubits if q not in operands]

            if num_operands == 1:
                gate_class = rng.choice(one_q_ops)
                gate = gate_class()
                qc.append(gate, [operands[0]])
            elif num_operands == 2:
                gate_class = rng.choice(two_q_ops)
                gate = gate_class()
                qc.append(gate, operands)

    return qc


def save_to_qasm20(circuit: QuantumCircuit, output_path: Path) -> None:
    output_path.write_text(dumps(circuit))


def parse_slicer_stats(stdout_text: str) -> dict[str, float | int]:
    parsed: dict[str, float | int] = {}
    for key, pattern in STATS_PATTERNS.items():
        match = pattern.search(stdout_text)
        if match:
            value = match.group(1)
            parsed[key] = float(value) if "time" in key else int(value)
        else:
            parsed[key] = np.nan
    return parsed


def run_slicer_on_qasm(
    slicer_bin: Path,
    qasm_path: Path,
    layout_file: Path,
    timeout_s: int | None = None,
) -> dict[str, object]:
    cmd = [
        str(slicer_bin),
        "-q",
        "-i",
        str(qasm_path),
        "--noslices",
        "-l",
        str(layout_file),
        "-f",
        "stats",
        "--graceful",
    ]

    t0 = time.perf_counter()
    try:
        result = subprocess.run(cmd, capture_output=True, text=True, timeout=timeout_s)
        elapsed_s = time.perf_counter() - t0
        stdout_text = result.stdout
        stderr_text = result.stderr
        returncode = result.returncode
        timed_out = False
    except subprocess.TimeoutExpired as exc:
        elapsed_s = time.perf_counter() - t0
        stdout_text = exc.stdout or ""
        stderr_text = (exc.stderr or "") + f"\nTimed out after {timeout_s} seconds"
        returncode = -1
        timed_out = True

    parsed = parse_slicer_stats(stdout_text)
    parsed.update(
        {
            "returncode": returncode,
            "wall_time_s": elapsed_s,
            "timed_out": timed_out,
            "stdout": stdout_text,
            "stderr": stderr_text,
        }
    )
    return parsed


def main() -> int:
    args = parse_args()

    root = args.root.resolve()
    artifact_dir = resolve_under_root(root, args.artifact_dir)
    qasm_dir = artifact_dir / args.qasm_subdir
    csv_path = artifact_dir / args.csv_name
    layout_file = resolve_under_root(root, args.layout_file)

    if not layout_file.exists():
        raise FileNotFoundError(f"Layout file not found: {layout_file}")

    slicer_bin = find_slicer_binary(root, args.slicer_bin)

    artifact_dir.mkdir(parents=True, exist_ok=True)
    qasm_dir.mkdir(parents=True, exist_ok=True)

    max_benchmark_timeout_s = args.max_benchmark_timeout_min * 60
    max_cycle_timeout_s = args.max_cycle_timeout_s or max_benchmark_timeout_s

    records: list[dict[str, object]] = []
    benchmark_start = time.perf_counter()
    n = args.qubit_start

    print(f"Using slicer executable: {slicer_bin}")
    print(f"QASM output directory: {qasm_dir}")
    print(
        f"Starting qubit sweep from n={args.qubit_start} in steps of {args.qubit_step}. "
        f"Stops after {max_benchmark_timeout_s // 60} minutes, timeout, interrupt, or --max-qubits."
    )

    try:
        while True:
            if args.max_qubits is not None and n > args.max_qubits:
                print(f"Reached --max-qubits={args.max_qubits}. Stopping sweep.")
                break

            elapsed_total_s = time.perf_counter() - benchmark_start
            if elapsed_total_s >= max_benchmark_timeout_s:
                print(f"Reached total benchmark timeout after {elapsed_total_s:.1f}s. Stopping sweep.")
                break

            depth = int(n**args.depth_power)
            print(
                f"Running {args.cycles_per_n} cycles on {n} qubits "
                f"(elapsed {elapsed_total_s:.1f}s/{max_benchmark_timeout_s}s)"
            )

            for cycle in range(args.cycles_per_n):
                elapsed_total_s = time.perf_counter() - benchmark_start
                if elapsed_total_s >= max_benchmark_timeout_s:
                    print(
                        f"Reached total benchmark timeout during n={n}, cycle={cycle + 1}. "
                        "Stopping sweep."
                    )
                    break

                seed = args.seed_base + n * 1000 + cycle
                circuit = qiskit_random_circuit(
                    num_qubits=n,
                    depth=depth,
                    max_operands=args.max_operands,
                    seed=seed,
                )

                qasm_path = qasm_dir / f"random_q{n}_c{cycle}_d{depth}.qasm"
                save_to_qasm20(circuit, qasm_path)

                stats = run_slicer_on_qasm(
                    slicer_bin=slicer_bin,
                    qasm_path=qasm_path,
                    layout_file=layout_file,
                    timeout_s=max_cycle_timeout_s,
                )
                stats.update(
                    {
                        "num_qubits": n,
                        "cycle": cycle,
                        "depth": depth,
                        "seed": seed,
                        "qasm_path": str(qasm_path),
                        "timeout_s": max_cycle_timeout_s,
                        "benchmark_elapsed_s": elapsed_total_s,
                    }
                )
                records.append(stats)

                status = "timed out" if stats["timed_out"] else f"rc={stats['returncode']}"
                print(
                    f"n={n:>3}, cycle={cycle + 1:>2}/{args.cycles_per_n}, depth={depth:>6}, "
                    f"{status}, wall={stats['wall_time_s']:.4f}s, total_volume={stats['total_volume']}, "
                    f"ls_instr={stats['ls_instructions_read']}, slices={stats['slices_count']}"
                )
            else:
                n += args.qubit_step
                continue

            break

    except KeyboardInterrupt:
        print("Manual interrupt received. Finalizing results collected so far.")

    df = pd.DataFrame(records)
    if df.empty:
        df = pd.DataFrame(columns=RESULT_COLUMNS)
    else:
        df = df.sort_values(["num_qubits", "cycle"]).reset_index(drop=True)

    df.to_csv(csv_path, index=False)
    print(f"Saved results to: {csv_path}")

    failed = df[df["returncode"] != 0] if not df.empty else df
    if not failed.empty:
        print("Some runs failed. First few stderr snippets:")
        for _, row in failed.head(3).iterrows():
            print(f"\nnum_qubits={row['num_qubits']} stderr:")
            print((row["stderr"] or "")[:600])
    else:
        print("All runs succeeded.")

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
