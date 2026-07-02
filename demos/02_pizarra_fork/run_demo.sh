#!/bin/bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
ROOT_DIR="$(cd "$SCRIPT_DIR/../.." && pwd)"
BIN_DIR="$SCRIPT_DIR/bin"
LOG_DIR="$SCRIPT_DIR/logs"
RUNTIME_DIR="$SCRIPT_DIR/runtime"

cd "$ROOT_DIR"

mkdir -p "$BIN_DIR" "$LOG_DIR" "$RUNTIME_DIR"
cp "$SCRIPT_DIR/fixtures/shared-board.initial" "$RUNTIME_DIR/shared-board.txt"

echo "Demo 2 - Dos procesos, una misma pizarra"
echo "Logs: $LOG_DIR"
echo

echo "Paso 0: compilando herramientas de la demo..."
gcc -Wall -Wextra -g -D_GNU_SOURCE -D'uint64_t=unsigned long' selfie.c -o selfie
./selfie -c demos/02_pizarra_fork/src/fork_shared_board.c -o "$BIN_DIR/fork_shared_board" > "$LOG_DIR/build.log" 2>&1

echo
echo "Paso 1: archivo inicial"
sed -n '1,2p' "$RUNTIME_DIR/shared-board.txt"

echo
echo "Paso 2: ejecucion padre/hijo"
./selfie -l "$BIN_DIR/fork_shared_board" -m 1 > "$LOG_DIR/run.log" 2>&1
grep -E "DEMO2|MMAP|PADRE|HIJO|MSYNC|MUNMAP" "$LOG_DIR/run.log"

echo
echo "Paso 3: archivo despues de msync"
sed -n '1,2p' "$RUNTIME_DIR/shared-board.txt"

if grep -q "DEMO2: PASS" "$LOG_DIR/run.log"; then
  echo
  echo "Resultado: DEMO 2 PASS"
else
  echo "ERROR: la demo 2 no imprimio PASS"
  exit 1
fi
