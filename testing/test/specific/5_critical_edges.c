#include <stdio.h>

// Test 1: Basic critical edge scenario
// A block with multiple successors to a block with multiple predecessors
void basic_critical_edge(int a, int b) {
    // Block with 2 successors
    if (a > 0) {
        printf("A is positive\n");
    } else {
        printf("A is non-positive\n");
    }
    
    // Block with 2 predecessors (from true and false branches above)
    if (b > 0) {
        printf("B is positive\n");
    } else {
        printf("B is non-positive\n");
    }
    
    printf("Done\n");
}

// Test 2: Multiple critical edges
void multiple_critical_edges(int x, int y, int z) {
    // First conditional creates branches
    if (x > 0) {
        printf("X positive\n");
    } else {
        printf("X non-positive\n");
    }
    
    // Second conditional (reached from both branches above)
    if (y > 0) {
        printf("Y positive\n");
    } else {
        printf("Y non-positive\n");
    }
    
    // Third conditional (reached from both branches above)
    if (z > 0) {
        printf("Z positive\n");
    } else {
        printf("Z non-positive\n");
    }
    
    printf("All done\n");
}

// Test 3: Critical edge with loop
void critical_edge_with_loop(int n) {
    int sum = 0;
    
    // Initial branch
    if (n > 0) {
        printf("Positive input\n");
    } else {
        printf("Non-positive input\n");
        n = 1;
    }
    
    // Loop with multiple predecessors (from both branches above)
    for (int i = 0; i < n; i++) {
        sum += i;
    }
    
    printf("Sum: %d\n", sum);
}

// Test 4: Diamond pattern creating critical edges
void diamond_pattern(int condition1, int condition2) {
    int result = 0;
    
    // Top of diamond
    if (condition1) {
        result += 10;
    }
    
    // Bottom of diamond - has multiple predecessors
    if (condition2) {
        result += 5;
    }
    
    printf("Result: %d\n", result);
}

// Test 5: Switch creating critical edges
void switch_critical_edges(int choice, int value) {
    // Switch with multiple cases
    switch (choice) {
        case 1:
            printf("Choice 1\n");
            break;
        case 2:
            printf("Choice 2\n");
            break;
        case 3:
            printf("Choice 3\n");
            break;
        default:
            printf("Default choice\n");
            break;
    }
    
    // Block with multiple predecessors (from all switch cases)
    if (value > 0) {
        printf("Value is positive\n");
    }
}

// Test 6: Nested conditions with critical edges
void nested_with_critical_edges(int a, int b, int c) {
    // Outer condition
    if (a > 0) {
        if (b > 0) {
            printf("A and B positive\n");
        } else {
            printf("A positive, B non-positive\n");
        }
    } else {
        printf("A non-positive\n");
    }
    
    // This block has 3 predecessors (critical edges from nested structure)
    if (c > 0) {
        printf("C is positive\n");
    }
}

// Test 7: Loop exit creating critical edges
void loop_exit_critical_edge(int n) {
    int i = 0;
    
    // Loop
    while (i < n) {
        if (i % 2 == 0) {
            printf("Even: %d\n", i);
        } else {
            printf("Odd: %d\n", i);
        }
        i++;
    }
    
    // Block reached from both loop body branches and loop exit
    printf("Loop finished\n");
}

// Test 8: Complex control flow with multiple critical edges
int complex_critical_edges(int x, int y) {
    int result = 0;
    
    if (x > 0) {
        if (y > 0) {
            result = 1;
        } else {
            result = 2;
        }
    } else {
        if (y > 0) {
            result = 3;
        } else {
            result = 4;
        }
    }
    
    // This block has 4 predecessors
    if (result % 2 == 0) {
        result *= 2;
    }
    
    return result;
}

int main() {
    printf("=== Test 5: Critical Edges ===\n\n");
    
    printf("Test 1: Basic critical edge\n");
    basic_critical_edge(5, -3);
    
    printf("\nTest 2: Multiple critical edges\n");
    multiple_critical_edges(1, -2, 3);
    
    printf("\nTest 3: Critical edge with loop\n");
    critical_edge_with_loop(5);
    
    printf("\nTest 4: Diamond pattern\n");
    diamond_pattern(1, 0);
    
    printf("\nTest 5: Switch critical edges\n");
    switch_critical_edges(2, 10);
    
    printf("\nTest 6: Nested with critical edges\n");
    nested_with_critical_edges(1, -1, 1);
    
    printf("\nTest 7: Loop exit critical edge\n");
    loop_exit_critical_edge(5);
    
    printf("\nTest 8: Complex critical edges\n");
    printf("Result: %d\n", complex_critical_edges(1, -1));
    
    return 0;
}
