#!/bin/bash
set -e

FQBN="esp32:esp32:esp32s3"
FLASH_SIZE="16M"
PSRAM="opi"
FLASH_MODE="qio"
CDC_ON_BOOT="cdc"

SKETCH_NAME=$(basename "$PWD")
SKETCH_PATH="."

BUILD_PATH="./.build_cache/output"

BUILD_PROPERTIES="FlashSize=${FLASH_SIZE},PSRAM=${PSRAM},FlashMode=${FLASH_MODE},CDCOnBoot=${CDC_ON_BOOT}"
BOARD_OPTIONS="FlashSize=${FLASH_SIZE},PSRAM=${PSRAM},FlashMode=${FLASH_MODE},CDCOnBoot=${CDC_ON_BOOT}"

show_usage() {
    echo "Usage: $0 [-compile] [-upload]"
    exit 1
}

check_dependencies() {
    if ! arduino-cli lib list | grep -q "LovyanGFX"; then
        echo "Installing LovyanGFX dependency..."
        arduino-cli lib install "LovyanGFX"
    fi
    
    LOCAL_LIB_DIR="$HOME/Arduino/libraries"
    mkdir -p "$LOCAL_LIB_DIR"
    if [ ! -d "$LOCAL_LIB_DIR/ESP-Arduino-Lua" ]; then
        echo "Cloning ESP-Arduino-Lua engine directly from source repository..."
        git clone https://github.com/sfranzyshen/ESP-Arduino-Lua.git "$LOCAL_LIB_DIR/ESP-Arduino-Lua"
    fi
}

prepare_assets() {
    if [ -d "assets" ]; then
        rm -rf data
        cp -r assets data
    fi
}

do_compile() {
    check_dependencies
    prepare_assets
    mkdir -p "$BUILD_PATH"
    arduino-cli compile \
      --verbose \
      --fqbn "$FQBN" \
      --build-property "$BUILD_PROPERTIES" \
      --build-path "$BUILD_PATH" \
      "$SKETCH_PATH"
}

do_upload() {
    PORT=$(arduino-cli board list | grep "esp32s3" | awk '{print $1}' | head -n 1)
    if [ -z "$PORT" ]; then
        PORT=$(arduino-cli board list | grep -E "(/dev/ttyACM|/dev/ttyUSB|COM)" | awk '{print $1}' | head -n 1)
    fi
    if [ -z "$PORT" ]; then
        exit 1
    fi
    
    arduino-cli upload \
      --verbose \
      -p "$PORT" \
      --fqbn "$FQBN" \
      --board-options "$BOARD_OPTIONS" \
      --build-path "$BUILD_PATH" \
      "$SKETCH_PATH"

    FS_BIN=$(find "$BUILD_PATH" -name "*.mbtiles" -o -name "*.littlefs.bin" -o -name "*.spiffs.bin" | head -n 1)
    if [ -n "$FS_BIN" ]; then
        python3 -m esptool --chip esp32s3 --port "$PORT" --baud 921600 write_flash 0x290000 "$FS_BIN"
    fi
}

if [ $# -eq 0 ]; then
    show_usage
fi

while [ $# -gt 0 ]; do
    case "$1" in
        -compile)
            do_compile
            shift
            ;;
        -upload)
            if [ ! -d "$BUILD_PATH" ]; then
                do_compile
            fi
            do_upload
            shift
            ;;
        *)
            show_usage
            ;;
    esac
done
