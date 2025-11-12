#include "UIManager.hpp"
#include "DatabaseManager.hpp"
#include "TicketManager.hpp"
#include "SprintManager.hpp"
#include "models.hpp"
#include <iostream>
#include <fstream>
#include <algorithm>
#include <filesystem>

DatabaseManager& DatabaseManager::getInstance()
{
    static DatabaseManager instance;
    return instance;
}

bool DatabaseManager::initialize(const std::string& db_path)
{
    // For retro feel, we use JSON files only
    use_sqlite_ = false;
    
    // Create data directory if it doesn't exist
    std::filesystem::create_directory("projects");
    
    return true;
}

bool DatabaseManager::initializeDemoData()
{
    if (!current_data_) {
        if (projects_.find("default") == projects_.end()) {
            createNewProject("default");
        }
        switchProject("default");
    }
    
    if (!current_data_) return false;
    
    // Create demo users
    User admin("admin", "admin", "admin");
    admin.id = current_data_->next_user_id++;
    current_data_->users.push_back(admin);
    
    User dev1("john", "password", "user");
    dev1.id = current_data_->next_user_id++;
    current_data_->users.push_back(dev1);
    
    User dev2("jane", "password", "user");
    dev2.id = current_data_->next_user_id++;
    current_data_->users.push_back(dev2);
    
    // Create demo sprint
    Sprint sprint1;
    sprint1.id = current_data_->next_sprint_id++;
    sprint1.name = "Sprint 1 - Launch";
    sprint1.goal = "Launch initial version";
    sprint1.start_date = std::time(nullptr);
    sprint1.end_date = std::time(nullptr) + (14 * 24 * 60 * 60); // 14 days from now
    sprint1.status = "active";
    current_data_->sprints.push_back(sprint1);
    
    // Create demo tickets
    Ticket ticket1;
    ticket1.id = current_data_->next_ticket_id++;
    ticket1.title = "Implement user authentication";
    ticket1.description = "Add login and user management features";
    ticket1.status = "in_progress";
    ticket1.priority = "high";
    ticket1.type = "feature";
    ticket1.assignee_id = dev1.id;
    ticket1.sprint_id = sprint1.id;
    ticket1.story_points = 5;
    current_data_->tickets.push_back(ticket1);
    
    Ticket ticket2;
    ticket2.id = current_data_->next_ticket_id++;
    ticket2.title = "Fix navigation bug";
    ticket2.description = "Navigation breaks on small screens";
    ticket2.status = "todo";
    ticket2.priority = "medium";
    ticket2.type = "bug";
    ticket2.assignee_id = dev2.id;
    ticket2.sprint_id = sprint1.id;
    ticket2.story_points = 3;
    current_data_->tickets.push_back(ticket2);
    
    Ticket ticket3;
    ticket3.id = current_data_->next_ticket_id++;
    ticket3.title = "Design database schema";
    ticket3.description = "Create initial database structure";
    ticket3.status = "done";
    ticket3.priority = "high";
    ticket3.type = "task";
    ticket3.assignee_id = admin.id;
    ticket3.sprint_id = sprint1.id;
    ticket3.story_points = 8;
    current_data_->tickets.push_back(ticket3);
    
    // Log some activities
    Activity activity1;
    activity1.id = current_data_->next_activity_id++;
    activity1.ticket_id = ticket1.id;
    activity1.user_id = admin.id;
    activity1.action = "created";
    activity1.description = "Created ticket: " + ticket1.title;
    activity1.timestamp = std::time(nullptr) - 3600;
    current_data_->activities.push_back(activity1);
    
    Activity activity2;
    activity2.id = current_data_->next_activity_id++;
    activity2.ticket_id = ticket1.id;
    activity2.user_id = dev1.id;
    activity2.action = "assigned";
    activity2.description = "Assigned ticket to John";
    activity2.timestamp = std::time(nullptr) - 1800;
    current_data_->activities.push_back(activity2);
    
    return saveProject("default");
}

bool DatabaseManager::createNewProject(const std::string& project_name)
{
    if (project_name.empty() || projects_.count(project_name) > 0)
    {
        return false;
    }
    
    // Create fresh project data
    projects_[project_name] = ProjectData{};
    
    // Set as current project
    return switchProject(project_name);
}

