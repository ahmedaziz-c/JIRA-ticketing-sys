#include "UIManager.hpp"
#include "DatabaseManager.hpp"
#include "TicketManager.hpp"
#include "SprintManager.hpp"
#include "models.hpp"
#include "board.hpp"
#include "db.hpp"
#include "input.hpp"
#include "globals.hpp"
#include <ncurses.h>
#include <locale.h>

int main(int argc, char* argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s project.db\n", argv[0]);
        return 1;
    }
    g::db_path = argv[1];

    setlocale(LC_ALL, "");
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    nodelay(stdscr, FALSE);
    raw();                       // steal F-keys
    start_color();
    init_pair(1, COLOR_GREEN, COLOR_BLACK);

    try {
        Db db(g::db_path);
        Board brd(db);
        brd.draw();
        while (true) {
            int ch = get_fkey();
            if (ch == g::KEY_F12 || ch == g::KEY_CTRL_Q) break;
            brd.handle_input(ch);
            brd.draw();
        }
    } catch (const std::exception& e) {
        endwin();
        fprintf(stderr, "Error: %s\n", e.what());
        return 1;
    }
    endwin();
    return 0;
} 