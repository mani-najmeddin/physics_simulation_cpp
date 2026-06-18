"""
plot.py  —  Quantum Gravity Physics Engine visualizer
Reads CSV files written by the C++ engine and plots all 6 modules.

Usage:
    1. Compile & run the C++ engine first:
           g++ -O2 -o engine quantum_gravity.cpp && ./engine
    2. Then run this script:
           python3 plot.py
"""

import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
import matplotlib.gridspec as gridspec
from matplotlib.patches import Circle
from matplotlib.collections import LineCollection
import numpy as np
import csv, os, sys

# ── Output directory (must match C++ mkdir "output") ──
OUT_DIR = "output"

def load(filename):
    """Load a CSV from the output directory into a dict of numpy arrays."""
    path = os.path.join(OUT_DIR, filename)
    if not os.path.exists(path):
        print(f"  [MISSING] {path}  — did you run the C++ engine first?")
        return None
    rows = []
    with open(path) as f:
        reader = csv.DictReader(f)
        headers = reader.fieldnames
        for row in reader:
            rows.append(row)
    if not rows:
        return None
    return {h: np.array([float(r[h]) for r in rows]) for h in headers}

# ── Dark palette ──────────────────────────────────────
BG      = '#0a0a0f'
PANEL   = '#10101a'
GRID    = '#1e1e30'
ACCENT1 = '#4fc3f7'   # ice blue
ACCENT2 = '#f9a825'   # amber
ACCENT3 = '#69f0ae'   # mint
ACCENT4 = '#ff6b6b'   # coral
ACCENT5 = '#ce93d8'   # lavender
WHITE   = '#e8eaf6'
DIM     = '#5c5c80'

SNAP_COLORS = [ACCENT1, ACCENT5, ACCENT3, ACCENT2]

def style_panel(ax, title):
    ax.set_facecolor(PANEL)
    for sp in ax.spines.values():
        sp.set_color(GRID)
    ax.tick_params(colors=DIM, labelsize=8)
    ax.xaxis.label.set_color(DIM)
    ax.yaxis.label.set_color(DIM)
    ax.set_title(title, color=ACCENT1, fontsize=11, fontfamily='monospace',
                 pad=10, loc='left', fontweight='bold')
    ax.grid(color=GRID, lw=0.5, alpha=0.6)

# ═══════════════════════════════════════════════════════
fig = plt.figure(figsize=(20, 26), facecolor=BG)
fig.suptitle('QUANTUM GRAVITY PHYSICS ENGINE', fontsize=22, color=WHITE,
             fontfamily='monospace', fontweight='bold', y=0.985)

gs = gridspec.GridSpec(3, 2, figure=fig, hspace=0.42, wspace=0.32,
                       left=0.07, right=0.96, top=0.96, bottom=0.03)

# ════════════════════════════════════════════════════════
# 01  FEYNMAN PATH INTEGRAL  — output/path_integral.csv
# ════════════════════════════════════════════════════════
ax1 = fig.add_subplot(gs[0, 0])
style_panel(ax1, '01  FEYNMAN PATH INTEGRAL')

