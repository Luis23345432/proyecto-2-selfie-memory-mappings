# README de presentacion: cambios importantes en `selfie.c`

Este archivo resume las partes mas importantes del codigo para explicar el avance del Proyecto 2: Memory Mappings en Selfie.

La idea central del proyecto es:

```text
mmap crea un mapping
-> el mapping queda registrado en el contexto del proceso
-> la page table apunta a cache frames
-> el page cache guarda paginas de archivo
-> fork comparte esos cache frames
-> msync persiste cambios dirty al archivo
```

## 1. Syscalls nuevas

Ubicacion aproximada: `selfie.c`, lineas 1297-1327 y 8329-8390.

Se agregaron tres syscalls:

```c
uint64_t SYSCALL_MMAP   = 222;
uint64_t SYSCALL_MUNMAP = 223;
uint64_t SYSCALL_MSYNC  = 224;
```

Tambien se agregaron sus wrappers:

```c
emit_mmap();
emit_munmap();
emit_msync();
```

Y se conectaron al dispatcher de syscalls:

```c
else if (a7 == SYSCALL_MMAP)
  implement_mmap(context);
else if (a7 == SYSCALL_MUNMAP)
  implement_munmap(context);
else if (a7 == SYSCALL_MSYNC)
  implement_msync(context);
```

Para explicar en la presentacion:

- `mmap` crea una region virtual respaldada por archivo.
- `munmap` elimina esa region, pero no guarda cambios.
- `msync` es quien guarda cambios al archivo.

## 2. Nuevas constantes y estructuras

Ubicacion aproximada: `selfie.c`, lineas 1340-1370.

### `mapping_entry`

Cada mapping activo se guarda en una entrada de 8 palabras:

| Offset | Campo | Funcion |
|---:|---|---|
| 0 | `next` | Siguiente mapping en la lista del proceso |
| 1 | `addr` | Direccion virtual inicial |
| 2 | `original_length` | Longitud pedida por el usuario |
| 3 | `rounded_length` | Longitud redondeada a paginas |
| 4 | `prot` | Permisos: read/write/read-write |
| 5 | `fd` | Descriptor del archivo |
| 6 | `file_offset` | Offset inicial en el archivo |
| 7 | `flags` | Estado del mapping, por ejemplo activo |

Lo importante:

```text
mapping_entry no guarda bytes del archivo.
Solo guarda metadata del mapping.
```

### `page_cache_entry`

Cada pagina de archivo cargada en memoria se guarda en una entrada de 7 palabras:

| Offset | Campo | Funcion |
|---:|---|---|
| 0 | `next` | Siguiente entrada del page cache global |
| 1 | `file_id` | Identificador del archivo; en este proyecto se usa `fd` |
| 2 | `file_page` | Numero de pagina dentro del archivo |
| 3 | `cache_frame` | Puntero al frame con los datos |
| 4 | `dirty` | Indica si la pagina fue modificada |
| 5 | `refcount` | Cuantos mappings usan ese frame |
| 6 | `flags` | Reservado/control |

Lo importante:

```text
page_cache_entry = metadata de una pagina de archivo
cache_frame = contenido real de esa pagina
```

## 3. Permisos de mmap

Ubicacion aproximada: `selfie.c`, lineas 1368-1370.

El proyecto usa esta convencion:

```c
uint64_t PROT_READ       = 0;
uint64_t PROT_WRITE      = 1;
uint64_t PROT_READ_WRITE = 2;
```

Esto es importante porque no se usa el esquema bitmask de Linux real. En la exposicion conviene decir:

```text
En este proyecto, los permisos son valores simples:
0 = lectura
1 = escritura
2 = lectura/escritura
```

## 4. Page cache global

Ubicacion aproximada: `selfie.c`, lineas 1378-1383 y 11996-12065.

Se agregaron variables globales:

```c
uint64_t* cache_frames;
uint64_t* page_cache_entries;
uint64_t next_cache_frame_index;
uint64_t cache_frames_allocated;
```

Que significan:

| Variable | Significado |
|---|---|
| `cache_frames` | Zona global donde viven los frames de 4096 bytes |
| `page_cache_entries` | Lista enlazada global de paginas cacheadas |
| `next_cache_frame_index` | Indice del siguiente frame libre |
| `cache_frames_allocated` | Cuantos frames ya fueron asignados |

Flujo del page cache:

```text
find_or_create_cache_entry(fd, file_page)
  -> busca si ya existe esa pagina del archivo
  -> si existe, reutiliza el cache frame
  -> si no existe, asigna frame nuevo
  -> carga la pagina con pread()
  -> crea page_cache_entry
```

Para explicar:

```text
El page cache es global, no pertenece a un proceso.
Por eso padre e hijo pueden compartir el mismo cache frame despues de fork.
```

## 5. Extension del contexto del proceso

