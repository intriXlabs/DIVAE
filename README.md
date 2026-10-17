# DIVAE

<div align="center">

![Version](https://img.shields.io/badge/version-1.0.0-blue.svg)
![C++](https://img.shields.io/badge/C%2B%2B-11%2B-orange.svg)
![Platform](https://img.shields.io/badge/platform-Linux%20%7C%20macOS%20%7C%20WSL-lightgrey.svg)
![Dependencies](https://img.shields.io/badge/dependencies-0-brightgreen.svg)
![Rendering](https://img.shields.io/badge/rendering-dot--matrix-blueviolet.svg)
![License](https://img.shields.io/badge/license-MIT-green.svg)

**A dot at a time. A frame at a time. The terminal is a canvas.**

*A zero-dependency C++ rendering engine for dot-matrix graphics, inspired by ChatGPT's image-generation animation — with a display buffer, shade primitives, and a full-frame render loop.*


</div>

---

# Quick Demos:

<h3 align="center">ChatGPT-Style Dot Reveal</h3>
<image src="demoGifs/CGSA.gif">

<h3 align="center">high quality iterative julia morpher</h3>
<image src="demoGifs/jm.gif">

<h3 align="center">Custom Dot Art</h3>
<image src="demoGifs/ra.gif">

---

## Important Note

`DIVAE` is designed for **POSIX terminals** (Linux, macOS, BSD, WSL) and requires **ANSI escape codes** and **24-bit true-color support**.

### Zero-Dependency by Design

The engine contains **no external dependencies** and **no platform-specific code**. It only needs:

* ANSI escape code support
* 24-bit true-color support
* A terminal size you provide at construction time

That's it. No `ioctl`, no window API, no `ncurses`. You give `DIVAE` a size, it gives you a display buffer. Terminal-size detection is left to the caller, so the same source works on Linux, Android, Windows (with ANSI enabled), or anywhere else.

**DIVAE handles the pixels. Where the terminal lives is up to you.**

---

## 📖 Table of Contents

- [Overview](#-overview)
- [Features](#-features)
- [The Display Model](#-the-display-model)
- [The Three Shades](#-the-three-shades)
- [Architecture](#-architecture)
- [Quick Start](#-quick-start)
- [Use Cases](#-use-cases)
- [Why DIVAE](#-why-divae)
- [Comparison](#-comparison)
- [Design Philosophy](#-design-philosophy)
- [API Reference](#-api-reference)
- [License](#-license)

---

## 🌟 Overview

`DIVAE` — **Dot-Induced Virtual Art Engine** — is a terminal rendering engine built around a single primitive: **the dot**. Every cell on the screen is a dot, every dot has a **shade**, and every frame is a **matrix of dots** rendered in one color.

It's the engine behind the animation you've seen when ChatGPT generates an image: a field of tiny dots appearing, brightening, and assembling into something. `DIVAE` doesn't ship with a specific effect — it ships with the **machinery** to build any dot-based effect you want.

The name says what it does: dots, induced into a virtual art surface.

### Why a Dot Engine?

Most terminal libraries treat the screen as a **text surface**. `DIVAE` treats it as a **raster**. There is no text, no words, no strings going into cells — just dots, shades, and color.

That shift is small but important:

- **A dot is the smallest unit.** A cell is a dot. A dot has one of three shades.
- **A buffer is the surface.** Every dot lives in memory until you `print()`.
- **A frame is a matrix.** Render the buffer, clear it, draw again.
- **A color is a whole scene.** One RGB value tints every dot on screen.

Once you see the screen as a matrix of dots, you can build anything: progress bars, sparklines, loader animations, sensor displays, dot-matrix typography, generative art. `DIVAE` gives you the canvas and the brush — the picture is yours.

---

## ✨ Features

### 🎨 Rendering
- **24-bit True Color** — Full RGB via `#RRGGBB` hex, applied to the whole frame
- **Three Shades** — `·` `•` `●` as the default dot palette
- **Full-Frame Print** — One `print()` call writes the entire buffer
- **Cursor Control** — `hideCursor()` / `showCursor()` for clean rendering

### 🧱 Display Model
- **Explicit Dimensions** — You pass rows and cols; no auto-detection
- **Flat Display Buffer** — A `std::string*` array indexed by `row * cols + col`
- **Bounds-Checked Writes** — `setDot()` returns `-1` for out-of-range coordinates
- **Clear-and-Redraw** — Full control over when the buffer resets

### 🧩 Developer-Friendly
- **One Class** — Construct, set dots, print
- **No Init, No Shutdown** — Everything lives in the constructor
- **No Global State** — Multiple buffers can coexist
- **Runtime Color** — `setColor()` can change mid-animation

---

## 🧱 The Display Model

`DIVAE` is built on a single abstraction: **the display buffer**.

```
                       THE DISPLAY BUFFER

   ┌────────────────────────────────────────────────────┐
   │  ·  ·  •  ●  ·  ·  ·  ·  ·  ·  ·  ·  ·  ·  ·  ·    │
   │  ·  •  ●  ●  •  ·  ·  ·  ·  ·  ·  ·  ·  ·  ·  ·    │
   │  •  ●  ●  ●  •  ·  ·  ·  ·  ·  ·  ·  ·  ·  ·  ·    │
   │  ·  •  ●  ●  •  ·  ·  ·  ·  ·  ·  ·  ·  ·  ·  ·    │
   │  ·  ·  •  ●  ·  ·  ·  ·  ·  ·  ·  ·  ·  ·  ·  ·    │
   └────────────────────────────────────────────────────┘

   One std::string per cell. Indexed as buffer[row * cols + col].

   A frame is what you see. The buffer is what you draw into.
```

The buffer is **in memory**, not on screen. You write to it with `setDot()`, and it appears on screen only when you call `print()`. That's the whole rendering model:

```
1. draw into the buffer      (setDot calls)
2. render the buffer         (print)
3. optionally clear          (ClearDisplayBuffer, called internally)
4. repeat
```

Every cell is one string. Every string is one shade — `·`, `•`, `●`, or a space. Nothing else exists in the display.

---

## 🎨 The Three Shades

`DIVAE` uses **three shades** of dot, plus a blank:

| Index | Glyph | Meaning |
|---|---|---|
| 0 | `·` | Smallest dot — faint, distant |
| 1 | `•` | Medium dot — present, visible |
| 2 | `●` | Largest dot — bright, solid |
| — | ` ` | No dot (empty cell) |

Each shade is a **visual weight**. A shape built from `·` reads as faint. The same shape from `●` reads as solid. Replacing the middle `•` with `·` softens an edge. Replacing it with `●` darkens it.

That's the entire artistic vocabulary — **three dot sizes** — and it turns out to be enough to draw progress bars, sparklines, letters, icons, and entire scenes.

`setDot(row, col, shadeIndex)` writes one shade into one cell. `setDot(row, col, -1)` or any out-of-range shade **clears** that cell.

```cpp
divae screen(20, 60);

screen.setColor("#00FFAA");

screen.setDot(10, 20, 0);   // a faint dot
screen.setDot(10, 21, 1);   // a medium dot
screen.setDot(10, 22, 2);   // a solid dot

screen.print();             // render
```

---

## 🏗 Architecture

```
┌─────────────────────────────────────────────────────────────┐
│                    Application Layer                        │
│   ┌──────────────┐  ┌────────────────┐  ┌───────────────┐   │
│   │  Constructor │  │    setDot()    │  │    print()    │   │
│   └──────────────┘  └────────────────┘  └───────────────┘   │
└─────────────────────────────────────────────────────────────┘
                            ↓
┌─────────────────────────────────────────────────────────────┐
│                    Display Buffer                           │
│   ┌────────────────────────────────────────────────────┐    │
│   │   std::string[rows * cols]                         │    │
│   │   indexed by row * cols + col                      │    │
│   └────────────────────────────────────────────────────┘    │
└─────────────────────────────────────────────────────────────┘
                            ↓
┌─────────────────────────────────────────────────────────────┐
│                     Shade Layer                             │
│   ┌────────────────────────────────────────────────────┐    │
│   │   dot  →  one of {"·", "•", "●", " "}              │    │
│   └────────────────────────────────────────────────────┘    │
└─────────────────────────────────────────────────────────────┘
                            ↓
┌─────────────────────────────────────────────────────────────┐
│                     Color Layer                             │
│   ┌────────────────────────────────────────────────────┐    │
│   │   hex  →  rgb  →  ANSI  (\033[38;2;r;g;bm)         │    │
│   └────────────────────────────────────────────────────┘    │
└─────────────────────────────────────────────────────────────┘
                            ↓
┌─────────────────────────────────────────────────────────────┐
│                     Terminal Layer                          │
│   ┌────────────────────────────────────────────────────┐    │
│   │   Full-frame write  •  Cursor home (\033[H)        │    │
│   │   Color reset per frame  •  Hide/show cursor       │    │
│   └────────────────────────────────────────────────────┘    │
└─────────────────────────────────────────────────────────────┘
                            ↓
                    Terminal Display
```

`DIVAE` is a **buffer-first** renderer. Unlike direct-write engines, nothing reaches the terminal until `print()`. That's what makes frame-based animation clean: you build the whole frame in memory, write it once, then clear and repeat.

The engine doesn't hide this. The buffer is the primitive. You're meant to own it.

---

## 🚀 Quick Start

### Prerequisites

- **C++11 or higher** compiler
- **POSIX terminal** — Linux, macOS, BSD, WSL
- **24-bit true-color support** — Most modern terminals handle this
- **ANSI escape code support** — For cursor home and color

### Installation

`DIVAE` is a **single header-free C++ class**. Drop it into your project.

```bash
# Clone or copy divae.cpp into your project

# Compile with optimizations
g++ -O3 -std=c++11 your_demo.cpp -o your_demo

# Run
./your_demo
```

### Minimal Example

```cpp
#include "divae.cpp"
#include <thread>
#include <chrono>

int main() {
    divae screen(20, 60);

    screen.hideCursor();
    screen.setColor("#00FFAA");

    // draw a diagonal line of dots
    for (int i = 0; i < 20; ++i) {
        screen.setDot(i, i * 3, i % 3);
    }
    screen.print();

    std::this_thread::sleep_for(std::chrono::seconds(2));

    screen.showCursor();
    return 0;
}
```

That's it. Twenty dots, one diagonal, one frame.

### Animated Example

```cpp
divae screen(20, 60);
screen.hideCursor();
screen.setColor("#FF0066");

for (int frame = 0; frame < 60; ++frame) {
    screen.ClearDisplayBuffer();    // private in current code — expose it

    for (int x = 0; x < 60; ++x) {
        int y = (int)(10 + 8 * std::sin((x + frame) * 0.1));
        screen.setDot(y, x, 1);
    }

    screen.print();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
}

screen.showCursor();
```

A sine wave, dot by dot, one frame at a time.

---

## 🎯 Use Cases

### 1. Recreating ChatGPT's Animation
The inspiration behind `DIVAE` — dots appearing, brightening, and assembling. Every frame is a shade map. Build the reveal once and `DIVAE` renders it.

### 2. Terminal Sparklines
A live sparkline chart updated every frame. Rows of `·` `•` `●` reading as a data series. `setColor()` to match the value's sentiment.

### 3. Dot-Matrix Typography
Draw letters out of dots. A header, a logo, a countdown — all rendered in shades and one color.

### 4. Loading Animations
A pulsing dot, a circle of dots, a spiral of dots. Anything that fits "dot that moves, dot that brightens, dot that fades."

### 5. Sensor & Status Displays
A grid of dots showing active/inactive states. Three shades for three levels. One color per scene.

### 6. Generative Art
Perlin-noise-driven dot fields, wave interference patterns, cellular automata. If you can compute a value per cell, `DIVAE` can render it.

### 7. Learning Tool
The buffer is one flat array. The shades are three strings. The renderer is one loop. It's the clearest possible introduction to "the terminal is a raster."

---

## 🌈 Why DIVAE

Most terminal libraries treat the screen as a **terminal**: cells are text, text is a string, a program prints it. `DIVAE` treats the screen as a **display**: cells are dots, dots are shades, a program *renders* it.

That difference is small — the underlying technology is identical — but the *mental model* is completely different. With `DIVAE`, you're not printing text. You're **painting a matrix**. Every frame is a full redraw. Every dot is a decision.

The name says it well: dots, induced into a virtual art surface. That's all `DIVAE` does — and it's enough to build any dot-based effect you can imagine.

---

## ⚖️ Comparison

| | **DIVAE** | Plain `printf` | `simulaii` | Graphics Libraries |
|---|---|---|---|---|
| **Dependencies** | None | None | None | Many |
| **Unit of Work** | A dot | A string | An animated tile | A pixel |
| **Renders To** | Display buffer | Directly | Directly | Framebuffer |
| **Frame Model** | Buffer → print | No frames | Tile → tick | Full redraw |
| **True Color** | Yes | Yes | Yes | N/A |
| **Shades** | 3 dot shades | N/A | 4 block chars | ∞ |
| **Runs in Terminal** | Yes | Yes | Yes | No |
| **Learning Curve** | Minutes | Seconds | Minutes | Days to weeks |
| **Scope** | Dot-matrix rendering | Text | Animated tiles | Pixels |

`DIVAE` doesn't replace your terminal library. It replaces the assumption that the terminal is for text.

---

## 🧬 Design Philosophy

`DIVAE` is deliberately small.

- It renders **one buffer**.
- It uses **three shades**.
- It exposes **one method** for advancing a frame: `print()`.
- It stores **everything in memory** before writing to the terminal.

That last point is the key. `DIVAE` is a **buffered** renderer, not a direct one. That means you can draw in any order, at any time, and the screen won't change until you call `print()`. You get full control over what a "frame" means for your program.

The three-shade palette is on purpose. Adding a fourth or fifth shade would make the API more flexible and the art harder. Three dot sizes are enough to express *weight* — and that's all a dot matrix needs.

You're not meant to leave it alone. Change the shades. Add a `clear()` you can call yourself. Wrap the print loop with a `sin()` and watch it move. The source is short enough to read in one sitting, and the abstraction is simple enough to extend in an afternoon.

It's a rendering engine for people who want to **draw with dots** — and who want the terminal to feel like a canvas.

---

## 📚 API Reference

### Constructor

```cpp
divae(int rows, int cols);
```

Creates an engine with a display buffer sized to `rows × cols`. The buffer starts empty (all spaces).

### `setDot()`

```cpp
int setDot(int row, int col, int shadeIndex);
```

Writes one shade into the cell at `(row, col)`.

- `shadeIndex` `0` — `·` (faint)
- `shadeIndex` `1` — `•` (medium)
- `shadeIndex` `2` — `●` (solid)
- Any other value — clears the cell to a space

Returns `0` on success, `-1` if the coordinates are out of bounds.

### `print()`

```cpp
int print();
```

Renders the entire display buffer to the terminal:

1. Moves the cursor home (`\033[H`)
2. Applies the current color
3. Writes every cell row by row
4. Resets color (`\033[0m`)

Returns `0` on success.

### `setColor()`

```cpp
int setColor(std::string hexCode);
```

Sets the foreground color for every dot. Accepts a `#RRGGBB` hex string.

Returns `0` on success, `-1` if the string is not a valid hex color.

### `hideCursor()` / `showCursor()`

```cpp
int hideCursor();
int showCursor();
```

Hides or shows the terminal cursor. Call `hideCursor()` before an animation and `showCursor()` after.

### Internal: `ClearDisplayBuffer()`

> **Note:** In the current code, `ClearDisplayBuffer()` is `private`. For animations that draw every frame, you'll want to expose it publicly (or add a `clear()` method) so the caller can reset the buffer between frames.

```cpp
void ClearDisplayBuffer();   // currently private
```

Fills every cell in the display buffer with a space.

---

## 📄 License

Licensed under the MIT License. See [LICENSE](LICENSE) for details.

---

<div align="center">

**Dots. Shades. Color. A frame at a time.**

*"Every cell is a dot. Every frame is a decision. The terminal is a canvas."*

[⬆ Back to Top](#divae)

</div>