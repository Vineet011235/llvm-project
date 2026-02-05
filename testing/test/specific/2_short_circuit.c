#include <stdio.h>
#include <stdbool.h>

// Test 1: AND short-circuit evaluation
int safe_divide_and(int a, int b) {
    // b != 0 must be evaluated first, second condition only if first is true
    if (b != 0 && a / b > 10) {
        printf("Result is greater than 10\n");
        return a / b;
    }
    return 0;
}

// Test 2: OR short-circuit evaluation
bool is_invalid_or(int value, int max) {
    // If first condition is true, second is not evaluated
    if (value < 0 || value > max) {
        return true;
    }
    return false;
}

// Test 3: Complex short-circuit with multiple conditions
int complex_and_or(int a, int b, int c) {
    // Tests: (A && B) || (C && D)
    if ((a > 0 && b > 0) || (c < 0 && a + b < c)) {
        return 1;
    }
    return 0;
}

// Test 4: Nested short-circuit conditions
bool validate_range(int value, int min, int max) {
    // First check bounds, then check specific conditions
    if (min < max && value >= min && value <= max) {
        if (value % 2 == 0 || value > 50) {
            return true;
        }
    }
    return false;
}

// Test 5: Short-circuit with function calls
int expensive_check() {
    printf("Expensive check called\n");
    return 1;
}

int cheap_check() {
    printf("Cheap check called\n");
    return 0;
}

void test_lazy_evaluation() {
    printf("\n=== Testing lazy evaluation ===\n");
    
    // cheap_check returns 0, so expensive_check should NOT be called
    if (cheap_check() && expensive_check()) {
        printf("Both conditions true\n");
    } else {
        printf("Short-circuited - expensive_check not called\n");
    }
    
    // cheap_check returns 0 (false), but with OR, expensive_check WILL be called
    if (cheap_check() || expensive_check()) {
        printf("At least one condition true\n");
    }
}

// Test 6: Short-circuit preventing null pointer dereference
void safe_pointer_access(int *ptr) {
    // ptr != NULL must be checked first to prevent crash
    if (ptr != NULL && *ptr > 0) {
        printf("Pointer value: %d\n", *ptr);
    } else {
        printf("Null pointer or non-positive value\n");
    }
}

int main() {
    printf("=== Test 2: Short-Circuit Evaluation ===\n\n");
    
    printf("Safe divide 100/5: %d\n", safe_divide_and(100, 5));
    printf("Safe divide 50/10: %d\n", safe_divide_and(50, 10));
    printf("Safe divide 10/0: %d\n", safe_divide_and(10, 0));
    
    printf("\nIs -5 invalid (max=100)? %d\n", is_invalid_or(-5, 100));
    printf("Is 50 invalid (max=100)? %d\n", is_invalid_or(50, 100));
    printf("Is 150 invalid (max=100)? %d\n", is_invalid_or(150, 100));
    
    printf("\nComplex AND/OR (5, 10, -3): %d\n", complex_and_or(5, 10, -3));
    printf("Complex AND/OR (-5, 10, -3): %d\n", complex_and_or(-5, 10, -3));
    
    printf("\nValidate range 30 [0-100]: %d\n", validate_range(30, 0, 100));
    printf("Validate range 60 [0-100]: %d\n", validate_range(60, 0, 100));
    
    test_lazy_evaluation();
    
    int value = 42;
    safe_pointer_access(&value);
    safe_pointer_access(NULL);
    
    return 0;
}
