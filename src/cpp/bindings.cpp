#include "inivation.hpp"
// #include "prophesee.hpp"
#include <nanobind/nanobind.h>
#include <nanobind/ndarray.h>
#include <nanobind/stl/optional.h>
#include <nanobind/stl/string.h>
#include <nanobind/stl/vector.h>
#include <nanobind/stl/tuple.h>

namespace nb = nanobind;

// std::string_view get_available_cameras() {
//   return get_available_prophesee_cameras();
// }

NB_MODULE(_event_camera_drivers, m) {
  // m.def("available_cameras", &get_available_cameras);

  nb::class_<Event>(m, "Event")
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
      .def(nb::init<size_t>(), nb::arg("buffer_size") = 1024)
      .def("next", [](InivationCamera &self) {
        try {
          return self.next();
        } catch (nb::python_error &e) {
          std::cerr << "Error getting events: " << e.what() << std::endl;
          e.discard_as_unraisable(__func__);
        } catch (const std::exception &e) {
          std::cerr << "Error getting events: " << e.what() << std::endl;
        }
        return std::vector<Event>();
        // nb::capsule owner(events, [](void *p) noexcept { delete (Event *)p; });
        // return nb::ndarray<nb::numpy, uint64_t, nb::ndim<1>>(
            // events, {self.get_buffer_size()}, owner, {sizeof(Event)});
        // return nb::ndarray<Event, nb::numpy,
        // nb::shape<event_size>>(events.data()).cast();
      })
      .def("resolution", &InivationCamera::get_resolution)
      .def("is_running", &InivationCamera::is_running);

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