// example.c
#include <stdio.h>

// Iterative fibonacci - O(n) time, O(1) space
int fibonacci(int n) {
    if (n <= 1)
        return n;
    
    int prev = 0, curr = 1;
    for (int i = 2; i <= n; i++) {
        int next = prev + curr;
        prev = curr;
        curr = next;
    }
    return curr;
}

int main() {
    int test_values[] = {0, 1, 5, 10, 15, 20};
    int num_tests = sizeof(test_values) / sizeof(test_values[0]);
    
    printf("Fibonacci Results:\n");
    for (int i = 0; i < num_tests; i++) {
        int n = test_values[i];
        printf("fibonacci(%d) = %d\n", n, fibonacci(n));
    }
    
    return 0;
}
