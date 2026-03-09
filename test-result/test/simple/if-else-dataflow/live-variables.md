# Live Variables Analysis Results

- **Test:** `if-else.cc`
- **File:** `test/simple/if-else.cc`
- **Generated:** 2026-03-09 11:58:13

---

## Source Code

```cpp

int main() {
    int x = 10;
    int y;
    if (x > 5) {
        y = x * 2;
    } else {
        y = 10 * x;
    }
    return 0;
}
```

---

## Analysis Output

```
--- Running Live Variable Analysis ---
Live Variable Results for function: main
Basic Block: %entry
  GEN : 
  KILL: %retval, %x, %y, %0, %cmp
  IN  : 
  OUT : %x, %y

Basic Block: %if.then
  GEN : %x, %y
  KILL: %1, %mul
  IN  : %x, %y
  OUT : 

Basic Block: %if.else
  GEN : %x, %y
  KILL: %2, %mul1
  IN  : %x, %y
  OUT : 

Basic Block: %if.end
  GEN : 
  KILL: 
  IN  : 
  OUT : 

```
