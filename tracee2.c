int f2(int y){
    y = y + 2;
    return y;
}

int f(int x){
    x = x + 1;
    return f2(x);
}

int main(void){
    int x = f(5);
    return 0;
}