#include <cstdio>
#include <iostream>
#include <stdexcept>
#include <string>

class FileGuard
{
public:
    explicit FileGuard(const std::string& path)
        : file_(std::fopen(path.c_str(), "w"))
    {
        if (file_ == nullptr)
        {
            throw std::runtime_error("Failed to open file: " + path);
        }
        std::cout << "[ctor] file opened\n";
    }

    ~FileGuard()
    {
        if (file_ != nullptr)
        {
            std::fclose(file_);
            std::cout << "[dtor] file closed automatically\n";
        }
    }

    FileGuard(const FileGuard&) = delete;
    FileGuard& operator=(const FileGuard&) = delete;

    void WriteLine(const std::string& line)
    {
        std::fprintf(file_, "%s\n", line.c_str());
        std::fflush(file_);
    }

private:
    std::FILE* file_;
};

static void NormalPath()
{
    std::cout << "=== Normal Path ===\n";
    FileGuard guard("raii_output.txt");
    guard.WriteLine("hello from RAII normal path");
    std::cout << "normal path done\n";
}

static void ExceptionPath()
{
    std::cout << "=== Exception Path ===\n";
    FileGuard guard("raii_output_exception.txt");
    guard.WriteLine("line before exception");
    throw std::runtime_error("simulated failure");
}

int main()
{
    try
    {
        NormalPath();

        try
        {
            ExceptionPath();
        }
        catch (const std::exception& e)
        {
            std::cout << "caught exception: " << e.what() << "\n";
        }
    }
    catch (const std::exception& e)
    {
        std::cerr << "fatal error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
