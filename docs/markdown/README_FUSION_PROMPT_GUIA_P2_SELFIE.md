# README - Prompt y Guía Fusionada para Proyecto 2: Memory Mappings en Selfie

## 0. Propósito de este documento

Este README fusiona lo mejor de tres enfoques:

1. Un **prompt maestro** más seguro para GitHub Copilot / Codex.
2. Una **guía paso a paso por fases** para implementar sin romper `selfie.c`.
3. Un **plan técnico concreto** con estructuras, syscalls, tests, checklist y criterios de rúbrica.

La idea es usar este documento como guía principal del equipo para implementar el Proyecto 2: **Memory Mappings en Selfie**, donde se deben agregar las syscalls:

```c
uint64_t mmap(uint64_t addr, uint64_t length, uint64_t prot, uint64_t fd, uint64_t offset);
uint64_t munmap(uint64_t addr);
uint64_t msync(uint64_t addr);
```

---

# PARTE 1: PROMPT MAESTRO PARA CODEX / GITHUB COPILOT

Copia y pega este prompt dentro del proyecto, en la raíz donde está `selfie.c` y donde también exista `README_contexto_selfie.md`.

```text
Actúa como un ingeniero senior de sistemas operativos, C*/StarC y arquitectura interna de Selfie.

Estoy trabajando en el Proyecto 2: Memory Mappings en Selfie.

OBJETIVO GENERAL
Implementar una versión simplificada de las syscalls:

- mmap(addr, length, prot, fd, offset)
- munmap(addr)
- msync(addr)

Estas syscalls deben permitir mapear regiones de un archivo en memoria virtual del proceso usando page cache y cache frames compartidos.

ANTES DE MODIFICAR CÓDIGO
Primero lee y analiza completamente:

1. selfie.c
2. README_contexto_selfie.md
3. Este README de implementación, si está disponible
4. Tests existentes, si existen

NO modifiques `selfie.c` inmediatamente.

Primero crea un archivo nuevo llamado:

PLAN_IMPLEMENTACION_MMAP.md

Ese archivo debe incluir:

1. Funciones exactas de `selfie.c` que vas a modificar.
2. Constantes nuevas que vas a agregar.
3. Campos nuevos del contexto que vas a agregar.
4. Diseño exacto de la estructura de mappings.
5. Diseño exacto del page cache.
6. Syscalls nuevas y números elegidos.
7. Estrategia para mmap.
8. Estrategia para munmap.
9. Estrategia para msync.
10. Estrategia para fork.
11. Estrategia de tests.
12. Riesgos principales.
13. Cómo verificar que no se rompieron syscalls anteriores.

Después de crear ese plan, recién implementa los cambios.

CONTEXTO TÉCNICO DE SELFIE
Selfie en este proyecto está principalmente en un solo archivo llamado `selfie.c`.

Selfie usa estilo C* / StarC:
- Usa `uint64_t` para casi todo.
- Representa contextos como arreglos de `uint64_t`, no como structs modernos.
- Usa punteros crudos y funciones getter/setter.
- Tiene su propio manejo de memoria, como `zmalloc`, `smalloc`, `palloc`, etc.
- No debes introducir dependencias externas.
- No debes hacer refactors grandes.
- No debes reescribir la arquitectura del proyecto.

FUNCIONES IMPORTANTES A REVISAR
Antes de implementar, ubica y entiende:

- `handle_system_call`
- `implement_openat`
- `implement_read`
- `implement_write`
- `implement_brk`
- `implement_fork`
- `implement_wait`
- `init_context`
- `create_context`
- `free_context`
- `delete_context`
- `tlb`
- `load_virtual_memory`
- `store_virtual_memory`
- `load_cached_virtual_memory`
- `store_cached_virtual_memory`
- `copy_buffer`
- `map_page`
- `map_and_store`
- `set_PTE_for_page`
- `get_frame_for_page`
- `is_virtual_address_mapped`
- `get_page_of_virtual_address`
- `handle_page_fault`

REGLA CRÍTICA DEL PROYECTO
- `mmap` NO debe escribir cambios al archivo.
- `munmap` NO debe escribir cambios al archivo.
- `msync` debe ser el ÚNICO mecanismo que persiste cambios al archivo.
- Los cambios hechos en memoria mapeada viven inicialmente en cache frames.
- Dos procesos que mapean la misma página del mismo archivo deben poder compartir el mismo cache frame.

REQUISITOS DE mmap
Implementa una syscall conceptual:

uint64_t mmap(uint64_t addr, uint64_t length, uint64_t prot, uint64_t fd, uint64_t offset);

Argumentos sugeridos:
- REG_A0 = addr
- REG_A1 = length
- REG_A2 = prot
- REG_A3 = fd
- REG_A4 = offset

Retorno:
- REG_A0 = dirección virtual inicial asignada
- REG_A0 = -1 o valor de error según estilo de Selfie si falla

Debe cumplir:

1. Crear un mapping entre una región de memoria virtual y una región de un archivo.
2. Registrar ese mapping en el contexto del proceso.
3. Usar `PAGESIZE = 4096` para:
   - redondear length hacia arriba
   - calcular número de páginas
   - validar/alinear offset
   - calcular offsets internos
4. Si `addr != 0`, usar esa dirección si es válida y está libre.
5. Si `addr == 0`, elegir una región virtual libre:
   - alineada a PAGESIZE
   - que no choque con code/data/heap/stack
   - que no choque con otros mappings
   - que no choque con páginas ya mapeadas
6. Crear o reutilizar cache frames desde el page cache.
7. Si la página `(file id, file page offset)` ya existe en el page cache, reutilizar el mismo cache frame.
8. Si no existe, crear cache frame, cargar el contenido del archivo y registrar la entrada.
9. Actualizar la page table o la lógica de traducción para que el acceso a la dirección mapeada llegue al cache frame.
10. No persistir cambios al archivo en mmap.

VALIDACIONES MÍNIMAS DE mmap
Implementa o documenta claramente:

1. `length > 0`.
2. `offset` múltiplo de PAGESIZE, o manejo explícito si no lo es.
3. `addr` alineado a PAGESIZE si `addr != 0`.
4. `prot` válido:
   - 0 = lectura
   - 1 = escritura
   - 2 = lectura/escritura
5. El rango `[addr, addr + rounded_length)` no debe desbordar.
6. El rango no debe colisionar con otros mappings.
7. El rango no debe colisionar con memoria normal del proceso.
8. El `fd` debe ser tratado con cuidado.

NOTA IMPORTANTE SOBRE fd
El enunciado indica que el `fd` es local al proceso. Eso significa que usar `fd` directamente como `file_id` puede funcionar para tests simples, pero conceptualmente no identifica globalmente el mismo archivo.

Implementa una solución simple pero documentada:
- Opción mínima aceptable: usar `fd` como `file_id` y documentar la limitación.
- Opción mejor: asociar cada archivo abierto con un identificador más estable si Selfie lo permite.
- No inventes una tabla compleja si rompe el estilo del proyecto.

REQUISITOS DE munmap
Implementa una syscall conceptual:

uint64_t munmap(uint64_t addr);

Argumentos:
- REG_A0 = addr

Retorno:
- REG_A0 = 0 si éxito
- REG_A0 = -1 si error

Debe cumplir:

1. `addr` debe coincidir exactamente con la dirección inicial de un mapping activo.
2. Buscar el mapping en `context->mappings`.
3. Invalidar o eliminar la relación entre páginas virtuales y cache frames.
4. Remover o marcar inactivo el mapping.
5. NO escribir cambios al archivo.
6. NO llamar a msync automáticamente.
7. No liberar cache frames a ciegas si otro proceso o mapping los puede estar usando.
8. Si se implementa refcount, decrementarlo; si no, documentar la limitación.

REQUISITOS DE msync
Implementa una syscall conceptual:

uint64_t msync(uint64_t addr);

Argumentos:
- REG_A0 = addr

Retorno:
- REG_A0 = 0 si éxito
- REG_A0 = -1 si error

Debe cumplir:

1. `addr` debe coincidir exactamente con la dirección inicial de un mapping activo.
2. Buscar el mapping.
3. Recorrer las páginas del mapping.
4. Para cada página:
   - encontrar el page cache entry correspondiente
   - obtener el cache frame
   - escribir el contenido del cache frame al archivo en el offset correcto
5. Usar `lseek` o mecanismo equivalente si existe para posicionar el archivo.
6. Si no existe `lseek`, analizar cómo agregarlo siguiendo el estilo de `read`, `write`, `open`.
7. Si `length` original no era múltiplo de PAGESIZE, no escribir bytes extra innecesarios en la última página.
8. Si se implementa dirty bit, escribir solo páginas dirty y luego marcarlas clean.
9. `msync` es el único mecanismo de persistencia.

REQUISITOS DEL PAGE CACHE
Implementa un page cache compartido.

Cada entrada representa:

(file identifier, file page offset) -> cache frame

Cada entrada debe guardar como mínimo:

- next entry, si usas lista enlazada
- file id o fd
- file page offset
- cache frame
- dirty/modified bit
- refcount opcional
- active/inactive si usas tabla fija

Puedes implementar usando:
- lista enlazada de entries, o
- arrays globales paralelos, o
- tabla fija.

Mantén el estilo C*/StarC del proyecto.

Funciones recomendadas:

- `init_page_cache()`
- `allocate_cache_frame()`
- `find_page_cache_entry(fd, file_page_offset)`
- `create_page_cache_entry(fd, file_page_offset)`
- `find_or_create_cache_entry(fd, file_page_offset)`
- `load_file_page_into_cache_frame(fd, file_page_offset, cache_frame)`
- `write_cache_frame_to_file(fd, file_page_offset, cache_frame, bytes)`
- `mark_page_cache_entry_dirty(cache_frame)` o equivalente

REGLAS DEL PAGE CACHE
1. Un cache frame es una página física de PAGESIZE bytes.
2. Los cache frames no pertenecen a un proceso específico.
3. Si dos mappings apuntan al mismo archivo y mismo offset de página, deben usar el mismo cache frame.
4. Si el page cache se llena, retornar error claro o implementar una estrategia simple.
5. No implementar LRU complejo salvo que sea necesario.
6. No duplicar cache frames durante fork.

REQUISITOS DEL CONTEXTO
Selfie representa el contexto como arreglo de `uint64_t`.

Debes agregar un nuevo campo para mappings, probablemente incrementando `CONTEXTENTRIES`.

Antes de hacerlo:
1. Verifica el valor actual de `CONTEXTENTRIES`.
2. Verifica que el offset elegido no choque con otros campos.
3. Crea getter/setter siguiendo el estilo del archivo.

Ejemplo conceptual:

- `get_mappings_head(context)`
- `set_mappings_head(context, head)`

Cada mapping debe guardar como mínimo:

- next mapping
- addr
- original length
- rounded length o number of pages
- prot
- fd/file id
- offset
- active/flags si aplica

Funciones recomendadas:

- `create_mapping_entry`
- `get_mapping_next`
- `get_mapping_addr`
- `get_mapping_length`
- `get_mapping_rounded_length`
- `get_mapping_prot`
- `get_mapping_fd`
- `get_mapping_offset`
- `set_mapping_next`
- `find_mapping(context, addr)`
- `find_mapping_for_address(context, vaddr)`
- `add_mapping_to_context(context, mapping)`
- `remove_mapping_from_context(context, addr)`
- `copy_mappings_to_child(parent, child)`

REQUISITOS DE TRADUCCIÓN DE MEMORIA
Hay dos estrategias válidas:

ESTRATEGIA A: page table apunta directamente a cache frames
- mmap crea PTEs donde virtual page -> cache frame.
- `tlb()` puede seguir funcionando casi igual.
- Hay que marcar dirty cuando se escribe sobre una página mapeada.
- Hay que cuidar fork para no copiar esas páginas.

ESTRATEGIA B: interceptar en load/store
- `load_virtual_memory` y `store_virtual_memory` detectan si vaddr está en un mapping.
- Si está en mapping, usan page cache.
- Si no, usan traducción normal.
- Es más explícito pero toca más rutas.

Elige la estrategia más simple y robusta para el código real de Selfie.

Si usas Estrategia A:
- Asegura que `store_virtual_memory` o una ruta equivalente pueda marcar dirty.
- Asegura que `munmap` pueda invalidar PTEs.
- Asegura que `fork` no duplique páginas file-backed.

Si usas Estrategia B:
- Asegura que todos los accesos relevantes pasen por las funciones modificadas.
- Asegura que `copy_buffer`, `read`, `write` y accesos normales funcionen.

REQUISITOS DE fork
Modificar `implement_fork` para que:

1. El hijo herede los mappings del padre.
2. El hijo tenga metadata propia de mappings, pero apuntando a los mismos cache frames.
3. Las páginas file-backed NO deben duplicarse con `map_and_store`.
4. La memoria normal puede seguir copiándose como antes.
5. Si hay refcount, incrementarlo para las entradas compartidas.
6. Padre e hijo deben poder observar cambios en memoria mapeada compartida.
7. `msync` debe persistir esos cambios al archivo.

IMPORTANTE:
Si el padre escribe en memoria mapeada, el hijo debería ver el cambio porque ambos apuntan al mismo cache frame. `msync` sirve para persistir al archivo, no para que el otro proceso vea el cambio en memoria.

SYSCALLS Y EMIT
Agrega constantes nuevas sin chocar con las existentes:

- SYSCALL_MMAP
- SYSCALL_MUNMAP
- SYSCALL_MSYNC

Una propuesta posible:
- SYSCALL_MMAP = 222
- SYSCALL_MUNMAP = 223
- SYSCALL_MSYNC = 224

Pero antes verifica que esos números no estén usados.

Agrega dispatch en `handle_system_call`.

Si Selfie requiere wrappers o funciones `emit_*` para compilar tests en C*, agrega:

- `emit_mmap`
- `emit_munmap`
- `emit_msync`

siguiendo el patrón de syscalls existentes como `fork`, `wait`, `read`, `write` u otras.

TESTS OBLIGATORIOS
Crea al menos estos tests:

TEST 1: lectura desde memoria mapeada
- Crear archivo con contenido conocido.
- Abrir archivo.
- mmap.
- Leer directamente desde el puntero retornado.
- Imprimir resultado claro.
- munmap.
Salida esperada con PASS/FAIL.

TEST 2: escritura + msync
- Crear archivo con contenido inicial.
- mmap con escritura.
- Modificar memoria mapeada.
- msync.
- Leer archivo de nuevo.
- Verificar que el cambio persistió.
Salida esperada con PASS/FAIL.

TEST 3: dos procesos comparten mapping
- Padre abre archivo.
- Padre hace mmap.
- fork.
- Un proceso escribe en memoria mapeada.
- El otro proceso lee y ve el cambio.
- msync al final para persistencia.
Salida esperada con PASS/FAIL.

TEST 4 recomendado: munmap no persiste
- mmap.
- modificar memoria.
- munmap sin msync.
- leer archivo.
- verificar que NO cambió.
Salida esperada con PASS/FAIL.

DOCUMENTACIÓN FINAL
Crea o actualiza:

README_P2_MEMORY_MAPPINGS.md

Debe incluir:

1. Resumen del proyecto.
2. Qué se implementó.
3. Qué partes de `selfie.c` se modificaron.
4. Diseño de mappings.
5. Diseño del page cache.
6. Funcionamiento de mmap.
7. Funcionamiento de munmap.
8. Funcionamiento de msync.
9. Funcionamiento de fork con mappings.
10. Cómo compilar.
11. Cómo ejecutar tests.
12. Salida esperada de tests.
13. Limitaciones conocidas.
14. Relación con la rúbrica.

CONTROL DE CALIDAD
Antes de finalizar:

1. Compila Selfie.
2. Ejecuta tests existentes.
3. Ejecuta tests nuevos.
4. Verifica que read/write/openat/fork/wait/exit sigan funcionando.
5. Verifica que mmap no escribe al archivo.
6. Verifica que munmap no escribe al archivo.
7. Verifica que msync sí escribe al archivo.
8. Verifica que fork comparte cache frames.
9. Elimina prints de debug excesivos.
10. Documenta limitaciones reales.

NO HAGAS
- No reescribas Selfie.
- No conviertas todo a structs modernos.
- No introduzcas librerías externas.
- No cambies syscalls existentes innecesariamente.
- No guardes cambios en mmap.
- No guardes cambios en munmap.
- No dupliques cache frames en fork.
- No liberes cache frames compartidos sin refcount.
- No dejes tests ambiguos.
- No inventes funciones que no existen sin revisar el código.

CRITERIOS DE ACEPTACIÓN
La implementación es aceptable si:

1. mmap crea mappings válidos.
2. Se puede leer desde memoria mapeada.
3. Se puede escribir en memoria mapeada.
4. msync persiste cambios al archivo.
5. munmap no persiste cambios.
6. fork hereda mappings.
7. Padre e hijo comparten cache frames.
8. Tests tienen salida clara.
9. Selfie compila.
10. No se rompieron syscalls anteriores.
11. Hay README final de implementación y pruebas.
```

