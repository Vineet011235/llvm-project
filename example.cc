#include <cstdint>

int clamp_shift(int x) {
  int bias = x + 5;
  int scaled = bias * 3;
  if (scaled > 100)
    return scaled - 100;
  return scaled + 7;
}

int main() {
  return clamp_shift(10);
}
