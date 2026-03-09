// Simple loop test for dataflow analysis
int sum_array(int arr[], int n) {
    int sum = 0;
    int i;
    
    for (i = 0; i < n; i++) {
        sum += arr[i];
    }
    
    return sum;
}

int main() {
    int numbers[] = {1, 2, 3, 4, 5};
    int result = sum_array(numbers, 5);
    return result;
}
