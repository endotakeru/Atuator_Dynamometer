import argparse
import csv
import json # (?)
import os # (?)
import queue # (?)
import threading # (?)
import time # (?)
from collections import deque # (?)
from datetime import datetime # (?)

import numpy as np
import serial

try:
    import matplotlib.pylot as plt
    from matplotlib.animation import FuncAnimation
    HAVE_MPL = True # (?)
except ImportError:
    HAVE_MPL = False # (?)

COLUMNS = ["t_m", "mode", "setpoint_rpm", "rpm", "torque_Nm", "voltage_V",
           "current_A", "elec_W", "brake_W", "eff", "servo_us", "demand"]

class SerialLink(threading.Thread): # (?)
    def __init__(self, port, baud, raw_csv_path, out_queue):
        super().__init__(daemon=True)
        self.ser = serial.Serial(port, baud, timeout=1.0)
        self.out_queue = out_queue
        self.latest = None
        self.running = True
        self._csv_f = open(raw_csv_path, "w", newline="")
        self._csv = csv.writer(self._csv_f)
        self._csv.writerow(["host_time"] + COLUMNS)
        time.sleep(2.0)
        self.ser.reset_input_buffer()

    def run(self): # (?)
        while self.running:
            try:
                line = self.ser.readline().decode("ascii", "ignore").strip()
            except Exception:
                continue
            if not line:
                continue
            if line.startswith("#") or line.startswith("t_ms"):
                print(line)
                continue
            parts = line.split(",")
            if len(parts) != len(COLUMNS):
                continue
            try:
                vals = [float(p) for p in parts]
            except ValueError:
                continue
            rec = dict(zip(COLUMNS, vals))
            rec["host_time"] = time.time()
            self.latest = rec
            self._csv.writerow([f"{rec['host_time']:.3f}"] + [rec[c] for c in COLUMNS])
            self._csv_f.flush()
            try:
                self.out_queue.put_nowait(rec)
            except queue.Full:
                pass

    def send(self, cmd): # (?)
        self.ser.write((cmd + "\n").encode("ascii"))
        self.ser.flush()

    def close(self): # (?)
        self.running = False
        try:
            self.send("R")
        except Exception:
            pass
        time.sleep(0.2)
        self._csv_f.close()
        self.ser.close()

class SweepController(threading.Thread):
    def __init__(self, link, setpoints, results, args): # (?)
        super().__init__(daemon=True)
        self.link = link
        self.setpoints = setpoints
        self.results = results
        self.args = args
        self.done = False
        self.status = "idle"

    def _wait_settle(self, target): # (?)
        t0 = time.time()
        held_since = None
        while time.time() - t0 < self.args.settle_timeout:
            rec = self.link.latest
            if rec is not None:
                if abs(rec["rpm"] - target) <= self.args.rpm_tol:
                    if held_since is None:
                        held_since = time.time()
                    elif time.time() - held_since >= self.args.settle_time:
                        return "settled"
                else:
                    held_since = None
            time.sleep(0.05)
        return "timeout"

    def _average_point(self, target): # (?)
        buf = []
        t0 = time.time()
        while time.time() - t0 < self.args.avg_time:
            rec = self.link.latest
            if rec is not None:
                buf.append(rec)
            time.sleep(0.05)
        if not buf:
            return None
        pt = {}
        for c in ["rpm", "torque_Nm", "voltage_V", "current_A", "servo_us"]:
            pt[c] = float(np.mean([b[c] for b in buf]))
        pt["rpm_std"] = float(np.std([b["rpm"] for b in buf]))
        pt["setpoint"] = target
        pt["brake_W"] = 2 * np.pi * pt["rpm"] * pt["torque_Nm"] / 60.0
        pt["elec_W"] = pt["voltage_V"] * pt["current_A"]
        pt["eff"] = pt["brake_W"] / pt["elec_W"] if pt["elec_W"] > 0.5 else 0.0
        return pt

    def run(self): # (?)
        print("\n# Sweep (speed) setpoints [rpm]:", self.setpoints)
        for target in self.setpoints:
            self.status = f"settling @ {target:g} rpm"
            print(f"# -> setpoint {target:g} rpm")
            self.link.send(f"S{target:g}")
            outcome = self._wait_settle(target)
            if outcome == "timeout":
                print(f"#   WARN: {target:g} rpm did not settle (recording anyway)")
                print( "#         brake saturated or relieving at the torque limit.")
            self.status = f"averaging @ {target:g} rpm"
            pt = self._average_point(target)
            if pt:
                pt["note"] = "" if outcome == "settled" else outcome
                self.results.append(pt)
                print(f"#   N={pt['rpm']:.0f}rpm  T={pt['torque_Nm']:.3f}Nm  "
                      f"V={pt['voltage_V']:.2f}  I={pt['current_A']:.2f}  "
                      f"BP={pt['brake_W']:.1f}W  IP={pt['elec_W']:.1f}W  "
                      f"eff={pt['eff']*100:.1f}%")
        self.link.send("R")
        self.status = "done"
        self.done = True
        self._write_summary()

    def _write_summary(self): # (?)
        path = self.args.out_prefix + "_sweep.csv"
        cols = ["setpoint", "rpm", "rpm_std", "torque_Nm", "voltage_V",
                "current_A", "elec_W", "brake_W", "eff", "servo_us", "note"]
        with open(path, "w", newline="") as f:
            w = csv.writer(f)
            w.writerow(["setpoint_rpm"] + cols[1:])
            for pt in self.results:
                row = []
                for c in cols:
                    v = pt.get(c, "")
                    row.append(f"{v:.5g}" if isinstance(v, float) else v)
                w.writerow(row)
        print(f"\n# Sweep summary written: {path}")
        if self.results:
            best = max(self.results, key=lambda p: p["eff"])
            print(f"# Peak efficiency {best['eff']*100:.1f}% at "
                  f"T={best['torque_Nm']:.3f} N·m, N={best['rpm']:.0f} rpm")


