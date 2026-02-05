#include <stdio.h>

// Function to calculate nth Fibonacci number using recursion
int fibonacci_recursive(int n) {
    if (n <= 1) {
        return n;
    }
    return fibonacci_recursive(n - 1) + fibonacci_recursive(n - 2);
}

// Function to calculate nth Fibonacci number using iteration (more efficient)
int fibonacci_iterative(int n) {
    if (n <= 1) {
        return n;
    }
    
    int prev = 0, curr = 1, next;
    
    for (int i = 2; i <= n; i++) {
        next = prev + curr;
        prev = curr;
        curr = next;
    }
    
    return curr;
}

// Function to print Fibonacci series up to n terms
void print_fibonacci_series(int n) {
    printf("Fibonacci series (first %d terms): ", n);
    for (int i = 0; i < n; i++) {
        printf("%d ", fibonacci_iterative(i));
    }
    printf("\n");
}

int main() {
    int n;
    
    printf("=== Fibonacci Calculator ===\n");
    printf("Enter the number of terms: ");
    scanf("%d", &n);
    
    if (n < 0) {
        printf("Please enter a non-negative number.\n");
        return 1;
    }
    
    // Print the series
    print_fibonacci_series(n);
    
    // Calculate a specific term
    if (n > 0) {
        printf("\nThe %dth Fibonacci number is: %d\n", n - 1, fibonacci_iterative(n - 1));
    }
    
    return 0;
}