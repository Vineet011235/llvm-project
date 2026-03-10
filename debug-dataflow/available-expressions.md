# Available Expressions Analysis Results

- **Test:** `debug.cc`
- **File:** `debug.cc`
- **Generated:** 2026-03-10 14:26:12

---

## Source Code

```cpp
#include <iostream>

int foo(int a, int b, bool cond) {
    int x;

    if (cond) {
        x = a + 1;
    } else {
        x = b + 1;
    }

    return x;
}

int main() {
    int a = 5, b = 10;
    bool cond;
    std::cin >> cond;
    return foo(a, b, cond);
}

```

---

## Analysis Output

```
```
