#include "../divae.cpp"

#include <iostream>
#include <string>
#include <vector>
#include <cmath>
#include <chrono>
#include <random>
#include <thread>

int main() {
    const int ROWS = 50;
    const int COLS = 210;
    const int FPS_ROW = 50;

    divae display(ROWS + 1, COLS);
    display.setColor("#0059ff");

    // --- Orb size scaled to grid dimensions ---
    // Base was sigma1=28, sigma2=24 designed for ~210x50.
    // Scale so orbs remain proportionally the same relative to the screen.
    const double refCols = 210.0;
    const double refRows = 50.0;
    double colScale = COLS / refCols;              // = 1.0 here
    double rowScale = ROWS / refRows;              // = 1.0 here
    double sigmaScale = (colScale + rowScale) * 0.5;

    const double sigma1 = 28.0 * sigmaScale;
    const double sigma2 = 24.0 * sigmaScale;

    // --- Lifecycle ---
    enum State { ACTIVE, COOLDOWN };
    State state = ACTIVE;

    double globalAlpha = 0.0;
    double lifeTime = 5.0;
    double lifeElapsed = 0.0;
    double cooldownTime = 1.5;
    double cooldownElapsed = 0.0;

    const double spawnFade = 0.8;
    const double despawnFade = 0.8;

    double c1x = 0, c1y = 0, c2x = 0, c2y = 0;
    double v1x = 0, v1y = 0, v2x = 0, v2y = 0;

    const double kAttract = 0.03;
    const double damping = 0.94;
    const double dt = 1.0;

    double phase = 0.0;
    std::mt19937 rng(12345);
    std::uniform_real_distribution<double> jitter(-0.05, 0.05);
    std::uniform_real_distribution<double> posX(COLS * 0.1, COLS * 0.9);
    std::uniform_real_distribution<double> posY(ROWS * 0.15, ROWS * 0.85);
    std::uniform_real_distribution<double> lifeJit(-0.5, 0.5);
    std::uniform_real_distribution<double> coolJit(-0.5, 0.5);

    auto spawnCenters = [&]() {
        c1x = posX(rng); c1y = posY(rng);
        c2x = posX(rng); c2y = posY(rng);
        v1x = v1y = v2x = v2y = 0.0;
        lifeTime = 5.0 + lifeJit(rng);
        lifeElapsed = 0.0;
        globalAlpha = 0.0;
        state = ACTIVE;
    };

    auto startCooldown = [&]() {
        cooldownTime = 1.5 + coolJit(rng);
        cooldownElapsed = 0.0;
        state = COOLDOWN;
    };

    spawnCenters();

    const double targetFrameTime = 1.0 / 60.0;
    auto lastFrame = std::chrono::high_resolution_clock::now();
    auto lastFpsTime = lastFrame;
    int frameCount = 0;
    int currentFps = 0;

    // Precompute exp denominators
    const double denom1 = 2.0 * sigma1 * sigma1;
    const double denom2 = 2.0 * sigma2 * sigma2;

    while (true) {
        auto frameStart = std::chrono::high_resolution_clock::now();
        double frameDt = std::chrono::duration<double>(frameStart - lastFrame).count();
        lastFrame = frameStart;
        if (frameDt > 0.1) frameDt = 0.1;

        // --- Lifecycle ---
        if (state == ACTIVE) {
            lifeElapsed += frameDt;
            if (lifeElapsed < spawnFade) {
                globalAlpha = lifeElapsed / spawnFade;
            } else if (lifeElapsed > lifeTime - despawnFade) {
                double remaining = lifeTime - lifeElapsed;
                if (remaining < 0) remaining = 0;
                globalAlpha = remaining / despawnFade;
            } else {
                globalAlpha = 1.0;
            }
            if (lifeElapsed >= lifeTime) startCooldown();
        } else {
            cooldownElapsed += frameDt;
            globalAlpha = 0.0;
            if (cooldownElapsed >= cooldownTime) spawnCenters();
        }

        // --- Physics ---
        if (state == ACTIVE) {
            double dx12 = c2x - c1x;
            double dy12 = c2y - c1y;
            double dist12 = std::sqrt(dx12 * dx12 + dy12 * dy12) + 1e-6;

            double a1x = kAttract * dx12 / dist12;
            double a1y = kAttract * dy12 / dist12;
            double a2x = -kAttract * dx12 / dist12;
            double a2y = -kAttract * dy12 / dist12;

            a1x += std::sin(phase * 1.3 + 0.0) * 0.08;
            a1y += std::cos(phase * 1.1 + 0.5) * 0.04;
            a2x += std::sin(phase * 0.9 + 2.1) * 0.08;
            a2y += std::cos(phase * 1.4 + 3.3) * 0.04;

            a1x += jitter(rng); a1y += jitter(rng);
            a2x += jitter(rng); a2y += jitter(rng);

            if (dist12 < 15.0) {
                double rep = (15.0 - dist12) * 0.02;
                a1x -= rep * dx12 / dist12;
                a1y -= rep * dy12 / dist12;
                a2x += rep * dx12 / dist12;
                a2y += rep * dy12 / dist12;
            }

            v1x = damping * v1x + a1x * dt;
            v1y = damping * v1y + a1y * dt;
            v2x = damping * v2x + a2x * dt;
            v2y = damping * v2y + a2y * dt;

            c1x += v1x * dt;
            c1y += v1y * dt;
            c2x += v2x * dt;
            c2y += v2y * dt;

            if (c1x < 5) { c1x = 5; v1x = -v1x * 0.6; }
            if (c1x > COLS - 6) { c1x = COLS - 6; v1x = -v1x * 0.6; }
            if (c1y < 5) { c1y = 5; v1y = -v1y * 0.6; }
            if (c1y > ROWS - 6) { c1y = ROWS - 6; v1y = -v1y * 0.6; }
            if (c2x < 5) { c2x = 5; v2x = -v2x * 0.6; }
            if (c2x > COLS - 6) { c2x = COLS - 6; v2x = -v2x * 0.6; }
            if (c2y < 5) { c2y = 5; v2y = -v2y * 0.6; }
            if (c2y > ROWS - 6) { c2y = ROWS - 6; v2y = -v2y * 0.6; }

            phase += 0.05;
            if (phase > 1e6) phase = 0.0;
        }

        // --- Render ---
        if (globalAlpha > 0.01) {
            for (int y = 0; y < ROWS; y++) {
                double py = y * 2.0;
                double py1 = py - c1y * 2.0;
                double py2 = py - c2y * 2.0;
                double py1sq = py1 * py1;
                double py2sq = py2 * py2;

                for (int x = 0; x < COLS; x++) {
                    double px = x;

                    double d1x = px - c1x;
                    double d2x = px - c2x;
                    double d1sq = d1x * d1x + py1sq;
                    double d2sq = d2x * d2x + py2sq;

                    double g1 = std::exp(-d1sq / denom1);
                    double g2 = std::exp(-d2sq / denom2);

                    double field = std::max(g1, g2) * globalAlpha;
                    if (field > 1.0) field = 1.0;

                    int shadeIndex = -1;
                    if (field > 0.6) shadeIndex = 2;
                    else if (field > 0.28) shadeIndex = 1;
                    else if (field > 0.08) shadeIndex = 0;

                    display.setDot(y, x, shadeIndex);
                }
            }
        } else {
            for (int y = 0; y < ROWS; y++)
                for (int x = 0; x < COLS; x++)
                    display.setDot(y, x, -1);
        }

        // --- FPS ---
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
                  << "  Sigma: " << static_cast<int>(sigma1) << "/" << static_cast<int>(sigma2)
                  << "     " << std::flush;

        // --- 60 fps cap ---
        auto frameEnd = std::chrono::high_resolution_clock::now();
        double workTime = std::chrono::duration<double>(frameEnd - frameStart).count();
        double sleepTime = targetFrameTime - workTime;
        if (sleepTime > 0.0)
            std::this_thread::sleep_for(std::chrono::duration<double>(sleepTime));
    }

    return 0;
}