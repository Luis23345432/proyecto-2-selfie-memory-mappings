#!/bin/bash
set -u

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
ROOT_DIR="$(cd "$SCRIPT_DIR/../.." && pwd)"
BIN_DIR="$SCRIPT_DIR/bin"
LOG_DIR="$SCRIPT_DIR/logs"
RUNTIME_DIR="$SCRIPT_DIR/runtime"

cd "$ROOT_DIR"

mkdir -p "$BIN_DIR" "$LOG_DIR" "$RUNTIME_DIR"
cp "$SCRIPT_DIR/fixtures/broken.initial.c" "$RUNTIME_DIR/broken.c"

echo "Demo 1 - Editor magico con mmap"
echo "Logs: $LOG_DIR"
echo

echo "Paso 0: compilando herramientas de la demo..."
gcc -Wall -Wextra -g -D_GNU_SOURCE -D'uint64_t=unsigned long' selfie.c -o selfie
./selfie -c demos/01_editor_magico/src/fix_without_msync.c -o "$BIN_DIR/fix_without_msync" > "$LOG_DIR/build_fix_without_msync.log" 2>&1
./selfie -c demos/01_editor_magico/src/fix_with_msync.c -o "$BIN_DIR/fix_with_msync" > "$LOG_DIR/build_fix_with_msync.log" 2>&1

echo
echo "Paso 1: archivo inicial"
sed -n '1,3p' "$RUNTIME_DIR/broken.c"

echo
echo "Paso 2: intento compilar broken.c (debe fallar)"
if ./selfie -c demos/01_editor_magico/runtime/broken.c -o "$BIN_DIR/broken_before" > "$LOG_DIR/compile_before.log" 2>&1; then
  echo "ERROR: compilo cuando debia fallar"
  exit 1
else
  echo "OK: la compilacion fallo como esperabamos"
fi

echo
echo "Paso 3: corrijo con mmap, pero sin msync"
./selfie -l "$BIN_DIR/fix_without_msync" -m 1 > "$LOG_DIR/run_fix_without_msync.log" 2>&1
grep -E "FIX|OPEN|MMAP|WRITE|MSYNC|MUNMAP" "$LOG_DIR/run_fix_without_msync.log"

echo
echo "Paso 4: archivo despues de munmap sin msync"
sed -n '1,3p' "$RUNTIME_DIR/broken.c"

echo
echo "Paso 5: compilo otra vez (debe seguir fallando)"
if ./selfie -c demos/01_editor_magico/runtime/broken.c -o "$BIN_DIR/broken_without_msync" > "$LOG_DIR/compile_without_msync.log" 2>&1; then
  echo "ERROR: compilo sin msync"
  exit 1
else
  echo "OK: sin msync el archivo sigue roto"
fi

echo
echo "Paso 6: corrijo con mmap + msync"
./selfie -l "$BIN_DIR/fix_with_msync" -m 1 > "$LOG_DIR/run_fix_with_msync.log" 2>&1
grep -E "FIX|OPEN|MMAP|WRITE|MSYNC|MUNMAP" "$LOG_DIR/run_fix_with_msync.log"

echo
echo "Paso 7: archivo despues de msync"
sed -n '1,3p' "$RUNTIME_DIR/broken.c"

echo
echo "Paso 8: compilo otra vez (ahora debe funcionar)"
if ./selfie -c demos/01_editor_magico/runtime/broken.c -o "$BIN_DIR/fixed_program" > "$LOG_DIR/compile_after_msync.log" 2>&1; then
  echo "PASS: con msync el archivo fue corregido y Selfie lo compila"
else
  echo "ERROR: el archivo no compilo despues de msync"
  cat "$LOG_DIR/compile_after_msync.log"
  exit 1
fi

echo
echo "Resultado: DEMO 1 PASS"