d = load("path_integral.csv")
if d is not None:
    # Column names: path_id, is_classical, t0..tN, action
    t_cols = [k for k in d.keys() if k.startswith('t') and k[1:].isdigit()]
    t_vals = np.array([float(k[1:]) for k in t_cols])
    # Normalize t to [0,1]
    t_norm = t_vals / t_vals.max()

    path_ids   = d['path_id'].astype(int)
    is_classical = d['is_classical'].astype(int)
    actions    = d['action']

    # Quantum paths
    for i in np.where(is_classical == 0)[0]:
        path_y = np.array([d[k][i] for k in t_cols])
        S = actions[i]
        alpha = 0.12 + 0.25 * np.exp(-abs(S) / 4)
        color = ACCENT5 if i % 3 == 0 else ACCENT1
        ax1.plot(t_norm, path_y, color=color, lw=0.6, alpha=alpha)

    # Classical path
    for i in np.where(is_classical == 1)[0]:
        path_y = np.array([d[k][i] for k in t_cols])
        ax1.plot(t_norm, path_y, color=ACCENT2, lw=2.4, zorder=5,
                 label=f'Classical path  S={actions[i]:.3f}')
        ax1.scatter([0, 1], [path_y[0], path_y[-1]], color=ACCENT3, s=60, zorder=6)
        ax1.text(0,    path_y[0] - 0.15, f'xᵢ={path_y[0]:.1f}',
                 color=ACCENT3, fontsize=8, ha='center')
        ax1.text(1.01, path_y[-1] + 0.12, f'xf={path_y[-1]:.1f}',
                 color=ACCENT3, fontsize=8)

    # Phase amplitude inset
    ax_in = ax1.inset_axes([0.68, 0.05, 0.28, 0.38])
    phases = np.linspace(0, 3 * np.pi, len(actions))
    xa = np.cumsum(np.cos(phases)) / len(actions)
    ya = np.cumsum(np.sin(phases)) / len(actions)
    theta_c = np.linspace(0, 2*np.pi, 200)
    ax_in.plot(np.cos(theta_c), np.sin(theta_c), color=GRID, lw=0.8)
    for j in range(1, len(xa)):
        ax_in.plot(xa[j-1:j+1]*5, ya[j-1:j+1]*5, color=ACCENT5, alpha=0.4, lw=0.6)
    ax_in.arrow(0, 0, xa[-1]*5, ya[-1]*5, head_width=0.09, color=ACCENT2, lw=1.2)
    ax_in.set_facecolor('#08080f')
    ax_in.set_xticks([]); ax_in.set_yticks([])
    for sp in ax_in.spines.values(): sp.set_color(GRID)
    ax_in.set_title('|K| amplitude', color=DIM, fontsize=7, pad=2)

ax1.legend(fontsize=7.5, facecolor=PANEL, edgecolor=GRID, labelcolor=WHITE)
ax1.set_xlabel('time τ (normalized)'); ax1.set_ylabel('x(τ)')
ax1.text(0.5, -0.13, 'K(xf,tf ; xi,ti) = ∫ D[x(t)] exp(iS[x]/ħ)',
         transform=ax1.transAxes, ha='center', color=DIM,
         fontsize=8, fontfamily='monospace')

# ════════════════════════════════════════════════════════
# 02  HAWKING RADIATION  — output/hawking_mass_sweep.csv
# ════════════════════════════════════════════════════════
ax2 = fig.add_subplot(gs[0, 1])
style_panel(ax2, '02  HAWKING RADIATION')

d = load("hawking_mass_sweep.csv")
if d is not None:
    ax2b = ax2.twinx()
    ax2b.tick_params(colors=ACCENT3, labelsize=8)
    ax2b.spines['right'].set_color(ACCENT3)
    for sp in ['top','bottom','left']:
        ax2b.spines[sp].set_color(GRID)

    ax2.plot(d['mass_solar'], d['temperature_K'], color=ACCENT4, lw=2.0, label='T_H (K)')
    evap_gyr = d['evap_time_s'] / (3.15e7 * 1e9)
    ax2b.plot(d['mass_solar'], evap_gyr, color=ACCENT3, lw=1.5,
              linestyle='--', label='t_evap (Gyr)')

    # Annotate 5 solar mass
    idx5 = np.argmin(np.abs(d['mass_solar'] - 5.0))
    T5   = d['temperature_K'][idx5]
    ax2.scatter([5], [T5], color=ACCENT2, s=80, zorder=5)
    ax2.annotate(f'5 M☉\nT={T5:.1e} K', xy=(5, T5), xytext=(30, T5*6),
                 color=ACCENT2, fontsize=7.5,
                 arrowprops=dict(arrowstyle='->', color=ACCENT2, lw=0.8))

    ax2.set_xscale('log'); ax2.set_yscale('log')
    ax2b.set_yscale('log')
    ax2.set_xlabel('Mass (M☉)')
    ax2.set_ylabel('Hawking Temperature (K)', color=ACCENT4)
    ax2b.set_ylabel('Evaporation Time (Gyr)', color=ACCENT3)
    l1, lb1 = ax2.get_legend_handles_labels()
    l2, lb2 = ax2b.get_legend_handles_labels()
    ax2.legend(l1+l2, lb1+lb2, fontsize=7.5, facecolor=PANEL,
               edgecolor=GRID, labelcolor=WHITE)

