#!/usr/bin/env bash
set -euo pipefail

# Usage:
#   ./run.sh --build    # configure and build (Debug)
#   ./run.sh --test     # configure, build, then run ctest

BUILD_DIR="build"
BUILD_TYPE="Debug"
GENERATOR="Ninja"

print_usage() {
  echo "Usage: $0 --build | --cleanbuild | --test"
  exit 1
}

configure() {
  echo "Configuring CMake in '${BUILD_DIR}' (type: ${BUILD_TYPE})"
  mkdir -p "$BUILD_DIR"

  if command -v ninja >/dev/null 2>&1; then
    cmake -S . -B "$BUILD_DIR" -G "$GENERATOR" -DCMAKE_BUILD_TYPE="$BUILD_TYPE" -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
  else
    cmake -S . -B "$BUILD_DIR" -DCMAKE_BUILD_TYPE="$BUILD_TYPE" -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
  fi

  # Symlink compile_commands.json to root for LSP
  if [[ -f "$BUILD_DIR/compile_commands.json" ]]; then
    ln -sf "$BUILD_DIR/compile_commands.json" compile_commands.json
  fi
}

build() {
  echo "Building in '${BUILD_DIR}'"
  cmake --build "$BUILD_DIR" --config "$BUILD_TYPE"
}

cleanbuild() {
  echo "Cleaning and building in '${BUILD_DIR}'"
  cmake --build "$BUILD_DIR" --config "$BUILD_TYPE --clean-first"
}

run_tests() {
  echo "Running tests in '${BUILD_DIR}'"
  ctest --test-dir "$BUILD_DIR" --verbose --output-on-failure
}

main() {
  if [[ $# -ne 1 ]]; then
    print_usage
  fi

  case "$1" in
    --build)
      configure
      build
      ;;
    --test)
      configure
      build
      run_tests
      ;;
    --cleanbuild)
      configure
      cleanbuild
      ;;
    *)
      print_usage
      ;;
  esac
}

main "$@"
