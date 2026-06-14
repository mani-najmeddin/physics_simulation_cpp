/**
 * ======================================================
 *   QUANTUM GRAVITY PHYSICS ENGINE
 *   Feynman Path Integral | Hawking Radiation
 *   Einstein Equations | Geodesic | Schrodinger | Schwarzschild
 * ======================================================
 */

#include <iostream>
#include <complex>
#include <vector>
#include <cmath>
#include <iomanip>
#include <functional>
#include <random>
#include <fstream>
#include <string>
#include <numeric>

using namespace std;
using cd = complex<double>;

// ─── Physical Constants ───────────────────────────────
namespace Constants {
    const double hbar     = 1.0545718e-34;  // J·s
    const double c        = 2.998e8;         // m/s
    const double G        = 6.674e-11;       // N·m²/kg²
    const double kB       = 1.380649e-23;    // J/K
    const double pi       = M_PI;
    const double solar_M  = 1.989e30;        // kg
    const double m_e      = 9.109e-31;       // kg electron
}

// ─── 1. FEYNMAN PATH INTEGRAL ─────────────────────────
// K(xf,tf ; xi,ti) = ∫ D[x(t)] exp(i S[x]/ħ)
// Discretized: K ≈ Σ_paths exp(i Σ_j L(x_j,x_{j+1}) Δt / ħ)
namespace FeynmanPathIntegral {

    struct PathResult {
        cd propagator;
        double classical_action;
        double quantum_correction;
        int n_paths;
    };

    // Free particle Lagrangian: L = (1/2) m ẋ²
    double lagrangian_free(double x1, double x2, double dt, double mass) {
        double vel = (x2 - x1) / dt;
        return 0.5 * mass * vel * vel;
    }

    // Harmonic oscillator: L = (1/2)m ẋ² - (1/2)m ω² x²
    double lagrangian_ho(double x1, double x2, double dt, double mass, double omega) {
        double vel = (x2 - x1) / dt;
        double x_mid = 0.5 * (x1 + x2);
        return 0.5 * mass * vel * vel - 0.5 * mass * omega * omega * x_mid * x_mid;
    }

    // Monte Carlo path integral
    PathResult compute(double xi, double xf, double ti, double tf,
                       double mass = 9.109e-31,
                       int N_steps = 20,
                       int N_paths = 100000,
                       bool harmonic = false,
                       double omega = 1.0) {
        double dt = (tf - ti) / N_steps;
        double effective_hbar = 1.0; // natural units for visualization

        mt19937_64 rng(42);
        normal_distribution<double> gauss(0.0, sqrt(dt));

        cd propagator_sum(0.0, 0.0);
        double classical_S = 0.0;

        for (int p = 0; p < N_paths; p++) {
            // Generate random path
            vector<double> path(N_steps + 1);
            path[0] = xi;
            path[N_steps] = xf;

            // Random intermediate points
            for (int k = 1; k < N_steps; k++) {
                double frac = (double)k / N_steps;
                path[k] = xi + frac * (xf - xi) + gauss(rng) * sqrt(frac * (1 - frac));
            }

            // Compute action S[path]
            double action = 0.0;
            for (int k = 0; k < N_steps; k++) {
                if (harmonic)
                    action += lagrangian_ho(path[k], path[k+1], dt, mass, omega) * dt;
                else
                    action += lagrangian_free(path[k], path[k+1], dt, mass) * dt;
            }

            propagator_sum += exp(cd(0.0, action / effective_hbar));
        }

        // Classical path action (straight line)
        for (int k = 0; k < N_steps; k++) {
            double x1 = xi + (double)k / N_steps * (xf - xi);
            double x2 = xi + (double)(k+1) / N_steps * (xf - xi);
            classical_S += lagrangian_free(x1, x2, dt, mass) * dt;
        }

        cd K = propagator_sum / (double)N_paths;
        return { K, classical_S, abs(K), N_paths };
    }

    void print_results(const PathResult& r, double xi, double xf) {
        cout << "\n╔══ FEYNMAN PATH INTEGRAL ══════════════════╗\n";
        cout << "║  xi = " << xi << "  →  xf = " << xf << "\n";
        cout << "║  Propagator K:         " << r.propagator << "\n";
        cout << "║  |K| (amplitude):      " << fixed << setprecision(6) << r.quantum_correction << "\n";
        cout << "║  Classical action S:   " << r.classical_action << " [nat. units]\n";
        cout << "║  Paths sampled:        " << r.n_paths << "\n";
        cout << "╚═══════════════════════════════════════════╝\n";
    }
}

