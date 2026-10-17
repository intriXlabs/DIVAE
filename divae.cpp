#include <iostream>
#include <string>
#include <vector>
// · ∙ • ●

class divae {

private:
    std::string shades[3] = {
        "·", "•", "●"
    };
    std::string color;

private:
    int terminalRows;
    int terinalCols;

    std::string* displayBuffer;

private:
    void ClearDisplayBuffer() {
        for (int i = 0; i < terminalRows * terinalCols; i++) {
            displayBuffer[i] = " ";
        }
    }

    std::string convertHexToAnsiColor(const std::string& hexCode) {
        if (hexCode.length() != 7 || hexCode[0] != '#') {
            return ""; // Invalid hex code
        }

        int r = std::stoi(hexCode.substr(1, 2), nullptr, 16);
        int g = std::stoi(hexCode.substr(3, 2), nullptr, 16);
        int b = std::stoi(hexCode.substr(5, 2), nullptr, 16);

        return "\033[38;2;" + std::to_string(r) + ";" + std::to_string(g) + ";" + std::to_string(b) + "m";
    }

public:
    divae(int rows, int cols){
        terminalRows = rows;
        terinalCols = cols;
        displayBuffer = new std::string[terminalRows * terinalCols];
        ClearDisplayBuffer();
    }

    int setDot(int row, int col, int shadeIndex) {
        if (row < 0 || row >= terminalRows || col < 0 || col >= terinalCols) {
            return -1; // Out of bounds
        }

        if(shadeIndex >= 0 & shadeIndex < 3) {
            displayBuffer[row * terinalCols + col] = shades[shadeIndex];
        }else{
            displayBuffer[row * terinalCols + col] = " "; // Set to empty space if shadeIndex is invalid
        }

        return 0;
    }

    int print() {
        std::string output;
        output= "\033[H";
        output += color;
        for (int i = 0; i < terminalRows; i++) {
            for (int j = 0; j < terinalCols; j++) {
                output += displayBuffer[i * terinalCols + j];
            }
            output += "\n";
        }
        output += "\033[0m";
        std::cout << output << std::flush;
        return 0;
    }

    int setColor(std::string hexCode) {
        color = convertHexToAnsiColor(hexCode);
        if (color.empty()) {
            return -1; // Invalid hex code
        }
        return 0;
    }

    int hideCursor() {
        std::cout << "\033[?25l" << std::flush;
        return 0;
    }

    int showCursor() {
        std::cout << "\033[?25h" << std::flush;
        return 0;
    }

};