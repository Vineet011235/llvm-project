#include <stdio.h>

int main() {
    int num1, num2;
    int sum, difference, product;
    float quotient;
    
    printf("Enter first number: ");
    scanf("%d", &num1);
    
    printf("Enter second number: ");
    scanf("%d", &num2);
    
    // Perform arithmetic operations
    sum = num1 + num2;
    difference = num1 - num2;
    product = num1 * num2;
    
    printf("\nArithmetic Operations:\n");
    printf("Sum: %d + %d = %d\n", num1, num2, sum);
    printf("Difference: %d - %d = %d\n", num1, num2, difference);
    printf("Product: %d * %d = %d\n", num1, num2, product);
    
    if (num2 != 0) {
        quotient = (float)num1 / num2;
        printf("Quotient: %d / %d = %.2f\n", num1, num2, quotient);
    } else {
        printf("Quotient: Cannot divide by zero\n");
    }
    
    return 0;
}