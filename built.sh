#!/bin/bash

# Navigate into the build directory
cd out

# Clear the screen (optional, but good for a clean output)
clear

# Run Ninja to build the project.
# Since CMake has already been run and the build files are generated,
# we just need to call 'ninja' here.
echo "Building project with Ninja..."
ninja

# Check if the build was successful before trying to run the executable.
# The `?` variable holds the exit status of the previous command. 0 means success.
if [ $? -eq 0 ]; then
    echo "Build successful! Running the executable..."
    # Execute the program.
    # The name of the executable is likely `LeadAndWater.exe` on Windows,
    # but on Linux, it would be just `LeadAndWater`.
    ./Warfront
else
    echo "Build failed! Please check the output above for errors."
fi

# Navigate back to the root directory
cd ..

