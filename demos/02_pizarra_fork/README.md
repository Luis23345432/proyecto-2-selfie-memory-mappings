# Demo 2: Dos procesos, una misma pizarra

Esta demo muestra que `fork` hereda los mappings y que padre e hijo comparten el mismo cache frame.

## Idea

El padre hace `mmap` de `shared-board.txt` y escribe `PARENT!!`. Luego hace `fork`.

El hijo:

1. Lee `PARENT!!` desde su mapping heredado.
2. Escribe `CHILD!!!` en el mismo mapping.
3. Hace `munmap`.

El padre:

1. Espera al hijo con `wait`.
2. Lee desde su propio mapping.
3. Ve `CHILD!!!`.
4. Hace `msync` y `munmap`.

## Que demuestra

- `fork` hereda mappings.
- Padre e hijo apuntan al mismo cache frame.
- El refcount evita perder la pagina compartida.
- `msync` persiste el cambio final.
- `munmap` libera el mapping.

## Como correr

Desde la raiz del repo:

```bash
demos/02_pizarra_fork/run_demo.sh
```

Salida esperada:

```text
PADRE: ESCRIBE PARENT!!
HIJO: VIO PARENT!!
HIJO: ESCRIBE CHILD!!!
PADRE: VIO CHILD!!!
MSYNC: OK
DEMO2: PASS
```
