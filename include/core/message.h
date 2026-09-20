#ifndef CORE_MESSAGE_H
#define CORE_MESSAGE_H

#include <string>

enum class Role { System, User, Assistant };

class Message {
public:
    Message() {
        role_value = Role::System;
        content_value = "";
    }

    Message(Role newRole, std::string newContent) {
        role_value = newRole;
        content_value = newContent;
    }

    Role role() const noexcept { return role_value; }
    const std::string& content() const noexcept { return content_value; }

private:
    Role role_value;
    std::string content_value;
};

#endif
