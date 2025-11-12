﻿//UIManager.cpp

#include "UIManager.hpp"
#include "DatabaseManager.hpp"
#include "TicketManager.hpp"
#include "SprintManager.hpp"
#include "models.hpp"
#include <iomanip>
#include <sstream>
#include <thread>
#include <iostream>
#include <ctime>
#include <algorithm>
#include <cstdio>

using namespace ftxui;

/* ========================== Constructor & Lifecycle ========================== */
UIManager::UIManager()
    : screen_(ScreenInteractive::Fullscreen()) {
    initializeComponents();
    loadData();
    
    // Initialize default ticket
    current_ticket_ = Ticket();
    current_ticket_.status = "todo";
    current_ticket_.priority = "medium";
    current_ticket_.type = "task";
    
    // Initialize default sprint
    current_sprint_ = Sprint();
    current_sprint_.status = "planned";
}

void UIManager::run() {
    screen_.Loop(main_container_);
}

/* ========================== Component Setup ========================== */
void UIManager::initializeComponents() {
    form_container_ = Container::Vertical({});
    
    input_handler_ = CatchEvent(form_container_, [this](Event event) {
        return handleGlobalInput(event);
    });
    
    main_container_ = Renderer(input_handler_, [this] {
        return renderMainLayout();
    });
}

void UIManager::loadData() {
    DatabaseManager& db = DatabaseManager::getInstance();
    sprints_    = db.getAllSprints();
    tickets_    = db.getAllTickets();
    activities_ = db.getRecentActivities(10);
    users_      = db.getAllUsers();
}

void UIManager::refreshData() { 
    loadData(); 
}

/* ========================== Input Handling ========================== */
bool UIManager::handleGlobalInput(Event event) {
    // Handle form-specific input first
    if (show_ticket_form_ || show_sprint_form_ || show_project_form_) {
        if (event == Event::Escape) {
            closeForms();
            return true;
        }
        if (event == Event::F2) { // Save
            if (show_ticket_form_) submitTicketForm();
            else if (show_sprint_form_) submitSprintForm();
            else if (show_project_form_) submitProjectForm();
            return true;
        }
        return false; // Let form handle other input
    }
    
    // Global shortcuts
    if (event == Event::Character('q') || event == Event::Character('Q')) {
        screen_.Exit();
        quit_ = true;
        return true;
    }
    
    if (event == Event::F1) {
        show_menu_ = !show_menu_;
        return true;
    }
    
    if (event == Event::F2) {
        handleCreateCommand();
        return true;
    }
    
    if (event == Event::F3) {
        handleEditCommand();
        return true;
    }
    
    if (event == Event::F4) {
        handleDeleteCommand();
        return true;
    }
    
    if (event == Event::F5) {
        refreshData();
        return true;
    }
    
    if (event == Event::F6) {
        handleProjectSwitch();
        return true;
    }
    
    if (event == Event::F7) {
        handleProjectCreate();
        return true;
    }
    
    if (event == Event::Tab) {
        toggleFocus();
        return true;
    }
    
    // Navigation in panes
    if (event == Event::ArrowUp) {
        if (current_focus_ == FocusPane::SPRINTS && selected_sprint_ > 0) {
            selected_sprint_--;
            return true;
        }
        if (current_focus_ == FocusPane::TICKETS && selected_ticket_ > 0) {
            selected_ticket_--;
            return true;
        }
    }
    
    if (event == Event::ArrowDown) {
        if (current_focus_ == FocusPane::SPRINTS && selected_sprint_ < int(sprints_.size()) - 1) {
            selected_sprint_++;
            return true;
        }
        if (current_focus_ == FocusPane::TICKETS && selected_ticket_ < int(tickets_.size()) - 1) {
            selected_ticket_++;
            return true;
        }
    }
    
    return false;
}

void UIManager::toggleFocus() {
    current_focus_ = static_cast<FocusPane>((int(current_focus_) + 1) % 3);
}

void UIManager::handleCreateCommand() {
    if (current_focus_ == FocusPane::TICKETS)   showTicketForm(false);
    else if (current_focus_ == FocusPane::SPRINTS) showSprintForm(false);
    else if (current_focus_ == FocusPane::ACTIVITY) showProjectForm();
}