---

# PARTE 2: GUÍA DETALLADA DE IMPLEMENTACIÓN

## 1. Resumen del proyecto

El proyecto pide implementar **memory mappings** en Selfie.

La relación esperada es:

```text
Proceso / contexto
→ dirección virtual
→ mapping registrado en contexto
→ tabla de páginas
→ page cache
→ cache frame
→ archivo
```

La idea es que el proceso pueda acceder a contenido de archivo como si fuera memoria.

---

## 2. Requisitos principales del enunciado

Se deben implementar:

```c
mmap(addr, length, prot, fd, offset)
munmap(addr)
msync(addr)
```

Además:

- Usar `PAGESIZE = 4096`.
- Redondear `length` a páginas.
- Guardar mappings en el contexto del proceso.
- Crear un page cache.
- Usar cache frames compartidos.
- `mmap` no persiste cambios.
- `munmap` no persiste cambios.
- `msync` sí persiste cambios.
- `fork` debe heredar mappings sin duplicar cache frames.
- Crear tests que demuestren lectura, escritura + msync, y compartición entre procesos.

---

## 3. Rúbrica

| Criterio | Puntaje |
|---|---:|
| Implementación | 8 pts |
| Tests | 5 pts |
| Avances en clase | 3 pts |
| Presentación | 4 pts |
| Total | 20 pts |

