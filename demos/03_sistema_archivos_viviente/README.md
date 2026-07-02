# Demo 3: Mini sistema de archivos viviente

Esta demo junta varias piezas en una sola historia: dos `fd`, dos mappings, `fork`, page cache compartido, dirty bit, `msync` y `munmap`.

## Idea

El archivo `live-note.txt` funciona como una nota compartida.

1. Se abre el mismo archivo dos veces.
2. Se crean dos mappings: `map1` y `map2`.
3. `map1` escribe `ALIVE001`.
4. `map2` lee `ALIVE001`, demostrando que ambos `fd` comparten page cache.
5. Se hace `fork`.
6. El hijo escribe `ALIVE002`.
7. El padre ve `ALIVE002` desde su mapping.
8. El padre hace `msync`.
9. Ambos mappings se liberan con `munmap`.

## Que demuestra

- `mmap` sobre dos `fd` del mismo archivo.
- Identidad de archivo por path (`fd -> file_id`).
- Page cache compartido.
- `fork` compartiendo cache frames.
- Dirty bit.
- `msync`.
- `munmap`.

## Como correr

Desde la raiz del repo:

```bash
demos/03_sistema_archivos_viviente/run_demo.sh
```

Salida esperada:

```text
MAP1: ESCRIBE ALIVE001
MAP2: VIO ALIVE001
HIJO: ESCRIBE ALIVE002
PADRE: VIO ALIVE002
MSYNC: OK
DEMO3: PASS
```
