#include <stdio.h>

// Calculator using switch statement
void calculator() {
    float num1, num2, result;
    char operator;
    
    printf("=== Simple Calculator ===\n");
    printf("Enter first number: ");
    scanf("%f", &num1);
    
    printf("Enter operator (+, -, *, /): ");
    scanf(" %c", &operator);
    
    printf("Enter second number: ");
    scanf("%f", &num2);
    
    switch (operator) {
        case '+':
            result = num1 + num2;
            printf("%.2f + %.2f = %.2f\n", num1, num2, result);
            break;
        case '-':
            result = num1 - num2;
            printf("%.2f - %.2f = %.2f\n", num1, num2, result);
            break;
        case '*':
            result = num1 * num2;
            printf("%.2f × %.2f = %.2f\n", num1, num2, result);
            break;
        case '/':
            if (num2 != 0) {
                result = num1 / num2;
                printf("%.2f ÷ %.2f = %.2f\n", num1, num2, result);
            } else {
                printf("Error: Division by zero!\n");
            }
            break;
        default:
            printf("Error: Invalid operator!\n");
    }
}

// Day of week using switch
void day_of_week() {
    int day;
    
    printf("\n=== Day of the Week ===\n");
    printf("Enter day number (1-7): ");
    scanf("%d", &day);
    
    printf("Day %d is: ", day);
    
    switch (day) {
        case 1:
            printf("Monday\n");
            break;
        case 2:
            printf("Tuesday\n");
            break;
        case 3:
            printf("Wednesday\n");
            break;
        case 4:
            printf("Thursday\n");
            break;
        case 5:
            printf("Friday\n");
            break;
        case 6:
            printf("Saturday\n");
            break;
        case 7:
            printf("Sunday\n");
            break;
        default:
            printf("Invalid day number!\n");
    }
}

// Grade evaluation using switch
void grade_evaluation() {
    char grade;
    
    printf("\n=== Grade Evaluation ===\n");
    printf("Enter your grade (A, B, C, D, F): ");
    scanf(" %c", &grade);
    
    switch (grade) {
        case 'A':
        case 'a':
            printf("Excellent! Outstanding performance.\n");
            break;
        case 'B':
        case 'b':
            printf("Good job! Above average performance.\n");
            break;
        case 'C':
        case 'c':
            printf("Fair. Average performance.\n");
            break;
        case 'D':
        case 'd':
            printf("Poor. Below average performance.\n");
            break;
        case 'F':
        case 'f':
            printf("Failed. Need improvement.\n");
            break;
        default:
            printf("Invalid grade!\n");
    }
}

// Menu-driven program
void menu() {
    int choice;
    
    printf("\n=== Menu Selection ===\n");
    printf("1. Calculator\n");
    printf("2. Day of Week\n");
    printf("3. Grade Evaluation\n");
    printf("4. Exit\n");
    printf("Enter your choice: ");
    scanf("%d", &choice);
    
    switch (choice) {
        case 1:
            calculator();
            break;
        case 2:
            day_of_week();
            break;
        case 3:
            grade_evaluation();
            break;
        case 4:
            printf("Goodbye!\n");
            break;
        default:
            printf("Invalid choice!\n");
    }
}

int main() {
    printf("╔════════════════════════════════════╗\n");
    printf("║   Switch-Case Statement Demo       ║\n");
    printf("╚════════════════════════════════════╝\n\n");
    
    menu();
    
    return 0;
}