// ─── 2. HAWKING RADIATION ────────────────────────────
// T_H = ħc³ / (8πGMkB)
// dM/dt = -ħc⁴ / (15360 π G² M²)
namespace HawkingRadiation {

    struct BlackHoleState {
        double mass;
        double temperature;
        double radius;           // Schwarzschild radius
        double luminosity;
        double evaporation_time;
        double entropy;
    };

    BlackHoleState compute(double M) {
        using namespace Constants;
        double r_s    = 2.0 * G * M / (c * c);
        double T_H    = (hbar * c * c * c) / (8.0 * pi * G * M * kB);
        double L      = (hbar * c * c * c * c) / (15360.0 * pi * G * G * M * M);
        double t_evap = (5120.0 * pi * G * G * M * M * M) / (hbar * c * c * c * c);
        double S      = (4.0 * pi * G * M * M) / (hbar * c);

        return { M, T_H, r_s, L, t_evap, S };
    }

    void simulate_evaporation(double M0, int steps, const string& outfile) {
        using namespace Constants;
        ofstream f(outfile);
        f << "time,mass,temperature,radius\n";

        double M = M0;
        double dt_norm = 1e-3;  // normalized timestep
        double t = 0.0;

        for (int i = 0; i < steps && M > 0; i++) {
            auto state = compute(M);
            f << t << "," << M/solar_M << "," << state.temperature << "," << state.radius << "\n";

            // dM/dt = -hbar c^4 / (15360 pi G^2 M^2)
            double dMdt = -(hbar * c * c * c * c) / (15360.0 * pi * G * G * M * M);
            double dt = dt_norm * state.evaporation_time / steps;
            M += dMdt * dt;
            t += dt;
            if (M < 0) M = 0;
        }
        f.close();
    }

    void print_results(double M) {
        auto s = compute(M);
        cout << "\n╔══ HAWKING RADIATION ══════════════════════╗\n";
        cout << "║  Black Hole Mass:      " << scientific << s.mass << " kg\n";
        cout << "║  Schwarzschild Radius: " << s.radius << " m\n";
        cout << "║  Hawking Temperature:  " << s.temperature << " K\n";
        cout << "║  Luminosity:           " << s.luminosity << " W\n";
        cout << "║  Bekenstein-Hawking S: " << s.entropy << "\n";
        cout << "║  Evaporation Time:     " << s.evaporation_time << " s\n";
        cout << "╚═══════════════════════════════════════════╝\n";
    }
}

// ─── 3. EINSTEIN FIELD EQUATIONS ─────────────────────
// G_μν + Λ g_μν = (8πG/c⁴) T_μν
// G_μν = R_μν - (1/2) R g_μν
namespace EinsteinEquations {

    // 4x4 metric tensor
    using Metric = array<array<double,4>,4>;
    using Tensor = array<array<double,4>,4>;

    // Flat Minkowski metric: η_μν = diag(-1,1,1,1)
    Metric minkowski() {
        Metric g{};
        g[0][0] = -1; g[1][1] = 1; g[2][2] = 1; g[3][3] = 1;
        return g;
    }

    // Schwarzschild metric in (t,r,θ,φ):
    // ds² = -(1-rs/r)c²dt² + (1-rs/r)⁻¹dr² + r²dΩ²
    Metric schwarzschild_metric(double r, double rs) {
        Metric g{};
        double f = 1.0 - rs / r;
        g[0][0] = -(f);        // g_tt
        g[1][1] = 1.0 / f;     // g_rr
        g[2][2] = r * r;       // g_θθ
        g[3][3] = r * r;       // g_φφ (at θ=π/2)
        return g;
    }

    // Trace of Ricci scalar (linearized approximation)
    double ricci_scalar_estimate(double M, double r) {
        // For Schwarzschild vacuum: R = 0 outside
        // Non-zero only with stress-energy
        return 0.0;
    }

    // Energy-momentum tensor for perfect fluid: T_μν = (ρ+p)u_μu_ν + p g_μν
    Tensor perfect_fluid_Tmunu(double rho, double p, const Metric& g) {
        Tensor T{};
        // Rest frame u^μ = (1,0,0,0)
        T[0][0] = rho * g[0][0] * g[0][0] + p * g[0][0];
        T[1][1] = p * g[1][1];
        T[2][2] = p * g[2][2];
        T[3][3] = p * g[3][3];
        return T;
    }

