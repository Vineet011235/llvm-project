#include <stdio.h>
#include <stdlib.h>

// Test 1: Unreachable code after return in all branches
int unreachable_after_return(int x) {
    if (x > 0) {
        return 1;
    } else {
        return -1;
    }
    
    // This code is unreachable
    printf("This should never execute\n");
    return 0;
}

// Test 2: Unreachable code after unconditional return
void unreachable_after_unconditional(int x) {
    printf("Before return\n");
    return;
    
    // Unreachable
    printf("After return - unreachable\n");
    x = x + 1;
}

// Test 3: Unreachable code after exit()
void unreachable_after_exit(int error_code) {
    if (error_code != 0) {
        printf("Fatal error\n");
        exit(error_code);
        
        // Unreachable after exit()
        printf("This won't print\n");
    }
    printf("Normal execution\n");
}

// Test 4: Unreachable else branch
int unreachable_else(int x) {
    if (x >= 0 || x < 0) {  // Always true
        return 1;
    } else {
        // Unreachable - condition above is always true
        printf("Impossible to reach\n");
        return 0;
    }
}

// Test 5: Unreachable code after infinite loop
void unreachable_after_infinite_loop(void) {
    printf("Before infinite loop\n");
    
    while (1) {
        // Infinite loop with no break
        printf("Looping forever...\n");
        break;  // Actually breaks, so code below IS reachable
    }
    
    printf("After loop - reachable\n");
    
    // Now truly infinite
    for (;;) {
        // This loop never exits
    }
    
    // Unreachable
    printf("After infinite for loop\n");
}

// Test 6: Unreachable switch case
void unreachable_switch_case(int x) {
    switch (x) {
        case 1:
            printf("Case 1\n");
            return;
        case 2:
            printf("Case 2\n");
            return;
        default:
            printf("Default\n");
            return;
    }
    
    // Unreachable - all cases return
    printf("After switch\n");
}

// Test 7: Dead code due to constant folding
int dead_code_constant(void) {
    int result = 0;
    
    if (1 == 0) {  // Always false
        // Dead code - compiler will likely remove this
        result = 100;
        printf("This is dead code\n");
    }
    
    if (5 > 3) {  // Always true
        result = 42;
    } else {
        // Dead code
        result = -1;
        printf("This is also dead code\n");
    }
    
    return result;
}

// Test 8: Unreachable after abort()
void unreachable_after_abort(int critical_error) {
    if (critical_error) {
        printf("Critical error - aborting\n");
        abort();
        
        // Unreachable
        printf("After abort\n");
    }
    printf("Normal path\n");
}

// Test 9: Code after noreturn function
_Noreturn void fatal_error(void) {
    fprintf(stderr, "Fatal error occurred\n");
    exit(1);
}

void use_noreturn_function(int error) {
    if (error) {
        fatal_error();
        // Unreachable - fatal_error never returns
        printf("After fatal error\n");
    }
    printf("Normal execution\n");
}

int main() {
    printf("=== Test 4: Unreachable Code ===\n\n");
    
    printf("Test 1: %d\n", unreachable_after_return(5));
    
    printf("\nTest 2:\n");
    unreachable_after_unconditional(10);
    
    printf("\nTest 3:\n");
    unreachable_after_exit(0);  // Don't pass non-zero to avoid actual exit
    
    printf("\nTest 4: %d\n", unreachable_else(5));
    
    printf("\nTest 5:\n");
    // unreachable_after_infinite_loop();  // Commented out to avoid infinite loop
    printf("(Skipped infinite loop test)\n");
    
    printf("\nTest 6:\n");
    unreachable_switch_case(1);
    
    printf("\nTest 7: %d\n", dead_code_constant());
    
    printf("\nTest 8:\n");
    unreachable_after_abort(0);  // Don't pass 1 to avoid actual abort
    
    printf("\nTest 9:\n");
    use_noreturn_function(0);  // Don't pass 1 to avoid actual exit
    
    return 0;
}
