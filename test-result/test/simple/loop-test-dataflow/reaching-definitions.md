# Reaching Definitions Analysis Results

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
--- Running Reaching Definitions ---
--- Running Reaching Definitions ---
```
