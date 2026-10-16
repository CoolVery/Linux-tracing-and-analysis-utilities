#include <stdio.h>
#include <math.h>

void hot_function(void) {
    double sum = 0;
    for (int i = 0; i < 100000000; i++) {
        sum += sin(i) * cos(i);   
    }
    printf("hot: %f\n", sum);
}

void cold_function(void) {
    long sum = 0;
    for (int i = 0; i < 5000000; i++) {   
        sum += i * 2;
    }
    printf("cold: %ld\n", sum);
}

int main(void) {
    for (int i = 0; i < 5; i++) {
        hot_function();    
        cold_function();   
    }
    return 0;
}