ax2.text(0.5, -0.13, 'T_H = ħc³ / (8πGMk_B)',
         transform=ax2.transAxes, ha='center', color=DIM,
         fontsize=8, fontfamily='monospace')

# ════════════════════════════════════════════════════════
# 03  GEODESIC  — output/geodesic_veff.csv + geodesic_orbit.csv
# ════════════════════════════════════════════════════════
ax3 = fig.add_subplot(gs[1, 0])
style_panel(ax3, '03  GEODESIC  &  SCHWARZSCHILD METRIC')

dv = load("geodesic_veff.csv")
do = load("geodesic_orbit.csv")
if dv is not None:
    c2 = (2.998e8)**2
    ax3.plot(dv['r_over_rs'], dv['V_eff_GR']  / c2, color=ACCENT1, lw=2.0, label='V_eff GR')
    ax3.plot(dv['r_over_rs'], dv['V_eff_Newton'] / c2, color=DIM, lw=1.3,
             linestyle='--', label='Newtonian')
    ax3.axvline(1.0, color=ACCENT4, lw=1, linestyle=':', alpha=0.8)
    ax3.axvline(3.0, color=ACCENT2, lw=1, linestyle=':', alpha=0.8)
    ax3.axvline(1.5, color=ACCENT5, lw=1, linestyle=':', alpha=0.8)
    ax3.text(1.05, ax3.get_ylim()[1] if ax3.get_ylim()[1] != 0 else 1.5,
             'Event\nhorizon', color=ACCENT4, fontsize=7, va='top')
    ax3.text(3.05, 1.5, 'ISCO',         color=ACCENT2, fontsize=7, va='top')
    ax3.text(1.55, 1.5, 'Photon\nsphere', color=ACCENT5, fontsize=7, va='top')
    ax3.set_xlim(1, 15); ax3.set_ylim(0.5, 1.6)
    ax3.legend(fontsize=7.5, facecolor=PANEL, edgecolor=GRID, labelcolor=WHITE)

if do is not None:
    ax_orb = ax3.inset_axes([0.62, 0.48, 0.36, 0.48])
    x_o = do['x_m']; y_o = do['y_m']
    pts  = np.array([x_o, y_o]).T.reshape(-1, 1, 2)
    segs = np.concatenate([pts[:-1], pts[1:]], axis=1)
    lc   = LineCollection(segs, color=ACCENT3, linewidth=0.8, alpha=0.6)
    ax_orb.add_collection(lc)
    # Draw event horizon
    rs_m = do['r_m'][0] / do['r_over_rs'][0]   # rs = r / (r/rs)
    ax_orb.add_patch(Circle((0, 0), rs_m, color=ACCENT4, zorder=5))
    ax_orb.set_facecolor('#08080f')
    ax_orb.set_aspect('equal')
    lim = np.max(np.abs(x_o)) * 1.1
    ax_orb.set_xlim(-lim, lim); ax_orb.set_ylim(-lim, lim)
    ax_orb.set_xticks([]); ax_orb.set_yticks([])
    for sp in ax_orb.spines.values(): sp.set_color(GRID)
    ax_orb.set_title('GR precessing orbit', color=DIM, fontsize=7, pad=2)

ax3.set_xlabel('r / rₛ'); ax3.set_ylabel('V_eff / c²')

