uint64_t main() {
  uint64_t fd;
  uint64_t mapped_addr;
  uint64_t* ptr;
  uint64_t value;
  uint64_t expected_header;
  uint64_t unmap_result;

  write(1, "TEST1V: START\n", 14);

  fd = open("artifacts/tests/runtime/mmap-fixture.txt", 0, 0);
  if (fd < 0) {
    write(1, "OPEN:   FAIL\n", 13);
    exit(-1);
  }
  write(1, "OPEN:   OK\n", 11);

  mapped_addr = mmap(0, 4096, 0, fd, 0);
  if (mapped_addr == 0) {
    write(1, "MMAP:   FAIL\n", 13);
    exit(-1);
  }
  write(1, "MMAP:   OK\n", 11);

  ptr = (uint64_t*) mapped_addr;
  value = *ptr;
  if (value == 0) {
    write(1, "READ:   FAIL\n", 13);
    munmap(mapped_addr);
    exit(-1);
  }
  write(1, "READ:   OK\n", 11);

  expected_header = 4707459272169049421;
  if (value == expected_header) {
    write(1, "HEADER: MATCH\n", 14);
  } else {
    write(1, "HEADER: FAIL\n", 13);
    munmap(mapped_addr);
    exit(-1);
  }

  unmap_result = munmap(mapped_addr);
  if (unmap_result != 0) {
    write(1, "UNMAP:  FAIL\n", 13);
    exit(-1);
  }
  write(1, "UNMAP:  OK\n", 11);

  write(1, "TEST1V: PASS\n", 13);
  exit(0);
}