Para apuntar al máximo:

- Implementación clara y completa.
- Tests consistentes y con salidas fáciles de interpretar.
- Explicación en clase de avances y obstáculos.
- Presentación de 12 minutos con diagramas y demos.

---

# PARTE 3: PLAN POR FASES

## FASE 0: Preparación

Antes de tocar código:

1. Abrir `selfie.c`.
2. Abrir `README_contexto_selfie.md`.
3. Ubicar funciones clave.
4. Crear `PLAN_IMPLEMENTACION_MMAP.md`.
5. Definir estrategia:
   - page table apunta a cache frames, o
   - load/store interceptan mappings.

Recomendación inicial: usar **page table apuntando a cache frames**, porque es más simple y se integra mejor con `tlb`.

---

## FASE 1: Análisis de funciones existentes

### Objetivo

Entender cómo funciona la memoria virtual, syscalls y fork.

### Funciones a analizar

```text
load_virtual_memory()
store_virtual_memory()
copy_buffer()
implement_read()
implement_write()
implement_openat()
implement_fork()
handle_system_call()
tlb()
map_page()
map_and_store()
set_PTE_for_page()
get_frame_for_page()
is_virtual_address_mapped()
init_context()
```

### Resultado esperado

Crear notas en `PLAN_IMPLEMENTACION_MMAP.md` indicando:

