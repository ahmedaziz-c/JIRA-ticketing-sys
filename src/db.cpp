#include "UIManager.hpp"
#include "DatabaseManager.hpp"
#include "TicketManager.hpp"
#include "SprintManager.hpp"
#include "models.hpp"
#include "db.hpp"
#include "globals.hpp"
#include <sqlite3.h>
#include <ctime>

class Db::Impl {
public:
    sqlite3* db = nullptr;
};

Db::Db(const std::string& file) : p(new Impl) {
    if (sqlite3_open(file.c_str(), &p->db) != SQLITE_OK)
        throw std::runtime_error("sqlite open failed");
    char* err = nullptr;
    sqlite3_exec(p->db,
        "CREATE TABLE IF NOT EXISTS tickets("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "title TEXT NOT NULL,"
        "status INTEGER NOT NULL,"
        "created TEXT NOT NULL);",
        nullptr, nullptr, &err);
    if (err) { sqlite3_free(err); throw std::runtime_error("create table"); }
}

Db::~Db() { sqlite3_close(p->db); delete p; }

std::vector<Ticket> Db::load_all() {
    std::vector<Ticket> out;
    sqlite3_stmt* st;
    sqlite3_prepare_v2(p->db, "SELECT id,title,status,created FROM tickets ORDER BY id;", -1, &st, nullptr);
    while (sqlite3_step(st) == SQLITE_ROW) {
        Ticket t;
        t.id      = sqlite3_column_int(st, 0);
        t.title   = reinterpret_cast<const char*>(sqlite3_column_text(st, 1));
        t.status  = sqlite3_column_int(st, 2);
        t.created = reinterpret_cast<const char*>(sqlite3_column_text(st, 3));
        out.push_back(std::move(t));
    }
    sqlite3_finalize(st);
    return out;
}

int Db::insert_ticket(const std::string& title) {
    sqlite3_stmt* st;
    sqlite3_prepare_v2(p->db,
        "INSERT INTO tickets(title,status,created) VALUES(?1,0,datetime('now'));", -1, &st, nullptr);
    sqlite3_bind_text(st, 1, title.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_step(st);
    int newid = static_cast<int>(sqlite3_last_insert_rowid(p->db));
    sqlite3_finalize(st);
    return newid;
}

void Db::move_ticket(int id, int new_status) {
    sqlite3_stmt* st;
    sqlite3_prepare_v2(p->db, "UPDATE tickets SET status=?1 WHERE id=?2;", -1, &st, nullptr);
    sqlite3_bind_int(st, 1, new_status);
    sqlite3_bind_int(st, 2, id);
    sqlite3_step(st);
    sqlite3_finalize(st);
}