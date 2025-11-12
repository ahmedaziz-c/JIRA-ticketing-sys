#pragma once
#include "DatabaseManager.hpp"
#include "RetroTUI.hpp"
#include <memory>

class ScrumJiraApp {
public:
    ScrumJiraApp();
    ~ScrumJiraApp();
    
    void run();
    
private:
    void initialize();
    void mainMenu();
    void projectManagement();
    void ticketManagement();
    void sprintManagement();
    void userManagement();
    
    void createNewProject();
    void loadExistingProject();
    void createNewTicket();
    void viewTickets();
    void createSprint();
    void viewSprints();
    
    void showTicketDetails(const Ticket& ticket);
    void showSprintDetails(const Sprint& sprint);
    
    std::unique_ptr<DatabaseManager> db_;
    std::unique_ptr<RetroTUI> ui_;
    bool running_;
};