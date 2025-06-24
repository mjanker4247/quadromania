# Quadromania Build Guide

This guide explains how to build Quadromania with all its dependencies.

## Prerequisites

Before building, ensure you have the following tools installed:

- **GCC** (GNU Compiler Collection) or **Clang**
- **Make** (GNU Make)
- **CMake** (version 3.16 or higher)
- **curl** (for downloading dependencies)
- **pkg-config** (for dependency detection)

### Installing Prerequisites

#### Ubuntu/Debian

```bash
sudo apt-get update
sudo apt-get install build-essential cmake curl pkg-config
```

#### CentOS/RHEL

```bash
sudo yum groupinstall "Development Tools"
sudo yum install cmake curl pkg-config
```

#### macOS

```bash
# Install Xcode Command Line Tools
xcode-select --install

# Install Homebrew (if not already installed)
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"

# Install required packages
brew install cmake curl pkg-config
```

## Building Quadromania

### Option 1: Build with Local Dependencies (Recommended)

This option downloads and builds all dependencies locally in the project directory.

1. **Install Dependencies:**

   ```bash
   ./install_dependencies.sh
   ```

   This script will:
   - Download SDL2, SDL2_image, SDL2_mixer, SDL2_ttf, and zlog
   - Build all dependencies with static linking
   - Install them in `deps/install/`
   - Create pkg-config files for dependency detection
   - Generate a `build.sh` script for building the project

2. **Build the Project:**

   ```bash
   ./build.sh
   ```

   Or manually:

   ```bash
   mkdir build
   cd build
   cmake ..
   make
   ```

### Option 2: Build with System Dependencies

If you prefer to use system-installed dependencies:

1. **Install System Dependencies:**

   **Ubuntu/Debian:**

   ```bash
   sudo apt-get install libsdl2-dev libsdl2-image-dev libsdl2-mixer-dev libsdl2-ttf-dev libzlog-dev
   ```

   **CentOS/RHEL:**

   ```bash
   sudo yum install SDL2-devel SDL2_image-devel SDL2_mixer-devel SDL2_ttf-devel zlog-devel
   ```

   **macOS:**

   ```bash
   brew install sdl2 sdl2_image sdl2_mixer sdl2_ttf zlog
   ```

2. **Build the Project:**

```bash
   mkdir build
   cd build
   cmake ..
   make
```

## Project Structure

After building with local dependencies, your project structure will look like:

```
quadromania-master/
├── deps/
│   ├── build/          # Build artifacts
│   ├── download/       # Downloaded source archives
│   └── install/        # Installed dependencies
├── src/                # Source code
├── data/               # Game assets
├── build/              # Build output
├── install_dependencies.sh  # Dependency installer
├── build.sh            # Build script (generated)
└── CMakeLists.txt      # CMake configuration
```

## Configuration

Quadromania supports various configuration options through a config file or command-line arguments.

### Config File

The application automatically creates `quadromania.cfg` in the executable directory with default settings:

```ini
# Display settings
fullscreen=false

# Debug settings
debug=false
log_file=quadromania.log
log_level=info
log_max_size=1024
log_max_files=1
log_to_stderr=false
log_overwrite=true
```

### Command Line Options

```bash
./quadromania [options]

Options:
  --config <file>          Load parameters from config file
  -f, --fullscreen         Run in fullscreen mode
  -d, --debug              Enable debug output (to stderr)
      --log-file <file>    Log debug output to file
      --log-level <level>  Set log level (error, warn, info, debug, trace)
      --log-max-size <n>   Max log file size in bytes before rotation
      --log-max-files <n>  Number of rotated log files to keep
      --no-stderr          Do not log to stderr
      --log-overwrite      Overwrite log file at startup
  -h, --help               Show this help message
```

## Logging

Quadromania uses the zlog library for professional logging capabilities:

- **Log Levels**: ERROR, WARN, INFO, DEBUG, TRACE
- **Output Destinations**: File, stderr, or both
- **Log Rotation**: Automatic rotation based on file size
- **Timestamps**: Automatic timestamp generation
- **Thread Safety**: Thread-safe logging operations

### Log File Location

- **Config File**: `quadromania.cfg` (in executable directory)
- **Log Files**: `quadromania.log` (in executable directory)
- **Highscore File**: `quadromania.scores` (in executable directory)

## Troubleshooting

### Common Issues

1. **CMake can't find dependencies:**
   - Ensure you've run `./install_dependencies.sh` for local dependencies
   - Or install system dependencies as described above

2. **Build fails with zlog errors:**
   - The project will build without zlog if it's not available
   - Logging will fall back to simple file/stderr output

3. **SDL2 not found:**
   - Check that SDL2 development packages are installed
   - For local builds, ensure the dependency installer completed successfully

4. **Permission denied errors:**
   - Make sure the installer script is executable: `chmod +x install_dependencies.sh`
   - Run with appropriate permissions for your system

### Getting Help

If you encounter issues:

1. Check the build output for specific error messages
2. Ensure all prerequisites are installed
3. Try building with local dependencies first
4. Check that you have sufficient disk space for dependencies (~500MB)

## Development

For developers working on Quadromania:

- The project uses CMake for build configuration
- Dependencies are managed through pkg-config
- Logging can be configured at runtime
- All game files are stored in the executable directory for portability
