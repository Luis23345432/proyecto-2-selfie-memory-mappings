uint64_t main() {
  uint64_t fd1;
  uint64_t fd2;
  uint64_t map1;
  uint64_t map2;
  uint64_t* ptr1;
  uint64_t* ptr2;
  uint64_t pid;
  uint64_t* status_ptr;
  uint64_t sync_result;

  write(1, "DEMO3: NOTA VIVA START\n", 23);

  fd1 = open("demos/03_sistema_archivos_viviente/runtime/live-note.txt", 2, 0);
  if (fd1 < 0) {
    write(1, "OPEN1: FAIL\n", 12);
    exit(-1);
  }

  fd2 = open("demos/03_sistema_archivos_viviente/runtime/live-note.txt", 2, 0);
  if (fd2 < 0) {
    write(1, "OPEN2: FAIL\n", 12);
    exit(-1);
  }

  map1 = mmap(0, 8, 2, fd1, 0);
  if (map1 == 0) {
    write(1, "MMAP1: FAIL\n", 12);
    exit(-1);
  }

  map2 = mmap(0, 8, 2, fd2, 0);
  if (map2 == 0) {
    write(1, "MMAP2: FAIL\n", 12);
    munmap(map1);
    exit(-1);
  }

  ptr1 = (uint64_t*) map1;
  ptr2 = (uint64_t*) map2;

  *ptr1 = 3544385981099101249;
  write(1, "MAP1: ESCRIBE ALIVE001\n", 23);

  if (*ptr2 != 3544385981099101249) {
    write(1, "MAP2: NO VIO MAP1\n", 18);
    munmap(map2);
    munmap(map1);
    exit(-1);
  }
  write(1, "MAP2: VIO ALIVE001\n", 19);

  pid = fork();
  if (pid == 0) {
    ptr1 = (uint64_t*) map1;
    if (*ptr1 != 3544385981099101249) {
      write(1, "HIJO: NO VIO ALIVE001\n", 22);
      munmap(map2);
      munmap(map1);
      exit(-1);
    }

    *ptr1 = 3616443575137029185;
    write(1, "HIJO: ESCRIBE ALIVE002\n", 23);
    munmap(map2);
    munmap(map1);
    exit(0);
  }

  status_ptr = malloc(8);
  wait(status_ptr);

  ptr2 = (uint64_t*) map2;
  if (*ptr2 != 3616443575137029185) {
    write(1, "PADRE: NO VIO ALIVE002\n", 23);
    munmap(map2);
    munmap(map1);
    exit(-1);
  }
  write(1, "PADRE: VIO ALIVE002\n", 20);

  sync_result = msync(map1);
  if (sync_result != 0) {
    write(1, "MSYNC: FAIL\n", 12);
    munmap(map2);
    munmap(map1);
    exit(-1);
  }
  write(1, "MSYNC: OK\n", 10);

  munmap(map2);
  munmap(map1);

  write(1, "DEMO3: PASS\n", 12);
  exit(0);
}
