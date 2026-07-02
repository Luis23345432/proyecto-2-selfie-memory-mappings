#!/bin/bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
ROOT_DIR="$(cd "$SCRIPT_DIR/../.." && pwd)"
BIN_DIR="$SCRIPT_DIR/bin"
LOG_DIR="$SCRIPT_DIR/logs"
RUNTIME_DIR="$SCRIPT_DIR/runtime"

cd "$ROOT_DIR"

mkdir -p "$BIN_DIR" "$LOG_DIR" "$RUNTIME_DIR"
cp "$SCRIPT_DIR/fixtures/live-note.initial" "$RUNTIME_DIR/live-note.txt"

echo "Demo 3 - Mini sistema de archivos viviente"
echo "Logs: $LOG_DIR"
echo

echo "Paso 0: compilando herramientas de la demo..."
gcc -Wall -Wextra -g -D_GNU_SOURCE -D'uint64_t=unsigned long' selfie.c -o selfie
./selfie -c demos/03_sistema_archivos_viviente/src/live_note.c -o "$BIN_DIR/live_note" > "$LOG_DIR/build.log" 2>&1

echo
echo "Paso 1: archivo inicial"
sed -n '1,2p' "$RUNTIME_DIR/live-note.txt"

echo
echo "Paso 2: ejecucion de mappings compartidos"
./selfie -l "$BIN_DIR/live_note" -m 1 > "$LOG_DIR/run.log" 2>&1
grep -E "DEMO3|MAP1|MAP2|HIJO|PADRE|MSYNC" "$LOG_DIR/run.log"

echo
echo "Paso 3: archivo despues de msync"
sed -n '1,2p' "$RUNTIME_DIR/live-note.txt"

if grep -q "DEMO3: PASS" "$LOG_DIR/run.log"; then
  echo
  echo "Resultado: DEMO 3 PASS"
else
  echo "ERROR: la demo 3 no imprimio PASS"
  exit 1
fi
