#pragma once
#include <string>
#include <vector>
#include <functional>

class RetroTUI {
public:
    enum Color {
        BLACK = 0,
        BLUE = 1,
        GREEN = 2,
        CYAN = 3,
        RED = 4,
        MAGENTA = 5,
        YELLOW = 6,
        WHITE = 7,
        BRIGHT = 8
    };

    RetroTUI();
    ~RetroTUI();

    void initialize();
    void cleanup();
    
    // Screen management
    void clearScreen();
    void refreshScreen();
    void setColor(Color foreground, Color background = BLACK);
    
    // Text output
    void printCentered(int y, const std::string& text);
    void printAt(int x, int y, const std::string& text);
    void drawBox(int x, int y, int width, int height);
    
    // Input handling
    int getKey();
    std::string getInput(int maxLength = 50);
    
    // Menu system
    int showMenu(const std::vector<std::string>& options, 
                const std::string& title = "");
    
    // ATM-style receipt formatting
    void drawReceiptHeader(const std::string& title);
    void drawReceiptLine(const std::string& label, const std::string& value = "");
    void drawReceiptSeparator();
    void drawReceiptFooter(const std::string& footer = "");
    
private:
    void setupColors();
    void enableRawMode();
    void disableRawMode();
    
    bool initialized_;
    int screenWidth_;
    int screenHeight_;
};