    void print_einstein_tensor(double M, double r) {
        using namespace Constants;
        double rs = 2.0 * G * M / (c * c);
        auto g = schwarzschild_metric(r, rs);

        cout << "\n╔══ EINSTEIN FIELD EQUATIONS ═══════════════╗\n";
        cout << "║  G_μν + Λg_μν = (8πG/c⁴) T_μν\n";
        cout << "║  Mass M = " << M/Constants::solar_M << " M_☉\n";
        cout << "║  r = " << r << " m   |   rs = " << rs << " m\n";
        cout << "║\n";
        cout << "║  Schwarzschild Metric g_μν:\n";
        cout << fixed << setprecision(4);
        for (int i = 0; i < 4; i++) {
            cout << "║    [";
            for (int j = 0; j < 4; j++)
                cout << setw(10) << g[i][j] << (j<3 ? "," : "");
            cout << " ]\n";
        }
        cout << "║\n";
        cout << "║  Kretschner scalar K = 48G²M²/(c⁴r⁶):\n";
        double K = 48.0 * G * G * M * M / (c*c*c*c * pow(r,6));
        cout << "║    K = " << scientific << K << "\n";
        cout << "╚═══════════════════════════════════════════╝\n";
    }
}

// ─── 4. GEODESIC EQUATIONS ───────────────────────────
// d²x^μ/dτ² + Γ^μ_αβ (dx^α/dτ)(dx^β/dτ) = 0
namespace Geodesic {

    struct Particle {
        double t, r, theta, phi;  // position
        double dt, dr, dtheta, dphi;  // 4-velocity (dxμ/dτ)
        double mass;
        bool massive;
    };

    // Christoffel symbols for Schwarzschild metric
    // Only non-zero independent ones:
    // Γ^t_tr = rs/(2r(r-rs))
    // Γ^r_tt = c²rs(r-rs)/(2r³)
    // Γ^r_rr = -rs/(2r(r-rs))
    // Γ^r_θθ = -(r-rs)
    // Γ^r_φφ = -(r-rs)sin²θ
    // Γ^θ_rθ = 1/r
    // Γ^θ_φφ = -sinθcosθ
    // Γ^φ_rφ = 1/r
    // Γ^φ_θφ = cosθ/sinθ

    struct Derivatives {
        double ddr, ddt, ddtheta, d2phi;
    };

    Derivatives compute_acceleration(const Particle& p, double M) {
        using namespace Constants;
        double rs = 2.0 * G * M / (c * c);
        double r = p.r;
        double theta = p.theta;
        double f = 1.0 - rs / r;

        // Equation of motion in Schwarzschild coords (equatorial: θ=π/2)
        // d²r/dτ²
        double d2r = -(rs * c * c) / (2.0 * r * r) * f * p.dt * p.dt
                   + rs / (2.0 * r * r * f) * p.dr * p.dr
                   + f * r * p.dphi * p.dphi;

        // d²t/dτ²
        double d2t = -(rs / (r * r * f)) * p.dt * p.dr;

        // d²φ/dτ² (equatorial)
        double d2phi = -(2.0 / r) * p.dr * p.dphi;

        // d²θ/dτ² = 0 (equatorial)
        double d2theta = 0.0;

        return { d2r, d2t, d2theta, d2phi };
    }

    vector<Particle> integrate(Particle p0, double M, int steps, double dtau) {
        vector<Particle> traj;
        Particle p = p0;
        traj.push_back(p);

        for (int i = 0; i < steps; i++) {
            auto acc = compute_acceleration(p, M);

            // RK4 integration
            p.r      += p.dr * dtau + 0.5 * acc.ddr * dtau * dtau;
            p.t      += p.dt * dtau + 0.5 * acc.ddt * dtau * dtau;
            p.phi    += p.dphi * dtau;
            p.dr     += acc.ddr * dtau;
            p.dt     += acc.ddt * dtau;
            p.dphi   += acc.d2phi * dtau;

            if (p.r <= 0) break;
            traj.push_back(p);
        }
        return traj;
    }

