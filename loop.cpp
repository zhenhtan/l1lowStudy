#include "stdio.h"
#include "iostream"
#include <memory>

int loopnumber(int n){
    if (n == 1)
    {printf("return caculate: %d\n", n);
        return 1;}
    else{
        printf("caculate: %d\n", n);
        return n *loopnumber(n-1);
    }
}

int fbnq(int n){
    if (n == 1 || n == 2)
    {printf("return caculate: %d\n", n);
        return 1;}
    else{
        printf("caculate: %d\n", n);
        return fbnq(n-1)+fbnq(n-2);
    }
}


void hanoi(int n, char source, char target, char auxiliary) {
    if (n == 1) {
        printf("Move disk 1 from %c to %c\n", source, target);
        return;
    }
    // Step 1: Move top n-1 disks from source to auxiliary
    hanoi(n-1, source, auxiliary, target);
    // Step 2: Move nth disk from source to target
    printf("Move disk %d from %c to %c\n", n, source, target);
    // Step 3: Move the n-1 disks from auxiliary to target
    hanoi(n-1, auxiliary, target, source);
}

void memory_leak_demo() {
    //int* ptr = new int(42); // 分配内存但未释放
    auto aaa = std::make_shared<int>(4200000); // 使用智能指针自动管理内存
    std::cout << "Value: " << *aaa << std::endl;
    int* ptr = new int(42);
    // 没有 delete ptr; 导致内存泄漏
}
int main(void)
{
    printf("%d\n", loopnumber(5));
    printf("%d\n", fbnq(5));

// Example usage:
// Move 3 disks from 'A' to 'C' using 'B' as auxiliary
    hanoi(3, 'A', 'C', 'B');
    memory_leak_demo();
}