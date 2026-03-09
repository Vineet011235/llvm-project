# Live Variables Analysis Results

- **Test:** `loop-test.c`
- **File:** `test/simple/loop-test.c`
- **Generated:** 2026-03-09 11:58:13

---

## Source Code

```cpp
// Simple loop test for dataflow analysis
int sum_array(int arr[], int n) {
    int sum = 0;
    int i;
    
    for (i = 0; i < n; i++) {
        sum += arr[i];
    }
    
    return sum;
}

int main() {
    int numbers[] = {1, 2, 3, 4, 5};
    int result = sum_array(numbers, 5);
    return result;
}

```

---

## Analysis Output

```
--- Running Live Variable Analysis ---
Live Variable Results for function: sum_array
Basic Block: %entry
  GEN : %arr, %n
  KILL: %arr.addr, %n.addr, %sum, %i
  IN  : %arr, %n
  OUT : %arr.addr, %n.addr, %sum, %i

Basic Block: %for.cond
  GEN : %n.addr, %i
  KILL: %0, %1, %cmp
  IN  : %arr.addr, %n.addr, %sum, %i
  OUT : %arr.addr, %n.addr, %sum, %i

Basic Block: %for.body
  GEN : %arr.addr, %sum, %i
  KILL: %2, %3, %idxprom, %arrayidx, %4, %5, %add
  IN  : %arr.addr, %n.addr, %sum, %i
  OUT : %arr.addr, %n.addr, %sum, %i

Basic Block: %for.inc
  GEN : %i
  KILL: %6, %inc
  IN  : %arr.addr, %n.addr, %sum, %i
  OUT : %arr.addr, %n.addr, %sum, %i

Basic Block: %for.end
  GEN : %sum
  KILL: %7
  IN  : %sum
  OUT : 

--- Running Live Variable Analysis ---
Live Variable Results for function: main
Basic Block: %entry
  GEN : 
  KILL: %retval, %numbers, %result, %arraydecay, %call, %0
  IN  : 
  OUT : 

```
