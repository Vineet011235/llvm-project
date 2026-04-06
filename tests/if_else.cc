int main(int argc, char **argv) {
  int x = 0;
  if (argc > 1) {
    x = argc + 100;
  } else {
    x = argc - 200;
  }
  int y = x + 50;
  return 0;
}