Ubicacion aproximada: `selfie.c`, lineas 2297, 2355, 2397, 2439 y 11393.

Se cambio:

```c
uint64_t CONTEXTENTRIES = 39;
```

Y se agrego el campo:

```c
context[38] = mappings_head
```

Funciones importantes:

```c
uint64_t mappings_head(uint64_t* context);
uint64_t* get_mappings_head(uint64_t* context);
void set_mappings_head(uint64_t* context, uint64_t* head);
```

Uso:

```text
mappings_head
  -> mapping_entry_2
  -> mapping_entry_1
  -> NULL
```

Para explicar:

```text
Cada proceso tiene su propia lista de mappings.
Pero los cache frames pueden ser compartidos globalmente.
```

## 6. Helpers de mappings

Ubicacion aproximada: `selfie.c`, lineas 12090-12265.

Funciones importantes:

| Funcion | Para que sirve |
|---|---|
| `create_mapping_entry` | Crea la metadata de un mapping |
| `find_mapping` | Busca un mapping por direccion inicial |
| `is_address_in_mapping` | Verifica si una direccion cae dentro de un mapping |
| `is_mmap_range_free` | Revisa que el rango no choque con code, data, heap, stack u otros mappings |
| `choose_mmap_address` | Elige una direccion libre si `addr == 0` |
| `add_mapping_to_context` | Inserta el mapping en la lista del proceso |
| `remove_mapping_from_context` | Marca y remueve el mapping de la lista |

La parte mas importante para defender:

```text
mmap no puede usar cualquier direccion.
Debe ser page-aligned y no puede solaparse con memoria normal ni otros mappings.
```

## 7. `implement_mmap`

Ubicacion aproximada: `selfie.c`, lineas 12429-12532.

Esta es una de las funciones mas importantes.

Flujo:

```text
1. Lee argumentos desde A0-A4:
   addr, length, prot, fd, offset

2. Valida:
   length != 0
   prot valido
   offset alineado a PAGESIZE

3. Redondea length a paginas completas.

4. Si addr == 0:
   elige direccion libre con choose_mmap_address()

5. Verifica que el rango este libre.

6. Crea mapping_entry.

7. Por cada pagina:
   calcula file_page
   busca o crea page_cache_entry
   incrementa refcount
   crea PTE que apunta al cache frame

8. Agrega mapping al contexto.

9. Retorna la direccion virtual.
```

Fragmento conceptual:

```c
cache_entry = find_or_create_cache_entry(fd, file_page);
increment_page_cache_refcount(cache_entry);
cache_frame = (uint64_t) get_page_cache_frame(cache_entry);
map_page(context, vpage, cache_frame);
```

Para explicar:

```text
mmap conecta tres cosas:
mapping_entry del proceso
page_cache_entry global
PTE de la page table hacia el cache frame
```

## 8. Lecturas y escrituras sobre mappings

Ubicacion aproximada: `selfie.c`, lineas 12147-12170 y 10117-10136.

Para que una direccion mapeada sea valida, se agregaron helpers:

```c
is_valid_mapping_read(vaddr);
is_valid_mapping_write(vaddr);
```

Cuando el programa escribe en una direccion mapeada, `do_store` termina llamando:

```c
mark_mapping_page_dirty(vaddr);
```

Eso busca el mapping correspondiente, encuentra su `page_cache_entry` y marca:

```text
dirty = 1
```

Para explicar:

```text
La escritura no va directo al archivo.
Primero modifica el cache frame y marca dirty.
```

## 9. Dirty bit y `msync`

Ubicacion aproximada: `selfie.c`, lineas 12319-12329 y 12572-12610.

`dirty` significa:

```text
La pagina fue modificada en memoria, pero todavia no se escribio al archivo.
```

`implement_msync` hace:

```text
1. Busca el mapping por addr.
2. Recorre todas las paginas del mapping.
3. Busca cada page_cache_entry.
4. Si dirty == 1:
   escribe el cache frame al archivo con pwrite()
   dirty = 0
```

Fragmento conceptual:

```c
if (get_page_cache_dirty(cache_entry)) {
  write_cache_frame_to_file(...);
  set_page_cache_dirty(cache_entry, 0);
}
```

Para explicar:

```text
msync es el unico mecanismo que persiste cambios al archivo.
munmap no guarda.
```

## 10. `implement_munmap`

Ubicacion aproximada: `selfie.c`, lineas 12534-12570.

Flujo:

```text
1. Lee addr desde A0.
2. Busca el mapping.
3. Por cada pagina:
   busca page_cache_entry
   decrementa refcount
   invalida el PTE
4. Remueve el mapping del contexto.
5. Retorna 0.
```

Fragmento importante:

```c
decrement_page_cache_refcount(cache_entry);
set_PTE_for_page(get_pt(context),
  get_page_of_virtual_address(addr + (i * PAGESIZE)), 0);
```

