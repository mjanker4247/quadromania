#!/bin/bash

# Exit on error
set -e

# Get the directory where the script is located
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

# Build directory
BUILD_DIR="$SCRIPT_DIR/build"
# Distribution directory
DIST_DIR="$SCRIPT_DIR/bin"

# Create distribution directory
echo "Creating distribution directory..."
rm -rf "$DIST_DIR"
mkdir -p "$DIST_DIR"

# Copy the binary
echo "Copying binary..."
cp "$BUILD_DIR/quadromania" "$DIST_DIR/"

# Create data directory in distribution folder
echo "Copying game assets..."
mkdir -p "$DIST_DIR/data"

# Copy all PNG files from data directory
echo "Copying PNG files..."
cp "$SCRIPT_DIR/data/"*.png "$DIST_DIR/data/"

# Copy icons directory
echo "Copying icons..."
mkdir -p "$DIST_DIR/data/icons"
cp "$SCRIPT_DIR/data/icons/"*.png "$DIST_DIR/data/icons/"
cp "$SCRIPT_DIR/data/icons/"*.ico "$DIST_DIR/data/icons/"

# Copy sound directory if it exists
if [ -d "$SCRIPT_DIR/data/sound" ]; then
    echo "Copying sound files..."
    mkdir -p "$DIST_DIR/data/sound"
    cp -r "$SCRIPT_DIR/data/sound/"* "$DIST_DIR/data/sound/"
fi

# Copy lowres directory if it exists
if [ -d "$SCRIPT_DIR/data/lowres" ]; then
    echo "Copying lowres files..."
    mkdir -p "$DIST_DIR/data/lowres"
    cp -r "$SCRIPT_DIR/data/lowres/"* "$DIST_DIR/data/lowres/"
fi

# Create a launcher script
echo "Creating launcher script..."
cat > "$DIST_DIR/run_quadromania.sh" << EOL
#!/bin/bash
cd "\$(dirname "\$0")"
./quadromania
EOL
chmod +x "$DIST_DIR/run_quadromania.sh"

# Create a README file
echo "Creating README..."
cat > "$DIST_DIR/README.md" << EOL
# Quadromania

This is the distribution package of Quadromania. To run the game:

1. Make sure you have SDL2 and SDL2_mixer installed
2. Run the game by executing ./run_quadromania.sh in this directory

## System Requirements
- SDL2
- SDL2_mixer
- SDL2_image

## Installation
On macOS with Homebrew:
\`\`\`bash
brew install sdl2 sdl2_mixer sdl2_image
\`\`\`

On Linux:
\`\`\`bash
sudo apt-get install libsdl2-dev libsdl2-mixer-dev libsdl2-image-dev
\`\`\`

## Graphics Resolution
The game is configured to use low-resolution graphics. If you want to use high-resolution graphics,
you can modify the SCREENRES setting in src/sysconfig.h and rebuild the game.
EOL

echo "Distribution package created in $DIST_DIR/" 