- Qué hace cada función.
- Qué se modificará.
- Qué no se debe tocar.

---

## FASE 2: Extender contexto con mappings

### Objetivo

Agregar al contexto un campo para lista de mappings.

### Pasos

1. Buscar `CONTEXTENTRIES`.
2. Verificar que actualmente sea 38 u otro valor.
3. Incrementarlo en 1 o más según necesidad.
4. Agregar offset para `mappings_head`.
5. Crear getter/setter.

Ejemplo conceptual:

```c
uint64_t* get_mappings_head(uint64_t* context);
void set_mappings_head(uint64_t* context, uint64_t* head);
```

### Inicialización

En `init_context()`:

```text
set_mappings_head(context, 0);
```

Si se usa tabla fija en vez de lista:

```text
reservar tabla de mappings
inicializar entradas como inactivas
guardar puntero en contexto
```

---

## FASE 3: Estructura de mapping_entry

### Diseño recomendado

Usar una entrada lineal de `uint64_t`, siguiendo el estilo de Selfie.

```text
MAPPING_ENTRY_SIZE = 8 o 9 words

Offset 0: next
Offset 1: addr
Offset 2: original_length
Offset 3: rounded_length
Offset 4: prot
Offset 5: fd/file_id
Offset 6: file_offset
Offset 7: active o flags
Offset 8: reservado, si se necesita
```

### Getters recomendados

```text
get_mapping_next(mapping)
get_mapping_addr(mapping)
get_mapping_original_length(mapping)
get_mapping_rounded_length(mapping)
get_mapping_prot(mapping)
get_mapping_fd(mapping)
get_mapping_offset(mapping)
get_mapping_active(mapping)
```

### Setters recomendados

```text
set_mapping_next(mapping, next)
set_mapping_addr(mapping, addr)
set_mapping_original_length(mapping, length)
set_mapping_rounded_length(mapping, length)
set_mapping_prot(mapping, prot)
set_mapping_fd(mapping, fd)
set_mapping_offset(mapping, offset)
set_mapping_active(mapping, active)
```

