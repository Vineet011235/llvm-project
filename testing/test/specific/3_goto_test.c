#include <stdio.h>
#include <stdlib.h>

// Test 1: Simple loop with goto
void goto_loop(int n) {
    int i = 0;
    
loop_start:
    if (i >= n) {
        goto loop_end;
    }
    printf("%d ", i);
    i++;
    goto loop_start;
    
loop_end:
    printf("\n");
}

// Test 2: Goto for cleanup/error handling
int process_file(const char *filename) {
    FILE *file = NULL;
    char *buffer = NULL;
    int result = 0;
    
    file = fopen(filename, "r");
    if (file == NULL) {
        goto error_file;
    }
    
    buffer = (char *)malloc(1024);
    if (buffer == NULL) {
        goto error_alloc;
    }
    
    // Simulate processing
    printf("Processing file...\n");
    result = 1;
    goto cleanup;
    
error_alloc:
    printf("Memory allocation failed\n");
    fclose(file);
    return -1;
    
error_file:
    printf("Failed to open file\n");
    return -1;
    
cleanup:
    if (buffer) free(buffer);
    if (file) fclose(file);
    return result;
}

// Test 3: Goto for breaking out of nested loops
void nested_loop_break(int rows, int cols) {
    printf("\nSearching in %dx%d grid:\n", rows, cols);
    
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            printf("(%d,%d) ", i, j);
            
            // Simulate finding target
            if (i == 2 && j == 3) {
                printf("\nTarget found!\n");
                goto found;
            }
        }
    }
    
    printf("\nTarget not found\n");
    return;
    
found:
    printf("Broke out of nested loops\n");
}

// Test 4: State machine with goto
void state_machine(int input) {
    enum State { START, PROCESSING, VALIDATING, DONE, ERROR };
    enum State state = START;
    
state_start:
    if (input < 0) goto state_error;
    state = PROCESSING;
    
state_processing:
    printf("Processing input: %d\n", input);
    if (input == 0) goto state_done;
    state = VALIDATING;
    
state_validating:
    printf("Validating...\n");
    if (input > 100) goto state_error;
    goto state_done;
    
state_error:
    printf("Error: Invalid input\n");
    return;
    
state_done:
    printf("Processing complete\n");
}

// Test 5: Forward and backward jumps
void jump_around(int x) {
    printf("\nStart with x=%d\n", x);
    
    if (x < 0) goto negative_handler;
    if (x == 0) goto zero_handler;
    
    printf("Positive path\n");
    goto end;
    
negative_handler:
    printf("Negative path\n");
    x = -x;  // Make positive
    if (x > 10) goto end;
    goto continue_processing;
    
zero_handler:
    printf("Zero path\n");
    goto end;
    
continue_processing:
    printf("Continue processing: x=%d\n", x);
    
end:
    printf("Done\n");
}

// Test 6: Goto in switch statement
void switch_with_goto(int option) {
    switch (option) {
        case 1:
            printf("Option 1\n");
            goto common_code;
        case 2:
            printf("Option 2\n");
            goto common_code;
        case 3:
            printf("Option 3\n");
            break;
        default:
            printf("Default option\n");
            return;
    }
    goto end;
    
common_code:
    printf("Common code executed\n");
    
end:
    printf("End of switch\n");
}

int main() {
    printf("=== Test 3: Goto Statements ===\n\n");
    
    printf("Test 1: Loop with goto\n");
    goto_loop(5);
    
    printf("\nTest 2: Error handling with goto\n");
    process_file("nonexistent.txt");
    
    printf("\nTest 3: Breaking nested loops\n");
    nested_loop_break(5, 5);
    
    printf("\nTest 4: State machine\n");
    state_machine(50);
    state_machine(-5);
    state_machine(150);
    
    printf("\nTest 5: Jump around\n");
    jump_around(5);
    jump_around(-15);
    jump_around(0);
    
    printf("\nTest 6: Goto in switch\n");
    switch_with_goto(1);
    switch_with_goto(3);
    
    return 0;
}
