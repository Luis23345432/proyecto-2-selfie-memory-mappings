# Demo 1: Editor magico con memoria mapeada

Esta demo muestra la diferencia entre escribir en memoria mapeada y persistir realmente al archivo.

## Idea

El archivo `broken.c` empieza roto:

```c
xint64_t main() {
  exit(0);
}
```

El fix correcto es cambiar los primeros 8 bytes:

```text
xint64_t -> uint64_t
```

Primero se intenta corregir con `mmap` pero sin `msync`; el archivo sigue roto. Luego se repite con `msync` y el archivo compila.

## Que demuestra

- `mmap` permite tratar el archivo como memoria.
- Escribir en el mapping marca la pagina como dirty.
- `munmap` sin `msync` no persiste el cambio.
- `msync` escribe la pagina dirty al archivo.
- El cambio persistido tiene efecto real: Selfie puede compilar el archivo corregido.

## Como correr

Desde la raiz del repo:

```bash
demos/01_editor_magico/run_demo.sh
```

Salida esperada:

```text
OK: la compilacion fallo como esperabamos
OK: sin msync el archivo sigue roto
PASS: con msync el archivo fue corregido y Selfie lo compila
========== DEMO 1 PASS ==========
```
