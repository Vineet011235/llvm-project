#include <stdio.h>

// Test 1: Basic switch with fall-through
void weekday_weekend(int day) {
    printf("Day %d: ", day);
    
    switch (day) {
        case 1:
        case 2:
        case 3:
        case 4:
        case 5:
            printf("Weekday\n");
            break;
        case 6:
        case 7:
            printf("Weekend\n");
            break;
        default:
            printf("Invalid day\n");
    }
}

// Test 2: Fall-through with code between cases
void fall_through_with_code(int level) {
    int permissions = 0;
    
    printf("Access level %d:\n", level);
    
    switch (level) {
        case 4:
            printf("  + Administrator rights\n");
            permissions += 1000;
            // Fall through to level 3
        case 3:
            printf("  + Write access\n");
            permissions += 100;
            // Fall through to level 2
        case 2:
            printf("  + Read access\n");
            permissions += 10;
            // Fall through to level 1
        case 1:
            printf("  + Basic access\n");
            permissions += 1;
            break;
        case 0:
            printf("  No access\n");
            break;
        default:
            printf("  Invalid level\n");
    }
    
    printf("Total permissions: %d\n\n", permissions);
}

// Test 3: Partial fall-through (some cases break, others fall through)
void mixed_fallthrough(char grade) {
    printf("Grade %c: ", grade);
    
    switch (grade) {
        case 'A':
            printf("Excellent - ");
            // Fall through
        case 'B':
            printf("Good - ");
            // Fall through
        case 'C':
            printf("Pass");
            break;  // Stop here
        case 'D':
            printf("Barely passing");
            // Fall through to fail message
        case 'F':
            printf(" - Need improvement");
            break;
        default:
            printf("Invalid grade");
    }
    printf("\n");
}

// Test 4: Complex fall-through with multiple paths
int calculate_discount(int membership_years, int purchase_amount) {
    int discount = 0;
    
    switch (membership_years) {
        case 10:
        case 9:
        case 8:
            discount += 5;  // Extra 5% for 8+ years
            printf("Loyalty bonus: +5%%\n");
            // Fall through
        case 7:
        case 6:
        case 5:
            discount += 10;  // 10% for 5+ years
            printf("Senior member: +10%%\n");
            // Fall through
        case 4:
        case 3:
            discount += 5;  // Additional 5% for 3+ years
            printf("Regular member: +5%%\n");
            // Fall through
        case 2:
        case 1:
            discount += 5;  // Base 5% for members
            printf("Member discount: +5%%\n");
            break;
        case 0:
            printf("No membership discount\n");
            break;
        default:
            if (membership_years > 10) {
                discount = 30;  // Cap at 30%
                printf("Maximum discount: 30%%\n");
            }
    }
    
    return (purchase_amount * discount) / 100;
}

// Test 5: Switch in loop with fall-through
void process_commands(char commands[], int size) {
    int position = 0;
    int state = 0;
    
    for (int i = 0; i < size; i++) {
        printf("Command '%c': ", commands[i]);
        
        switch (commands[i]) {
            case 'U':  // Up
            case 'u':
                position++;
                printf("Move up to %d\n", position);
                break;
            case 'D':  // Down
            case 'd':
                position--;
                printf("Move down to %d\n", position);
                break;
            case 'R':  // Reset
                position = 0;
                printf("Reset position\n");
                // Fall through to show state
            case 'S':  // Status
            case 's':
                printf("Current position: %d, state: %d\n", position, state);
                break;
            case 'Q':  // Quit
            case 'q':
                printf("Quitting\n");
                return;
            default:
                printf("Unknown command\n");
        }
    }
}

// Test 6: Nested switch with fall-through
void nested_switch_fallthrough(int category, int subcategory) {
    printf("Category %d, Subcategory %d: ", category, subcategory);
    
    switch (category) {
        case 1:
            switch (subcategory) {
                case 1:
                case 2:
                    printf("Basic service\n");
                    break;
                case 3:
                    printf("Premium service\n");
                    break;
                default:
                    printf("Unknown subcategory\n");
            }
            break;
        case 2:
            printf("Category 2 - ");
            // Fall through
        case 3:
            switch (subcategory) {
                case 1:
                    printf("Standard\n");
                    break;
                case 2:
                    printf("Advanced\n");
                    break;
                default:
                    printf("Default subcategory\n");
            }
            break;
        default:
            printf("Invalid category\n");
    }
}

// Test 7: Fall-through creating complex CFG
void season_activities(int month) {
    printf("Month %d: ", month);
    
    switch (month) {
        case 12:
        case 1:
        case 2:
            printf("Winter - ");
            switch (month) {
                case 12:
                    printf("Holiday season, ");
                    // Fall through
                case 1:
                case 2:
                    printf("Skiing season");
                    break;
            }
            break;
        case 3:
        case 4:
        case 5:
            printf("Spring - Gardening season");
            break;
        case 6:
        case 7:
        case 8:
            printf("Summer - ");
            printf("Beach season");
            // Fall through to add general summer info
        case 9:
            if (month == 9) printf("Early Fall - ");
            printf(" Good weather");
            break;
        case 10:
        case 11:
            printf("Fall - Harvest season");
            break;
        default:
            printf("Invalid month");
    }
    printf("\n");
}

// Test 8: Duff's Device pattern (loop unrolling with fall-through)
void copy_bytes_duff(char *dest, const char *src, int count) {
    int n = (count + 7) / 8;
    
    switch (count % 8) {
        case 0: do { *dest++ = *src++;
        case 7:      *dest++ = *src++;
        case 6:      *dest++ = *src++;
        case 5:      *dest++ = *src++;
        case 4:      *dest++ = *src++;
        case 3:      *dest++ = *src++;
        case 2:      *dest++ = *src++;
        case 1:      *dest++ = *src++;
               } while (--n > 0);
    }
}

int main() {
    printf("=== Test 8: Switch Fall-Through ===\n\n");
    
    printf("Test 1: Weekday/Weekend\n");
    weekday_weekend(3);
    weekday_weekend(6);
    weekday_weekend(9);
    
    printf("\nTest 2: Cumulative permissions\n");
    fall_through_with_code(4);
    fall_through_with_code(2);
    
    printf("Test 3: Mixed fall-through\n");
    mixed_fallthrough('A');
    mixed_fallthrough('C');
    mixed_fallthrough('D');
    
    printf("\nTest 4: Discount calculation\n");
    printf("Discount for 8 years, $100: $%d\n", calculate_discount(8, 100));
    printf("Discount for 3 years, $100: $%d\n", calculate_discount(3, 100));
    
    printf("\nTest 5: Command processing\n");
    char cmds[] = {'U', 'U', 'D', 'S', 'R', 's'};
    process_commands(cmds, 6);
    
    printf("\nTest 6: Nested switch\n");
    nested_switch_fallthrough(1, 2);
    nested_switch_fallthrough(2, 1);
    
    printf("\nTest 7: Season activities\n");
    season_activities(12);
    season_activities(6);
    season_activities(9);
    
    printf("\nTest 8: Duff's device\n");
    char src[] = "Hello, World!";
    char dest[20] = {0};
    copy_bytes_duff(dest, src, 13);
    printf("Copied: %s\n", dest);
    
    return 0;
}