# ════════════════════════════════════════════════════════
# 04  SCHRÖDINGER  — output/schrodinger_wave.csv
# ════════════════════════════════════════════════════════
ax4 = fig.add_subplot(gs[1, 1])
style_panel(ax4, '04  SCHRÖDINGER EQUATION')

d = load("schrodinger_wave.csv")
if d is not None:
    prob_cols = [k for k in d.keys() if k.startswith('prob_t')]
    snap_labels = [k.replace('prob_', '') for k in prob_cols]

    ax4.fill_between(d['x'], d['potential'], alpha=0.12, color=ACCENT5)
    ax4.plot(d['x'], d['potential'], color=ACCENT5, lw=0.8, alpha=0.5, linestyle=':',
             label='V(x) (scaled)')

    for i, (col, lbl) in enumerate(zip(prob_cols, snap_labels)):
        alpha_v = 0.4 + 0.6 * (i / len(prob_cols))
        ax4.plot(d['x'], d[col], color=SNAP_COLORS[i % len(SNAP_COLORS)],
                 lw=1.5, alpha=alpha_v, label=lbl)

    ax4.set_xlim(-15, 15)
    ax4.legend(fontsize=7.5, facecolor=PANEL, edgecolor=GRID, labelcolor=WHITE)

ax4.set_xlabel('x'); ax4.set_ylabel('|ψ(x,t)|²')
ax4.text(0.5, -0.13, 'iħ ∂ψ/∂t = [-ħ²/2m ∇² + V(x)] ψ',
         transform=ax4.transAxes, ha='center', color=DIM,
         fontsize=8, fontfamily='monospace')

# ════════════════════════════════════════════════════════
# 05  EINSTEIN / FLAMM PARABOLOID  — output/einstein_metric.csv
# ════════════════════════════════════════════════════════
ax5 = fig.add_subplot(gs[2, 0])
style_panel(ax5, '05  EINSTEIN EQUATIONS  —  SPACETIME CURVATURE')

