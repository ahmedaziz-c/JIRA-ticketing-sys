//DatabaseManager.hpp
#pragma once
#include "models.hpp"
#include <vector>
#include <string>
#include <memory>
#include <map>
#include <fstream>

// Use the EXACT path to your json.hpp file
#include "../external/nlohmann/json/single_include/nlohmann/json.hpp"
using json = nlohmann::json;

class DatabaseManager {
public:
    static DatabaseManager& getInstance();
    
    bool saveProject(const std::string& project_name);
    bool loadProject(const std::string& project_name);

    bool initialize(const std::string& db_path = "");
    bool initializeDemoData();
    bool switchProject(const std::string& project_name);
    bool createNewProject(const std::string& project_name);
    std::vector<std::string> getAvailableProjects();
    std::string getCurrentProjectName() const;

    // User operations
    bool createUser(const User& user);
    User getUser(int id);
    std::vector<User> getAllUsers();
    bool updateUser(const User& user);
    bool deleteUser(int id);

    // Ticket operations
    bool createTicket(Ticket& ticket);
    Ticket getTicket(int id);
    std::vector<Ticket> getAllTickets();
    std::vector<Ticket> getTicketsBySprint(int sprint_id);
    bool updateTicket(const Ticket& ticket);
    bool deleteTicket(int id);

    // Sprint operations
    bool createSprint(Sprint& sprint);
    Sprint getSprint(int id);
    std::vector<Sprint> getAllSprints();
    bool updateSprint(const Sprint& sprint);
    bool deleteSprint(int id);

    // Activity operations
    bool logActivity(const Activity& activity);
    std::vector<Activity> getRecentActivities(int limit = 50);

    // Add this destructor
    ~DatabaseManager();

private:
    DatabaseManager() = default;
    bool createTables();
    bool isSQLiteAvailable() const;
    std::string getProjectFilePath(const std::string& project_name);
    void clearInMemoryData();
    void loadInMemoryData();

    std::string db_path_;
    std::string current_project_;
    bool use_sqlite_ = false;

    // In-memory storage organized by project
    struct ProjectData {
        std::vector<User> users;
        std::vector<Ticket> tickets;
        std::vector<Sprint> sprints;
        std::vector<Activity> activities;
        int next_user_id = 1;
        int next_ticket_id = 1;
        int next_sprint_id = 1;
        int next_activity_id = 1;
    };

    std::map<std::string, ProjectData> projects_;
    ProjectData* current_data_ = nullptr;
};