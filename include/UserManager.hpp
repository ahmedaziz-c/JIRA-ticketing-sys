//UserMaanger.hpp
#pragma once
#include "models.hpp"
#include <vector>

class UserManager
{
public:
    static UserManager &getInstance();

    bool authenticate(const std::string &username, const std::string &password);
    bool createUser(const User &user);
    bool updateUser(const User &user);
    bool deleteUser(int id);
    std::vector<User> getAllUsers();
    User getUserById(int id);
    User getUserByUsername(const std::string &username);
    bool isAdmin(int user_id);
    bool canModifyTickets(int user_id);

private:
    UserManager() = default;
};