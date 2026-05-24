#include "iostream"
#include <algorithm>
#include "memory"

class Buffer {
public:
int* data;
size_t size;


Buffer(size_t n)
    : data(new int[n]{}), size(n) {}

~Buffer() {
    delete[] data;
}

// 拷贝构造：深拷贝
Buffer(const Buffer& other)
    : data(new int[other.size]), size(other.size) {
    std::copy(other.data, other.data + size, data);
}

// 拷贝赋值：深拷贝 + 自赋值保护
Buffer& operator=(const Buffer& other) {
    if (this == &other) return *this;
    int* newData = new int[other.size];
    std::copy(other.data, other.data + other.size, newData);
    delete[] data;
    data = newData;
    size = other.size;
    return *this;
}

// 移动构造：右值引用，偷资源
Buffer(Buffer&& other) noexcept
    : data(other.data), size(other.size) {
    other.data = nullptr;
    other.size = 0;
}

// 移动赋值：释放旧资源，接管新资源
Buffer& operator=(Buffer&& other) noexcept {
    if (this == &other) return *this;
    delete[] data;
    data = other.data;
    size = other.size;
    other.data = nullptr;
    other.size = 0;
    return *this;
}
};
class Buffer2 {
public:
std::unique_ptr<int[]> data;
size_t size;
Buffer2(size_t n)
    : data(std::make_unique<int[]>(n)), size(n) {}

// unique_ptr 不可拷贝，可移动
Buffer2(Buffer2&& other) noexcept 
{
    data = std::move(other.data);
    size = other.size;
    other.size = 0; 
};
Buffer2& operator=(Buffer2&&) noexcept = default;

// 如果不希望拷贝，直接禁用
Buffer2(const Buffer2&) = delete;
Buffer2& operator=(const Buffer2&) = delete;

};

int main() {
    Buffer buf1(5);
    for (size_t i = 0; i < buf1.size; ++i) {
        buf1.data[i] = static_cast<int>(i + 1);
    }

    Buffer buf2 = std::move(buf1); // 移动构造
    std::cout << "buf2 size: " << buf2.size << ", data: ";
    for (size_t i = 0; i < buf2.size; ++i) {
        std::cout << buf2.data[i] << " ";
    }
    std::cout << "\nbuf1 size: " << buf1.size << ", data pointer: " << buf1.data << std::endl;

    Buffer2 buf3(5);
    for (size_t i = 0; i < buf3.size; ++i)  {
        buf3.data[i] = static_cast<int>(i + 1);
    }
    std::cout << "\nbuf3 size Before: " << buf3.size << ", data pointer: " << buf3.data.get() << std::endl;

    Buffer2 buf4 = std::move(buf3); // 移动构造
    std::cout << "buf4 size: " << buf4.size << ", data: ";
    for (size_t i = 0; i < buf4.size; ++i) {
        std::cout << buf4.data[i] << " ";
    }
    std::cout << "\nbuf3 size: " << buf3.size << ", data pointer: " << buf3.data.get() << std::endl;
    return 0;
}