### Helpers recomendados

```text
create_mapping_entry(addr, original_length, rounded_length, prot, fd, offset)
find_mapping(context, addr)
find_mapping_for_address(context, vaddr)
add_mapping_to_context(context, mapping)
remove_mapping_from_context(context, addr)
is_address_inside_mapping(mapping, vaddr)
ranges_overlap(start1, len1, start2, len2)
```

---

## FASE 4: Page cache y cache frames

### Objetivo

Crear una estructura global compartida que relacione:

```text
(file id, file page offset) -> cache frame
```

### Diseño recomendado con lista enlazada

```text
PAGE_CACHE_ENTRY_SIZE = 7 words

Offset 0: next
Offset 1: file_id/fd
Offset 2: file_page_offset
Offset 3: cache_frame
Offset 4: modified/dirty
Offset 5: refcount
Offset 6: reserved/active
```

### Variables globales sugeridas

```c
uint64_t CACHEFRAMESIZE = 4096;
uint64_t NUMBEROFCACHEFRAMES = 1024;
uint64_t* cache_frames = (uint64_t*) 0;
uint64_t* page_cache_entries = (uint64_t*) 0;
uint64_t next_cache_frame_index = 0;
```

También se puede usar `PAGESIZE` directamente en lugar de `CACHEFRAMESIZE`.

### Funciones recomendadas

```text
init_page_cache()
allocate_cache_frame()
find_page_cache_entry(file_id, file_page_offset)
create_page_cache_entry(file_id, file_page_offset, frame)
find_or_create_cache_entry(file_id, file_page_offset)
load_file_page_into_cache_frame(fd, file_page_offset, frame)
write_cache_frame_to_file(fd, file_page_offset, frame, bytes)
mark_page_cache_entry_dirty_by_frame(frame)
increment_page_cache_refcount(entry)
decrement_page_cache_refcount(entry)
```

### Carga de página

Cuando no existe en cache:

1. Reservar frame.
2. Hacer seek al offset correcto.
3. Leer hasta `PAGESIZE`.
4. Rellenar con 0 si el archivo no tiene tantos bytes.
5. Crear entry.
6. Retornar frame.

### Persistencia

Solo en `msync`:

1. Buscar entry.
2. Hacer seek al offset correcto.
3. Escribir bytes desde frame.
4. Marcar clean si usa dirty bit.

---

## FASE 5: Implementar mmap

### Syscall

Agregar número:

```c
uint64_t SYSCALL_MMAP = 222;
```

Antes verificar que no exista conflicto.

### Argumentos

```text
REG_A0 = addr
REG_A1 = length
REG_A2 = prot
REG_A3 = fd
REG_A4 = offset
```

### Algoritmo

```text
1. Leer argumentos.
2. Validar length > 0.
3. Validar prot.
4. Validar offset.
5. Redondear length a PAGESIZE.
6. Si addr == 0:
      buscar región virtual libre.
   Si addr != 0:
      validar alineación y disponibilidad.
7. Crear mapping_entry.
8. Por cada página:
      file_page_offset = offset + i * PAGESIZE
      entry = find_or_create_cache_entry(fd, file_page_offset)
      frame = get_cache_frame(entry)
      virtual_page = get_page_of_virtual_address(addr + i * PAGESIZE)
      set_PTE_for_page(get_pt(context), virtual_page, frame)
      incrementar refcount si existe
9. Agregar mapping al contexto.
10. Retornar addr en REG_A0.
```

### Qué NO debe hacer

```text
No escribir al archivo.
No llamar msync.
No duplicar cache frame si ya existe.
```

---

## FASE 6: Implementar munmap

### Syscall

```c
uint64_t SYSCALL_MUNMAP = 223;
```

### Argumentos

```text
REG_A0 = addr
```

### Algoritmo

```text
1. Leer addr.
2. Buscar mapping exacto por addr.
3. Si no existe, retornar -1.
4. Recorrer páginas del mapping.
5. Invalidar PTEs asociadas.
6. Decrementar refcount si se implementó.
7. No escribir al archivo.
8. Remover mapping del contexto.
9. Retornar 0.
```

### Cuidado

No liberar cache frames si otros procesos/mappings los siguen usando.

---

## FASE 7: Implementar msync

### Syscall

```c
uint64_t SYSCALL_MSYNC = 224;
```

### Argumentos

```text
REG_A0 = addr
```

### Algoritmo

```text
1. Leer addr.
2. Buscar mapping exacto.
3. Si no existe, retornar -1.
4. pages = rounded_length / PAGESIZE.
5. Para cada página:
      file_page_offset = mapping.offset + i * PAGESIZE
      entry = find_page_cache_entry(fd, file_page_offset)
      frame = entry.cache_frame
      bytes_to_write = PAGESIZE
      si es última página:
          ajustar según original_length
      si no hay dirty bit o dirty == 1:
          seek al offset correcto
          write desde frame al archivo
          dirty = 0
6. Retornar 0.
```

### Cuidado con la última página

Si `original_length = 5000`, internamente se mapean 8192 bytes, pero `msync` no debería escribir 8192 bytes reales si solo correspondían 5000 del mapping.

---

## FASE 8: Dirty bit

### Problema

Para que `msync` sepa qué páginas cambiaron, conviene marcar dirty cuando se escribe.

### Opciones

1. **Simple:** `msync` escribe todas las páginas del mapping.
   - Más fácil.
   - Suficiente para proyecto.
   - No requiere detectar store.

