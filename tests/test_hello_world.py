import numpy as np

# Assuming Event and InivationCamera are imported from the bindings module
from _event_camera_drivers import Event, InivationCamera as InivationCameraDriver
from event_camera_drivers import EVENTS_DTYPE, InivationCamera

def test_event_repr():
    mock_event = Event(t=123, x=456, y=789, p=True)
    assert mock_event.t == 123
    assert mock_event.x == 456
    assert mock_event.y == 789
    assert mock_event.p == True
    expected_repr = "Event(t=123, x=456, y=789, p=1)"
    assert repr(mock_event) == expected_repr

def test_inivation_camera_driver_dtype():
    # Mock init
    old_init = InivationCameraDriver.__init__
    def new_init(self, mock=False):
        self.cam = InivationCameraDriver(mock=True)
    InivationCamera.__init__ = new_init
    mock_camera = InivationCamera()
    InivationCamera.__init__ = old_init

    # Call a number of times to check gc
    events = None
    for _ in range(10):
        events = next(mock_camera)
        assert len(events) == 3

    assert len(events) == 3
    assert np.array_equal(tuple(events[0]), [1, 2, 3, True])
    assert np.array_equal(tuple(events[1]), [4, 5, 6, False])
    assert np.array_equal(tuple(events[2]), [7, 8, 9, True])