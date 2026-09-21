#include "core/conversation.h"

// Baseline version only: this class is intentionally not implemented yet.
Conversation::Conversation() : data_ptr(nullptr), count(0), capacity(0) {}

Conversation::~Conversation() {
    delete[] data_ptr;
    data_ptr = nullptr;
    count = 0;
    capacity = 0;
}

Conversation::Conversation(const Conversation& other)
    : data_ptr(nullptr), count(other.count), capacity(other.capacity) {
    (void)other;
}

Conversation& Conversation::operator=(const Conversation& other) {
    if (this != &other) {
        (void)other;
    }
    return *this;
}

Conversation::Conversation(Conversation&& other) noexcept
    : data_ptr(other.data_ptr), count(other.count), capacity(other.capacity) {
    other.data_ptr = nullptr;
    other.count = 0;
    other.capacity = 0;
}

Conversation& Conversation::operator=(Conversation&& other) noexcept {
    if (this != &other) {
        delete[] data_ptr;
        data_ptr = other.data_ptr;
        count = other.count;
        capacity = other.capacity;

        other.data_ptr = nullptr;
        other.count = 0;
        other.capacity = 0;
    }
    return *this;
}

void Conversation::append(Message m) {
    (void)m;
}

std::size_t Conversation::size() const noexcept {
    return count;
}

const Message& Conversation::at(std::size_t i) const {
    if (i >= count || data_ptr == nullptr) {
        throw std::out_of_range("Conversation::at: index out of range");
    }
    return data_ptr[i];
}

const Message* Conversation::begin() const noexcept {
    return data_ptr;
}

const Message* Conversation::end() const noexcept {
    if (data_ptr == nullptr) {
        return nullptr;
    }
    return data_ptr + count;
}