bool DatabaseManager::switchProject(const std::string& project_name)
{
    auto it = projects_.find(project_name);
    if (it == projects_.end())
    {
        // Try to load from file
        if (!loadProject(project_name))
        {
            return false;
        }
        it = projects_.find(project_name);
    }
    
    current_project_ = project_name;
    current_data_ = &it->second;
    return true;
}

bool DatabaseManager::saveProject(const std::string& project_name)
{
    auto it = projects_.find(project_name);
    if (it == projects_.end())
    {
        return false;
    }
    
    json project_data;
    auto& data = it->second;
    
    project_data["users"] = data.users;
    project_data["tickets"] = data.tickets;
    project_data["sprints"] = data.sprints;
    project_data["activities"] = data.activities;
    project_data["next_ids"] = {
        {"user", data.next_user_id},
        {"ticket", data.next_ticket_id},
        {"sprint", data.next_sprint_id},
        {"activity", data.next_activity_id}
    };
    
    std::string file_path = getProjectFilePath(project_name);
    std::ofstream file(file_path);
    
    if (!file.is_open())
    {
        return false;
    }
    
    file << project_data.dump(4);
    return true;
}

bool DatabaseManager::loadProject(const std::string& project_name)
{
    std::string file_path = getProjectFilePath(project_name);
    std::ifstream file(file_path);
    
    if (!file.is_open())
    {
        return false;
    }
    
    try
    {
        json project_data;
        file >> project_data;
        
        ProjectData data;
        data.users = project_data["users"].get<std::vector<User>>();
        data.tickets = project_data["tickets"].get<std::vector<Ticket>>();
        data.sprints = project_data["sprints"].get<std::vector<Sprint>>();
        data.activities = project_data["activities"].get<std::vector<Activity>>();
        
        auto next_ids = project_data["next_ids"];
        data.next_user_id = next_ids["user"];
        data.next_ticket_id = next_ids["ticket"];
        data.next_sprint_id = next_ids["sprint"];
        data.next_activity_id = next_ids["activity"];
        
        projects_[project_name] = data;
        return true;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error loading project: " << e.what() << std::endl;
        return false;
    }
}

// User operations
bool DatabaseManager::createUser(const User& user)
{
    if (!current_data_)
    {
        return false;
    }
    
    // Check for duplicate username
    for (const auto& existing_user : current_data_->users)
    {
        if (existing_user.username == user.username)
        {
            return false;
        }
    }
    
    User new_user = user;
    new_user.id = current_data_->next_user_id++;
    current_data_->users.push_back(new_user);
    
    // Log activity
    Activity activity;
    activity.id = current_data_->next_activity_id++;
    activity.user_id = new_user.id;
    activity.action = "user_created";
    activity.description = "Created user: " + new_user.username;
    activity.timestamp = std::time(nullptr);
    current_data_->activities.push_back(activity);
    
    return true;
}

User DatabaseManager::getUser(int id)
{
    if (!current_data_)
    {
        return User{};
    }
    
    auto it = std::find_if(current_data_->users.begin(), current_data_->users.end(),
                          [id](const User& u) { return u.id == id; });
    
    return it != current_data_->users.end() ? *it : User{};
}

std::vector<User> DatabaseManager::getAllUsers()
{
    return current_data_ ? current_data_->users : std::vector<User>{};
}

bool DatabaseManager::updateUser(const User& user)
{
    if (!current_data_)
    {
        return false;
    }
    
    auto it = std::find_if(current_data_->users.begin(), current_data_->users.end(),
                          [&user](const User& u) { return u.id == user.id; });
    
    if (it != current_data_->users.end())
    {
        *it = user;
        it->updated_at = std::time(nullptr);
        
        // Log activity
        Activity activity;
        activity.id = current_data_->next_activity_id++;
        activity.user_id = user.id;
        activity.action = "user_updated";
        activity.description = "Updated user: " + user.username;
        activity.timestamp = std::time(nullptr);
        current_data_->activities.push_back(activity);
        
        return true;
    }
    
    return false;
}

bool DatabaseManager::deleteUser(int id)
{
    if (!current_data_)
    {
        return false;
    }
    
    auto it = std::find_if(current_data_->users.begin(), current_data_->users.end(),
                          [id](const User& u) { return u.id == id; });
    
    if (it != current_data_->users.end())
    {
        std::string username = it->username;
        current_data_->users.erase(it);
        
        // Log activity
        Activity activity;
        activity.id = current_data_->next_activity_id++;
        activity.action = "user_deleted";
        activity.description = "Deleted user: " + username;
        activity.timestamp = std::time(nullptr);
        current_data_->activities.push_back(activity);
        
        return true;
    }
    
    return false;
}

