//TicketManager.hpp
#pragma once
#include "models.hpp"
#include <vector>

class TicketManager
{
public:
	static TicketManager &getInstance();

	bool createTicket(const Ticket &ticket);
	bool updateTicket(const Ticket &ticket);
	bool deleteTicket(int id);
	std::vector<Ticket> getTicketsBySprint(int sprint_id);
	std::vector<Ticket> searchTickets(const std::string &query);
	std::vector<Ticket> getTicketsByStatus(const std::string &status);
	std::vector<Ticket> getTicketsByAssignee(int assignee_id);

private:
	TicketManager() = default;
};