#Plotting with live animation
def run_plot(link, results, sweep): # (?)
    fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(12, 5))
    fig.canvas.manager.set_window_title("Dynamometer — constant-speed (rpm sweep)")
    live_hist = deque(maxlen=400)

    def update(_frame): # (?)
        rec = link.latest
        if rec is not None:
            live_hist.append((rec["rpm"], rec["torque_Nm"], rec["eff"]))
        ax1.clear(); ax2.clear()

        if results:
            rs = np.array([p["rpm"] for p in results])
            ts = np.array([p["torque_Nm"] for p in results])
            es = np.array([p["eff"] * 100 for p in results])
            order = np.argsort(rs)
            ax1.plot(rs[order], ts[order], "o-", color="#2f6fed", label="operating pts")
            ax2.plot(rs[order], es[order], "o-", color="#17a06b", label="operating pts")

        if live_hist:
            arr = np.array(live_hist)
            ax1.scatter(arr[:, 0], arr[:, 1], s=6, color="#b0b0b0", alpha=0.5, label="live")

        ax1.set_xlabel("Speed (rpm)"); ax1.set_ylabel("Torque (N·m)")
        ax1.set_title("Torque vs Speed"); ax1.grid(True, alpha=0.3); ax1.legend(loc="best")
        ax2.set_xlabel("Speed (rpm)")
        ax2.set_ylabel("Efficiency (%)"); ax2.set_title("Efficiency")
        ax2.grid(True, alpha=0.3); ax2.set_ylim(0, 100)

        status = sweep.status if sweep else "monitor"
        if rec is not None:
            fig.suptitle(f"[{status}]  N={rec['rpm']:.0f} rpm  T={rec['torque_Nm']:.3f} N·m  "
                         f"V={rec['voltage_V']:.2f} V  I={rec['current_A']:.2f} A  "
                         f"eff={rec['eff']*100:.1f}%  servo={rec['servo_us']:.0f}µs")
        return []

    _anim = FuncAnimation(fig, update, interval=200, cache_frame_data=False)
    plt.tight_layout()
    plt.show()


#Setpoint / profile resolution
def _range_list(start, stop, step): # (?)
    pts, n = [], start
    if step == 0:
        raise ValueError("step cannot be 0")
    while (step > 0 and n <= stop + 1e-9) or (step < 0 and n >= stop - 1e-9):
        pts.append(round(n, 6))
        n += step
    return pts