// Ticket operations
bool DatabaseManager::createTicket(Ticket& ticket)
{
    if (!current_data_)
    {
        return false;
    }
    
    ticket.id = current_data_->next_ticket_id++;
    ticket.created_at = std::time(nullptr);
    ticket.updated_at = std::time(nullptr);
    current_data_->tickets.push_back(ticket);
    
    // Log activity
    Activity activity;
    activity.id = current_data_->next_activity_id++;
    activity.ticket_id = ticket.id;
    activity.action = "ticket_created";
    activity.description = "Created ticket: " + ticket.title;
    activity.timestamp = std::time(nullptr);
    current_data_->activities.push_back(activity);
    
    return true;
}

Ticket DatabaseManager::getTicket(int id)
{
    if (!current_data_)
    {
        return Ticket{};
    }
    
    auto it = std::find_if(current_data_->tickets.begin(), current_data_->tickets.end(),
                          [id](const Ticket& t) { return t.id == id; });
    
    return it != current_data_->tickets.end() ? *it : Ticket{};
}

std::vector<Ticket> DatabaseManager::getAllTickets()
{
    return current_data_ ? current_data_->tickets : std::vector<Ticket>{};
}

std::vector<Ticket> DatabaseManager::getTicketsBySprint(int sprint_id)
{
    if (!current_data_)
    {
        return std::vector<Ticket>{};
    }
    
    std::vector<Ticket> result;
    std::copy_if(current_data_->tickets.begin(), current_data_->tickets.end(),
                std::back_inserter(result),
                [sprint_id](const Ticket& t) { return t.sprint_id == sprint_id; });
    
    return result;
}

bool DatabaseManager::updateTicket(const Ticket& ticket)
{
    if (!current_data_)
    {
        return false;
    }
    
    auto it = std::find_if(current_data_->tickets.begin(), current_data_->tickets.end(),
                          [&ticket](const Ticket& t) { return t.id == ticket.id; });
    
    if (it != current_data_->tickets.end())
    {
        Ticket old_ticket = *it;
        *it = ticket;
        it->updated_at = std::time(nullptr);
        
        // Log activity if status changed
        if (old_ticket.status != ticket.status)
        {
            Activity activity;
            activity.id = current_data_->next_activity_id++;
            activity.ticket_id = ticket.id;
            activity.action = "status_changed";
            activity.description = "Changed ticket status from " + old_ticket.status + " to " + ticket.status;
            activity.timestamp = std::time(nullptr);
            current_data_->activities.push_back(activity);
        }
        
        return true;
    }
    
    return false;
}

bool DatabaseManager::deleteTicket(int id)
{
    if (!current_data_)
    {
        return false;
    }
    
    auto it = std::find_if(current_data_->tickets.begin(), current_data_->tickets.end(),
                          [id](const Ticket& t) { return t.id == id; });
    
    if (it != current_data_->tickets.end())
    {
        std::string title = it->title;
        current_data_->tickets.erase(it);
        
        // Log activity
        Activity activity;
        activity.id = current_data_->next_activity_id++;
        activity.action = "ticket_deleted";
        activity.description = "Deleted ticket: " + title;
        activity.timestamp = std::time(nullptr);
        current_data_->activities.push_back(activity);
        
        return true;
    }
    
    return false;
}

// Sprint operations
bool DatabaseManager::createSprint(Sprint& sprint)
{
    if (!current_data_)
    {
        return false;
    }
    
    sprint.id = current_data_->next_sprint_id++;
    current_data_->sprints.push_back(sprint);
    
    // Log activity
    Activity activity;
    activity.id = current_data_->next_activity_id++;
    activity.action = "sprint_created";
    activity.description = "Created sprint: " + sprint.name;
    activity.timestamp = std::time(nullptr);
    current_data_->activities.push_back(activity);
    
    return true;
}

Sprint DatabaseManager::getSprint(int id)
{
    if (!current_data_)
    {
        return Sprint{};
    }
    
    auto it = std::find_if(current_data_->sprints.begin(), current_data_->sprints.end(),
                          [id](const Sprint& s) { return s.id == id; });
    
    return it != current_data_->sprints.end() ? *it : Sprint{};
}

