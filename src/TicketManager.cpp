//TicketManager.cpp
#include "UIManager.hpp"
#include "DatabaseManager.hpp"
#include "TicketManager.hpp"
#include "SprintManager.hpp"
#include "models.hpp"
#include <algorithm>
#include <iterator>

TicketManager &TicketManager::getInstance()
{
	static TicketManager instance;
	return instance;
}

bool TicketManager::createTicket(const Ticket &ticket)
{
	Ticket new_ticket = ticket;
	return DatabaseManager::getInstance().createTicket(new_ticket);
}

bool TicketManager::updateTicket(const Ticket &ticket)
{
	return DatabaseManager::getInstance().updateTicket(ticket);
}

bool TicketManager::deleteTicket(int id)
{
	return DatabaseManager::getInstance().deleteTicket(id);
}

std::vector<Ticket> TicketManager::getTicketsBySprint(int sprint_id)
{
	return DatabaseManager::getInstance().getTicketsBySprint(sprint_id);
}

std::vector<Ticket> TicketManager::searchTickets(const std::string &query)
{
	auto all_tickets = DatabaseManager::getInstance().getAllTickets();
	std::vector<Ticket> results;

	std::copy_if(all_tickets.begin(), all_tickets.end(),
			 std::back_inserter(results),
			 [&query](const Ticket &ticket)
			 {
				 return ticket.title.find(query) != std::string::npos ||
					  ticket.description.find(query) != std::string::npos;
			 });

	return results;
}

std::vector<Ticket> TicketManager::getTicketsByStatus(const std::string &status)
{
	auto all_tickets = DatabaseManager::getInstance().getAllTickets();
	std::vector<Ticket> results;

	std::copy_if(all_tickets.begin(), all_tickets.end(),
			 std::back_inserter(results),
			 [&status](const Ticket &ticket)
			 {
				 return ticket.status == status;
			 });

	return results;
}

std::vector<Ticket> TicketManager::getTicketsByAssignee(int assignee_id)
{
	auto all_tickets = DatabaseManager::getInstance().getAllTickets();
	std::vector<Ticket> results;

	std::copy_if(all_tickets.begin(), all_tickets.end(),
			 std::back_inserter(results),
			 [assignee_id](const Ticket &ticket)
			 {
				 return ticket.assignee_id == assignee_id;
			 });

	return results;
}