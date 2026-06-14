# Quantum Gravity Physics Engine

A numerical physics engine in C++ simulating core equations of general relativity and quantum mechanics.

## Modules

| Module | Equation | Method |
|--------|----------|--------|
| Feynman Path Integral | K = ∫ D[x(t)] exp(iS/ħ) | Monte Carlo (100,000 paths) |
| Hawking Radiation | T_H = ħc³/(8πGMk_B) | Analytical + ODE |
| Einstein Field Equations | G_μν + Λg_μν = (8πG/c⁴)T_μν | Schwarzschild tensor |
| Geodesic Equations | d²x^μ/dτ² + Γ^μ_αβ u^α u^β = 0 | RK4 integration |
| Schrödinger Equation | iħ ∂ψ/∂t = [−ħ²/2m ∇² + V]ψ | Crank-Nicolson |
| Schwarzschild Metric | ds² = −(1−rs/r)c²dt² + ... | Analytical |

## What it computes

- Hawking temperature, luminosity, entropy, and evaporation time for any black hole mass
- Full evaporation curve exported to CSV
- Schwarzschild metric tensor g_μν at any radius
- Kretschner curvature scalar K = 48G²M²/(c⁴r⁶)
- ISCO radius, photon sphere, gravitational redshift, time dilation
- Quantum propagator via path integral Monte Carlo
- Wavepacket time evolution with norm conservation

## Build

```bash
g++ -std=c++17 -O2 -o physics_engine main.cpp -lm
./physics_engine
```

## Author
Mani Najmeddin — CS Student | Physics Enthusiast | Cosmology & Algorithms
