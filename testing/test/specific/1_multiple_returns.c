#include <stdio.h>

// Test 1: Early return in conditional
int find_first_positive(int arr[], int size) {
    if (size <= 0) {
        return -1;  // Early return
    }
    
    for (int i = 0; i < size; i++) {
        if (arr[i] > 0) {
            return i;  // Return from loop
        }
    }
    
    return -1;  // Default return
}

// Test 2: Multiple returns in nested conditions
int classify_number(int n) {
    if (n < 0) {
        return -1;  // Negative
    }
    
    if (n == 0) {
        return 0;  // Zero
    }
    
    if (n > 100) {
        return 2;  // Large
    }
    
    return 1;  // Small positive
}

// Test 3: Return in switch cases
int days_in_month(int month) {
    switch (month) {
        case 1: case 3: case 5: case 7: case 8: case 10: case 12:
            return 31;
        case 4: case 6: case 9: case 11:
            return 30;
        case 2:
            return 28;  // Simplified, ignoring leap years
        default:
            return -1;  // Invalid month
    }
}

// Test 4: Guard clauses pattern
int divide_safe(int a, int b) {
    if (b == 0) {
        printf("Error: Division by zero\n");
        return 0;
    }
    
    if (a == 0) {
        return 0;
    }
    
    if (a < 0 && b < 0) {
        return (-a) / (-b);
    }
    
    return a / b;
}

int main() {
    int arr[] = {-5, -3, 0, 4, 7};
    
    printf("=== Test 1: Multiple Returns ===\n\n");
    
    printf("First positive index: %d\n", find_first_positive(arr, 5));
    printf("Empty array: %d\n", find_first_positive(arr, 0));
    
    printf("\nClassify 50: %d\n", classify_number(50));
    printf("Classify -10: %d\n", classify_number(-10));
    printf("Classify 0: %d\n", classify_number(0));
    printf("Classify 150: %d\n", classify_number(150));
    
    printf("\nDays in January: %d\n", days_in_month(1));
    printf("Days in February: %d\n", days_in_month(2));
    printf("Days in April: %d\n", days_in_month(4));
    
    printf("\nDivide 10/2: %d\n", divide_safe(10, 2));
    printf("Divide 10/0: %d\n", divide_safe(10, 0));
    
    return 0;
}
