#pragma once
#include "ftxui/component/component.hpp"
#include "ftxui/component/screen_interactive.hpp"
#include "ftxui/dom/elements.hpp"
#include <string>

struct LoginScreen {
    std::string username;
    std::string password;
    bool cancelled = false;

    bool Show() {
        using namespace ftxui;
        auto username_in = Input(&username, "Username");
        auto password_in = Input(&password, "Password");

        auto ok = Button("Login", [this] { screen.Exit(); });
        auto cancel = Button("Cancel", [this] { cancelled = true; screen.Exit(); });

        auto component = Container::Vertical({
            username_in,
            password_in,
            Container::Horizontal({ ok, cancel })
        });

        auto renderer = Renderer(component, [&] {
            return vbox({
                text(" RETRO-TICKET LOGIN ") | bold | center,
                separator(),
                vbox({
                    hbox(text("Username: "), username_in->Render()),
                    hbox(text("Password: "), password_in->Render()),
                    separator(),
                    hbox(ok->Render(), text("  "), cancel->Render())
                }) | border
            }) | center;
        });

        screen.Loop(renderer);
        return !cancelled;
    }

private:
    ftxui::ScreenInteractive screen = ftxui::ScreenInteractive::TerminalOutput();
};