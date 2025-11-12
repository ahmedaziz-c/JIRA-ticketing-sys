#pragma once
#include <vector>
#include <string>

struct Ticket {
    int         id;
    std::string title;
    int         status;   // 0,1,2
    std::string created;
};

class Db {
public:
    explicit Db(const std::string& file);
    ~Db();
    std::vector<Ticket> load_all();
    int insert_ticket(const std::string& title);
    void move_ticket(int id, int new_status);
private:
    class Impl;
    Impl* p;
};