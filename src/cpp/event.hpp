#pragma once

// #include "warning.hpp"
#include <chrono>
#include <cstdint>
#include <mutex>
#include <optional>
#include <queue>
#include <thread>
#include <vector>

// Defines an event type according to the Numpy dtype
// EVENTS_DTYPE: numpy.dtype = numpy.dtype(
//     [("t", "<u8"), ("x", "<u2"), ("y", "<u2"), (("p", "on"), "?")]
// )
struct Event {
  const uint64_t t;
  const uint16_t x;
  const uint16_t y;
  const bool p;
  
  Event(uint64_t t_, uint16_t x_, uint16_t y_, bool p_) noexcept
      : t(t_), x(x_), y(y_), p(p_) {}
};

// struct SharedEventQueue {
//   std::queue<std::vector<Event>> queue;
//   std::mutex mutex;
//   size_t max_size;

//   SharedEventQueue(size_t max_size = 1024) : max_size(max_size) {}

//   void push(std::vector<Event> event) {
//     if (queue.size() >= max_size) {
//       WARN("Event queue is full");
//       return;
//     }
//     std::lock_guard<std::mutex> lock(mutex);
//     queue.push(event);
//   }

//   std::optional<std::vector<Event>>
//   pop(std::chrono::milliseconds timeout = std::chrono::milliseconds(200)) {
//     auto start = std::chrono::steady_clock::now();
//     while (queue.empty()) {
//       if (std::chrono::steady_clock::now() - start > timeout) {
//         return std::nullopt;
//       }
//       std::this_thread::sleep_for(std::chrono::milliseconds(1));
//     }
//     {
//       std::lock_guard<std::mutex> lock(mutex);
//       if (queue.empty()) { // Check again after acquiring lock
//         return std::nullopt;
//       }
//       std::vector<Event> event = queue.front();
//       queue.pop();
//       return event;
//     }
//   }
// };
