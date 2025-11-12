#pragma once
#include <vector>
#include "db.hpp"

class Board {
public:
    explicit Board(Db& db);
    void draw();               // ncurses paint
    void handle_input(int ch); // F-keys or arrows
private:
    Db& db_;
    std::vector<Ticket> tickets_;
    int highlight_ = 0;        // 0..N-1 index into tickets_
    void reload();
};