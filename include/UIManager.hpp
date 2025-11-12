// UIManager.hpp - Enhanced version
#pragma once
#include "ftxui/component/component.hpp"
#include "ftxui/component/screen_interactive.hpp"
#include "ftxui/dom/elements.hpp"
#include "models.hpp"
#include "DatabaseManager.hpp"
#include "TicketManager.hpp"
#include "SprintManager.hpp"
#include <memory>
#include <string>
#include <vector>
#include <chrono>

class UIManager {
public:
    UIManager();
    void run();

private:
    ftxui::Component main_container_;
    ftxui::Component input_handler_;
    ftxui::ScreenInteractive screen_;

    enum class FocusPane { SPRINTS, TICKETS, ACTIVITY };
    FocusPane current_focus_ = FocusPane::SPRINTS;

    int selected_sprint_ = 0;
    int selected_ticket_ = 0;
    std::vector<Sprint> sprints_;
    std::vector<Ticket> tickets_;
    std::vector<Activity> activities_;
    std::vector<User> users_;

    bool show_menu_ = false;
    bool quit_ = false;
    
    // Form states
    bool show_ticket_form_ = false;
    bool show_sprint_form_ = false;
    bool show_project_form_ = false;
    bool is_editing_ = false;

    // Current working objects
    Ticket current_ticket_;
    Sprint current_sprint_;
    std::string new_project_name_;
    
    // Form input buffers
    std::string story_points_str_;
    std::string assignee_id_str_;
    std::string sprint_id_str_;
    
    // Form components
    ftxui::Component form_container_;
    ftxui::Component title_input_;
    ftxui::Component desc_input_;
    ftxui::Component points_input_;
    ftxui::Component assignee_input_;
    ftxui::Component sprint_input_;

    // Retro color scheme - ATM style
    struct RetroColors {
        static constexpr ftxui::Color RECEIPT_GREEN = ftxui::Color::RGB(0, 180, 0);
        static constexpr ftxui::Color RECEIPT_AMBER = ftxui::Color::RGB(255, 176, 0);
        static constexpr ftxui::Color RECEIPT_WHITE = ftxui::Color::RGB(240, 240, 240);
        static constexpr ftxui::Color RECEIPT_BG = ftxui::Color::RGB(20, 20, 20);
    };

    // Initialization
    void initializeComponents();
    void loadData();
    
    // Input handling
    bool handleGlobalInput(ftxui::Event event);
    void toggleFocus();
    void handleCreateCommand();
    void handleEditCommand();
    void handleDeleteCommand();
    
    // Project management
    void showProjectMenu();
    void handleProjectSwitch();
    void handleProjectCreate();
    
    // Form management
    void showTicketForm(bool editing = false);
    void showSprintForm(bool editing = false);
    void showProjectForm();
    void closeForms();
    void submitTicketForm();
    void submitSprintForm();
    void submitProjectForm();
    
    // Component builders
    ftxui::Component makeTicketForm();
    ftxui::Component makeSprintForm();
    ftxui::Component makeProjectForm();
    
    // Rendering methods with ATM receipt style
    ftxui::Element renderMainLayout();
    ftxui::Element renderStatusBar();
    ftxui::Element renderSprintPane();
    ftxui::Element renderTicketPane();
    ftxui::Element renderActivityPane();
    ftxui::Element renderMainMenu();
    ftxui::Element renderFormOverlay();
    
    // ATM-style receipt rendering helpers
    ftxui::Element renderReceiptLine(const std::string& left, const std::string& right = "");
    ftxui::Element renderReceiptHeader(const std::string& title);
    ftxui::Element renderReceiptSeparator();
    
    // Utility methods
    std::string formatTime(time_t timestamp);
    std::string getFocusIndicator(FocusPane pane);
    void refreshData();
};