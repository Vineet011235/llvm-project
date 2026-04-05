int main(int argc, char **argv) {
  int x = argc;
  int y = 0;
  if (x < 10 && x > 0) {
    y = x + 1;
  } else {
    y = x - 1;
  }
  return y ^ (argv != nullptr);
}
