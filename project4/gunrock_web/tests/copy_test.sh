#!/bin/bash

# Check if both arguments are provided
if [ $# -ne 2 ]; then
    echo "Usage: $0 source_number destination_number"
    echo "Example: $0 1 6"
    exit 1
fi

source=$1
dest=$2

# Loop through all files starting with source number
for file in $source.*; do
    # Extract the extension
    ext="${file#$source.}"
    
    # Create the new filename
    newfile="$dest.$ext"
    
    # Copy the file
    cp "$file" "$newfile"
    echo "Copied $file to $newfile"
done
