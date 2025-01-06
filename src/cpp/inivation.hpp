#pragma once

#include <atomic>
#include <csignal>
#include <iostream>
#include <libcaer/devices/device.h>
#include <libcaer/devices/device_discover.h>
#include <libcaercpp/devices/davis.hpp>
#include <libcaercpp/devices/dvs128.hpp>
#include <libcaercpp/devices/dvxplorer.hpp>
#include <libcaercpp/devices/dynapse.hpp>
#include <libcaercpp/devices/samsung_evk.hpp>
#include <optional>

#include "event.hpp"

struct InivationDeviceAddress {
  const std::string camera;
  const std::uint16_t deviceId;
  const std::uint16_t deviceAddress;
};

inline std::optional<std::tuple<libcaer::devices::device *, size_t, size_t>>
find_device() {
  caerDeviceDiscoveryResult discovered;
  ssize_t result = caerDeviceDiscover(CAER_DEVICE_DISCOVER_ALL, &discovered);

  if (result <= 0) {
    return {};
  } else {
    const auto device = discovered[0];
    switch (device.deviceType) {
    case CAER_DEVICE_DAVIS: {
      libcaer::devices::davis *davis_handle = new libcaer::devices::davis(1);
      auto davis_info = davis_handle->infoGet();
      return std::make_tuple(davis_handle, davis_info.dvsSizeX,
                             davis_info.dvsSizeY);
    }

    case CAER_DEVICE_DVS128: {
      libcaer::devices::dvs128 *dvs128_handle = new libcaer::devices::dvs128(1);
      auto dvs128_info = dvs128_handle->infoGet();
      return std::make_tuple(dvs128_handle, dvs128_info.dvsSizeX,
                             dvs128_info.dvsSizeY);
    }

    case CAER_DEVICE_DVXPLORER: {
      libcaer::devices::dvXplorer *dvxplorer_handle =
          new libcaer::devices::dvXplorer(1);
      auto dvxplorer_info = dvxplorer_handle->infoGet();
      return std::make_tuple(dvxplorer_handle, dvxplorer_info.dvsSizeX,
                             dvxplorer_info.dvsSizeY);
    }

      // case CAER_DEVICE_SAMSUNG_EVK:
      // case CAER_DEVICE_DAVIS_FX2:
      // case CAER_DEVICE_DAVIS_FX3:
      // case CAER_DEVICE_DAVIS_RPI:
      // case CAER_DEVICE_DYNAPSE:

    default:
      throw std::runtime_error("Cannot connect to unknown device of type " +
                               std::to_string(device.deviceType));
    };
  }
}

static std::atomic<bool> inivation_shutdown_flag{false};

static void inivation_signal_handler(int signal) {
  inivation_shutdown_flag.store(true);
}

static void shutdownHandler(void *ptr) { inivation_shutdown_flag.store(true); }

class InivationCamera {
  size_t buffer_size;
  libcaer::devices::device *handle;
  std::unique_ptr<libcaer::events::EventPacketContainer> packet_container;

  uint32_t container_interval = 128;
  size_t packet_index = 0;
  size_t packet_event_index = 0;
  size_t event_buffer_index = 0;
  std::mutex event_buffer_mutex;

  size_t resolution_x;
  size_t resolution_y;

