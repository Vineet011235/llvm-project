int main(int argc, char **argv) {
  int x = 0;
  if (argc > 2) {
    x = 4;
  } else if (argc > 1) {
    x = 6;
  } else {
    x = 8;
  }
  int y = x + 50;
  return x;
}