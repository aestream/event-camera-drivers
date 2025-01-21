from importlib.metadata import version, PackageNotFoundError

import numpy as np
import logging

from _event_camera_drivers import (
    InivationCamera as InivationCameraDriver,
    Event,
)


try:
    __version__ = version("event_camera_drivers")
except PackageNotFoundError:
    logging.error("event_camera_drivers is not installed properly")

EVENTS_DTYPE: np.dtype = np.dtype(
    [("t", "<u8"), ("x", "<u2"), ("y", "<u2"), (("p", "on"), "?")]
)

class InivationCamera:
    """
    Opens an Inivation event cameras.
    """

    def __init__(self, buffer_size=1024):
        """
        Searches for the first available Inivation camera and opens it.

        Args:
            buffer_size (int): Size of the event buffer. Defaults to 1024.
        """
        self.buffer_size = buffer_size
        self.cam = InivationCameraDriver(buffer_size)

    def __iter__(self):
        """
        Makes the camera object iterable.

        Returns:
            self: Returns the iterator object.
        """
        return self

    def __next__(self):
        """
        Returns the next batch of events from the camera.

        Returns:
            numpy.ndarray: Array of events from the camera.

        Raises:
            StopIteration: When the camera is no longer running.
        """
        if self.cam.is_running():
            return np.frombuffer(self.cam.next(), dtype=EVENTS_DTYPE)
        else:
            raise StopIteration

    def is_running(self):
        """
        Checks if the camera is currently streaming events.

        Returns:
            bool: True if the camera is actively streaming events, False otherwise.
        """
        return self.cam.is_running()

    def resolution(self):
        """
        Gets the resolution of the camera sensor.

        Returns:
            tuple: A tuple of (width, height) representing the camera resolution.
        """
        return self.cam.resolution()


__all__ = ["InivationCamera", "Event"]
