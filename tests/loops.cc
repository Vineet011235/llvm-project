int main(){
    int x  = 100;
    int y = x + 4;
    for (int i = 0; i < 10; ++i) {
        y += 10;
        x += i;
    }
    int z = y + x;
    return 0;
}