
int dead_code_test(int a, int b) {
    // 'dead_val' is defined (KILLED in LLVM terms) but never used (never GEN'd)
    int dead_val = a * 100; 
    
    // 'c' is defined and used
    int c = b + 5;
    
    return c;
}

int main(){
    int result = dead_code_test(5, 10);
    return 0;
}