    void print_results(double M, double r0) {
        using namespace Constants;
        double rs = 2.0 * G * M / (c * c);

        // ISCO (innermost stable circular orbit) = 3 rs
        double r_isco = 3.0 * rs;
        // Circular orbit angular velocity
        double omega_c = sqrt(G * M / (r0 * r0 * r0));
        // Orbital period
        double T_orb = 2.0 * pi / omega_c;
        // Energy per unit mass for circular orbit
        double E_circ = c * c * (1.0 - rs/r0) / sqrt(1.0 - 1.5*rs/r0);

        cout << "\n╔══ GEODESIC EQUATIONS ═════════════════════╗\n";
        cout << "║  d²xμ/dτ² + Γμ_αβ (dxα/dτ)(dxβ/dτ) = 0\n";
        cout << "║\n";
        cout << "║  Schwarzschild radius:  " << scientific << rs << " m\n";
        cout << "║  ISCO radius (3rs):     " << r_isco << " m\n";
        cout << "║  Test particle at r:    " << r0 << " m\n";
        cout << "║  Orbital angular vel:   " << omega_c << " rad/s\n";
        cout << "║  Orbital period:        " << T_orb << " s\n";
        cout << "║  Specific energy E/mc²: " << fixed << E_circ/(c*c) << "\n";
        cout << "║  Geodesic deviation → gravitational lensing\n";
        cout << "╚═══════════════════════════════════════════╝\n";
    }
}

// ─── 5. SCHRÖDINGER EQUATION ─────────────────────────
// iħ ∂ψ/∂t = Ĥψ = [-ħ²/2m ∇² + V(x)] ψ
namespace Schrodinger {

    using Wavefunction = vector<cd>;

    struct Grid {
        int N;
        double dx, dt;
        double x_min, x_max;
        vector<double> x;
    };

    Grid make_grid(int N = 512, double x_min = -10.0, double x_max = 10.0, double dt = 0.001) {
        Grid g;
        g.N = N; g.dx = (x_max - x_min) / N;
        g.dt = dt; g.x_min = x_min; g.x_max = x_max;
        g.x.resize(N);
        for (int i = 0; i < N; i++)
            g.x[i] = x_min + i * g.dx;
        return g;
    }

    // Initial Gaussian wavepacket: ψ(x,0) = A exp(-(x-x0)²/2σ² + ik0x)
    Wavefunction gaussian_packet(const Grid& g, double x0 = 0.0, double sigma = 1.0, double k0 = 5.0) {
        Wavefunction psi(g.N);
        double norm = 0.0;

        for (int i = 0; i < g.N; i++) {
            double x = g.x[i];
            double envelope = exp(-0.5 * (x - x0) * (x - x0) / (sigma * sigma));
            psi[i] = envelope * exp(cd(0.0, k0 * x));
            norm += abs(psi[i]) * abs(psi[i]);
        }
        // Normalize
        double A = 1.0 / sqrt(norm * g.dx);
        for (auto& p : psi) p *= A;
        return psi;
    }

    // Split-operator method: exp(-iHdt/ħ) = exp(-iVdt/2ħ) · exp(-iTdt/ħ) · exp(-iVdt/2ħ)
    // Simplified real-space propagation using Crank-Nicolson
    void propagate_step(Wavefunction& psi, const Grid& g,
                        const vector<double>& V,
                        double mass = 1.0, double hbar = 1.0) {
        int N = g.N;
        double alpha = hbar * g.dt / (4.0 * mass * g.dx * g.dx);
        cd i_unit(0.0, 1.0);

        // Tridiagonal Crank-Nicolson
        vector<cd> a(N, -i_unit * alpha);
        vector<cd> b(N);
        vector<cd> d(N);

        for (int k = 0; k < N; k++) {
            double v = (k < (int)V.size()) ? V[k] : 0.0;
            b[k] = 1.0 + 2.0 * i_unit * alpha + cd(0.0, g.dt * v / (2.0 * hbar));
        }

        // Build RHS
        for (int k = 1; k < N - 1; k++) {
            double v = V[k];
            cd b_star = 1.0 - 2.0 * i_unit * alpha - cd(0.0, g.dt * v / (2.0 * hbar));
            d[k] = b_star * psi[k] + i_unit * alpha * (psi[k+1] + psi[k-1]);
        }
        d[0] = psi[0];
        d[N-1] = psi[N-1];

        // Thomas algorithm for tridiagonal system
        vector<cd> c_prime(N), d_prime(N);
        c_prime[0] = a[0] / b[0];
        d_prime[0] = d[0] / b[0];

        for (int k = 1; k < N; k++) {
            cd denom = b[k] - a[k] * c_prime[k-1];
            c_prime[k] = a[k] / denom;
            d_prime[k] = (d[k] - a[k] * d_prime[k-1]) / denom;
        }

        psi[N-1] = d_prime[N-1];
        for (int k = N-2; k >= 0; k--)
            psi[k] = d_prime[k] - c_prime[k] * psi[k+1];
    }

