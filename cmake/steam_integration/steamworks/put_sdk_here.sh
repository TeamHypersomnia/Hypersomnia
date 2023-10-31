#!/bin/bash

# Directory where this script is located
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

# Check if a zip file path is provided as an argument
if [ $# -eq 1 ]; then
    ZIP_FILE="$1"
else
    echo "Usage: $0 <path_to_sdk_zip_file>"
    exit 1
fi

# Check if the provided zip file exists
if [ ! -f "$ZIP_FILE" ]; then
    echo "Error: The provided zip file does not exist."
    exit 1
fi

# Unpack the provided zip file to the script directory
echo "Unpacking Steamworks SDK..."
unzip -q -o "$ZIP_FILE" -d "$SCRIPT_DIR"

# Check if the unpacking was successful
if [ $? -ne 0 ]; then
    echo "Failed to unpack the SDK."
    exit 1
fi

echo "Steamworks SDK unpacked in $SCRIPT_DIR."
