# Como usar el HTML educativo

Archivo principal:

```bash
explicacion_memory_mappings_selfie.html
```

Para abrirlo, haz doble clic en el archivo desde el explorador de archivos o usa el navegador:

```bash
firefox explicacion_memory_mappings_selfie.html
```

Tambien puedes abrirlo con Chrome/Chromium si lo tienes instalado.

## Orden recomendado para estudiar

1. Lee primero la portada, conceptos clave y arquitectura general.
2. Luego estudia las estructuras: PCB/contexto, `mapping_entry`, page cache, cache frames y page table.
3. Despues repasa los flujos en este orden: `mmap`, lectura, escritura, `msync`, `munmap`.
4. Dedica tiempo especial a la seccion de fork, porque ahi se entiende por que el cache frame es compartido.
5. Usa la seccion de tests para preparar la exposicion: cada test demuestra una propiedad concreta.

## Para preparar la exposicion

- Explica primero la frase final del HTML: `mmap` crea la relacion, page cache guarda las paginas, page table permite accederlas, fork las comparte y `msync` las persiste.
- Usa el Test 4 para aclarar que `munmap` no guarda cambios.
- Usa el Test 3 para aclarar que fork no duplica cache frames de mappings.
- Menciona la simplificacion importante: el proyecto usa `fd` como `file_id`, mientras Linux real usa una identidad de archivo mas robusta.
