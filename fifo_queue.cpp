#include "fifo_queue.h"
#include "constants.h"

FifoQueue::FifoQueue(int size) : max_size(size) {}

bool FifoQueue::enqueue(std::shared_ptr<U2> user) {
    if (buffer.size() >= max_size) return false;
    buffer.push(user);
    return true;
}

std::shared_ptr<U2> FifoQueue::dequeue() {
    if (buffer.empty()) return nullptr;
    auto user = buffer.front();
    buffer.pop();
    return user;
}

bool FifoQueue::is_full() const { return buffer.size() >= max_size; }
bool FifoQueue::is_empty() const { return buffer.empty(); }
int FifoQueue::size() const { return buffer.size(); }