2. **Mejor:** marcar dirty en `store_virtual_memory`.
   - Más correcto.
   - Requiere detectar si el frame pertenece al page cache.

### Recomendación

Primero implementar opción simple: `msync` escribe todas las páginas.  
Luego, si queda tiempo, agregar dirty bit.

---

## FASE 9: Traducción de direcciones

### Estrategia recomendada

Hacer que `mmap` cree PTEs que apunten directamente a cache frames.

Entonces:

```text
vaddr
→ page table
→ cache frame
→ load/store normal
```

### Ventaja

No hay que reescribir completamente `load_virtual_memory`.

### Qué revisar

Asegurar que:

- `set_PTE_for_page` puede apuntar a cache frames.
- `tlb` no distingue memoria normal de cache frames.
- `load_physical_memory` y `store_physical_memory` funcionan con esos punteros.
- `fork` no copie esas páginas como si fueran memoria normal.
- `munmap` pueda invalidar esas páginas.

---

## FASE 10: Implementar fork con mappings

### Problema

`fork` actual copia memoria del padre al hijo. Eso crea frames nuevos.

Para mappings, eso está mal.

### Solución

En `implement_fork`:

```text
1. Copiar memoria normal como antes.
2. Copiar metadata de mappings.
3. Para cada página de cada mapping:
      obtener frame del padre
      set_PTE_for_page(get_pt(child), page, frame)
      NO copiar contenido
      incrementar refcount si existe
4. Asegurar que hijo y padre apuntan al mismo cache frame.
```

### Pseudocódigo conceptual

```c
uint64_t* parent_mapping = get_mappings_head(context);

while (parent_mapping != (uint64_t*) 0) {
  uint64_t* child_mapping = create_mapping_entry(
    get_mapping_addr(parent_mapping),
    get_mapping_original_length(parent_mapping),
    get_mapping_rounded_length(parent_mapping),
    get_mapping_prot(parent_mapping),
    get_mapping_fd(parent_mapping),
    get_mapping_offset(parent_mapping)
  );

  add_mapping_to_context(child, child_mapping);

  i = 0;
  while (i < get_mapping_rounded_length(parent_mapping) / PAGESIZE) {
    vaddr = get_mapping_addr(parent_mapping) + i * PAGESIZE;
    page = get_page_of_virtual_address(vaddr);
    frame = get_frame_for_page(get_pt(context), page);

    set_PTE_for_page(get_pt(child), page, frame);

    i = i + 1;
  }

  parent_mapping = get_mapping_next(parent_mapping);
}
```

Codex debe adaptar este pseudocódigo al estilo exacto de `selfie.c`.

---

# PARTE 4: PROMPTS POR FASE

Estos prompts se pueden usar después del prompt maestro, cuando quieras avanzar poco a poco.

---

## Prompt 1: Analizar funciones clave

```text
Analiza `selfie.c` y explícame con nombres exactos de funciones:

1. Cómo funciona `load_virtual_memory`.
2. Cómo funciona `store_virtual_memory`.
3. Cómo funciona `copy_buffer`.
4. Cómo funcionan `implement_read` y `implement_write`.
5. Cómo funciona `implement_fork`.
6. Cómo se despachan syscalls en `handle_system_call`.

No modifiques código. Agrega tus conclusiones al archivo `PLAN_IMPLEMENTACION_MMAP.md`.
```

---

## Prompt 2: Extender contexto

```text
Ahora implementa solo la extensión del contexto para soportar mappings.

Requisitos:
1. Verifica el valor actual de `CONTEXTENTRIES`.
2. Agrega un campo nuevo para `mappings_head`.
3. Crea getter/setter siguiendo el estilo de `selfie.c`.
4. Inicializa `mappings_head` en `init_context`.
5. No implementes todavía mmap/munmap/msync.
6. Compila o deja instrucciones para compilar.

No modifiques otras partes.
```

---

## Prompt 3: Crear mapping_entry

```text
Implementa solo la estructura interna de `mapping_entry`.

Requisitos:
1. Usar arreglo lineal de `uint64_t`, no struct moderno si el estilo de Selfie no usa structs.
2. Campos:
   - next
   - addr
   - original_length
   - rounded_length
   - prot
   - fd/file_id
   - file_offset
   - active/flags
3. Crear getters y setters.
4. Crear:
   - create_mapping_entry
   - find_mapping
   - find_mapping_for_address
   - add_mapping_to_context
   - remove_mapping_from_context
5. No implementes syscalls todavía.
6. Mantén estilo C*/StarC.
```

---

## Prompt 4: Crear page cache

```text
Implementa solo el page cache y cache frames.

Requisitos:
1. Crear variables globales para cache frames y lista/tabla de page cache entries.
2. Crear `init_page_cache`.
3. Llamar `init_page_cache` desde la inicialización adecuada.
4. Crear `allocate_cache_frame`.
5. Crear page_cache_entry con campos:
   - next
   - file_id/fd
   - file_page_offset
   - cache_frame
   - modified/dirty
   - refcount
6. Crear getters y setters.
7. Crear:
   - find_page_cache_entry
   - create_page_cache_entry
   - find_or_create_cache_entry
8. Por ahora, si no sabes cómo leer archivo por offset, deja helper preparado y documenta qué falta.
9. No implementes mmap todavía.
```

---

## Prompt 5: Implementar mmap

