#include "inivation.hpp"
// #include "prophesee.hpp"
#include <nanobind/nanobind.h>
#include <nanobind/ndarray.h>
#include <nanobind/stl/optional.h>
#include <nanobind/stl/string.h>
#include <nanobind/stl/vector.h>
#include <nanobind/stl/tuple.h>

namespace nb = nanobind;
using namespace nb::literals;

// std::string_view get_available_cameras() {
//   return get_available_prophesee_cameras();
// }

#include <iomanip>

NB_MODULE(_event_camera_drivers, m) {
  // m.def("available_cameras", &get_available_cameras);

  nb::class_<Event>(m, "Event")
      .def(nb::init<uint64_t, uint16_t, uint16_t, bool>(),
           nb::arg("t"), nb::arg("x"), nb::arg("y"), nb::arg("p"))
      .def_ro("t", &Event::t)
      .def_ro("x", &Event::x)
      .def_ro("y", &Event::y)
      .def_ro("p", &Event::p)
      .def("__repr__", [](const Event &self) {
        return "Event(t=" + std::to_string(self.t) +
               ", x=" + std::to_string(self.x) +
               ", y=" + std::to_string(self.y) +
               ", p=" + std::to_string(self.p) + ")";
      });

  nb::class_<InivationCamera>(m, "InivationCamera")
      .def(nb::init<size_t, bool>(), nb::arg("buffer_size") = 1024, nb::arg("mock") = false)
      .def("next", [](InivationCamera &self) {
        auto events = self.next();
        // We move the events to a new vector to ensure that the memory is
        // managed by the capsule
        auto* events_ptr = new std::vector<Event>(std::move(events));
        nb::capsule owner(events_ptr, [](void *p) noexcept {
            delete static_cast<std::vector<Event> *>(p);
        });

        auto bytes_ptr = reinterpret_cast<uint8_t *>(events_ptr->data());
        
        return nb::ndarray<nb::numpy, uint8_t, nb::ndim<1>>(
            /* data    */ bytes_ptr,
            /* shape   */ {events_ptr->size() * sizeof(Event)},
            /* capsule */ owner,
            /* stride  */ {1}
        );
      })
      .def("resolution", &InivationCamera::get_resolution)
      .def("is_running", &InivationCamera::is_running)
      .def("close", &InivationCamera::close);

  // nb::class_<PropheseeCamera>(m, "PropheseeCamera")
  //     .def(nb::init<std::optional<std::string>,uint32_t>(),
  //     nb::arg("serial_number") = nb::none(), nb::arg("buffer_size") = 1024)
  //     .def("next", [](PropheseeCamera& self) {
  //       std::vector<Event> events = self.next();
  //       std::vector<Event>* events_ptr = &events;
  //       nb::capsule owner(events_ptr,
  //           [](void* p) { delete (std::vector<Event>*) p; });
  //       return nb::ndarray<nb::numpy, Event, nb::ndim<1>>(events.data(),
  //       {events.size()}, owner);
  //     });

  // Register signal handler
  signal(SIGINT, inivation_signal_handler);
}