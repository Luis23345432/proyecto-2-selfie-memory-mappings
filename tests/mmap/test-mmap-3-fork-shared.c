uint64_t main() {
  uint64_t fd;
  uint64_t mapped_addr;
  uint64_t* ptr;
  uint64_t pid;
  uint64_t* status_ptr;
  uint64_t sync_result;
  uint64_t unmap_result;

  fd = open("artifacts/tests/runtime/mmap-fixture.txt", 2, 0);
  if (fd < 0) {
    exit(-1);
  }

  mapped_addr = mmap(0, 4096, 2, fd, 0);
  if (mapped_addr == 0) {
    exit(-1);
  }

  ptr = (uint64_t*) (mapped_addr + 256);
  *ptr = 3405691582;

  pid = fork();
  if (pid == 0) {
    ptr = (uint64_t*) (mapped_addr + 256);
    if (*ptr != 3405691582) {
      munmap(mapped_addr);
      exit(-1);
    }

    *ptr = 3735928559;
    sync_result = msync(mapped_addr);
    if (sync_result != 0) {
      munmap(mapped_addr);
      exit(-1);
    }

    unmap_result = munmap(mapped_addr);
    if (unmap_result != 0) {
      exit(-1);
    }

    exit(0);
  }

  status_ptr = malloc(8);
  wait(status_ptr);

  ptr = (uint64_t*) (mapped_addr + 256);
  if (*ptr != 3735928559) {
    munmap(mapped_addr);
    exit(-1);
  }

  sync_result = msync(mapped_addr);
  if (sync_result != 0) {
    munmap(mapped_addr);
    exit(-1);
  }

  unmap_result = munmap(mapped_addr);
  if (unmap_result != 0) {
    exit(-1);
  }

  exit(0);
}
