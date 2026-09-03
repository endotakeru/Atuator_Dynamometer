import argparse
import csv
import os

import numpy as np

try:
    import matplotlib.pyplot as plt
except ImportError:
    raise SystemExit("matplotlib required:  pip install matplotlib numpy")

def load_sweep(path): # Turn one sweep CSV into a dictionary of NumPy Arrays
    rows = []
    with open(path, newline="") as f:
        for rec in csv.DictReader(f):
            try:
                row = {k: (float(v) if v not in ("", None) else np.nan)
                       for k, v in rec.items() if k != "note"}
                # Keep the note as text. daq.py writes "timeout" here when a
                # setpoint never settled; dropping it made a bad point look
                # identical to a good one.
                row["_note"] = (rec.get("note") or "").strip()
                rows.append(row)
            except ValueError:
                continue
    if not rows:
        raise SystemExit(f"no usable rows in {path}")
    keys = [k for k in rows[0].keys() if k != "_note"]
    out = {k: np.array([r[k] for r in rows]) for k in keys}
    out["_note"] = np.array([r["_note"] for r in rows])   # text, kept out of the floats
    return out

def summarize(d, label): # Report a summary after each dataset
    eff = d["eff"] * 100
    if np.all(np.isnan(eff)):     # nanargmax raises on an all-NaN column
        return f"--- {label} ---\n  no usable efficiency data (all rows blank)"
    best = int(np.nanargmax(eff)) # Peak efficiency
    lines = [
        f"--- {label} ---",
        f"  points            : {len(eff)}",
        f"  speed range       : {np.nanmin(d['rpm']):.0f} - {np.nanmax(d['rpm']):.0f} rpm",
        f"  torque range      : {np.nanmin(d['torque_Nm']):.3f} - {np.nanmax(d['torque_Nm']):.3f} N.m",
        f"  peak brake power  : {np.nanmax(d['brake_W']):.2f} W",
        f"  PEAK EFFICIENCY   : {eff[best]:.1f}% at "
        f"T={d['torque_Nm'][best]:.3f} N.m, N={d['rpm'][best]:.0f} rpm, "
        f"V={d['voltage_V'][best]:.2f} V, I={d['current_A'][best]:.2f} A",
    ]
    return "\n".join(lines)

def main(): # Return 4 plots, x vs y: (RPM vs Nm, Nm vs Eff, RPM vs Eff, Nm vs W)
    ap = argparse.ArgumentParser(description="Analyze dynamometer sweep CSVs")
    ap.add_argument("csv", nargs="+", help="one or more *_sweep.csv files")
    ap.add_argument("--labels", help="comma-separated labels, one per file")
    ap.add_argument("--save", action="store_true", help="write PNG instead of showing")
    ap.add_argument("--out", default="dyno_analysis.png")
    args = ap.parse_args()

    labels = ([s.strip() for s in args.labels.split(",")] if args.labels
              else [os.path.splitext(os.path.basename(p))[0].replace("_sweep", "")
                    for p in args.csv])
    if len(labels) != len(args.csv):
        raise SystemExit("--labels count must match the number of CSV files")

    datasets = [load_sweep(p) for p in args.csv]

    fig, axes = plt.subplots(2, 2, figsize=(13, 9))
    (ax_ts, ax_et), (ax_es, ax_pw) = axes

    for d, lab in zip(datasets, labels):
        order_n = np.argsort(d["rpm"])
        order_t = np.argsort(d["torque_Nm"])

        ax_ts.plot(d["rpm"][order_n], d["torque_Nm"][order_n], "o-", label=lab)
        ax_et.plot(d["torque_Nm"][order_t], d["eff"][order_t] * 100, "o-", label=lab)
        ax_es.plot(d["rpm"][order_n], d["eff"][order_n] * 100, "o-", label=lab)
        ax_pw.plot(d["torque_Nm"][order_t], d["brake_W"][order_t], "o-",
                   label=f"{lab} mech")
        ax_pw.plot(d["torque_Nm"][order_t], d["elec_W"][order_t], "s--",
                   alpha=0.6, label=f"{lab} elec")

        # Ring any point that did not settle, so it reads as less trustworthy.
        bad = np.array([n != "" for n in d["_note"]])
        if bad.any():
            ax_ts.plot(d["rpm"][bad], d["torque_Nm"][bad], "o", ms=13, mfc="none",
                       mec="#d62728", mew=1.6, label=f"{lab} unsettled")

        if not np.all(np.isnan(d["eff"])):
            best = int(np.nanargmax(d["eff"]))
            ax_et.plot(d["torque_Nm"][best], d["eff"][best] * 100, "*",
                       ms=16, color="#d62728", zorder=5)

    ax_ts.set(xlabel="Speed (rpm)", ylabel="Torque (N·m)", title="Torque vs Speed")
    # An efficiency above 100% is impossible, so it is the clearest sign that
    # counts_per_Nm or the shunt value is wrong. Let the axis grow to show it
    # rather than drawing the point off the edge of the chart.
    eff_top = max([100.0] + [np.nanmax(d["eff"]) * 100 for d in datasets]) * 1.05
    ax_et.set(xlabel="Torque (N·m)", ylabel="Efficiency (%)",
              title="Efficiency vs Torque  (★ = peak)", ylim=(0, eff_top))
    ax_es.set(xlabel="Speed (rpm)", ylabel="Efficiency (%)",
              title="Efficiency vs Speed", ylim=(0, eff_top))
    if eff_top > 105:
        for ax in (ax_et, ax_es):
            ax.axhline(100, color="#d62728", ls=":", lw=1.2)
        print("# WARNING: efficiency exceeds 100% - check counts_per_Nm and INA_SHUNT_OHMS")
    ax_pw.set(xlabel="Torque (N·m)", ylabel="Power (W)",
              title="Mechanical vs Electrical Power")

    for ax in (ax_ts, ax_et, ax_es, ax_pw):
        ax.grid(True, alpha=0.3)
        ax.legend(loc="best", fontsize=8)

    plt.tight_layout()

    print()
    for d, lab in zip(datasets, labels):
        print(summarize(d, lab))
        print()

    if args.save:
        fig.savefig(args.out, dpi=150)
        print(f"# wrote {args.out}")
        plt.close(fig)
    else:
        plt.show()

if __name__ == "__main__":
    main()