#include "UIManager.hpp"
#include "DatabaseManager.hpp"
#include "TicketManager.hpp"
#include "SprintManager.hpp"
#include "models.hpp"
#include "RetroTUI.hpp"
#ifdef _WIN32
    #include <windows.h>
    #include <conio.h>
#else
    #include <termios.h>
    #include <unistd.h>
    #include <sys/ioctl.h>
#endif
#include <iostream>
#include <cstdlib>

RetroTUI::RetroTUI() : initialized_(false), screenWidth_(80), screenHeight_(24) {}

RetroTUI::~RetroTUI()
{
    cleanup();
}

void RetroTUI::initialize()
{
#ifdef _WIN32
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD dwMode = 0;
    GetConsoleMode(hOut, &dwMode);
    dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    SetConsoleMode(hOut, dwMode);
    
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(hOut, &csbi);
    screenWidth_ = csbi.srWindow.Right - csbi.srWindow.Left + 1;
    screenHeight_ = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
#else
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    screenWidth_ = w.ws_col;
    screenHeight_ = w.ws_row;
    
    enableRawMode();
#endif

    setupColors();
    clearScreen();
    initialized_ = true;
}

void RetroTUI::cleanup()
{
    setColor(WHITE, BLACK);
    clearScreen();
    
#ifndef _WIN32
    disableRawMode();
#endif
    
    std::cout << "\033[0m" << std::flush;
}

void RetroTUI::setupColors()
{
    // Setup basic color pairs - simplified for cross-platform
}

void RetroTUI::clearScreen()
{
    std::cout << "\033[2J\033[H";
}

void RetroTUI::refreshScreen()
{
    std::cout << std::flush;
}

void RetroTUI::setColor(Color foreground, Color background)
{
    int fg = 30 + foreground;
    int bg = 40 + background;
    std::cout << "\033[" << fg << "m";
    if (background != BLACK)
    {
        std::cout << "\033[" << bg << "m";
    }
}

void RetroTUI::printCentered(int y, const std::string& text)
{
    int x = (screenWidth_ - text.length()) / 2;
    printAt(std::max(0, x), y, text);
}

void RetroTUI::printAt(int x, int y, const std::string& text)
{
    std::cout << "\033[" << y + 1 << ";" << x + 1 << "H" << text;
}

void RetroTUI::drawBox(int x, int y, int width, int height)
{
    // Simple box drawing
    std::string topBottom = "+" + std::string(width - 2, '-') + "+";
    std::string middle = "|" + std::string(width - 2, ' ') + "|";
    
    printAt(x, y, topBottom);
    for (int i = 1; i < height - 1; ++i)
    {
        printAt(x, y + i, middle);
    }
    printAt(x, y + height - 1, topBottom);
}

int RetroTUI::getKey()
{
#ifdef _WIN32
    return _getch();
#else
    char c;
    read(STDIN_FILENO, &c, 1);
    return c;
#endif
}

std::string RetroTUI::getInput(int maxLength)
{
    std::string input;
    char c;
    
    while (input.length() < maxLength)
    {
        c = getKey();
        
        if (c == '\n' || c == '\r')
        {
            break;
        }
        else if (c == 127 || c == 8) // Backspace
        {
            if (!input.empty())
            {
                input.pop_back();
                std::cout << "\b \b";
            }
        }
        else if (c >= 32 && c <= 126) // Printable characters
        {
            input += c;
            std::cout << c;
        }
    }
    
    return input;
}

int RetroTUI::showMenu(const std::vector<std::string>& options, const std::string& title)
{
    int selected = 0;
    bool chosen = false;
    
    while (!chosen)
    {
        clearScreen();
        
        if (!title.empty())
        {
            printCentered(2, title);
            printCentered(3, std::string(title.length(), '='));
        }
        
        for (size_t i = 0; i < options.size(); ++i)
        {
            if (i == selected)
            {
                setColor(BLACK, WHITE);
                printCentered(6 + i, "> " + options[i] + " <");
                setColor(WHITE, BLACK);
            }
            else
            {
                printCentered(6 + i, "  " + options[i] + "  ");
            }
        }
        
        printCentered(6 + options.size() + 2, "Use arrow keys to navigate, ENTER to select");
        refreshScreen();
        
        int key = getKey();
        
        // Handle arrow keys (ANSI escape sequences)
        if (key == 27) // ESC
        {
            getKey(); // Skip '['
            switch (getKey())
            {
                case 'A': // Up
                    selected = (selected > 0) ? selected - 1 : options.size() - 1;
                    break;
                case 'B': // Down
                    selected = (selected < options.size() - 1) ? selected + 1 : 0;
                    break;
            }
        }
        else if (key == '\n' || key == '\r')
        {
            chosen = true;
        }
        else if (key == 'q' || key == 'Q')
        {
            return -1;
        }
    }
    
    return selected;
}

void RetroTUI::drawReceiptHeader(const std::string& title)
{
    setColor(WHITE, BLACK);
    printCentered(2, "╔══════════════════════════════════════════════════╗");
    printCentered(3, "║                  " + title + "                   ║");
    printCentered(4, "╚══════════════════════════════════════════════════╝");
}

void RetroTUI::drawReceiptLine(const std::string& label, const std::string& value)
{
    std::string line = "│ " + label;
    if (!value.empty())
    {
        int spacing = 40 - label.length() - value.length();
        if (spacing > 0)
        {
            line += std::string(spacing, ' ');
        }
        line += value;
    }
    line += " │";
    std::cout << "\033[?7l" << line << "\033[?7h" << std::endl;
}

void RetroTUI::drawReceiptSeparator()
{
    std::cout << "├────────────────────────────────────────────────────┤" << std::endl;
}

void RetroTUI::drawReceiptFooter(const std::string& footer)
{
    std::cout << "╰────────────────────────────────────────────────────╯" << std::endl;
    if (!footer.empty())
    {
        printCentered(screenHeight_ - 2, footer);
    }
}

#ifndef _WIN32
void RetroTUI::enableRawMode()
{
    struct termios term;
    tcgetattr(STDIN_FILENO, &term);
    term.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &term);
}

void RetroTUI::disableRawMode()
{
    struct termios term;
    tcgetattr(STDIN_FILENO, &term);
    term.c_lflag |= (ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &term);
}
#endif