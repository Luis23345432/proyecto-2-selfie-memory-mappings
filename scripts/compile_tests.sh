#!/bin/bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
ROOT_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"
BIN_DIR="$ROOT_DIR/artifacts/tests/bin"

cd "$ROOT_DIR"

mkdir -p "$BIN_DIR"

echo "Cleaning old mmap test binaries in artifacts/tests/bin..."
rm -f "$BIN_DIR"/test1 "$BIN_DIR"/test2 "$BIN_DIR"/test3 "$BIN_DIR"/test4 "$BIN_DIR"/test5 "$BIN_DIR"/test6

echo "Compiling selfie..."
gcc -Wall -Wextra -g -D_GNU_SOURCE -D'uint64_t=unsigned long' selfie.c -o selfie

echo

for i in 1 2 3 4 5 6; do
  case "$i" in
    1) src="tests/mmap/test-mmap-1-read.c" ;;
    2) src="tests/mmap/test-mmap-2-write-sync.c" ;;
    3) src="tests/mmap/test-mmap-3-fork-shared.c" ;;
    4) src="tests/mmap/test-mmap-4-no-sync-no-persist.c" ;;
    5) src="tests/mmap/test-mmap-5-two-fds-shared.c" ;;
    6) src="tests/mmap/test-mmap-6-lru-reuse.c" ;;
    *) echo "Test desconocido: $i"; exit 1 ;;
  esac

  echo "Compilando test${i}..."
  ./selfie -c "$src" -o "$BIN_DIR/test${i}"
done

echo "Todos los tests compilaron exitosamente en artifacts/tests/bin"
