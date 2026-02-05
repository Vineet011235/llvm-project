#include <stdio.h>

// Demonstration of for loop
void for_loop_demo() {
    printf("=== For Loop Demo ===\n");
    printf("Counting from 1 to 10:\n");
    
    for (int i = 1; i <= 10; i++) {
        printf("%d ", i);
    }
    printf("\n\n");
}

// Demonstration of while loop
void while_loop_demo() {
    printf("=== While Loop Demo ===\n");
    printf("Counting down from 10 to 1:\n");
    
    int i = 10;
    while (i >= 1) {
        printf("%d ", i);
        i--;
    }
    printf("\n\n");
}

// Demonstration of do-while loop
void do_while_loop_demo() {
    printf("=== Do-While Loop Demo ===\n");
    printf("Even numbers from 2 to 20:\n");
    
    int i = 2;
    do {
        printf("%d ", i);
        i += 2;
    } while (i <= 20);
    printf("\n\n");
}

// Nested loop example
void nested_loop_demo() {
    printf("=== Nested Loop Demo ===\n");
    printf("Multiplication table (1-5):\n\n");
    
    for (int i = 1; i <= 5; i++) {
        for (int j = 1; j <= 5; j++) {
            printf("%4d", i * j);
        }
        printf("\n");
    }
    printf("\n");
}

// Loop with break statement
void break_demo() {
    printf("=== Break Statement Demo ===\n");
    printf("Finding first number divisible by 7 between 50-100:\n");
    int x;
    scanf("%d", &x);
    for (int i = 50; i <= 100; i++) {
        if (i % x == 0) {
            printf("Found: %d\n", i);
            break;
        }
    }
    printf("\n");
}

// Loop with continue statement
void continue_demo() {
    printf("=== Continue Statement Demo ===\n");
    printf("Odd numbers from 1 to 20:\n");
    
    for (int i = 1; i <= 20; i++) {
        if (i % 2 == 0) {
            continue;  // Skip even numbers
        }
        printf("%d ", i);
    }
    printf("\n\n");
}

int main() {
    printf("╔════════════════════════════════════╗\n");
    printf("║   C Programming Loop Examples      ║\n");
    printf("╚════════════════════════════════════╝\n\n");
    
    for_loop_demo();
    while_loop_demo();
    do_while_loop_demo();
    nested_loop_demo();
    break_demo();
    continue_demo();
    
    return 0;
}