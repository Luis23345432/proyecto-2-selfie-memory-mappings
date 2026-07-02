# Demos en vivo

Esta carpeta contiene tres demos replicables para presentar el proyecto en clase.

## Demo recomendada para abrir

```bash
demos/01_editor_magico/run_demo.sh
```

Muestra que `mmap` puede modificar un archivo en memoria, pero que el cambio solo llega al disco con `msync`.

## Demo de fork

```bash
demos/02_pizarra_fork/run_demo.sh
```

Muestra que padre e hijo comparten el mismo cache frame despues de `fork`.

## Demo completa

```bash
demos/03_sistema_archivos_viviente/run_demo.sh
```

Muestra dos `fd` del mismo archivo, page cache compartido, `fork`, `msync` y `munmap`.

## Recomendacion para la exposicion

Para una presentacion de 12 minutos, usar:

1. `01_editor_magico`
2. `02_pizarra_fork`

La demo 3 puede quedar como respaldo o cierre si queda tiempo.
