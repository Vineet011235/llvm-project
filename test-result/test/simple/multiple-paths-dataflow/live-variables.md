# Live Variables Analysis Results

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
--- Running Live Variable Analysis ---
Live Variable Results for function: calculate
Basic Block: %entry
  GEN : %x, %y, %mode
  KILL: %x.addr, %y.addr, %mode.addr, %result, %0, %cmp
  IN  : %x, %y, %mode
  OUT : %x.addr, %y.addr, %mode.addr, %result

Basic Block: %if.then
  GEN : %x.addr, %y.addr, %result
  KILL: %1, %2, %add
  IN  : %x.addr, %y.addr, %result
  OUT : %result

Basic Block: %if.else
  GEN : %mode.addr
  KILL: %3, %cmp1
  IN  : %x.addr, %y.addr, %mode.addr, %result
  OUT : %x.addr, %y.addr, %mode.addr, %result

Basic Block: %if.then2
  GEN : %x.addr, %y.addr, %result
  KILL: %4, %5, %sub
  IN  : %x.addr, %y.addr, %result
  OUT : %result

Basic Block: %if.else3
  GEN : %mode.addr
  KILL: %6, %cmp4
  IN  : %x.addr, %y.addr, %mode.addr, %result
  OUT : %x.addr, %y.addr, %result

Basic Block: %if.then5
  GEN : %x.addr, %y.addr, %result
  KILL: %7, %8, %mul
  IN  : %x.addr, %y.addr, %result
  OUT : %result

Basic Block: %if.else6
  GEN : %result
  KILL: 
  IN  : %result
  OUT : %result

Basic Block: %if.end
  GEN : 
  KILL: 
  IN  : %result
  OUT : %result

Basic Block: %if.end7
  GEN : 
  KILL: 
  IN  : %result
  OUT : %result

Basic Block: %if.end8
  GEN : %result
  KILL: %9
  IN  : %result
  OUT : 

--- Running Live Variable Analysis ---
Live Variable Results for function: main
Basic Block: %entry
  GEN : 
  KILL: %retval, %a, %b, %c, %call, %call1, %call2, %0, %1, %add, %2, %add3
  IN  : 
  OUT : 

```