def load_dut_profile(path): # (?)
    with open(path) as f:
        prof = json.load(f)
    print(f"# DUT profile: {prof.get('name', path)}")
    return prof


def resolve_setpoints(args, profile): # (?)
    """Return (setpoints, torque_limit) honoring the documented precedence."""
    limit = args.torque_limit
    if args.setpoints:
        return [float(x) for x in args.setpoints.split(",")], limit
    if args.sweep:
        a, b, c = (float(x) for x in args.sweep.split(":"))
        return _range_list(a, b, c), limit
    if profile:
        sp = profile.get("rpm_setpoints")
        if not sp and "rpm_sweep" in profile:
            r = profile["rpm_sweep"]
            sp = _range_list(r["start"], r["stop"], r["step"])
        if not sp:
            raise SystemExit("DUT profile has no rpm_setpoints or rpm_sweep")
        return sp, (limit or profile.get("torque_limit_Nm"))
    raise SystemExit("need one of: --setpoints, --sweep, or --dut <profile.json>")


def main(): # (?)
    ap = argparse.ArgumentParser(description="Dynamometer DAQ / sweep controller")
    ap.add_argument("--port", required=True, help="Serial port, e.g. COM5 or /dev/ttyUSB0")
    ap.add_argument("--baud", type=int, default=115200)
    # setpoint sources (precedence: setpoints > sweep > profile)
    ap.add_argument("--dut", help="per-DUT JSON profile (see duts/)")
    ap.add_argument("--sweep", help="start:stop:step in rpm, e.g. 2500:250:-250")
    ap.add_argument("--setpoints", help="explicit comma list of rpm")
    ap.add_argument("--torque-limit", type=float,
                    help="firmware torque safeguard (N·m); overrides profile")
    # settle / averaging
    ap.add_argument("--rpm-tol", type=float, default=40, help="rpm settle tol")
    ap.add_argument("--settle-time", type=float, default=3.0, help="s held to accept")
    ap.add_argument("--settle-timeout", type=float, default=25.0, help="s max wait")
    ap.add_argument("--avg-time", type=float, default=3.0, help="s to average a point")
    ap.add_argument("--no-plot", action="store_true")
    ap.add_argument("--out-prefix", default=None)
    args = ap.parse_args()

    profile = load_dut_profile(args.dut) if args.dut else None
    setpoints, torque_limit = resolve_setpoints(args, profile)

    if args.out_prefix is None:
        tag = ""
        if profile and profile.get("name"):
            tag = "_" + "".join(c if c.isalnum() else "_" for c in profile["name"])[:24]
        args.out_prefix = "dyno" + tag + "_" + datetime.now().strftime("%Y%m%d_%H%M%S")

    raw_csv = args.out_prefix + "_raw.csv"
    q = queue.Queue(maxsize=1000)

    print(f"# Opening {args.port} @ {args.baud} …")
    link = SerialLink(args.port, args.baud, raw_csv, q)
    link.start()
    print(f"# Logging raw stream -> {raw_csv}")

    if torque_limit:
        link.send(f"X{torque_limit:g}")
        print(f"# Set firmware torque limit -> {torque_limit:g} N·m")
    span = (profile or {}).get("brake_rpm_span")
    if span:
        link.send(f"N{span:g}")
        print(f"# Set brake rpm span -> {span:g} rpm at full clamp")
    if profile and "pid" in profile:
        p = profile["pid"]
        link.send(f"G{p['kp']:g} {p['ki']:g} {p['kd']:g}")
        print(f"# Set speed PID gains -> {p}")
    time.sleep(0.3)

    results = []
    sweep = SweepController(link, setpoints, results, args)
    sweep.start()

    try:
        if HAVE_MPL and not args.no_plot:
            run_plot(link, results, sweep)
        else:
            while not sweep.done:
                time.sleep(0.5)
    except KeyboardInterrupt:
        print("\n# Interrupted.")
    finally:
        link.close()
        print("# Brake released, serial closed. Bye.")


if __name__ == "__main__": # (?)
    main()