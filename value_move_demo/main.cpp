#include <cstring>
#include <iostream>
#include <string>
#include <utility>

class Buffer
{
public:
    explicit Buffer(size_t n = 0)
        : size_(n), data_(n ? new char[n] : nullptr)
    {
        std::cout << "ctor(size=" << size_ << ")\n";
    }

    ~Buffer()
    {
        delete[] data_;
        std::cout << "dtor(size=" << size_ << ")\n";
    }

    // Value semantics: deep copy so each object owns its own data.
    Buffer(const Buffer& other)
        : size_(other.size_), data_(other.size_ ? new char[other.size_] : nullptr)
    {
        if (data_ != nullptr)
        {
            std::memcpy(data_, other.data_, size_);
        }
        std::cout << "copy ctor(size=" << size_ << ")\n";
    }

    Buffer& operator=(const Buffer& other)
    {
        if (this == &other)
        {
            return *this;
        }

        char* newData = other.size_ ? new char[other.size_] : nullptr;
        if (newData != nullptr)
        {
            std::memcpy(newData, other.data_, other.size_);
        }

        delete[] data_;
        data_ = newData;
        size_ = other.size_;

        std::cout << "copy assign(size=" << size_ << ")\n";
        return *this;
    }

    // Move semantics: transfer ownership, avoid deep copy.
    Buffer(Buffer&& other) noexcept
        : size_(other.size_), data_(other.data_)
    {
        other.size_ = 0;
        other.data_ = nullptr;
        std::cout << "move ctor(size=" << size_ << ")\n";
    }

    Buffer& operator=(Buffer&& other) noexcept
    {
        if (this == &other)
        {
            return *this;
        }

        delete[] data_;
        data_ = other.data_;
        size_ = other.size_;

        other.data_ = nullptr;
        other.size_ = 0;

        std::cout << "move assign(size=" << size_ << ")\n";
        return *this;
    }

    size_t Size() const { return size_; }

private:
    size_t size_{};
    char* data_{};
};

Buffer MakeBuffer(size_t n)
{
    Buffer b(n);
    return b; // may trigger move or RVO
}

void TakeByValue(Buffer b)
{
    std::cout << "TakeByValue got size=" << b.Size() << "\n";
}

int main()
{
    std::cout << "=== Value semantics (copy) ===\n";
    Buffer a(16);
    Buffer b = a;  // copy ctor
    Buffer c;
    c = a;         // copy assign

    std::cout << "\n=== Move semantics ===\n";
    Buffer d = MakeBuffer(32);  // RVO or move ctor
    Buffer e;
    e = std::move(d);           // move assign

    std::cout << "\n=== Pass-by-value trigger ===\n";
    TakeByValue(a);             // copy into parameter
    TakeByValue(std::move(e));  // move into parameter

    return 0;
}