Para explicar:

```text
munmap limpia la relacion virtual.
No llama pwrite.
Si habia cambios dirty sin msync, no se persisten.
```

## 11. I/O real: `pread` y `pwrite`

Ubicacion aproximada: `selfie.c`, lineas 12383-12427.

Leer una pagina:

```c
load_file_page_into_cache_frame(fd, file_page, frame);
```

Internamente usa:

```c
pread(host_fd, frame, PAGESIZE, file_offset);
```

Escribir una pagina:

```c
write_cache_frame_to_file(fd, file_page, frame, bytes_to_write);
```

Internamente usa:

```c
pwrite(host_fd, frame, bytes_to_write, file_offset);
```

Para explicar:

```text
mmap carga paginas con pread.
msync guarda paginas con pwrite.
```

## 12. Cambios en `fork`

Ubicacion aproximada: `selfie.c`, lineas 8210-8248 y 12333-12378.

Antes, fork copiaba memoria del padre al hijo.

Ahora se agrego:

```c
inherit_mappings_on_fork(context, child);
```

Esa funcion:

```text
1. Recorre los mappings del padre.
2. Crea mapping_entry equivalente en el hijo.
3. Para cada pagina:
   busca el page_cache_entry
   incrementa refcount
   mapea la page table del hijo al mismo cache frame
4. Agrega el mapping al contexto del hijo.
```

Fragmento conceptual:

```c
increment_page_cache_refcount(cache_entry);
cache_frame = (uint64_t) get_page_cache_frame(cache_entry);
map_page(child, vpage, cache_frame);
```

Tambien se evito copiar como memoria normal las paginas que pertenecen a mmap:

```c
if (is_address_in_any_mapping(context, vaddr) == 0)
  map_and_store(child, vaddr, load_virtual_memory(...));
```

Para explicar:

```text
La memoria normal se copia.
Las paginas mmap no se copian: se comparten por cache frame.
```

Ejemplo:

```text
Antes de fork:
Padre -> PTE -> Frame F0, refcount = 1

Despues de fork:
Padre -> PTE -> Frame F0
Hijo  -> PTE -> Frame F0
refcount = 2
```

## 13. Relacion completa entre piezas

Este es el dibujo mental para la exposicion:

```text
Proceso
  |
  v
Contexto / PCB
  |
  v
mappings_head
  |
  v
mapping_entry
  | addr
  v
VAS / direccion virtual
  |
  v
Page Table / PTE
  |
  v
Cache Frame
  ^
  |
Page Cache Entry
  |
  v
Archivo: fd + file_page
```

## 14. Que demuestra cada test

| Test | Que demuestra | Idea para decir en la presentacion |
|---|---|---|
| `test-mmap-1-read.c` | Leer desde archivo mapeado | `mmap` crea PTE hacia cache frame cargado con `pread` |
| `test-mmap-2-write-sync.c` | Escribir y persistir con `msync` | Dirty se vuelve 1, `msync` hace `pwrite` y dirty vuelve a 0 |
| `test-mmap-3-fork-shared.c` | Padre e hijo comparten mapping | Fork no duplica cache frame; incrementa refcount |
| `test-mmap-4-no-sync-no-persist.c` | `munmap` no guarda | Sin `msync`, el archivo conserva el valor original |

## 15. Frases cortas para defender

- `mapping_entry` describe el mapping, pero no contiene los bytes del archivo.
- `page_cache_entry` identifica una pagina de archivo: `fd + file_page`.
- `cache_frame` contiene los 4096 bytes reales de esa pagina.
- La page table hace que la direccion virtual apunte al cache frame.
- `dirty = 1` significa "modificado en memoria, no persistido".
- `msync` es quien escribe al archivo.
- `munmap` solo desmapea; no guarda.
- `fork` copia metadata, pero comparte cache frames.
- `refcount` evita descartar un frame que todavia usa otro proceso.

## 16. Limitaciones que conviene mencionar

- El proyecto usa `fd` como `file_id`, una simplificacion valida para el alcance del proyecto.
- El page cache es simple: lista enlazada global y hasta 256 cache frames.
- No hay politica avanzada tipo LRU.
- La persistencia fue simplificada: se asume que `pread`/`pwrite` funcionan correctamente.

## 17. Resumen final

Si tienes que explicarlo en 30 segundos:

```text
Agregamos mmap, munmap y msync.
mmap crea un mapping en el contexto del proceso y hace que la page table apunte a cache frames globales.
El page cache guarda paginas de archivo identificadas por fd y file_page.
Cuando se escribe, se marca dirty.
msync persiste los dirty frames al archivo.
munmap elimina el mapping sin guardar.
fork hereda los mappings y comparte los mismos cache frames aumentando refcount.
```