```text
Implementa la syscall `mmap`.

Requisitos:
1. Definir `SYSCALL_MMAP` sin conflicto.
2. Agregar dispatch en `handle_system_call`.
3. Crear `implement_mmap`.
4. Leer argumentos desde REG_A0 a REG_A4.
5. Validar length, prot, addr y offset.
6. Redondear length a múltiplo de PAGESIZE.
7. Si addr == 0, buscar región libre.
8. Crear mapping_entry.
9. Para cada página:
   - obtener o crear page cache entry
   - obtener cache frame
   - crear PTE que apunte al cache frame
10. Agregar mapping al contexto.
11. Retornar dirección base o -1.
12. No escribir al archivo.
13. No tocar munmap/msync todavía.
```

---

## Prompt 6: Implementar munmap

```text
Implementa la syscall `munmap`.

Requisitos:
1. Definir `SYSCALL_MUNMAP`.
2. Agregar dispatch en `handle_system_call`.
3. Crear `implement_munmap`.
4. Buscar mapping por dirección exacta.
5. Si no existe, retornar -1.
6. Invalidar PTEs del rango.
7. Remover mapping del contexto.
8. No escribir al archivo.
9. No liberar cache frames compartidos sin refcount.
10. Retornar 0 en éxito.
```

---

## Prompt 7: Implementar msync

```text
Implementa la syscall `msync`.

Requisitos:
1. Definir `SYSCALL_MSYNC`.
2. Agregar dispatch en `handle_system_call`.
3. Crear `implement_msync`.
4. Buscar mapping por dirección exacta.
5. Recorrer páginas del mapping.
6. Para cada página:
   - localizar page cache entry
   - obtener cache frame
   - posicionar archivo en offset correcto
   - escribir bytes al archivo
7. Si original_length no llena la última página, ajustar bytes de escritura.
8. msync debe ser el único mecanismo de persistencia.
9. Retornar 0 o -1.
```

---

## Prompt 8: Integrar fork

```text
Modifica `implement_fork` para heredar mappings.

Requisitos:
1. El hijo debe copiar metadata de mappings del padre.
2. Las páginas mapeadas deben apuntar a los mismos cache frames.
3. No duplicar cache frames.
4. Memoria normal puede seguir copiándose como antes.
5. Si hay refcount, incrementarlo.
6. Agregar comentarios cortos explicando la diferencia entre páginas normales y file-backed.
7. Crear o actualizar test para demostrar que padre e hijo ven cambios compartidos.
```

---

## Prompt 9: Crear tests

```text
Crea tests para el Proyecto 2.

Tests mínimos:

1. test_mmap_read:
   - crea/abre archivo con contenido conocido
   - mmap
   - lee desde memoria mapeada
   - imprime PASS/FAIL

2. test_mmap_write_msync:
   - mmap con escritura
   - modifica memoria
   - msync
   - lee archivo
   - imprime PASS/FAIL

3. test_mmap_fork_shared:
   - mmap
   - fork
   - proceso hijo o padre escribe
   - el otro lee el cambio desde memoria mapeada
   - imprime PASS/FAIL

4. test_munmap_no_persist recomendado:
   - mmap
   - modificar memoria
   - munmap sin msync
   - verificar que archivo no cambió
   - imprime PASS/FAIL

Usa C* compatible con Selfie. No uses librerías no soportadas.
Incluye instrucciones de compilación y ejecución.
```

---

## Prompt 10: Crear README final

```text
Crea `README_P2_MEMORY_MAPPINGS.md`.

Debe incluir:
1. Resumen del proyecto.
2. Diseño de mappings.
3. Diseño de page cache.
4. Syscalls agregadas.
5. Funcionamiento de mmap.
6. Funcionamiento de munmap.
7. Funcionamiento de msync.
8. Cómo se maneja fork.
9. Tests implementados.
10. Comandos para compilar y ejecutar.
11. Salidas esperadas.
12. Limitaciones conocidas.
13. Cómo se cumple la rúbrica.
```

---

# PARTE 5: TESTS DETALLADOS

## Test 1: lectura desde memoria mapeada

### Objetivo

Demostrar que `mmap` permite leer contenido de archivo desde memoria.

### Flujo

```text
1. Crear archivo con "Hello Selfie mmap".
2. Abrir archivo.
3. addr = mmap(0, 4096, PROT_READ, fd, 0).
4. Leer caracteres desde addr.
5. Comparar con texto esperado.
6. Imprimir PASS/FAIL.
7. munmap(addr).
```

### Salida esperada

```text
TEST_MMAP_READ: PASS
READ_FROM_MMAP: Hello Selfie mmap
```

---

## Test 2: escritura y msync

### Objetivo

Demostrar que cambios en memoria mapeada persisten solo con `msync`.

### Flujo

```text
1. Crear archivo con "ABCDE".
2. mmap con lectura/escritura.
3. Escribir 'Z' en addr[0].
4. msync(addr).
5. Leer archivo.
6. Verificar que empieza con "Z".
7. Imprimir PASS/FAIL.
```

### Salida esperada

```text
TEST_MMAP_WRITE_MSYNC: PASS
AFTER_MSYNC: ZBCDE
```

---

## Test 3: munmap no persiste

### Objetivo

Demostrar que `munmap` no escribe cambios al archivo.

### Flujo

```text
1. Crear archivo con "ABCDE".
2. mmap con escritura.
3. Escribir 'X' en addr[0].
4. munmap(addr), sin msync.
5. Leer archivo.
6. Verificar que sigue "ABCDE".
7. Imprimir PASS/FAIL.
```

### Salida esperada

```text
TEST_MUNMAP_NO_PERSIST: PASS
FILE_AFTER_MUNMAP: ABCDE
```

