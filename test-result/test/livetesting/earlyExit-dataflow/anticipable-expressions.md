# Anticipable Expressions Analysis Results

- **Test:** `earlyExit.cc`
- **File:** `test/livetesting/earlyExit.cc`
- **Generated:** 2026-03-10 13:52:05

---

## Source Code

```cpp
int early_exit_test(int condition, int x, int y) {
    int result = 0;
    
    if (condition < 0) {
        // EXIT 1: 'y' is completely dead on this path.
        // The OUT set of this block must be perfectly empty.
        return -1; 
    }
    
    // 'y' is only live if the condition was >= 0.
    if (condition == 0) {
        result = x + 10;
    } else {
        result = x * y; // 'y' is GEN'd here
    }
    
    // EXIT 2
    return result; 
}

int main(){
    int result = early_exit_test(-5, 10, 20);
    int x = result;
    return 0;
}
```

---

## Analysis Output

```
```
