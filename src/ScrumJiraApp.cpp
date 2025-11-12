#include "UIManager.hpp"
#include "DatabaseManager.hpp"
#include "TicketManager.hpp"
#include "SprintManager.hpp"
#include "models.hpp"
#include "ScrumJiraApp.hpp"
#include <iostream>

ScrumJiraApp::ScrumJiraApp() : running_(false) {}

ScrumJiraApp::~ScrumJiraApp()
{
    if (db_)
    {
        db_->saveProject(db_->getCurrentProjectName());
    }
}

void ScrumJiraApp::initialize()
{
    ui_ = std::make_unique<RetroTUI>();
    db_ = std::make_unique<DatabaseManager>();
    
    ui_->initialize();
    db_->initialize();
    
    // Initialize demo data if no projects exist
    if (db_->getAvailableProjects().empty())
    {
        db_->initializeDemoData();
    }
}

void ScrumJiraApp::run()
{
    initialize();
    running_ = true;
    
    while (running_)
    {
        mainMenu();
    }
}

void ScrumJiraApp::mainMenu()
{
    std::vector<std::string> options = {
        "Project Management",
        "Ticket Management", 
        "Sprint Management",
        "User Management",
        "Reports & Analytics",
        "Settings",
        "Exit"
    };
    
    ui_->drawReceiptHeader("SCRUM JIRA MANAGER v1.0");
    ui_->drawReceiptLine("Current Project:", db_->getCurrentProjectName());
    ui_->drawReceiptSeparator();
    
    int choice = ui_->showMenu(options, "MAIN MENU");
    
    switch (choice)
    {
        case 0: projectManagement(); break;
        case 1: ticketManagement(); break;
        case 2: sprintManagement(); break;
        case 3: userManagement(); break;
        case 4: /* reports */ break;
        case 5: /* settings */ break;
        case 6: running_ = false; break;
        default: break;
    }
}

void ScrumJiraApp::projectManagement()
{
    std::vector<std::string> options = {
        "Create New Project",
        "Load Project", 
        "Switch Project",
        "Delete Project",
        "Back to Main Menu"
    };
    
    int choice = ui_->showMenu(options, "PROJECT MANAGEMENT");
    
    switch (choice)
    {
        case 0: createNewProject(); break;
        case 1: loadExistingProject(); break;
        case 2: /* switch project */ break;
        case 3: /* delete project */ break;
        default: break;
    }
}

void ScrumJiraApp::createNewProject()
{
    ui_->clearScreen();
    ui_->drawReceiptHeader("CREATE NEW PROJECT");
    
    ui_->printAt(10, 6, "Project Name: ");
    std::string name = ui_->getInput(30);
    
    if (!name.empty() && db_->createNewProject(name))
    {
        ui_->printAt(10, 8, "Project created successfully!");
    }
    else
    {
        ui_->printAt(10, 8, "Failed to create project!");
    }
    
    ui_->printAt(10, 10, "Press any key to continue...");
    ui_->getKey();
}

void ScrumJiraApp::loadExistingProject()
{
    auto projects = db_->getAvailableProjects();
    if (projects.empty())
    {
        ui_->clearScreen();
        ui_->drawReceiptHeader("LOAD PROJECT");
        ui_->printAt(10, 6, "No projects available!");
        ui_->printAt(10, 8, "Press any key to continue...");
        ui_->getKey();
        return;
    }
    
    int choice = ui_->showMenu(projects, "SELECT PROJECT");
    if (choice >= 0 && choice < projects.size())
    {
        if (db_->switchProject(projects[choice]))
        {
            ui_->clearScreen();
            ui_->drawReceiptHeader("PROJECT LOADED");
            ui_->printAt(10, 6, "Project '" + projects[choice] + "' loaded successfully!");
            ui_->printAt(10, 8, "Press any key to continue...");
            ui_->getKey();
        }
    }
}

void ScrumJiraApp::ticketManagement()
{
    std::vector<std::string> options = {
        "Create New Ticket",
        "View All Tickets", 
        "View Tickets by Sprint",
        "Update Ticket",
        "Delete Ticket",
        "Back to Main Menu"
    };
    
    int choice = ui_->showMenu(options, "TICKET MANAGEMENT");
    
    switch (choice)
    {
        case 0: createNewTicket(); break;
        case 1: viewTickets(); break;
        case 2: /* view by sprint */ break;
        case 3: /* update ticket */ break;
        case 4: /* delete ticket */ break;
        default: break;
    }
}

void ScrumJiraApp::createNewTicket()
{
    ui_->clearScreen();
    ui_->drawReceiptHeader("CREATE NEW TICKET");
    
    Ticket ticket;
    ticket.id = 0; // Will be set by database
    
    ui_->printAt(5, 6, "Title: ");
    ticket.title = ui_->getInput(50);
    
    ui_->printAt(5, 8, "Description: ");
    ticket.description = ui_->getInput(200);
    
    ui_->printAt(5, 10, "Type (Bug/Feature/Task): ");
    ticket.type = ui_->getInput(20);
    
    ui_->printAt(5, 12, "Priority (Low/Medium/High): ");
    ticket.priority = ui_->getInput(10);
    
    if (db_->createTicket(ticket))
    {
        ui_->printAt(5, 15, "Ticket created successfully! ID: " + std::to_string(ticket.id));
    }
    else
    {
        ui_->printAt(5, 15, "Failed to create ticket!");
    }
    
    ui_->printAt(5, 17, "Press any key to continue...");
    ui_->getKey();
}

void ScrumJiraApp::viewTickets()
{
    auto tickets = db_->getAllTickets();
    
    ui_->clearScreen();
    ui_->drawReceiptHeader("ALL TICKETS");
    
    if (tickets.empty())
    {
        ui_->printAt(10, 6, "No tickets available!");
    }
    else
    {
        for (size_t i = 0; i < tickets.size() && i < 15; ++i)
        {
            std::string line = "#" + std::to_string(tickets[i].id) + " - " + tickets[i].title;
            if (line.length() > 50)
            {
                line = line.substr(0, 47) + "...";
            }
            ui_->printAt(5, 6 + i, line);
        }
    }
    
    ui_->printAt(5, 22, "Press any key to continue...");
    ui_->getKey();
}

// Implement other methods similarly...