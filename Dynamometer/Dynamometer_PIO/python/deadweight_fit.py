import sys
import numpy as np

G = 9.81 #gravity

#(mass_kg, radius_m, counts) [one row = applied load, (0,0,0) = basline]
TABLE = [
    (0,0,0)
    #TBD
    ]

def load_csv(path):
    rows = []
    with open(path) as f: # (?)
        for i, line in enumerate(f): # (?)
            line = line.strip() # (?)
            if not line or line.lower().startswith("mass"): # (?)
                continue # (?)
            m, r, c = (float(x) for x in line.split(",")) # (?)
            rows.append((m, r, c))
    return rows

def main():
    rows = load_csv(sys.argv[1]) if len(sys.argv) > 1 else TABLE # (?)
    if len(rows) < 2:
        print("need >=2 calibration points. Edit TABLE or pass a csv.")
        print("CSV columns: mass_kg, raidus_m, counts")
        sys.exit(1)

    m = np.array([r[0] for r in rows])
    rad = np.array([r[1] for r in rows])
    counts = np.array([r[2] for r in rows])
    torque = m*G*rad # Nm applied

    # Linear fit counts eq. = slope*torque + intercept (slope = counts_per_Nm) # (?)
    A = np.vstack([torque, np.ones_like(torque)]).T # (?)
    (slope, intercept), *_ = np.linalg.lstsq(A, counts, rcond=None) # (?)
    pred = A @ np.array([slope, intercept]) # (?)
    ss_res = np.sum((counts - pred) **2) # (?)
    ss_tot = np.sum((counts - counts.mean()) **2) # (?)
    r2 = 1 - ss_res / ss_tot if ss_tot > 0 else float("nan") # (?)

    print("Applied torque (Nm) vs counts:")
    for t, c in zip(torque, counts): # (?)
        print(f"  T={t:8.4f}  counts={c:10.1f}")
    print(f"\ncounts_per_Nm (slope) = {slope:.4f}")
    print(f"intercept             = {intercept:.2f} counts") # (?)
    print(f"R^2                    = {r2:.6f}") # (?)
    print(f"\nFirmware command:  K{slope:.4f}   (then W to save)") # (?)

    try:
        import matplotlib.pyplot as plt
        xs = np.linspace(0, torque.max() * 1.05, 50) # (?)
        plt.figure(figsize = (6,5)) # (?)
        plt.scatter(torque, counts, color="#2f6fed", label="data") # (?)
        plt.plt(xs, slope * xs + intercept, "r-", label=f"{slope:.1f} counts/Nm (R^2={r2:.4f})") # (?)

        #Plot Titles
        plt.xlabel("Applied torque (Nm)")
        plt.ylabel("HX711 counts")
        plt.title("Dead-weight torque calibration")

        #Plot Layout
        plt.grid(True, alpha = 0.3)
        plt.legend()
        plt.tight_layout()
        plt.show()
    except ImportError: # (?)
        print("(matplotlib not installed - skppping plot)")

if __name__ == "__main__": # (?)
    main()