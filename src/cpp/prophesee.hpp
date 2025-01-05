#pragma once

#include <atomic>
#include <optional>

#include <metavision/sdk/base/events/event_cd.h>
#include <metavision/sdk/stream/camera.h>

#include "event.hpp"

struct PropheseeCamera {

  PropheseeCamera(const std::optional<std::string> serial_number,
                  uint32_t buffer_size)
      : bufferSize(buffer_size) {
    Metavision::Camera cam;
    this->bufferSize = buffer_size;

    // get camera
    try {
      if (serial_number.has_value()) {
        this->cam = Metavision::Camera::from_serial(serial_number.value());
      } else {
        this->cam = Metavision::Camera::from_first_available();
      }
    } catch (const std::exception &e) {
      if (serial_number.has_value()) {
        std::cerr << "Failure with serial number '" << serial_number.value()
                  << "': " << e.what() << std::endl;
      } else {
        std::cerr << "Failure to identify Prophesee camera: " << e.what()
                  << std::endl;
      }

      Metavision::AvailableSourcesList available_systems =
          this->cam.list_online_sources();

      for (int i = 0;
           i < available_systems[Metavision::OnlineSourceType::USB].size();
           i++) {
        std::cerr << "- "
                  << available_systems[Metavision::OnlineSourceType::USB][i]
                  << std::endl;
      }

      throw std::invalid_argument("Please choose one of the above listed "
                                  "serial numbers and run again!");
    }
    // add event callback -> will set ev_start and ev_final to respective begin
    // and end of event buffer
    this->cam.cd().add_callback(
        [this](const Metavision::EventCD *ev_begin,
               const Metavision::EventCD *ev_end) -> void {
          this->ev_start = ev_begin;
          this->ev_final = ev_end;
        });

    // start camera
    this->cam.start();
  }

  ~PropheseeCamera() { close(); }

  bool isRunning() { return this->cam.is_running(); }
  std::vector<Event> next() {
    std::vector<Event> event_buffer;
    if ((this->ev_start != NULL) && (this->ev_final != NULL) &&
        this->cam.is_running()) {
      for (const Metavision::EventCD *ev = this->ev_start; ev < this->ev_final;
           ++ev) {
        Event event = {
            (uint64_t)ev->t,
            ev->x,
            ev->y,
            (bool)ev->p,
        };
        event_buffer.push_back(event);

        // return buffer if full
        if (event_buffer.size() >= this->bufferSize) {
          return event_buffer;
        }
      }
      this->ev_start = NULL;
      this->ev_final = NULL;
    }

    // Return empty array if buffer_size not reached
    return event_buffer;
  }

private:
  uint32_t bufferSize;
  Metavision::Camera cam;
  const Metavision::EventCD *ev_start = NULL, *ev_final = NULL;
  inline void close() { this->cam.stop(); }
};

std::string_view get_available_prophesee_cameras() {
  Metavision::AvailableSourcesList available_systems =
      Metavision::Camera::list_online_sources();
  std::stringstream ss;
  for (int i = 0;
       i < available_systems[Metavision::OnlineSourceType::USB].size(); i++) {
    ss << available_systems[Metavision::OnlineSourceType::USB][i] << std::endl;
  }
  std::string result = ss.str();

  if (result.empty()) {
    return "No cameras found";
  }
  return result;
}