void UIManager::handleEditCommand() {
    if (current_focus_ == FocusPane::TICKETS && selected_ticket_ >= 0 && selected_ticket_ < int(tickets_.size())) {
        showTicketForm(true);
    }
    else if (current_focus_ == FocusPane::SPRINTS && selected_sprint_ >= 0 && selected_sprint_ < int(sprints_.size())) {
        showSprintForm(true);
    }
}

void UIManager::handleDeleteCommand() {
    if (current_focus_ == FocusPane::TICKETS && selected_ticket_ >= 0 && selected_ticket_ < int(tickets_.size())) {
        TicketManager::getInstance().deleteTicket(tickets_[selected_ticket_].id);
        refreshData();
    }
    else if (current_focus_ == FocusPane::SPRINTS && selected_sprint_ >= 0 && selected_sprint_ < int(sprints_.size())) {
        SprintManager::getInstance().deleteSprint(sprints_[selected_sprint_].id);
        refreshData();
    }
}

/* ========================== Project Management ========================== */
void UIManager::handleProjectSwitch() {
    auto projects = DatabaseManager::getInstance().getAvailableProjects();
    if (projects.empty()) {
        return;
    }
    
    // Simple project switching - in real implementation, use a proper dialog
    if (!projects.empty()) {
        DatabaseManager::getInstance().switchProject(projects[0]);
        refreshData();
    }
}

void UIManager::handleProjectCreate() {
    showProjectForm();
}

/* ========================== Form Management ========================== */
ftxui::Component UIManager::makeTicketForm() {
    title_input_  = Input(&current_ticket_.title, "Title");
    desc_input_   = Input(&current_ticket_.description, "Description");
    points_input_ = Input(&story_points_str_, "Story Points");
    assignee_input_ = Input(&assignee_id_str_, "Assignee ID");
    sprint_input_ = Input(&sprint_id_str_, "Sprint ID");

    auto save_btn   = Button("Save", [this] {
        try {
            if (!story_points_str_.empty()) 
                current_ticket_.story_points = std::stoi(story_points_str_);
            if (!assignee_id_str_.empty()) 
                current_ticket_.assignee_id = std::stoi(assignee_id_str_);
            if (!sprint_id_str_.empty()) 
                current_ticket_.sprint_id = std::stoi(sprint_id_str_);
        } catch (...) {
            // Handle conversion errors
        }
        submitTicketForm();
    });
    
    auto cancel_btn = Button("Cancel", [this] { closeForms(); });

    return Container::Vertical({
        title_input_,
        desc_input_,
        points_input_,
        assignee_input_,
        sprint_input_,
        Container::Horizontal({ save_btn, cancel_btn })
    });
}

ftxui::Component UIManager::makeSprintForm() {
    auto name_input = Input(&current_sprint_.name, "Sprint Name");
    auto goal_input = Input(&current_sprint_.goal, "Goal");

    auto save_btn   = Button("Save", [this] { submitSprintForm(); });
    auto cancel_btn = Button("Cancel", [this] { closeForms(); });

    return Container::Vertical({
        name_input,
        goal_input,
        Container::Horizontal({ save_btn, cancel_btn })
    });
}

ftxui::Component UIManager::makeProjectForm() {
    auto name_input = Input(&new_project_name_, "New Project Name");

    auto create_btn = Button("Create", [this] { submitProjectForm(); });
    auto cancel_btn = Button("Cancel", [this] { closeForms(); });

    return Container::Vertical({
        name_input,
        Container::Horizontal({ create_btn, cancel_btn })
    });
}

void UIManager::showTicketForm(bool editing) {
    is_editing_ = editing;
    if (editing && selected_ticket_ >= 0 && selected_ticket_ < int(tickets_.size())) {
        current_ticket_ = tickets_[selected_ticket_];
    } else {
        current_ticket_ = Ticket();
        current_ticket_.status = "todo";
        current_ticket_.priority = "medium";
        current_ticket_.type = "task";
    }

    story_points_str_ = std::to_string(current_ticket_.story_points);
    assignee_id_str_  = std::to_string(current_ticket_.assignee_id);
    sprint_id_str_    = std::to_string(current_ticket_.sprint_id);

    form_container_   = makeTicketForm();
    show_ticket_form_ = true;
    show_sprint_form_ = false;
    show_project_form_ = false;
}