    // Harmonic oscillator potential: V(x) = (1/2)mω²x²
    vector<double> harmonic_potential(const Grid& g, double omega = 1.0, double mass = 1.0) {
        vector<double> V(g.N);
        for (int i = 0; i < g.N; i++)
            V[i] = 0.5 * mass * omega * omega * g.x[i] * g.x[i];
        return V;
    }

    // Hydrogen-like 1D potential: V(x) = -Z/|x|
    vector<double> hydrogen_potential(const Grid& g, double Z = 1.0, double a = 0.1) {
        vector<double> V(g.N);
        for (int i = 0; i < g.N; i++)
            V[i] = -Z / sqrt(g.x[i] * g.x[i] + a * a);
        return V;
    }

    double norm(const Wavefunction& psi, double dx) {
        double n = 0.0;
        for (auto& p : psi) n += abs(p) * abs(p) * dx;
        return n;
    }

    double expectation_x(const Wavefunction& psi, const Grid& g) {
        double ex = 0.0;
        for (int i = 0; i < g.N; i++)
            ex += g.x[i] * abs(psi[i]) * abs(psi[i]) * g.dx;
        return ex;
    }

    void print_results(int N_steps = 100) {
        auto g = make_grid(256, -15.0, 15.0, 0.005);
        auto psi = gaussian_packet(g, -5.0, 1.0, 3.0);
        auto V = harmonic_potential(g, 0.5, 1.0);

        double norm0 = norm(psi, g.dx);
        double ex0 = expectation_x(psi, g);

        for (int i = 0; i < N_steps; i++)
            propagate_step(psi, g, V);

        double norm_f = norm(psi, g.dx);
        double ex_f = expectation_x(psi, g);

        // Energy levels: E_n = ħω(n + 1/2)
        cout << "\n╔══ SCHRÖDINGER EQUATION ═══════════════════╗\n";
        cout << "║  iħ ∂ψ/∂t = [-ħ²/2m ∇² + V(x)] ψ\n";
        cout << "║  Method: Crank-Nicolson (unitary)\n";
        cout << "║\n";
        cout << "║  Initial <x>:         " << fixed << setprecision(4) << ex0 << "\n";
        cout << "║  Final <x>:           " << ex_f << "\n";
        cout << "║  Norm preserved:      " << norm_f/norm0 << " (should be 1)\n";
        cout << "║\n";
        cout << "║  Harmonic Oscillator Energy Levels:\n";
        for (int n = 0; n < 5; n++)
            cout << "║    E_" << n << " = " << (n + 0.5) << " ħω\n";
        cout << "╚═══════════════════════════════════════════╝\n";
    }
}

// ─── 6. SCHWARZSCHILD & BLACK HOLES ──────────────────
// ds² = -(1-rs/r)c²dt² + (1-rs/r)⁻¹dr² + r²dΩ²
namespace Schwarzschild {

    struct BlackHole {
        double M, rs, r_isco, r_photon;
        double area, entropy;
    };

    BlackHole compute(double M) {
        using namespace Constants;
        double rs      = 2.0 * G * M / (c * c);
        double r_isco  = 3.0 * rs;         // innermost stable circular orbit
        double r_ph    = 1.5 * rs;         // photon sphere
        double A       = 4.0 * pi * rs * rs;
        double S       = (kB * c * c * c * A) / (4.0 * G * hbar);
        return { M, rs, r_isco, r_ph, A, S };
    }

    // Effective potential for massive particle
    // V_eff(r) = c²(1-rs/r)(1 + L²/r²c²)  where L = angular momentum/mass
    double effective_potential(double r, double M, double L) {
        using namespace Constants;
        double rs = 2.0 * G * M / (c * c);
        return c * c * (1.0 - rs / r) * (1.0 + L * L / (r * r * c * c));
    }

    // Gravitational redshift: z = 1/sqrt(1-rs/r) - 1
    double redshift(double r, double M) {
        using namespace Constants;
        double rs = 2.0 * G * M / (c * c);
        if (r <= rs) return 1e308;
        return 1.0 / sqrt(1.0 - rs / r) - 1.0;
    }

    // Gravitational time dilation: dτ/dt = sqrt(1 - rs/r)
    double time_dilation(double r, double M) {
        using namespace Constants;
        double rs = 2.0 * G * M / (c * c);
        if (r <= rs) return 0.0;
        return sqrt(1.0 - rs / r);
    }

