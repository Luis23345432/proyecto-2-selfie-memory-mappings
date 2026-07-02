#!/bin/bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
ROOT_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"
BIN_DIR="$ROOT_DIR/artifacts/tests/bin"
LOG_DIR="$ROOT_DIR/artifacts/tests/logs"
FIXTURE_SRC="$ROOT_DIR/tests/mmap/fixtures/mmap-fixture.initial"
RUNTIME_DIR="$ROOT_DIR/artifacts/tests/runtime"
RUNTIME_FIXTURE="$RUNTIME_DIR/mmap-fixture.txt"

cd "$ROOT_DIR"

mkdir -p "$LOG_DIR" "$RUNTIME_DIR"

"$SCRIPT_DIR/compile_tests.sh"

echo "========== EJECUTANDO TEST SUITE =========="
echo

failed=0

for i in 1 2 3 4 5 6; do
  test_binary="$BIN_DIR/test${i}"
  log="$LOG_DIR/test${i}.log"
  cmd=("./selfie" -l "$test_binary" -m 1)

  cp "$FIXTURE_SRC" "$RUNTIME_FIXTURE"
  echo "TEST $i: ${cmd[*]}"
  if "${cmd[@]}" > "$log" 2>&1; then
    if grep -q "exit code 0" "$log"; then
      echo "TEST $i: PASS"
    else
      echo "TEST $i: FAIL"
      echo "No se encontro exit code 0 en $log"
      cat "$log"
      failed=1
    fi
  else
    echo "TEST $i: FAIL"
    cat "$log"
    failed=1
  fi
done

echo
echo "========== RESUMEN =========="
if [ "$failed" -eq 0 ]; then
  echo "Todos los tests completaron correctamente."
  echo "Logs disponibles en artifacts/tests/logs"
else
  echo "Al menos un test fallo."
  exit 1
fi
