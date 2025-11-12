//SprintManager.hpp
#pragma once
#include "models.hpp"
#include <vector>

class SprintManager
{
public:
    static SprintManager &getInstance();

    bool createSprint(const Sprint &sprint);
    bool updateSprint(const Sprint &sprint);
    bool deleteSprint(int id);
    std::vector<Sprint> getAllSprints();
    Sprint getActiveSprint();
    Sprint getSprintById(int id);
    std::vector<Sprint> getSprintsByStatus(const std::string &status);
    bool startSprint(int sprint_id);
    bool completeSprint(int sprint_id);

private:
    SprintManager() = default;
};