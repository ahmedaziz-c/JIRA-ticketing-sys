//SprintManager.cpp
#include "UIManager.hpp"
#include "DatabaseManager.hpp"
#include "TicketManager.hpp"
#include "SprintManager.hpp"
#include "models.hpp"
#include "SprintManager.hpp"
#include "DatabaseManager.hpp"
#include <algorithm>
#include <iterator>

SprintManager &SprintManager::getInstance()
{
	static SprintManager instance;
	return instance;
}

bool SprintManager::createSprint(const Sprint &sprint)
{
	Sprint new_sprint = sprint;
	return DatabaseManager::getInstance().createSprint(new_sprint);
}

bool SprintManager::updateSprint(const Sprint &sprint)
{
	return DatabaseManager::getInstance().updateSprint(sprint);
}

std::vector<Sprint> SprintManager::getAllSprints()
{
	return DatabaseManager::getInstance().getAllSprints();
}

Sprint SprintManager::getActiveSprint()
{
	auto sprints = DatabaseManager::getInstance().getAllSprints();

	auto it = std::find_if(sprints.begin(), sprints.end(),
				     [](const Sprint &sprint)
				     {
					     return sprint.status == "active";
				     });

	return it != sprints.end() ? *it : Sprint();
}

Sprint SprintManager::getSprintById(int id)
{
	return DatabaseManager::getInstance().getSprint(id);
}

bool SprintManager::deleteSprint(int id)
{
	return DatabaseManager::getInstance().deleteSprint(id);
}

std::vector<Sprint> SprintManager::getSprintsByStatus(const std::string &status)
{
	auto all_sprints = DatabaseManager::getInstance().getAllSprints();
	std::vector<Sprint> results;

	std::copy_if(all_sprints.begin(), all_sprints.end(),
			 std::back_inserter(results),
			 [&status](const Sprint &sprint)
			 {
				 return sprint.status == status;
			 });

	return results;
}

bool SprintManager::startSprint(int sprint_id)
{
	Sprint sprint = DatabaseManager::getInstance().getSprint(sprint_id);
	if (sprint.id == 0)
		return false; // Sprint not found

	sprint.status = "active";
	return DatabaseManager::getInstance().updateSprint(sprint);
}

bool SprintManager::completeSprint(int sprint_id)
{
	Sprint sprint = DatabaseManager::getInstance().getSprint(sprint_id);
	if (sprint.id == 0)
		return false; // Sprint not found

	sprint.status = "completed";
	return DatabaseManager::getInstance().updateSprint(sprint);
}