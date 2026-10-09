"""
Static side-by-side comparison of the ORIGINAL (broken) and FIXED swing trajectory
from quadruped.ino. No animation: one figure, saved as a PNG.

Top row    : side-view foot path of one leg over a full gait cycle (colour = time).
Bottom row : the leg's pose on the tick just before and just after the swing->stance switch.

Run:  python gait_side_by_side.py            (saves gait_comparison.png and opens it)
      python gait_side_by_side.py --save my_figure.png
Needs: numpy, matplotlib
"""
import argparse
import math
import numpy as np
import matplotlib.pyplot as plt

# ---- firmware constants (mm) ----
L_THIGH, L_SHANK = 55.0, 57.0
GROUND, STEP_LEN, STEP_H, STEPDX = 60.0, 25.0, 5.0, 1.0
STEPS = int(STEP_LEN / STEPDX)


def ik(x, z):
    """ik.ino, planar part. Returns (thetaT, thetaK) in degrees; z is DOWN from the hip."""
    R = math.hypot(x, z)
    knee = math.acos((L_THIGH**2 + L_SHANK**2 - R * R) / (2 * L_THIGH * L_SHANK))
    thigh = math.acos((L_THIGH**2 - L_SHANK**2 + R * R) / (2 * L_THIGH * R))
    return math.degrees(math.atan2(x, z) - thigh), math.degrees(math.pi - knee)


def leg_pose(x, z):
    """Hip, knee, foot as (forward, height-below-hip as negative y) for a side-view stick figure."""
    T, K = (math.radians(a) for a in ik(x, z))
    knee = (L_THIGH * math.sin(T), L_THIGH * math.cos(T))
    foot = (knee[0] + L_SHANK * math.sin(T + K), knee[1] + L_SHANK * math.cos(T + K))
    return [(0, 0), (knee[0], -knee[1]), (foot[0], -foot[1])]


def swing_z(xs, fixed):
    if fixed:   # starts and ends on the ground, peaks at 2 * STEP_H
        return GROUND - STEP_H * (1 + math.cos(xs / STEP_LEN * 2 * math.pi))
    return GROUND - STEP_H - STEP_H * math.cos(xs / STEP_LEN * math.pi)   # starts/ends STEP_H up


def one_cycle(fixed):
    """Leg A over one full cycle (swing phase then stance phase), as the firmware would run it."""
    ticks_per_phase = STEPS if fixed else STEPS + 1       # n >= steps vs n > steps
    pts = []
    for n in range(ticks_per_phase):                      # phase 0: swing
        x = -STEP_LEN / 2 + STEPDX * n
        pts.append((x, swing_z(x, fixed)))
    for n in range(ticks_per_phase):                      # phase 1: stance
        pts.append((STEP_LEN / 2 - STEPDX * n, GROUND))
    return np.array(pts), ticks_per_phase


def stats(pts):
    """Cyclic tick-to-tick changes (the sequence repeats, so the last tick wraps to the first)."""
    prev = np.roll(pts, 1, axis=0)
    dz = np.abs(pts[:, 1] - prev[:, 1])
    dx = np.abs(pts[:, 0] - prev[:, 0])
    knee = np.array([170 - ik(x, z)[1] for x, z in pts])          # front-right knee servo (move1.ino)
    dk = np.abs(knee - np.roll(knee, 1))
    return dz.max(), int((dx < 1e-9).sum()), dk.max(), dz


def main(save):
    fig, axes = plt.subplots(2, 2, figsize=(12, 8.5), gridspec_kw={"height_ratios": [1, 1.25]},
                             constrained_layout=True)
    cfg = [(False, "ORIGINAL (broken)", "tab:red"), (True, "FIXED", "tab:green")]

    for col, (fixed, title, color) in enumerate(cfg):
        pts, per_phase = one_cycle(fixed)
        height = GROUND - pts[:, 1]
        jump, stalls, knee_jump, dz = stats(pts)

        # ---------- top: foot path ----------
        ax = axes[0, col]
        ax.plot(pts[:, 0], height, "-", color="0.75", lw=1, zorder=1)
        sc = ax.scatter(pts[:, 0], height, c=np.arange(len(pts)), cmap="viridis", s=22, zorder=2)
        ax.axhline(0, color="k", lw=1)
        for i in np.where(dz > 2.0)[0]:                    # draw the snaps as thick dashed segments
            a, b = pts[i - 1], pts[i]
            ax.plot([a[0], b[0]], [GROUND - a[1], GROUND - b[1]], "--", color="red", lw=3, zorder=3)
            ax.annotate(f"{dz[i]:.0f} mm snap", ((a[0] + b[0]) / 2, (GROUND - a[1] + GROUND - b[1]) / 2),
                        xytext=(8 if b[0] > 0 else -8, 0), textcoords="offset points",
                        ha="left" if b[0] > 0 else "right", va="center", color="red", fontsize=9)
        ax.set_title(f"{title}\nmax foot jump {jump:.2f} mm | stalled ticks/cycle {stalls} | "
                     f"max knee-servo jump {knee_jump:.1f} deg", fontsize=10, color=color)
        ax.set_xlabel("foot x (mm, + = forward)")
        ax.set_ylabel("foot height above ground (mm)")
        ax.set_xlim(-22, 22); ax.set_ylim(-2, 13)
        ax.grid(alpha=0.3)
        if col == 1:
            fig.colorbar(sc, ax=axes[0, :], label="tick in cycle", shrink=0.8, pad=0.02, location="right")

        # ---------- bottom: pose just before / after the swing -> stance switch ----------
        ax2 = axes[1, col]
        before, after = pts[per_phase - 1], pts[per_phase]
        for p, c, lab in ((before, "tab:blue", "last swing tick"), (after, "tab:orange", "first stance tick")):
            xy = np.array(leg_pose(*p))
            ax2.plot(xy[:, 0], xy[:, 1], "o-", color=c, lw=3, ms=6, label=lab)
        ax2.axhline(-GROUND, color="k", lw=1)
        ax2.text(-48, -GROUND - 4, "ground", fontsize=8, va="top")
        ax2.set_aspect("equal"); ax2.set_xlim(-50, 70); ax2.set_ylim(-80, 8)
        ax2.set_title(f"Leg pose across the swing -> stance switch\n"
                      f"foot height {GROUND - before[1]:.1f} mm  ->  {GROUND - after[1]:.1f} mm", fontsize=10)
        ax2.set_xlabel("x (mm)"); ax2.set_ylabel("y (mm), hip at 0")
        ax2.legend(loc="lower right", fontsize=8)
        ax2.grid(alpha=0.3)

    fig.suptitle("quadruped.ino swing trajectory: original vs fixed (one leg, one gait cycle)", fontsize=13)
    fig.savefig(save, dpi=140)
    print("saved", save)
    plt.show()


if __name__ == "__main__":
    ap = argparse.ArgumentParser()
    ap.add_argument("--save", default="gait_comparison.png")
    main(ap.parse_args().save)
