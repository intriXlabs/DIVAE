#include "../divae.cpp"

#include <iostream>
#include <string>
#include <vector>
#include <cmath>
#include <chrono>
#include <unistd.h>

int main() {
    const int ROWS = 50;
    const int COLS = 210;
    const int FPS_ROW = 50;

    divae display(ROWS + 1, COLS);

    double t = 0.0;

    auto lastFpsTime = std::chrono::high_resolution_clock::now();
    int frameCount = 0;
    int currentFps = 0;

    const double cx = COLS * 0.5;
    const double cy = ROWS * 0.5;
    const double aspect = 2.0;

    while (true) {
        // Precompute sin/cos palette
        const double sinT  = std::sin(t * 0.03);
        const double cosT  = std::cos(t * 0.03);
        const double sinT2 = std::sin(t * 0.011);
        const double cosT2 = std::cos(t * 0.017);
        const double sinT3 = std::sin(t * 0.007);
        const double rotA  = std::cos(t * 0.012);
        const double rotB  = std::sin(t * 0.012);

        for (int y = 0; y < ROWS; y++) {
            // aspect-corrected normalized coords -1..1
            double ny = (y - cy) / cy;
            double py = ny * aspect;

            for (int x = 0; x < COLS; x++) {
                double nx = (x - cx) / cx;
                double px = nx;

                // polar
                double r = std::sqrt(px * px + py * py);
                if (r < 1e-6) r = 1e-6;
                double a = std::atan2(py, px);

                // --- LAYER 1: rotating kaleidoscope (6-fold) ---
                double ka = a + t * 0.02;
                double sector = 6.28318530718 / 6.0;
                ka = ka - std::floor(ka / sector) * sector;
                ka = std::fabs(ka - sector * 0.5);
                double k1 = std::sin(r * 22.0 - t * 0.09 + ka * 8.0);

                // --- LAYER 2: nested spiral arms ---
                double spiral = std::sin(a * 3.0 + r * 18.0 - t * 0.10);

                // --- LAYER 3: pulsing concentric rings w/ breathing ---
                double breathe = 1.0 + 0.15 * sinT;
                double rings = std::sin(r * 30.0 * breathe - t * 0.12);

                // --- LAYER 4: moiré interference ---
                double m1 = std::sin(px * 30.0 + t * 0.04 + sinT2 * 3.0);
                double m2 = std::sin(py * 24.0 - t * 0.05 + cosT2 * 3.0);
                double moire = m1 * m2;

                // --- LAYER 5: radial wave from center ---
                double wave = std::sin(r * 40.0 - t * 0.15);

                // --- LAYER 6: rotating quad twist ---
                double qx = px * rotA - py * rotB;
                double qy = px * rotB + py * rotA;
                double twist = std::sin(qx * 14.0) * std::cos(qy * 10.0 + t * 0.03);

                // --- LAYER 7: heartbeat pulse ---
                double beat = 0.5 + 0.5 * std::sin(t * 0.08 + r * 6.0);

                // --- COMBINE with distance-dependent weights ---
                double w1 = std::exp(-r * 1.2);         // near center
                double w2 = std::exp(-std::fabs(r - 0.6) * 3.0); // mid ring
                double w3 = std::exp(-std::fabs(r - 1.2) * 2.0); // outer

                double field =
                    0.55 * k1 * (0.5 + 0.5 * w1) +
                    0.35 * spiral * (0.5 + 0.5 * w2) +
                    0.30 * rings * w1 +
                    0.25 * moire * w3 +
                    0.25 * wave * w2 +
                    0.20 * twist * (0.5 + 0.5 * w2) +
                    0.15 * beat * w1;

                // Normalize roughly to 0..1
                double v = field * 0.5 + 0.5;
                if (v < 0.0) v = 0.0;
                if (v > 1.0) v = 1.0;

                // Sharpen contrast for punchier look
                v = v * v * (3.0 - 2.0 * v); // smoothstep

                int shadeIndex = -1;
                if (v > 0.72) shadeIndex = 2;       // ●
                else if (v > 0.45) shadeIndex = 1;  // •
                else if (v > 0.18) shadeIndex = 0;  // ·

                display.setDot(y, x, shadeIndex);
            }
        }

        // FPS
        frameCount++;
        auto now = std::chrono::high_resolution_clock::now();
        double fpsElapsed = std::chrono::duration<double>(now - lastFpsTime).count();
        if (fpsElapsed >= 1.0) {
            currentFps = static_cast<int>(frameCount / fpsElapsed);
            frameCount = 0;
            lastFpsTime = now;
        }

        display.print();
        std::cout << "\033[" << (FPS_ROW + 1) << ";1H"
                  << "FPS: " << currentFps
                  << "  [kaleido+moire+spiral+rings]"
                  << "     " << std::flush;

        t += 1.0;
        usleep(16000); // ~60 FPS
    }

    return 0;
}