# Plan de mejoras del page cache

Este documento explica los cambios que se van a implementar para resolver las dos limitaciones actuales del proyecto:

1. El page cache identifica archivos usando `fd`.
2. El page cache no tiene politica de reemplazo cuando se llenan los cache frames.

La idea es mantener la implementacion actual funcionando, pero hacerla mas parecida a un sistema operativo real.

## 1. Problema actual: el page cache usa `fd`

Actualmente cada entrada del page cache se identifica con:

```text
(fd, file_page)
```

Esto funciona para los tests actuales, especialmente para `fork`, porque padre e hijo comparten el mismo mapping y los mismos cache frames. Sin embargo, tiene una limitacion importante:

```c
fd1 = open("archivo.txt", ...);
fd2 = open("archivo.txt", ...);
```

Aunque `fd1` y `fd2` apunten al mismo archivo real, el page cache actual los trataria como archivos distintos:

```text
(fd1, page 0) != (fd2, page 0)
```

En un sistema operativo real, el page cache no se basa solamente en el descriptor. Usa una identidad estable del archivo.

## 2. Mejora propuesta: identidad real de archivo

La solucion recomendada es obtener la identidad del archivo usando `fstat`.

Con `fstat(fd, &st)`, Linux entrega dos datos importantes:

```text
st_dev = dispositivo donde vive el archivo
st_ino = inode del archivo
```

La combinacion:

```text
(st_dev, st_ino)
```

identifica al archivo real aunque se abra varias veces con distintos file descriptors.

La nueva clave del page cache pasaria de:

```text
(fd, file_page)
```

a:

```text
(file_device, file_inode, file_page)
```

Asi, si dos procesos abren el mismo archivo con distintos `fd`, ambos mappings pueden compartir la misma pagina cacheada.

## 3. Cambios esperados en el codigo

Se agregaran campos nuevos al `page_cache_entry`.

Actualmente representa algo parecido a:

```text
next
file_id
file_page
cache_frame
dirty
refcount
flags
```

La version mejorada deberia guardar:

```text
next
file_device
file_inode
file_page
cache_frame
dirty
refcount
last_used
flags
```

Por eso habra que actualizar:

- `PAGE_CACHE_ENTRY_SIZE`
- offsets del `page_cache_entry`
- getters y setters
- `find_page_cache_entry`
- `find_or_create_cache_entry`
- llamadas desde `mmap`, `munmap`, `msync` y `fork`

## 4. Problema actual: no hay LRU

El page cache actual tiene un numero fijo de cache frames:

```c
NUMBEROFCACHEFRAMES = 256;
```

Si se necesitan mas paginas que frames disponibles, actualmente no hay una politica avanzada para decidir que pagina sacar o reutilizar.

En un sistema operativo real se usa una politica de reemplazo. Una de las mas conocidas es:

```text
LRU = Least Recently Used
```

Es decir, se reemplaza la pagina que lleva mas tiempo sin usarse.

## 5. Mejora propuesta: LRU simple

Se agregara un contador global:

```text
page_cache_clock
```

Cada vez que una pagina del page cache se use, se actualizara:

```text
entry->last_used = page_cache_clock++;
```

Cuando no haya cache frames libres, se buscara una entrada candidata para reemplazo:

```text
refcount == 0
```

Esto es importante: no se debe reemplazar una pagina que todavia esta mapeada por algun proceso.

Entre las entradas con `refcount == 0`, se elige la de menor `last_used`, o sea la menos usada recientemente.

## 6. Que pasa si la pagina victima esta dirty

Si la pagina elegida para reemplazo esta marcada como dirty:

```text
dirty == 1
```

primero se debe escribir al archivo con `pwrite`, igual que hace `msync`.

Luego se puede reutilizar el cache frame para otra pagina.

Flujo esperado:

```text
cache lleno
buscar victima con refcount == 0
si dirty:
    escribir al archivo
reutilizar frame
cargar nueva pagina con pread
actualizar page_cache_entry
```

## 7. Orden recomendado de implementacion

El orden mas seguro es:

1. Agregar identidad real de archivo con `fstat`.
2. Cambiar el page cache para buscar por `(device, inode, page)`.
3. Agregar `last_used` y `page_cache_clock`.
4. Actualizar `last_used` cada vez que se encuentra o crea una entrada.
5. Implementar reemplazo LRU cuando no haya frames libres.
6. Si la victima esta dirty, persistirla antes de reutilizarla.
7. Agregar tests nuevos:
   - dos `open` distintos del mismo archivo deben compartir cache
   - muchas paginas deben forzar reemplazo
   - pagina dirty reemplazada debe persistirse correctamente

## 8. Resultado esperado

Con estas mejoras, el page cache sera mas robusto:

- Dos `fd` distintos del mismo archivo podran compartir la misma pagina cacheada.
- El cache podra reutilizar frames cuando se llene.
- Las paginas dirty no se perderan al ser reemplazadas.
- La implementacion se parecera mas al comportamiento de un sistema operativo real.



## 9. Implementacion realizada

Se implementaron las mejoras de forma puntual y compatible con la estructura actual de Selfie.

### Identidad estable de archivo

Para evitar agregar dependencias grandes de headers del sistema, la implementacion final usa una tabla interna:

```text
fd -> file_id
file_id -> path copiado
```

Cuando `openat` abre un archivo correctamente, se registra el path en esta tabla. Si el mismo path se abre dos veces, ambos `fd` reciben el mismo `file_id` interno. El page cache ahora busca por:

```text
(file_id, file_page)
```

en vez de depender directamente de:

```text
(fd, file_page)
```

Esto resuelve el caso practico del proyecto donde dos `open` distintos del mismo archivo deben compartir la misma pagina cacheada. Una mejora futura aun mas cercana a Linux seria reemplazar esta identidad por `(st_dev, st_ino)` usando `fstat`.

### LRU simple

Se agrego `last_used` a cada `page_cache_entry` y un contador global `page_cache_clock`. Cada vez que una entrada se encuentra o se crea, se actualiza su uso:

```text
entry->last_used = page_cache_clock++
```

Cuando ya no quedan cache frames libres, se busca una victima con:

```text
refcount == 0
```

Entre esas candidatas se elige la que tenga menor `last_used`, es decir, la menos usada recientemente. Si estuviera dirty, se intenta persistir antes de reutilizar el frame.

### Cambios exactos en `selfie.c`

| Zona | Cambio realizado | Funcion o variable principal |
|---|---|---|
| Estructura `page_cache_entry` | Paso de 7 a 9 palabras para guardar `fd` y `last_used`. | `PAGE_CACHE_ENTRY_SIZE`, `PAGECACHE_FD`, `PAGECACHE_LAST_USED` |
| Configuracion global | Se agregaron limites para la tabla interna de archivos. | `MAX_TRACKED_FILE_DESCRIPTORS`, `MAX_TRACKED_FILES` |
| Estado global del page cache | Se agrego reloj LRU y tabla de identidad de archivos. | `page_cache_clock`, `fd_file_ids`, `file_id_names`, `next_file_id` |
| Accessors | Se agregaron getters/setters para los nuevos campos. | `get_page_cache_fd`, `set_page_cache_fd`, `get_page_cache_last_used`, `set_page_cache_last_used` |
| Registro de archivo abierto | Al terminar `openat`, se registra el `fd` junto con el path abierto. | `register_file_identity(opened_fd, filename_buffer)` en `implement_openat` |
| Identidad estable | Si el mismo path se abre varias veces, se reutiliza el mismo `file_id`. | `register_file_identity`, `get_file_id_for_fd` |
| Busqueda en page cache | La busqueda recibe `fd`, lo traduce a `file_id` y compara `(file_id, file_page)`. | `find_page_cache_entry` |
| Creacion de cache entry | La entrada nueva guarda `file_id`, `fd`, `file_page`, frame, dirty, refcount y `last_used`. | `find_or_create_cache_entry` |
| Actualizacion LRU | Cada acceso a una entrada actualiza su uso reciente. | `touch_page_cache_entry` |
| Reemplazo LRU | Cuando no hay frames libres, se busca una entrada con `refcount == 0` y menor `last_used`. | `evict_page_cache_entry` |
| Persistencia en reemplazo | Si la victima esta dirty, se intenta escribir con `pwrite` antes de reutilizar el frame. | `write_cache_frame_to_file` dentro de `evict_page_cache_entry` |

### Cambios exactos en scripts

| Archivo | Cambio |
|---|---|
| `scripts/compile_tests.sh` | Ahora limpia y compila `test1` a `test6`. |
| `scripts/run_tests.sh` | Ahora ejecuta `test1` a `test6`, restaurando el fixture antes de cada test. |

### Tests nuevos

Se agregaron dos tests:

| Test | Proposito | Resultado esperado |
|---|---|---|
| `tests/mmap/test-mmap-5-two-fds-shared.c` | Abre el mismo archivo dos veces, mapea ambos `fd`, escribe por el primer mapping y lee por el segundo. | Si ambos mappings comparten page cache, el segundo ve el valor escrito y el test termina con exit code 0. |
| `tests/mmap/test-mmap-6-lru-reuse.c` | Mapea y desmapea 260 paginas, superando los 256 cache frames disponibles. | Si el LRU reutiliza frames con `refcount == 0`, el test termina con exit code 0. |

### Alcance exacto de la identidad de archivo

La mejora implementada identifica archivos por path registrado en `openat`, no por `(st_dev, st_ino)`. Esto fue intencional para mantener el cambio pequeno y compatible con el estilo autocontenido de `selfie.c`.

Resuelve este caso:

```c
fd1 = open("archivo.txt", ...);
fd2 = open("archivo.txt", ...);
```

si ambos usan el mismo string de path. No resuelve completamente casos mas avanzados de un sistema Linux real, por ejemplo:

```text
./archivo.txt
/ruta/absoluta/al/mismo/archivo.txt
hard links distintos al mismo inode
```

Para resolver esos casos, la siguiente mejora seria cambiar la tabla interna por identidad de inode usando `fstat(fd, &st)` y la clave `(st_dev, st_ino, file_page)`.

Validacion final:

```text
TEST 1: PASS
TEST 2: PASS
TEST 3: PASS
TEST 4: PASS
TEST 5: PASS
TEST 6: PASS
```
