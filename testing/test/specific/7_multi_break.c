#include <stdio.h>
#include <stdbool.h>

// Test 1: Loop with multiple break conditions
int find_value_multi_break(int arr[], int size, int target) {
    for (int i = 0; i < size; i++) {
        if (arr[i] == target) {
            printf("Found at index %d\n", i);
            return i;  // Break 1
        }
        
        if (arr[i] < 0) {
            printf("Encountered negative at index %d, stopping\n", i);
            return -1;  // Break 2
        }
        
        if (i > 100) {
            printf("Too many iterations, giving up\n");
            return -2;  // Break 3
        }
    }
    
    printf("Not found\n");
    return -3;
}

// Test 2: While loop with multiple exit points
int process_with_breaks(int start, int limit) {
    int value = start;
    int count = 0;
    
    while (value < limit) {
        count++;
        
        if (value % 13 == 0) {
            printf("Unlucky number %d encountered\n", value);
            break;  // Break 1
        }
        
        if (value > 1000) {
            printf("Value exceeded 1000\n");
            break;  // Break 2
        }
        
        if (count > 50) {
            printf("Too many iterations\n");
            break;  // Break 3
        }
        
        value += 7;
    }
    
    return value;
}

// Test 3: Nested loops with multiple breaks
void nested_loop_multi_break(int rows, int cols) {
    bool found = false;
    
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            printf("(%d,%d) ", i, j);
            
            if (i * j > 20) {
                printf("\nProduct too large\n");
                found = true;
                break;  // Inner break 1
            }
            
            if (i == j && i > 3) {
                printf("\nDiagonal limit reached\n");
                found = true;
                break;  // Inner break 2
            }
        }
        
        if (found) {
            break;  // Outer break
        }
        
        if (i > 10) {
            printf("\nRow limit reached\n");
            break;  // Outer break 2
        }
        
        printf("\n");
    }
}

// Test 4: Search with multiple termination conditions
int binary_search_with_checks(int arr[], int size, int target) {
    int left = 0;
    int right = size - 1;
    int iterations = 0;
    
    while (left <= right) {
        iterations++;
        
        if (iterations > 20) {
            printf("Too many iterations in binary search\n");
            return -1;  // Break 1
        }
        
        int mid = left + (right - left) / 2;
        
        if (arr[mid] == target) {
            printf("Found at index %d\n", mid);
            return mid;  // Break 2
        }
        
        if (arr[mid] < 0) {
            printf("Encountered invalid negative value\n");
            return -2;  // Break 3
        }
        
        if (arr[mid] < target) {
            left = mid + 1;
        } else {
            right = mid - 1;
        }
    }
    
    return -3;  // Not found
}

// Test 5: State machine with multiple exits
int state_machine_multi_exit(int input[], int size) {
    enum State { INIT, READING, PROCESSING, VALIDATING, ERROR, DONE };
    enum State state = INIT;
    int i = 0;
    
    while (i < size) {
        int value = input[i];
        
        switch (state) {
            case INIT:
                if (value < 0) {
                    state = ERROR;
                    break;  // Switch break
                }
                state = READING;
                break;
                
            case READING:
                if (value == 0) {
                    printf("Terminator found\n");
                    return i;  // Exit 1
                }
                if (value > 100) {
                    state = ERROR;
                } else {
                    state = PROCESSING;
                }
                break;
                
            case PROCESSING:
                state = VALIDATING;
                break;
                
            case VALIDATING:
                if (value % 2 != 0) {
                    printf("Validation failed\n");
                    return -1;  // Exit 2
                }
                state = READING;
                break;
                
            case ERROR:
                printf("Error state\n");
                return -2;  // Exit 3
                
            case DONE:
                return i;  // Exit 4
        }
        
        i++;
        
        if (i > 1000) {
            printf("Loop limit exceeded\n");
            return -3;  // Exit 5
        }
    }
    
    return i;
}

// Test 6: Loop with break in nested conditions
void complex_break_conditions(int n) {
    for (int i = 0; i < n; i++) {
        printf("Iteration %d: ", i);
        
        if (i % 2 == 0) {
            if (i % 4 == 0) {
                if (i > 10) {
                    printf("Breaking on even-4 condition\n");
                    break;  // Break 1 (nested in two ifs)
                }
            }
        } else {
            if (i % 3 == 0) {
                printf("Breaking on odd-3 condition\n");
                break;  // Break 2 (nested in two ifs)
            }
        }
        
        if (i * i > 100) {
            printf("Breaking on square condition\n");
            break;  // Break 3
        }
        
        printf("continuing\n");
    }
}

// Test 7: Do-while with multiple breaks
int do_while_multi_break(int start) {
    int value = start;
    int count = 0;
    
    do {
        count++;
        printf("Value: %d\n", value);
        
        if (value % 10 == 0) {
            printf("Multiple of 10\n");
            break;  // Break 1
        }
        
        if (value > 50) {
            printf("Exceeded threshold\n");
            break;  // Break 2
        }
        
        if (count > 20) {
            printf("Too many iterations\n");
            break;  // Break 3
        }
        
        value += 3;
        
    } while (value < 100);
    
    return value;
}

int main() {
    printf("=== Test 7: Multiple Break Points ===\n\n");
    
    printf("Test 1: Array search with multiple breaks\n");
    int arr1[] = {5, 10, -3, 20, 25};
    find_value_multi_break(arr1, 5, 10);
    find_value_multi_break(arr1, 5, 30);
    
    printf("\nTest 2: While loop with breaks\n");
    process_with_breaks(5, 100);
    
    printf("\nTest 3: Nested loops with breaks\n");
    nested_loop_multi_break(10, 10);
    
    printf("\nTest 4: Binary search with checks\n");
    int arr2[] = {2, 4, 6, 8, 10, 12, 14};
    binary_search_with_checks(arr2, 7, 8);
    
    printf("\nTest 5: State machine\n");
    int input[] = {10, 20, 4, 6, 8, 0};
    state_machine_multi_exit(input, 6);
    
    printf("\nTest 6: Complex break conditions\n");
    complex_break_conditions(20);
    
    printf("\nTest 7: Do-while with breaks\n");
    do_while_multi_break(5);
    
    return 0;
}
