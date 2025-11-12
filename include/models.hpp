//models.hpp
#pragma once
#include <string>
#include <vector>
#include <chrono>
#include <ctime>

struct User {
    int id;
    std::string username;
    std::string password;  // In real app, hash this!
    std::string role;      // "admin", "user", "viewer"
    time_t created_at;
    
    User() : id(0), created_at(std::time(nullptr)) {}
    User(std::string uname, std::string pwd, std::string r = "user") 
        : id(0), username(uname), password(pwd), role(r), created_at(std::time(nullptr)) {}
};

struct Ticket {
    int id;
    std::string title;
    std::string description;
    std::string status;      // "todo", "in_progress", "review", "done"
    std::string priority;    // "low", "medium", "high", "critical"
    std::string type;        // "bug", "feature", "task", "story"
    int assignee_id;
    int sprint_id;
    int story_points;
    time_t created_at;
    time_t updated_at;
    
    Ticket() : id(0), assignee_id(0), sprint_id(0), story_points(0), 
               created_at(std::time(nullptr)), updated_at(std::time(nullptr)) {}
};

struct Sprint {
    int id;
    std::string name;
    std::string goal;
    time_t start_date;
    time_t end_date;
    std::string status;      // "planned", "active", "completed"
    
    Sprint() : id(0), start_date(std::time(nullptr)), 
               end_date(std::time(nullptr)), status("planned") {}
};

struct Activity {
    int id;
    int ticket_id;
    int user_id;
    std::string action;
    std::string description;
    time_t timestamp;
    
    Activity() : id(0), ticket_id(0), user_id(0), timestamp(std::time(nullptr)) {}
};