uint64_t main() {
  uint64_t fd;
  uint64_t mapped_addr;
  uint64_t* ptr;
  uint64_t pid;
  uint64_t* status_ptr;
  uint64_t sync_result;
  uint64_t unmap_result;

  write(1, "DEMO2: PADRE CREA MMAP\n", 23);

  fd = open("demos/02_pizarra_fork/runtime/shared-board.txt", 2, 0);
  if (fd < 0) {
    write(1, "OPEN: FAIL\n", 11);
    exit(-1);
  }

  mapped_addr = mmap(0, 8, 2, fd, 0);
  if (mapped_addr == 0) {
    write(1, "MMAP: FAIL\n", 11);
    exit(-1);
  }
  write(1, "MMAP: OK\n", 9);

  ptr = (uint64_t*) mapped_addr;
  *ptr = 2387281972630274384;
  write(1, "PADRE: ESCRIBE PARENT!!\n", 24);

  pid = fork();
  if (pid == 0) {
    ptr = (uint64_t*) mapped_addr;
    if (*ptr != 2387281972630274384) {
      write(1, "HIJO: NO VIO AL PADRE\n", 22);
      munmap(mapped_addr);
      exit(-1);
    }
    write(1, "HIJO: VIO PARENT!!\n", 19);

    *ptr = 2387225854704437315;
    write(1, "HIJO: ESCRIBE CHILD!!!\n", 23);

    unmap_result = munmap(mapped_addr);
    if (unmap_result != 0) {
      exit(-1);
    }

    exit(0);
  }

  status_ptr = malloc(8);
  wait(status_ptr);

  ptr = (uint64_t*) mapped_addr;
  if (*ptr != 2387225854704437315) {
    write(1, "PADRE: NO VIO AL HIJO\n", 22);
    munmap(mapped_addr);
    exit(-1);
  }
  write(1, "PADRE: VIO CHILD!!!\n", 20);

  sync_result = msync(mapped_addr);
  if (sync_result != 0) {
    write(1, "MSYNC: FAIL\n", 12);
    munmap(mapped_addr);
    exit(-1);
  }
  write(1, "MSYNC: OK\n", 10);

  unmap_result = munmap(mapped_addr);
  if (unmap_result != 0) {
    exit(-1);
  }
  write(1, "MUNMAP: OK\n", 11);

  write(1, "DEMO2: PASS\n", 12);
  exit(0);
}
