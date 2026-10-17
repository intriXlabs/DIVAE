#include "../divae.cpp"

#include <iostream>
#include <string>
#include <vector>
#include <cmath>
#include <chrono>

int main() {
    const int ROWS = 50;
    const int COLS = 210;
    const int FPS_ROW = 50;

    divae display(ROWS + 1, COLS);

    // Julia set constants — animated to "morph"
    double cRe = -0.7;
    double cIm = 0.27;

    // Aspect correction: terminal cells ~2x taller than wide
    const double aspect = 2.0;

    // Map screen to complex plane
    const double reMin = -1.6, reMax = 1.6;
    const double imMin = -1.0, imMax = 1.0;

    // HIGHEST QUALITY: more iterations = finer detail, sharper boundaries
    const int maxIter = 200;

    double t = 0.0;

    auto lastFpsTime = std::chrono::high_resolution_clock::now();
    int frameCount = 0;
    int currentFps = 0;

    while (true) {
        // Animate Julia constant along a Lissajous-ish path
        cRe = -0.75 + 0.20 * std::cos(t * 0.031);
        cIm =  0.15 + 0.28 * std::sin(t * 0.047);

        double imStep = (imMax - imMin) / ROWS;
        double reStep = (reMax - reMin) / COLS;

        for (int y = 0; y < ROWS; y++) {
            double zy0 = imMin + y * imStep;
            for (int x = 0; x < COLS; x++) {
                double zx = reMin + x * reStep;
                double zy = zy0;

                int iter = 0;
                double zx2 = zx * zx;
                double zy2 = zy * zy;

                // Smooth iteration count for higher quality banding
                double smoothIter = 0.0;

                while (iter < maxIter && (zx2 + zy2) < 4.0) {
                    zy = 2.0 * zx * zy + cIm;
                    zx = zx2 - zy2 + cRe;
                    zx2 = zx * zx;
                    zy2 = zy * zy;
                    iter++;
                }

                int shadeIndex = -1;
                if (iter == maxIter) {
                    shadeIndex = 2; // inside — brightest
                } else {
                    // Smooth coloring: continuous iteration value
                    double logZn = std::log(zx2 + zy2) * 0.5;
                    double nu = std::log(logZn / std::log(2.0)) / std::log(2.0);
                    smoothIter = iter + 1.0 - nu;
                    if (smoothIter < 0) smoothIter = 0;

                    // Normalize and map to three shade buckets
                    double norm = smoothIter / maxIter;
                    if (norm > 0.30) shadeIndex = 1;      // •
                    else if (norm > 0.04) shadeIndex = 0; // ·
                }

                display.setDot(y, x, shadeIndex);
            }
        }

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
                  << "  iter: " << maxIter
                  << "  c = " << cRe << " + " << cIm << "i"
                  << "     " << std::flush;

        t += 1.0;
        // No sleep — max throughput
    }

    return 0;
}