  inline void
  fill_event_buffer(std::vector<Event> &event_buffer,
                    std::unique_ptr<libcaer::events::EventPacketContainer>
                        &packet_container) {
    for (; this->packet_index < this->packet_container->size();
         this->packet_index++) {
      auto packet = this->packet_container->getEventPacket(this->packet_index);
      if (packet == nullptr) {
        continue;
      }
      if (packet->getEventType() == POLARITY_EVENT) {
        std::shared_ptr<const libcaer::events::PolarityEventPacket> polarity =
            std::static_pointer_cast<libcaer::events::PolarityEventPacket>(
                packet);
        for (size_t packet_event_index = 0;
             packet_event_index < polarity->size(); packet_event_index++) {

          const auto evt = polarity->getEvent(packet_event_index);
          if (!evt.isValid()) {
            continue;
          }

          // Add event to buffer
          event_buffer.push_back(
              Event(static_cast<uint64_t>(evt.getTimestamp64(*polarity)),
                    static_cast<uint16_t>(evt.getX()),
                    static_cast<uint16_t>(evt.getY()),
                    static_cast<bool>(evt.getPolarity())));

          this->event_buffer_index++;

          if (event_buffer_index >= this->buffer_size) {
            return;
          }
        }
        this->packet_event_index = 0;
      }
    }
    this->packet_container = nullptr;
  }

  inline void close() {
    // Ensure that we're not actively reading events
    std::lock_guard<std::mutex> lock(event_buffer_mutex);
    try {
      if (handle != nullptr) {
        handle->dataStop();
        delete handle;
        handle = nullptr;
      }
    } catch (std::runtime_error &e) {
      std::cerr << "Error stopping data stream: " << e.what() << std::endl;
    }
  }

public:
  inline InivationCamera(size_t buffer_size = 1024)
      : buffer_size(buffer_size) {
    std::optional<std::tuple<libcaer::devices::device *, size_t, size_t>>
        found_device = find_device();
    if (found_device.has_value()) {
      handle = std::get<0>(found_device.value());
      resolution_x = std::get<1>(found_device.value());
      resolution_y = std::get<2>(found_device.value());
      std::cout << "InivationCamera: Found device" << std::endl;
    } else {
      throw std::invalid_argument("No inivation device found.");
    }

    // Send the default configuration before using the device.
    // No configuration is sent automatically!
    handle->sendDefaultConfig();

    // Set parsing intervall where container interval is in [10μs] unit
    // davisHandle.configSet(CAER_HOST_CONFIG_PACKETS,
    // CAER_HOST_CONFIG_PACKETS_MAX_CONTAINER_INTERVAL, container_interval);

    // Set number of events per packet
    handle->configSet(CAER_HOST_CONFIG_PACKETS,
                      CAER_HOST_CONFIG_PACKETS_MAX_CONTAINER_PACKET_SIZE,
                      container_interval);

    // Configs about buffer
    handle->configSet(CAER_HOST_CONFIG_DATAEXCHANGE,
                      CAER_HOST_CONFIG_DATAEXCHANGE_BUFFER_SIZE,
                      this->buffer_size);

    // Start data stream
    handle->dataStart(nullptr, nullptr, nullptr, &shutdownHandler, nullptr);

    // Let's turn on blocking data-get mode to avoid wasting resources.
    handle->configSet(CAER_HOST_CONFIG_DATAEXCHANGE,
                      CAER_HOST_CONFIG_DATAEXCHANGE_BLOCKING, true);
  }

  ~InivationCamera() {
    inivation_shutdown_flag.store(true);
    close();
  }

  std::vector<Event> next() {
    // Reserve space for buffer_size events
    std::vector<Event> event_buffer;
    event_buffer.reserve(this->buffer_size);
    this->event_buffer_index = 0;

    do {
      if (inivation_shutdown_flag.load()) {
        return event_buffer;
      }
      {
        std::lock_guard<std::mutex> lock(event_buffer_mutex);
        if (this->packet_container == nullptr) {
          this->packet_container = this->handle->dataGet();
          if (this->packet_container == nullptr) {
            continue;
          }
          this->packet_index = 0;
        }
        fill_event_buffer(event_buffer, this->packet_container);
      }
    } while (event_buffer_index < this->buffer_size);
    event_buffer_index = 0;
    return event_buffer;
  }

  size_t get_buffer_size() { return this->buffer_size; }

  bool is_running() { return !inivation_shutdown_flag.load(); }

  std::tuple<size_t, size_t> get_resolution() {
    return {resolution_x, resolution_y};
  }
};