std::vector<Sprint> DatabaseManager::getAllSprints()
{
    return current_data_ ? current_data_->sprints : std::vector<Sprint>{};
}

bool DatabaseManager::updateSprint(const Sprint& sprint)
{
    if (!current_data_)
    {
        return false;
    }
    
    auto it = std::find_if(current_data_->sprints.begin(), current_data_->sprints.end(),
                          [&sprint](const Sprint& s) { return s.id == sprint.id; });
    
    if (it != current_data_->sprints.end())
    {
        *it = sprint;
        
        // Log activity
        Activity activity;
        activity.id = current_data_->next_activity_id++;
        activity.action = "sprint_updated";
        activity.description = "Updated sprint: " + sprint.name;
        activity.timestamp = std::time(nullptr);
        current_data_->activities.push_back(activity);
        
        return true;
    }
    
    return false;
}

bool DatabaseManager::deleteSprint(int id)
{
    if (!current_data_)
    {
        return false;
    }
    
    auto it = std::find_if(current_data_->sprints.begin(), current_data_->sprints.end(),
                          [id](const Sprint& s) { return s.id == id; });
    
    if (it != current_data_->sprints.end())
    {
        std::string name = it->name;
        current_data_->sprints.erase(it);
        
        // Log activity
        Activity activity;
        activity.id = current_data_->next_activity_id++;
        activity.action = "sprint_deleted";
        activity.description = "Deleted sprint: " + name;
        activity.timestamp = std::time(nullptr);
        current_data_->activities.push_back(activity);
        
        return true;
    }
    
    return false;
}

// Activity operations
bool DatabaseManager::logActivity(const Activity& activity)
{
    if (!current_data_)
    {
        return false;
    }
    
    Activity new_activity = activity;
    new_activity.id = current_data_->next_activity_id++;
    new_activity.timestamp = std::time(nullptr);
    current_data_->activities.push_back(new_activity);
    
    return true;
}

std::vector<Activity> DatabaseManager::getRecentActivities(int limit)
{
    if (!current_data_)
    {
        return std::vector<Activity>{};
    }
    
    std::vector<Activity> result;
    int start = std::max(0, static_cast<int>(current_data_->activities.size()) - limit);
    
    for (size_t i = start; i < current_data_->activities.size(); ++i)
    {
        result.push_back(current_data_->activities[i]);
    }
    
    // Sort by timestamp (newest first)
    std::sort(result.begin(), result.end(), 
              [](const Activity& a, const Activity& b) { 
                  return a.timestamp > b.timestamp; 
              });
    
    return result;
}

std::vector<std::string> DatabaseManager::getAvailableProjects()
{
    std::vector<std::string> projects;
    
    // Check if projects directory exists
    if (!std::filesystem::exists("projects"))
    {
        return projects;
    }
    
    // Scan project directory for JSON files
    for (const auto& entry : std::filesystem::directory_iterator("projects"))
    {
        if (entry.is_regular_file() && entry.path().extension() == ".json")
        {
            std::string project_name = entry.path().stem().string();
            projects.push_back(project_name);
        }
    }
    
    return projects;
}

std::string DatabaseManager::getCurrentProjectName() const
{
    return current_project_;
}

bool DatabaseManager::isSQLiteAvailable() const
{
    // For retro feel, we don't use SQLite
    return false;
}

bool DatabaseManager::createTables()
{
    // Not used in JSON mode
    return true;
}

std::string DatabaseManager::getProjectFilePath(const std::string& project_name)
{
    return "projects/" + project_name + ".json";
}

void DatabaseManager::clearInMemoryData()
{
    // Clear current project data
    if (current_data_)
    {
        current_data_->users.clear();
        current_data_->tickets.clear();
        current_data_->sprints.clear();
        current_data_->activities.clear();
        current_data_->next_user_id = 1;
        current_data_->next_ticket_id = 1;
        current_data_->next_sprint_id = 1;
        current_data_->next_activity_id = 1;
    }
}

void DatabaseManager::loadInMemoryData()
{
    // This is handled by loadProject now
}

// Auto-save on destruction
DatabaseManager::~DatabaseManager()
{
    if (!current_project_.empty())
    {
        saveProject(current_project_);
    }
}

// Auto-save on destruction
DatabaseManager::~DatabaseManager()
{
    if (!current_project_.empty())
    {
        saveProject(current_project_);
    }
}