void UIManager::showSprintForm(bool editing) {
    is_editing_ = editing;
    if (editing && selected_sprint_ >= 0 && selected_sprint_ < int(sprints_.size())) {
        current_sprint_ = sprints_[selected_sprint_];
    } else {
        current_sprint_ = Sprint();
        current_sprint_.status = "planned";
    }
    form_container_   = makeSprintForm();
    show_sprint_form_ = true;
    show_ticket_form_ = false;
    show_project_form_ = false;
}

void UIManager::showProjectForm() {
    new_project_name_.clear();
    form_container_    = makeProjectForm();
    show_project_form_ = true;
    show_ticket_form_  = false;
    show_sprint_form_  = false;
}

void UIManager::closeForms() {
    show_ticket_form_  = false;
    show_sprint_form_  = false;
    show_project_form_ = false;
    is_editing_        = false;
}

void UIManager::submitTicketForm() {
    if (current_ticket_.title.empty()) return;
    
    if (is_editing_) {
        TicketManager::getInstance().updateTicket(current_ticket_);
    } else {
        TicketManager::getInstance().createTicket(current_ticket_);
    }
    closeForms();
    refreshData();
}

void UIManager::submitSprintForm() {
    if (current_sprint_.name.empty()) return;
    
    if (is_editing_) {
        SprintManager::getInstance().updateSprint(current_sprint_);
    } else {
        SprintManager::getInstance().createSprint(current_sprint_);
    }
    closeForms();
    refreshData();
}

void UIManager::submitProjectForm() {
    if (!new_project_name_.empty()) {
        DatabaseManager::getInstance().createNewProject(new_project_name_);
        closeForms();
        refreshData();
    }
}

/* ========================== Retro Styling Helpers ========================== */
Element UIManager::renderReceiptHeader(const std::string& title) {
    return vbox({
        text("┌────────────────────────────────────────────────────┐") | color(RetroColors::RECEIPT_GREEN),
        hbox({
            text("│ ") | color(RetroColors::RECEIPT_GREEN),
            text(title) | bold | color(RetroColors::RECEIPT_WHITE) | center,
            text(" │") | color(RetroColors::RECEIPT_GREEN)
        }),
        text("└────────────────────────────────────────────────────┘") | color(RetroColors::RECEIPT_GREEN)
    });
}

Element UIManager::renderReceiptLine(const std::string& left, const std::string& right) {
    std::string line = "│ " + left;
    if (!right.empty()) {
        int padding = 46 - left.length() - right.length();
        if (padding > 0) {
            line += std::string(padding, ' ');
        }
        line += right;
    }
    line += " │";
    return text(line) | color(RetroColors::RECEIPT_WHITE);
}

Element UIManager::renderReceiptSeparator() {
    return text("├────────────────────────────────────────────────────┤") | color(RetroColors::RECEIPT_GREEN);
}

/* ========================== Main Rendering ========================== */
Element UIManager::renderMainLayout() {
    auto main_screen = vbox({
        renderStatusBar(),
        separator() | color(RetroColors::RECEIPT_GREEN),
        hbox({
            renderSprintPane() | flex,
            separator() | color(RetroColors::RECEIPT_GREEN),
            renderTicketPane() | flex,
        }) | flex,
        separator() | color(RetroColors::RECEIPT_GREEN),
        renderActivityPane() | size(HEIGHT, EQUAL, 6),
    }) | bgcolor(RetroColors::RECEIPT_BG);
    
    // Overlay menus
    if (show_menu_) {
        return dbox({
            main_screen,
            renderMainMenu() | center | clear_under | bgcolor(RetroColors::RECEIPT_BG) | border | color(RetroColors::RECEIPT_AMBER)
        });
    }
    
    if (show_ticket_form_ || show_sprint_form_ || show_project_form_) {
        return dbox({
            main_screen,
            renderFormOverlay() | center | clear_under | bgcolor(RetroColors::RECEIPT_BG) | border | color(RetroColors::RECEIPT_AMBER)
        });
    }
    
    return main_screen;
}

