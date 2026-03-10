# Anticipable Expressions Analysis Results

- **Test:** `loop.cc`
- **File:** `test/livetesting/loop.cc`
- **Generated:** 2026-03-10 13:52:05

---

## Source Code

```cpp

int loop_stress_test(int a, int b) {
    int sum = a;     // 'a' is dead after this, 'sum' becomes live
    int i = 0;       // 'i' becomes live
    
    // The Loop Header: IN set must eventually contain 'sum' and 'i' and 'b'
    while (i < b) {  
        sum = sum + i; // 'sum' is both GEN and KILL
        i++;           // 'i' is both GEN and KILL
    }
    
    // 'b' and 'i' are now dead. Only 'sum' is live.
    return sum; 
}

int main(){
    int result = loop_stress_test(5, 10);
    return 0;
}
```

---

## Analysis Output

```
```
