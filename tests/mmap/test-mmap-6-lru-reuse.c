uint64_t main() {
  uint64_t fd;
  uint64_t i;
  uint64_t mapped_addr;
  uint64_t unmap_result;

  fd = open("artifacts/tests/runtime/mmap-fixture.txt", 0, 0);
  if (fd < 0) {
    exit(-1);
  }

  i = 0;
  while (i < 260) {
    mapped_addr = mmap(0, 4096, 0, fd, i * 4096);
    if (mapped_addr == 0) {
      exit(-1);
    }

    unmap_result = munmap(mapped_addr);
    if (unmap_result != 0) {
      exit(-1);
    }

    i = i + 1;
  }

  exit(0);
}