Element UIManager::renderStatusBar() {
    auto left = text(" RETRO-SCRUM v1.0 ") | bold | color(RetroColors::RECEIPT_GREEN);
    auto center = text("[FOCUS: " + getFocusIndicator(current_focus_) + "]") | color(RetroColors::RECEIPT_AMBER);
    auto right = text("F1=Help F2=Create F3=Edit F4=Del F5=Refresh F6=SwitchProj F7=NewProj Q=Quit") | color(RetroColors::RECEIPT_GREEN);
    
    return hbox({ 
        left, 
        center | flex, 
        right 
    }) | bgcolor(RetroColors::RECEIPT_BG) | size(HEIGHT, EQUAL, 1);
}

Element UIManager::renderSprintPane() {
    if (sprints_.empty()) {
        return vbox({
            renderReceiptHeader("SPRINTS"),
            text("│ No sprints available") | color(RetroColors::RECEIPT_WHITE),
            text("└────────────────────────────────────────────────────┘") | color(RetroColors::RECEIPT_GREEN)
        });
    }
    
    std::vector<Element> rows;
    rows.push_back(renderReceiptHeader("SPRINTS"));
    
    for (size_t i = 0; i < sprints_.size(); ++i) {
        const Sprint& s = sprints_[i];
        std::string status_indicator = "○";
        if (s.status == "active") status_indicator = "▶";
        if (s.status == "completed") status_indicator = "✓";
        
        auto line = renderReceiptLine(
            status_indicator + " " + s.name,
            "ID:" + std::to_string(s.id)
        );
        
        if (static_cast<int>(i) == selected_sprint_ && current_focus_ == FocusPane::SPRINTS) {
            line = line | inverted | bgcolor(RetroColors::RECEIPT_AMBER) | color(RetroColors::RECEIPT_BG);
        }
        
        rows.push_back(line);
        
        // Add goal if not empty and this sprint is selected
        if (!s.goal.empty() && static_cast<int>(i) == selected_sprint_) {
            std::string goal = s.goal;
            if (goal.length() > 35) {
                goal = goal.substr(0, 32) + "...";
            }
            rows.push_back(renderReceiptLine("  Goal: " + goal, ""));
        }
        
        // Add date info for selected sprint
        if (static_cast<int>(i) == selected_sprint_) {
            char start_date[11], end_date[11];
            std::strftime(start_date, sizeof(start_date), "%m/%d/%Y", std::localtime(&s.start_date));
            std::strftime(end_date, sizeof(end_date), "%m/%d/%Y", std::localtime(&s.end_date));
            rows.push_back(renderReceiptLine("  Dates: " + std::string(start_date) + " - " + std::string(end_date), ""));
        }
    }
    
    rows.push_back(text("└────────────────────────────────────────────────────┘") | color(RetroColors::RECEIPT_GREEN));
    
    return vbox(std::move(rows)) | yframe;
}