---

## Test 4: fork compartiendo mapping

### Objetivo

Demostrar que padre e hijo comparten cache frame.

### Flujo

```text
1. Padre crea/abre archivo.
2. Padre hace mmap.
3. Padre hace fork.
4. Hijo escribe 'C' en addr[0].
5. Padre espera o sincroniza según posibilidades del proyecto.
6. Padre lee addr[0].
7. Padre debe ver 'C'.
8. Imprimir PASS/FAIL.
```

### Salida esperada

```text
CHILD_WROTE: C
PARENT_SEES: C
TEST_MMAP_FORK_SHARED: PASS
```

---

# PARTE 6: CHECKLIST FINAL

## Implementación

```text
[ ] Selfie compila.
[ ] `SYSCALL_MMAP` agregado.
[ ] `SYSCALL_MUNMAP` agregado.
[ ] `SYSCALL_MSYNC` agregado.
[ ] `handle_system_call` despacha las nuevas syscalls.
[ ] `implement_mmap` existe.
[ ] `implement_munmap` existe.
[ ] `implement_msync` existe.
[ ] El contexto tiene `mappings`.
[ ] `mappings` se inicializa en `init_context`.
[ ] Existe page cache.
[ ] Existen cache frames.
[ ] mmap registra mapping.
[ ] mmap conecta virtual pages con cache frames.
[ ] munmap elimina mapping sin persistir.
[ ] msync persiste cambios.
[ ] fork hereda mappings.
[ ] fork no duplica cache frames file-backed.
```

## Tests

```text
[ ] Test lectura mmap.
[ ] Test escritura + msync.
[ ] Test fork compartido.
[ ] Test munmap no persiste.
[ ] Cada test imprime PASS/FAIL.
[ ] Cada test tiene salida esperada documentada.
```

## Calidad

```text
[ ] No se rompió read.
[ ] No se rompió write.
[ ] No se rompió openat.
[ ] No se rompió fork.
[ ] No se rompió wait.
[ ] No se rompió exit.
[ ] No hay debug prints excesivos.
[ ] No hay cambios innecesarios.
[ ] README final existe.
```

---

# PARTE 7: GUÍA PARA PRESENTACIÓN DE 12 MINUTOS

## Estructura recomendada

### Slide 1: Título

Proyecto 2: Memory Mappings en Selfie  
Integrantes del grupo

### Slide 2: Problema

Explicar que normalmente se usa `read/write`, pero `mmap` permite acceder al archivo como memoria.

### Slide 3: Idea principal

```text
Archivo
→ page cache
→ cache frame
→ page table
→ dirección virtual del proceso
```

### Slide 4: mmap

Explicar:
- registra mapping
- crea/reutiliza cache frames
- actualiza page table
- no escribe al archivo

### Slide 5: munmap

Explicar:
- elimina mapping
- invalida relación virtual
- no persiste cambios

### Slide 6: msync

Explicar:
- recorre páginas
- escribe cache frames al archivo
- es el único mecanismo de persistencia

### Slide 7: fork

Explicar:
- hijo hereda mappings
- no se duplican cache frames
- padre e hijo comparten cambios en memoria mapeada

### Slide 8: Page cache

Mostrar tabla:

```text
file id | file page offset | cache frame | dirty | refcount
```

### Slide 9: Test lectura

Mostrar salida del test.

### Slide 10: Test escritura + msync

Mostrar salida del test.

### Slide 11: Test fork compartido

Mostrar salida del test.

### Slide 12: Conclusión

Mencionar:
- qué se logró
- limitaciones
- aprendizajes
- relación con Linux mmap real

---

# PARTE 8: Riesgos y cómo evitarlos

| Riesgo | Qué puede pasar | Solución |
|---|---|---|
| Usar `fd` como file id | Dos procesos con distinto fd al mismo archivo no comparten cache | Documentarlo o crear file id más estable si es viable |
| Liberar cache frame en munmap | Otro proceso puede seguir usándolo | Usar refcount o no liberar durante el proyecto |
| msync escribe bytes extra | Archivo queda con basura | Guardar original_length y ajustar última página |
| fork copia páginas mapeadas | Padre e hijo no comparten cambios | Detectar mappings y mapear mismo frame |
| Dirty bit no se marca | msync no escribe cambios | Al inicio escribir todas las páginas en msync |
| addr automático choca | Se corrompe memoria | Buscar región libre con validaciones |
| Se rompe read/write | Tests anteriores fallan | Ejecutar pruebas básicas después de cada fase |

---

# PARTE 9: Recomendación de estrategia

La estrategia más segura es:

```text
1. Crear PLAN_IMPLEMENTACION_MMAP.md.
2. Extender contexto.
3. Crear mappings.
4. Crear page cache.
5. Implementar mmap solo lectura.
6. Probar lectura.
7. Agregar escritura.
8. Agregar msync.
9. Agregar munmap.
10. Ajustar fork.
11. Crear tests finales.
12. Documentar.
```

No conviene pedirle a Codex que implemente todo de golpe sin plan.  
Lo mejor es usar el **prompt maestro** primero y luego avanzar con los **prompts por fase**.

---

# PARTE 10: Criterios finales de éxito

El proyecto estará bien encaminado si pueden demostrar:

```text
mmap crea mapping.
Se lee archivo desde memoria.
Se escribe en memoria mapeada.
munmap no persiste.
msync sí persiste.
fork comparte cache frames.
Tests imprimen PASS.
Selfie sigue compilando.
Presentación muestra demos claras.
```
