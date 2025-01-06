# C++/Python event camera drivers

C++ drivers wrapped in a Python library for interfacing with event-based cameras, supporting both Prophesee and iniVation devices.

## Features

- Support for Prophesee event-based cameras via OpenEB
- Support for iniVation DVS cameras via libcaer
- Python bindings using nanobind
- Cross-platform support (Linux, macOS, Windows)

## Installation

### From PyPI

... Coming soon

### From Source

#### Using Nix (recommended)

If you have Nix installed with flakes enabled:

```bash
# Enter development environment
nix develop

# Install in development mode
pip install -e .
```

#### Manual Installation

Prerequisites:
- CMake (3.16 or higher)
- C++ compiler with C++17 support
- Python 3.9 or higher
- OpenEB 5.0.0
- libcaer

1. Clone the repository:
```bash
git clone https://github.com/aestream/event-camera-drivers
cd event-camera-drivers
```

2. Create and activate a virtual environment:
```bash
python -m venv .venv
source .venv/bin/activate  # On Windows: .venv\Scripts\activate
```

3. Install development dependencies:
```bash
pip install scikit-build-core pytest build nanobind
```

4. Install in development mode:
```bash
pip install -e .
```

## Usage

```python
import event_camera_drivers as evd

camera = evd.InivationCamera()
for packet in camera:
    print(packet)
```

Or, in conjuction with [Faery](https://github.com/aestream/faery) (read more in the [Faery documentation](https://aestream.github.io/faery/)):

```python
import faery

faery.events_stream_from_inivation_camera()
  ...
```

## Development

### Setup Development Environment

1. Install Nix package manager (recommended):
```bash
curl -L https://nixos.org/nix/install | sh
```

2. Enable Nix flakes (if not already enabled).

3. Enter development environment:
```bash
nix develop
```

### Building Wheels

```bash
pip install build
python -m build
```

## Contributing

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/amazing-feature`)
3. Commit your changes (`git commit -m 'Add amazing feature'`)
4. Push to the branch (`git push origin feature/amazing-feature`)
5. Open a Pull Request

## License

This project is licensed under LGPLv3 - see the [LICENSE](LICENSE) file for details.

## Acknowledgments

- [OpenEB](https://github.com/prophesee-ai/openeb) - Prophesee's Open Event-Based Vision SDK
- [libcaer](https://gitlab.com/inivation/dv/libcaer) - Minimal C library to access neuromorphic sensors
```