d = load("einstein_metric.csv")
if d is not None:
    r_rs = d['r_over_rs']
    # Flamm paraboloid embedding: z = sqrt(rs*(r-rs)) → z/rs = sqrt(r/rs - 1)
    z_rs = np.sqrt(np.maximum(r_rs - 1.0, 0))

    # Draw concentric rings (perspective flatten)
    phi_e = np.linspace(0, 2*np.pi, 60)
    step = max(1, len(r_rs)//14)
    for idx in range(0, len(r_rs), step):
        rr = r_rs[idx]
        zz = z_rs[idx]
        x_ring = rr * np.cos(phi_e)
        y_flat = rr * np.sin(phi_e) * 0.35
        brightness = 0.2 + 0.7 * (rr - r_rs[0]) / (r_rs[-1] - r_rs[0])
        ax5.plot(x_ring, zz + y_flat * 0.4, color=ACCENT1, lw=0.8, alpha=brightness)

    # Radial lines
    for angle in np.linspace(0, 2*np.pi, 16, endpoint=False):
        ax5.plot(r_rs * np.cos(angle),
                 z_rs + r_rs * np.sin(angle) * 0.35 * 0.4,
                 color=ACCENT5, lw=0.5, alpha=0.25)

    ax5.scatter([0], [0], color=ACCENT4, s=150, zorder=6, marker='*')
    ax5.text(0.2, -0.1, 'Singularity', color=ACCENT4, fontsize=8)

    # Kretschner on twin axis
    ax5r = ax5.twinx()
    ax5r.plot(r_rs, d['kretschner_K'], color=ACCENT2, lw=1.5,
              linestyle='--', alpha=0.8, label='Kretschner K')
    ax5r.set_yscale('log')
    ax5r.set_ylabel('Kretschner scalar K', color=ACCENT2, fontsize=8)
    ax5r.tick_params(colors=ACCENT2, labelsize=7)
    ax5r.spines['right'].set_color(ACCENT2)
    for sp in ['top','bottom','left']:
        ax5r.spines[sp].set_color(GRID)
    ax5r.legend(fontsize=7.5, facecolor=PANEL, edgecolor=GRID,
                labelcolor=WHITE, loc='upper right')

    ax5.set_xlim(-r_rs.max()*1.05, r_rs.max()*1.05)
    ax5.set_ylim(-0.3, z_rs.max() * 1.1 + 0.5)

ax5.set_xlabel('r / rₛ (projected)'); ax5.set_ylabel('Flamm embedding z / rₛ')
ax5.text(0.5, -0.13, 'G_μν + Λg_μν = (8πG/c⁴) T_μν',
         transform=ax5.transAxes, ha='center', color=DIM,
         fontsize=8, fontfamily='monospace')

# ════════════════════════════════════════════════════════
# 06  SCHWARZSCHILD EFFECTS  — output/schwarzschild_effects.csv
# ════════════════════════════════════════════════════════
ax6 = fig.add_subplot(gs[2, 1])
style_panel(ax6, '06  SCHWARZSCHILD EFFECTS')

d = load("schwarzschild_effects.csv")
if d is not None:
    r_rs = d['r_over_rs']
    # Clip redshift for display
    z_clip = np.clip(d['redshift_z'], 0, 20)

    ax6.plot(r_rs, d['time_dilation'], color=ACCENT3, lw=2.0, label='Time dilation dτ/dt')
    ax6.plot(r_rs, d['light_deflect_deg'], color=ACCENT5, lw=1.5,
             linestyle='--', label='Light deflection (°)')

    ax6b = ax6.twinx()
    ax6b.plot(r_rs, z_clip, color=ACCENT4, lw=1.8, label='Redshift z (clipped at 20)')
    ax6b.set_ylabel('Gravitational redshift z', color=ACCENT4, fontsize=8)
    ax6b.tick_params(colors=ACCENT4, labelsize=7)
    ax6b.spines['right'].set_color(ACCENT4)
    for sp in ['top','bottom','left']:
        ax6b.spines[sp].set_color(GRID)

    ax6.axvline(1.0, color=ACCENT4, lw=1, linestyle=':', alpha=0.7)
    ax6.axvline(3.0, color=ACCENT2, lw=1, linestyle=':', alpha=0.7)
    ax6.text(1.05, 0.95, 'Horizon', transform=ax6.get_xaxis_transform(),
             color=ACCENT4, fontsize=7, va='top')
    ax6.text(3.05, 0.95, 'ISCO', transform=ax6.get_xaxis_transform(),
             color=ACCENT2, fontsize=7, va='top')

    ax6.set_xlim(1, 20); ax6.set_ylim(-0.05, 1.1)
    l1, lb1 = ax6.get_legend_handles_labels()
    l2, lb2 = ax6b.get_legend_handles_labels()
    ax6.legend(l1+l2, lb1+lb2, fontsize=7.5, facecolor=PANEL,
               edgecolor=GRID, labelcolor=WHITE, loc='center right')

ax6.set_xlabel('r / rₛ')
ax6.set_ylabel('Value (dimensionless)')
ax6.text(0.5, -0.13,
         'ds² = -(1-rₛ/r)c²dt² + (1-rₛ/r)⁻¹dr² + r²dΩ²',
         transform=ax6.transAxes, ha='center', color=DIM,
         fontsize=8, fontfamily='monospace')

# ── Footer ────────────────────────────────────────────
fig.text(0.5, 0.005,
         'Data computed by quantum_gravity.cpp  ·  5 M☉ black hole reference  ·  SI units',
         ha='center', color=DIM, fontsize=7.5, fontfamily='monospace')

# ── Save ─────────────────────────────────────────────
outpath = os.path.join(OUT_DIR, "quantum_gravity_plot.png")
plt.savefig(outpath, dpi=150, bbox_inches='tight', facecolor=BG)
plt.close()
print(f"\n✓ Plot saved to {outpath}\n")
