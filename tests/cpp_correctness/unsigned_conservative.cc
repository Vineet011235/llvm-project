int main(int argc, char **argv) {
  unsigned x = (unsigned)argc;
  int y = 0;
  if (x < 8u) {
    y = (int)x + 1;
  } else {
    y = (int)x - 1;
  }
  return y ^ (argv != nullptr);
}
