uint64_t main() {
  uint64_t fd1;
  uint64_t fd2;
  uint64_t map1;
  uint64_t map2;
  uint64_t* ptr1;
  uint64_t* ptr2;
  uint64_t value;

  value = 1234605616436508552;

  fd1 = open("artifacts/tests/runtime/mmap-fixture.txt", 2, 0);
  if (fd1 < 0) {
    exit(-1);
  }

  fd2 = open("artifacts/tests/runtime/mmap-fixture.txt", 2, 0);
  if (fd2 < 0) {
    exit(-1);
  }

  map1 = mmap(0, 4096, 2, fd1, 0);
  if (map1 == 0) {
    exit(-1);
  }

  map2 = mmap(0, 4096, 2, fd2, 0);
  if (map2 == 0) {
    munmap(map1);
    exit(-1);
  }

  ptr1 = (uint64_t*) (map1 + 512);
  ptr2 = (uint64_t*) (map2 + 512);

  *ptr1 = value;

  if (*ptr2 != value) {
    munmap(map2);
    munmap(map1);
    exit(-1);
  }

  munmap(map2);
  munmap(map1);

  exit(0);
}