Element UIManager::renderTicketPane() {
    if (tickets_.empty()) {
        return vbox({
            renderReceiptHeader("TICKETS"),
            text("│ No tickets available") | color(RetroColors::RECEIPT_WHITE),
            text("└────────────────────────────────────────────────────┘") | color(RetroColors::RECEIPT_GREEN)
        });
    }
    
    std::vector<Element> rows;
    rows.push_back(renderReceiptHeader("TICKETS"));
    
    for (size_t i = 0; i < tickets_.size() && i < 15; ++i) {
        const Ticket& t = tickets_[i];
        
        std::string title = t.title;
        if (title.length() > 25) {
            title = title.substr(0, 22) + "...";
        }
        
        std::string status_symbol = "○";
        if (t.status == "in_progress") status_symbol = "▶";
        if (t.status == "review") status_symbol = "◐";
        if (t.status == "done") status_symbol = "✓";
        
        // Priority indicator
        std::string priority_indicator = "";
        if (t.priority == "high") priority_indicator = "!";
        if (t.priority == "critical") priority_indicator = "!!";
        
        auto line = renderReceiptLine(
            status_symbol + " " + priority_indicator + " #" + std::to_string(t.id) + " " + title,
            t.status
        );
        
        if (static_cast<int>(i) == selected_ticket_ && current_focus_ == FocusPane::TICKETS) {
            line = line | inverted | bgcolor(RetroColors::RECEIPT_AMBER) | color(RetroColors::RECEIPT_BG);
        }
        
        rows.push_back(line);
        
        // Show additional info for selected ticket
        if (static_cast<int>(i) == selected_ticket_) {
            if (t.story_points > 0) {
                rows.push_back(renderReceiptLine("  Points: " + std::to_string(t.story_points) + " SP", ""));
            }
            if (t.assignee_id > 0) {
                rows.push_back(renderReceiptLine("  Assignee: ID " + std::to_string(t.assignee_id), ""));
            }
        }
    }
    
    rows.push_back(text("└────────────────────────────────────────────────────┘") | color(RetroColors::RECEIPT_GREEN));
    
    return vbox(std::move(rows)) | yframe;
}

Element UIManager::renderActivityPane() {
    if (activities_.empty()) {
        return vbox({
            renderReceiptHeader("RECENT ACTIVITY"),
            text("│ No recent activity") | color(RetroColors::RECEIPT_WHITE),
            text("└────────────────────────────────────────────────────┘") | color(RetroColors::RECEIPT_GREEN)
        });
    }
    
    std::vector<Element> rows;
    rows.push_back(renderReceiptHeader("RECENT ACTIVITY"));
    
    for (size_t i = 0; i < activities_.size() && i < 5; ++i) {
        const Activity& a = activities_[i];
        
        std::string desc = a.description;
        if (desc.length() > 40) {
            desc = desc.substr(0, 37) + "...";
        }
        
        rows.push_back(renderReceiptLine(
            formatTime(a.timestamp),
            desc
        ));
    }
    
    rows.push_back(text("└────────────────────────────────────────────────────┘") | color(RetroColors::RECEIPT_GREEN));
    
    return vbox(std::move(rows));
}

Element UIManager::renderMainMenu() {
    auto content = vbox({
        text(" RETRO-SCRUM HELP ") | bold | center | color(RetroColors::RECEIPT_GREEN),
        separator() | color(RetroColors::RECEIPT_AMBER),
        text(" NAVIGATION:") | color(RetroColors::RECEIPT_AMBER) | bold,
        text("   Tab        Switch focus between panes") | color(RetroColors::RECEIPT_WHITE),
        text("   ↑↓         Navigate items in focused pane") | color(RetroColors::RECEIPT_WHITE),
        text("   F1         Toggle this help menu") | color(RetroColors::RECEIPT_WHITE),
        separator() | color(RetroColors::RECEIPT_AMBER),
        text(" ACTIONS:") | color(RetroColors::RECEIPT_AMBER) | bold,
        text("   F2         Create new item in focused pane") | color(RetroColors::RECEIPT_WHITE),
        text("   F3         Edit selected item") | color(RetroColors::RECEIPT_WHITE),
        text("   F4         Delete selected item") | color(RetroColors::RECEIPT_WHITE),
        text("   F5         Refresh data") | color(RetroColors::RECEIPT_WHITE),
        separator() | color(RetroColors::RECEIPT_AMBER),
        text(" PROJECTS:") | color(RetroColors::RECEIPT_AMBER) | bold,
        text("   F6         Switch project") | color(RetroColors::RECEIPT_WHITE),
        text("   F7         Create new project") | color(RetroColors::RECEIPT_WHITE),
        separator() | color(RetroColors::RECEIPT_AMBER),
        text("   ESC        Close dialogs/cancel") | color(RetroColors::RECEIPT_WHITE),
        text("   Q          Quit application") | color(RetroColors::RECEIPT_WHITE)
    });
    
    return window(text(""), content) | size(WIDTH, EQUAL, 60) | center | color(RetroColors::RECEIPT_AMBER);
}

