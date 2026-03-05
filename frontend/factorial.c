#include <stdio.h>

int factorial(int n) {
    if (n <= 1) {
        return 1;
    }
    return (n * factorial(n - 1));
}

void factorial_helper(int n) {
    if (n == 0) {
        return;
    }
    printf("%d\n", n);
    printf("%d\n", factorial(n));
    factorial_helper(n-1);
    return;
}

int main() {
    factorial_helper(24);
    return 1;
}