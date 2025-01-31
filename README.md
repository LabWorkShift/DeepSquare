# DeepSquare

A UCI-compatible chess engine written in modern C++.

## Features

- UCI protocol support
- Advanced search techniques
  - Alpha-beta pruning
  - Iterative deepening
  - Move ordering
  - Transposition tables
- NNUE evaluation
- Multi-threading support
- Time management
- Configurable hash size
- Skill level adjustment

## Building

### Prerequisites

- C++17 compatible compiler
- CMake 3.15 or higher
- Git

### Build Instructions

```bash
git clone https://github.com/LabWorkShift/DeepSquare.git
cd DeepSquare
mkdir build && cd build
cmake ..
cmake --build . --config Release
```

## Supported Platforms

- Windows
- Linux
- macOS

## Usage

The engine supports the UCI (Universal Chess Interface) protocol. You can use it with any UCI-compatible chess GUI like:

- Arena
- Cutechess
- Fritz
- Chessbase

### UCI Options

- **Hash**: Hash table size in MB (default: 128)
- **Threads**: Number of threads to use (default: 1)
- **MultiPV**: Number of principal variations to search (default: 1)
- **Skill Level**: Engine playing strength (0-20, default: 20)
- **Ponder**: Think on opponent's time (default: false)

## Contributing

Contributions are welcome! Please feel free to submit a Pull Request.

### Development Guidelines

1. Fork the repository
2. Create your feature branch (`git checkout -b feature/AmazingFeature`)
3. Commit your changes (`git commit -m 'Add some AmazingFeature'`)
4. Push to the branch (`git push origin feature/AmazingFeature`)
5. Open a Pull Request

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## Acknowledgments

- Thanks to the Stockfish team for inspiration
- NNUE implementation based on research by Yu Nasu
- All contributors and testers

## Contact

Project Link: [https://github.com/LabWorkShift/DeepSquare](https://github.com/LabWorkShift/DeepSquare)
