# Anticipable Expressions Analysis Results

- **Test:** `multiple-paths.c`
- **File:** `test/simple/multiple-paths.c`
- **Generated:** 2026-03-09 11:58:14

---

## Source Code

```cpp
// Function with multiple return paths
int calculate(int x, int y, int mode) {
    int result;
    
    if (mode == 0) {
        result = x + y;
    } else if (mode == 1) {
        result = x - y;
    } else if (mode == 2) {
        result = x * y;
    } else {
        result = 0;
    }
    
    return result;
}

int main() {
    int a = calculate(10, 5, 0);
    int b = calculate(10, 5, 1);
    int c = calculate(10, 5, 2);
    
    return a + b + c;
}

```

---

## Analysis Output

```
--- Running Anticipable Expressions ---
--- Running Anticipable Expressions ---
```
