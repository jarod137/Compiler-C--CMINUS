void h;

int foo2(void y) {
    return 5;
}


void vfoo(void){
    write "success\n";
}

int foo(int b){
    int x;
    int f;
    x=4+3-2*5;
    vfoo();
    f = -foo2(h);
    return x+f;
}


int foo3(void) {
    return 0;
}

int main(void) {
    int count;
    int A[100],B[100];
    write foo(2);
    write "\n";
    write foo3();
    write "\n";
    if(foo3()<5) {
        write "success\n";
    }
    if(foo3()<-1) {
        write "fail\n";
    }
    else {
        write "success\n";
    }
    count = 10;
    while(count > 0){
        if(count == 10){
            write "first\n";
        }
        write count;
        write "\n";
        count = count - 1;
    }
    count = 4;
    if(count == 4){
        write "success\n";
    }
    if(count != 5){
        write "success\n";
    }
    if(count<=4){
        write "success\n";
    }
    if(count>=4){
        write "success\n";
    }
    B[0]=40;
    A[B[0]]=5;
    write A[40];
}
