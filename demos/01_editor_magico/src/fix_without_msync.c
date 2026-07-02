uint64_t main() {
  uint64_t fd;
  uint64_t mapped_addr;
  uint64_t* ptr;
  uint64_t unmap_result;

  write(1, "FIX SIN MSYNC: START\n", 21);

  fd = open("demos/01_editor_magico/runtime/broken.c", 2, 0);
  if (fd < 0) {
    write(1, "OPEN: FAIL\n", 11);
    exit(-1);
  }
  write(1, "OPEN: OK\n", 9);

  mapped_addr = mmap(0, 8, 2, fd, 0);
  if (mapped_addr == 0) {
    write(1, "MMAP: FAIL\n", 11);
    exit(-1);
  }
  write(1, "MMAP: OK\n", 9);

  ptr = (uint64_t*) mapped_addr;
  *ptr = 8385478439673424245;
  write(1, "WRITE: uint64_t EN MEMORIA\n", 27);

  unmap_result = munmap(mapped_addr);
  if (unmap_result != 0) {
    write(1, "MUNMAP: FAIL\n", 13);
    exit(-1);
  }

  write(1, "MSYNC: NO USADO\n", 16);
  write(1, "FIX SIN MSYNC: END\n", 19);
  exit(0);
}
