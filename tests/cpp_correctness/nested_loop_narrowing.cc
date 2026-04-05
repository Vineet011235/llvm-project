int main() {
  int acc = 0;
  for (int i = 0; i < 3; ++i) {
    for (int j = 0; j < 2; ++j) {
      acc += i + j;
    }
  }
  return acc;
}
