int nested_loop(int n) {
  int acc = 0;
  for (int i = 0; i < n; ++i) {
    for (int j = 0; j < 3; ++j)
      acc = acc + i + j;
  }
  return acc;
}
