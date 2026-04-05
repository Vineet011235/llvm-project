int main(int argc, char **argv) {
  int x = argc;
  int y = 0;
  if (7 > x) {
    y = x + 2;
  } else {
    y = x - 2;
  }
  return y ^ (argv != nullptr);
}