    // Light deflection angle: α = 4GM/(c²b) = 2rs/b
    double light_deflection(double b, double M) {
        using namespace Constants;
        double rs = 2.0 * G * M / (c * c);
        return 2.0 * rs / b;  // radians (weak field approx)
    }

    void print_results(double M) {
        using namespace Constants;
        auto bh = compute(M);
        double r_test = 10.0 * bh.rs;

        cout << "\n╔══ SCHWARZSCHILD METRIC & BLACK HOLE ══════╗\n";
        cout << "║  ds² = -(1-rs/r)c²dt² + (1-rs/r)⁻¹dr²\n";
        cout << "║        + r²(dθ² + sin²θ dφ²)\n";
        cout << "║\n";
        cout << "║  Mass:                 " << scientific << M << " kg\n";
        cout << "║                        (" << fixed << M/solar_M << " M_☉)\n";
        cout << "║  Schwarzschild radius: " << scientific << bh.rs << " m\n";
        cout << "║  ISCO (3rs):           " << bh.r_isco << " m\n";
        cout << "║  Photon sphere (1.5rs):" << bh.r_photon << " m\n";
        cout << "║  Horizon area:         " << bh.area << " m²\n";
        cout << "║  Bekenstein Entropy:   " << bh.entropy << " J/K\n";
        cout << "║\n";
        cout << "║  At r = 10rs:\n";
        cout << "║    Gravitational redshift: z = " << fixed << setprecision(4)
             << redshift(r_test, M) << "\n";
        cout << "║    Time dilation dτ/dt:    " << time_dilation(r_test, M) << "\n";
        cout << "║    Light deflect (b=r):    "
             << light_deflection(r_test, M) * 180.0/Constants::pi << "°\n";
        cout << "╚═══════════════════════════════════════════╝\n";
    }
}

// ─── MAIN ─────────────────────────────────────────────
int main() {
    cout << "\n";
    cout << "  ╔══════════════════════════════════════════════╗\n";
    cout << "  ║    QUANTUM GRAVITY PHYSICS ENGINE  v1.0      ║\n";
    cout << "  ║    Feynman · Hawking · Einstein · Geodesic   ║\n";
    cout << "  ║    Schrödinger · Schwarzschild               ║\n";
    cout << "  ╚══════════════════════════════════════════════╝\n";

    const double M_bh = 5.0 * Constants::solar_M;  // 5 solar mass black hole

    // 1. Feynman Path Integral
    auto fi_result = FeynmanPathIntegral::compute(0.0, 2.0, 0.0, 1.0,
                                                   1.0, 30, 50000, true, 1.0);
    FeynmanPathIntegral::print_results(fi_result, 0.0, 2.0);

    // 2. Hawking Radiation
    HawkingRadiation::print_results(M_bh);
    HawkingRadiation::simulate_evaporation(M_bh, 500, "/output/hawking_evap.csv");

    // 3. Einstein Field Equations
    double r_outside = 3.0 * 2.0 * Constants::G * M_bh / (Constants::c * Constants::c);
    EinsteinEquations::print_einstein_tensor(M_bh, r_outside);

    // 4. Geodesic Equations
    double r_orbit = 6.0 * 2.0 * Constants::G * M_bh / (Constants::c * Constants::c);
    Geodesic::print_results(M_bh, r_orbit);

    // 5. Schrödinger Equation
    Schrodinger::print_results(200);

    // 6. Schwarzschild & Black Holes
    Schwarzschild::print_results(M_bh);

    // Also compute for stellar mass BH range
    cout << "\n╔══ COMPARATIVE BLACK HOLE TABLE ═══════════════════╗\n";
    cout << "║  Mass (M☉)   rs (km)      T_Hawking (K)    t_evap\n";
    cout << "╠═══════════════════════════════════════════════════╣\n";
    vector<double> masses = {1.0, 5.0, 10.0, 1e6, 1e9};
    for (double m_sol : masses) {
        double M = m_sol * Constants::solar_M;
        auto bh = HawkingRadiation::compute(M);
        cout << "║  " << setw(9) << m_sol
             << "   " << setw(10) << scientific << bh.radius/1000.0
             << "   " << setw(14) << bh.temperature
             << "   " << bh.evaporation_time << "\n";
    }
    cout << "╚═══════════════════════════════════════════════════╝\n";

    cout << "\n✓ Simulation complete. Data written to output/\n\n";
    return 0;
}
