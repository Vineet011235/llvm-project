#include <stdio.h>

// Test 1: Grade evaluation with nested if-else
void grade_evaluation_nested(int score) {
    if (score >= 90) {
        if (score >= 95) {
            if (score >= 98) {
                printf("A+ (Outstanding)\n");
            } else {
                printf("A+ (Excellent)\n");
            }
        } else {
            printf("A\n");
        }
    } else if (score >= 80) {
        if (score >= 85) {
            printf("B+\n");
        } else {
            printf("B\n");
        }
    } else if (score >= 70) {
        if (score >= 75) {
            printf("C+\n");
        } else {
            printf("C\n");
        }
    } else if (score >= 60) {
        printf("D\n");
    } else {
        printf("F\n");
    }
}

// Test 2: Deep nesting with multiple conditions
int deeply_nested_conditions(int a, int b, int c, int d) {
    if (a > 0) {
        if (b > 0) {
            if (c > 0) {
                if (d > 0) {
                    printf("All positive\n");
                    return 4;
                } else {
                    printf("First three positive\n");
                    return 3;
                }
            } else {
                if (d > 0) {
                    printf("A, B, D positive\n");
                    return 2;
                } else {
                    printf("A, B positive\n");
                    return 2;
                }
            }
        } else {
            if (c > 0) {
                printf("A, C positive\n");
                return 2;
            } else {
                printf("Only A positive\n");
                return 1;
            }
        }
    } else {
        if (b > 0) {
            printf("Only B positive\n");
            return 1;
        } else {
            printf("None or A positive\n");
            return 0;
        }
    }
}

// Test 3: Nested loops with conditionals
void nested_loops_with_conditionals(int rows, int cols) {
    printf("\nNested loops with conditionals:\n");
    
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            if (i == j) {
                if (i % 2 == 0) {
                    printf("E ");  // Even diagonal
                } else {
                    printf("O ");  // Odd diagonal
                }
            } else {
                if (i > j) {
                    printf("L ");  // Lower triangle
                } else {
                    printf("U ");  // Upper triangle
                }
            }
        }
        printf("\n");
    }
}

// Test 4: Permission checking with nested conditions
int check_permissions(int user_level, int resource_level, int is_owner, int is_admin) {
    if (is_admin) {
        printf("Admin access granted\n");
        return 1;
    } else {
        if (is_owner) {
            if (user_level >= resource_level) {
                printf("Owner with sufficient level\n");
                return 1;
            } else {
                printf("Owner but insufficient level\n");
                return 0;
            }
        } else {
            if (user_level > resource_level) {
                if (resource_level < 5) {
                    printf("High level user, low security resource\n");
                    return 1;
                } else {
                    printf("High level but restricted resource\n");
                    return 0;
                }
            } else {
                printf("Access denied\n");
                return 0;
            }
        }
    }
}

// Test 5: Tree-like decision structure
const char* classify_triangle(int a, int b, int c) {
    if (a <= 0 || b <= 0 || c <= 0) {
        return "Invalid";
    } else {
        if (a + b <= c || b + c <= a || a + c <= b) {
            return "Invalid";
        } else {
            if (a == b) {
                if (b == c) {
                    return "Equilateral";
                } else {
                    return "Isosceles";
                }
            } else {
                if (b == c || a == c) {
                    return "Isosceles";
                } else {
                    if (a*a + b*b == c*c || b*b + c*c == a*a || a*a + c*c == b*b) {
                        return "Right-angled";
                    } else {
                        return "Scalene";
                    }
                }
            }
        }
    }
}

// Test 6: Deeply nested validation
int validate_input_nested(int value, int min, int max, int step) {
    if (min < max) {
        if (value >= min) {
            if (value <= max) {
                if (step > 0) {
                    if ((value - min) % step == 0) {
                        printf("Valid input with correct step\n");
                        return 1;
                    } else {
                        printf("Invalid step\n");
                        return 0;
                    }
                } else {
                    printf("Invalid step size\n");
                    return 0;
                }
            } else {
                printf("Value too large\n");
                return 0;
            }
        } else {
            printf("Value too small\n");
            return 0;
        }
    } else {
        printf("Invalid range\n");
        return 0;
    }
}

// Test 7: Complex nested if-else-if chain
void categorize_number(int n) {
    if (n < 0) {
        if (n < -100) {
            if (n < -1000) {
                printf("Very large negative\n");
            } else {
                printf("Large negative\n");
            }
        } else {
            if (n < -10) {
                printf("Medium negative\n");
            } else {
                printf("Small negative\n");
            }
        }
    } else if (n > 0) {
        if (n > 100) {
            if (n > 1000) {
                printf("Very large positive\n");
            } else {
                printf("Large positive\n");
            }
        } else {
            if (n > 10) {
                printf("Medium positive\n");
            } else {
                printf("Small positive\n");
            }
        }
    } else {
        printf("Zero\n");
    }
}

int main() {
    printf("=== Test 6: Nested Conditionals ===\n\n");
    
    printf("Test 1: Grade evaluation\n");
    grade_evaluation_nested(97);
    grade_evaluation_nested(83);
    grade_evaluation_nested(55);
    
    printf("\nTest 2: Deep nesting\n");
    deeply_nested_conditions(1, 1, 1, 1);
    deeply_nested_conditions(1, -1, 1, -1);
    
    printf("\nTest 3: Nested loops\n");
    nested_loops_with_conditionals(5, 5);
    
    printf("\nTest 4: Permission checking\n");
    check_permissions(8, 5, 0, 0);
    check_permissions(3, 5, 1, 0);
    check_permissions(2, 8, 0, 1);
    
    printf("\nTest 5: Triangle classification\n");
    printf("(3,3,3): %s\n", classify_triangle(3, 3, 3));
    printf("(3,3,4): %s\n", classify_triangle(3, 3, 4));
    printf("(3,4,5): %s\n", classify_triangle(3, 4, 5));
    printf("(3,4,6): %s\n", classify_triangle(3, 4, 6));
    
    printf("\nTest 6: Input validation\n");
    validate_input_nested(15, 10, 50, 5);
    validate_input_nested(17, 10, 50, 5);
    
    printf("\nTest 7: Number categorization\n");
    categorize_number(-1500);
    categorize_number(50);
    categorize_number(0);
    
    return 0;
}
