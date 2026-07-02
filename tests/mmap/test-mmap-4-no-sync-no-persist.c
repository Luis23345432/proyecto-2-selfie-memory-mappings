uint64_t main() {
  uint64_t fd;
  uint64_t mapped_addr;
  uint64_t mapped_addr_2;
  uint64_t* ptr;
  uint64_t original;
  uint64_t unmap_result;

  fd = open("artifacts/tests/runtime/mmap-fixture.txt", 0, 0);
  if (fd < 0) {
    exit(-1);
  }

  mapped_addr = mmap(0, 4096, 0, fd, 0);
  if (mapped_addr == 0) {
    exit(-1);
  }

  ptr = (uint64_t*) (mapped_addr + 256);
  original = *ptr;
  unmap_result = munmap(mapped_addr);
  if (unmap_result != 0) {
    exit(-1);
  }

  fd = open("artifacts/tests/runtime/mmap-fixture.txt", 2, 0);
  mapped_addr = mmap(0, 4096, 2, fd, 0);
  if (mapped_addr == 0) {
    exit(-1);
  }

  ptr = (uint64_t*) (mapped_addr + 256);
  *ptr = 3735928559;
  if (*ptr != 3735928559) {
    munmap(mapped_addr);
    exit(-1);
  }

  unmap_result = munmap(mapped_addr);
  if (unmap_result != 0) {
    exit(-1);
  }

  fd = open("artifacts/tests/runtime/mmap-fixture.txt", 0, 0);
  mapped_addr_2 = mmap(0, 4096, 0, fd, 0);
  if (mapped_addr_2 == 0) {
    exit(-1);
  }

  ptr = (uint64_t*) (mapped_addr_2 + 256);
  if (*ptr == original) {
    munmap(mapped_addr_2);
    exit(0);
  }

  munmap(mapped_addr_2);
  exit(-1);
}
