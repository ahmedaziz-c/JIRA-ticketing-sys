cmake_minimum_required(VERSION 3.15)
project(retro-scrum LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Download FTXUI automatically
include(FetchContent)
FetchContent_Declare(
  ftxui
  GIT_REPOSITORY https://github.com/ArthurSonzogni/ftxui
  GIT_TAG v4.1.1
)
FetchContent_MakeAvailable(ftxui)

# Main executable
add_executable(retro-scrum
    src/main.cpp
    src/UIManager.cpp
    src/DatabaseManager.cpp
    src/TicketManager.cpp
    src/SprintManager.cpp
    src/UserManager.cpp
)

# Include directories - CORRECT PATH for your structure
target_include_directories(retro-scrum PRIVATE 
    src 
    Include
    external/nlohmann/json/single_include  # ← This matches your actual path
)

# Link with FTXUI
target_link_libraries(retro-scrum PRIVATE 
    ftxui::component
    ftxui::dom
    ftxui::screen
)

# For Windows
if(WIN32)
    target_compile_definitions(retro-scrum PRIVATE _WIN32_WINNT=0x0A00)
endif()