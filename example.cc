int main(int argc, char **argv) {
  int x = argc;
  int y = 0;
  if (x > 5) {
    y = x + 2;
  } else {
    y = x - 2;
  }
  y += 1;
  return y ^ (argv != nullptr);
}