Element UIManager::renderFormOverlay() {
    if (show_ticket_form_)  return renderTicketForm();
    if (show_sprint_form_)  return renderSprintForm();
    if (show_project_form_) return renderProjectForm();
    return text("");
}

Element UIManager::renderTicketForm() {
    auto form_renderer = Renderer(form_container_, [this] {
        return vbox({
            text(" TICKET FORM ") | bold | center | color(RetroColors::RECEIPT_GREEN),
            separator() | color(RetroColors::RECEIPT_AMBER),
            hbox({ text("Title:       "), title_input_->Render() }) | color(RetroColors::RECEIPT_WHITE),
            hbox({ text("Description: "), desc_input_->Render() }) | color(RetroColors::RECEIPT_WHITE),
            hbox({ text("Story Points:"), points_input_->Render() }) | color(RetroColors::RECEIPT_WHITE),
            hbox({ text("Assignee ID: "), assignee_input_->Render() }) | color(RetroColors::RECEIPT_WHITE),
            hbox({ text("Sprint ID:   "), sprint_input_->Render() }) | color(RetroColors::RECEIPT_WHITE),
            separator() | color(RetroColors::RECEIPT_AMBER),
            hbox({
                text("F2=Save  ESC=Cancel") | color(RetroColors::RECEIPT_AMBER)
            }) | center
        }) | border | size(WIDTH, EQUAL, 70) | bgcolor(RetroColors::RECEIPT_BG);
    });
    
    return form_renderer;
}

Element UIManager::renderSprintForm() {
    auto form_renderer = Renderer(form_container_, [this] {
        return vbox({
            text(" SPRINT FORM ") | bold | center | color(RetroColors::RECEIPT_GREEN),
            separator() | color(RetroColors::RECEIPT_AMBER),
            hbox({ text("Name: "), form_container_->ChildAt(0)->Render() }) | color(RetroColors::RECEIPT_WHITE),
            hbox({ text("Goal: "), form_container_->ChildAt(1)->Render() }) | color(RetroColors::RECEIPT_WHITE),
            separator() | color(RetroColors::RECEIPT_AMBER),
            form_container_->ChildAt(2)->Render() | center,
            separator() | color(RetroColors::RECEIPT_AMBER),
            hbox({
                text("F2=Save  ESC=Cancel") | color(RetroColors::RECEIPT_AMBER)
            }) | center
        }) | border | size(WIDTH, EQUAL, 60) | bgcolor(RetroColors::RECEIPT_BG);
    });
    
    return form_renderer;
}

Element UIManager::renderProjectForm() {
    auto form_renderer = Renderer(form_container_, [this] {
        return vbox({
            text(" NEW PROJECT ") | bold | center | color(RetroColors::RECEIPT_GREEN),
            separator() | color(RetroColors::RECEIPT_AMBER),
            hbox({ text("Project Name: "), form_container_->ChildAt(0)->Render() }) | color(RetroColors::RECEIPT_WHITE),
            separator() | color(RetroColors::RECEIPT_AMBER),
            form_container_->ChildAt(1)->Render() | center,
            separator() | color(RetroColors::RECEIPT_AMBER),
            hbox({
                text("F2=Create  ESC=Cancel") | color(RetroColors::RECEIPT_AMBER)
            }) | center
        }) | border | size(WIDTH, EQUAL, 50) | bgcolor(RetroColors::RECEIPT_BG);
    });
    
    return form_renderer;
}

/* ========================== Utility Methods ========================== */
std::string UIManager::formatTime(time_t timestamp) {
    char buf[16];
    std::strftime(buf, sizeof(buf), "%m/%d %H:%M", std::localtime(&timestamp));
    return std::string(buf);
}

std::string UIManager::getFocusIndicator(FocusPane pane) {
    switch (pane) {
    case FocusPane::SPRINTS:  return "SPRINTS";
    case FocusPane::TICKETS:  return "TICKETS";
    case FocusPane::ACTIVITY: return "ACTIVITY";
    default: return "UNKNOWN";
    }
}