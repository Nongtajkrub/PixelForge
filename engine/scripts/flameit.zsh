#!/usr/bin/env zsh

# Exit immediately if a command exits with a non-zero status
set -e

# Validate arguments
if [[ $# -eq 0 ]]; then
    echo "Usage: $0 <path_to_executable> [args...]"
    echo "Example: $0 ./my_cpp_program --fast"
    exit 1
fi

FLAMEGRAPH_DIR="$HOME/tools/FlameGraph"
TARGET_CMD="$*"

# Check if FlameGraph repository exists
if [[ ! -d "$FLAMEGRAPH_DIR" ]]; then
    echo "❌ Error: FlameGraph directory not found at $FLAMEGRAPH_DIR"
    exit 1
fi

# Define intermediate files
TEMP_STACKS="temp_$$.stacks"
TEMP_FOLDED="temp_$$.folded"
OUTPUT_SVG="flamegraph.svg"

echo "🔥 Profiling '$TARGET_CMD' using DTrace..."
echo "⚠️  Note: You will be prompted for your password because DTrace requires sudo."

# 1. Run DTrace
# -c runs the command
# -n defines the DTrace script: sample at 997Hz, target our specific PID, and record the user stack
sudo dtrace -c "$TARGET_CMD" -o "$TEMP_STACKS" -n 'profile-997 /pid == $target/ { @[ustack()] = count(); }' 2>/dev/null

echo "🔥 Folding stacks and demangling C++ symbols..."
# 2. Collapse the stacks and pipe through c++filt to demangle C++ function names
"$FLAMEGRAPH_DIR/stackcollapse.pl" "$TEMP_STACKS" | c++filt > "$TEMP_FOLDED"

echo "🔥 Generating FlameGraph SVG..."
# 3. Generate the actual flamegraph
"$FLAMEGRAPH_DIR/flamegraph.pl" "$TEMP_FOLDED" > "$OUTPUT_SVG"

echo "🧹 Cleaning up temporary files..."
# 4. Clean up
rm -f "$TEMP_STACKS" "$TEMP_FOLDED"

echo "✅ Done! Your flamegraph has been saved to: $OUTPUT_SVG"
# Optional: Automatically open it in your default browser
open -a "Arc" "$OUTPUT_SVG"
