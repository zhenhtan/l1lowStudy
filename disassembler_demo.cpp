#include <cstdint>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

struct DecodeResult {
    std::size_t nextPc;
    std::string text;
    bool ok;
};

static std::string hexByte(uint8_t v) {
    std::ostringstream os;
    os << "0x" << std::hex << std::uppercase << std::setw(2) << std::setfill('0')
       << static_cast<unsigned>(v);
    return os.str();
}

static bool readU32(const std::vector<uint8_t>& code, std::size_t pos, uint32_t& out) {
    if (pos + 4 > code.size()) {
        return false;
    }
    out = static_cast<uint32_t>(code[pos]) |
          (static_cast<uint32_t>(code[pos + 1]) << 8) |
          (static_cast<uint32_t>(code[pos + 2]) << 16) |
          (static_cast<uint32_t>(code[pos + 3]) << 24);
    return true;
}

static DecodeResult decodeOne(const std::vector<uint8_t>& code, std::size_t pc) {
    if (pc >= code.size()) {
        return {pc, "<eof>", false};
    }

    const uint8_t op = code[pc];

    if (op == 0x55) {
        return {pc + 1, "push rbp", true};
    }
    if (op == 0xC3) {
        return {pc + 1, "ret", true};
    }
    if (op == 0x90) {
        return {pc + 1, "nop", true};
    }
    if (op == 0x48 && pc + 2 < code.size() && code[pc + 1] == 0x89 && code[pc + 2] == 0xE5) {
        return {pc + 3, "mov rbp, rsp", true};
    }
    if (op == 0xB8) {
        uint32_t imm = 0;
        if (!readU32(code, pc + 1, imm)) {
            return {pc + 1, "db " + hexByte(op) + " ; truncated imm32", false};
        }
        std::ostringstream os;
        os << "mov eax, " << imm;
        return {pc + 5, os.str(), true};
    }
    if (op == 0x05) {
        uint32_t imm = 0;
        if (!readU32(code, pc + 1, imm)) {
            return {pc + 1, "db " + hexByte(op) + " ; truncated imm32", false};
        }
        std::ostringstream os;
        os << "add eax, " << imm;
        return {pc + 5, os.str(), true};
    }
    if (op == 0xE9) {
        uint32_t rel = 0;
        if (!readU32(code, pc + 1, rel)) {
            return {pc + 1, "db " + hexByte(op) + " ; truncated rel32", false};
        }
        const int32_t relSigned = static_cast<int32_t>(rel);
        const std::size_t target = static_cast<std::size_t>(
            static_cast<int64_t>(pc) + 5 + static_cast<int64_t>(relSigned));
        std::ostringstream os;
        os << "jmp " << target;
        return {pc + 5, os.str(), true};
    }

    return {pc + 1, "db " + hexByte(op) + " ; unknown opcode", false};
}

static void disassemble(const std::vector<uint8_t>& code) {
    std::size_t pc = 0;
    while (pc < code.size()) {
        DecodeResult r = decodeOne(code, pc);
        std::cout << std::setw(4) << pc << ": " << r.text;
        if (!r.ok) {
            std::cout << "  [fallback]";
        }
        std::cout << '\n';

        if (r.nextPc <= pc) {
            break;
        }
        pc = r.nextPc;
    }
}

static bool readBinaryFile(const std::string& path, std::vector<uint8_t>& out) {
    std::ifstream ifs(path, std::ios::binary);
    if (!ifs) {
        return false;
    }

    ifs.seekg(0, std::ios::end);
    const std::streamoff size = ifs.tellg();
    if (size < 0) {
        return false;
    }
    ifs.seekg(0, std::ios::beg);

    out.resize(static_cast<std::size_t>(size));
    if (!out.empty()) {
        ifs.read(reinterpret_cast<char*>(out.data()), size);
    }
    return static_cast<bool>(ifs) || ifs.eof();
}

static std::vector<uint8_t> builtInDemoCode() {
    // A tiny handcrafted machine-code buffer used for demo purposes.
    return {
        0x55,                         // push rbp
        0x48, 0x89, 0xE5,             // mov rbp, rsp
        0xB8, 0x2A, 0x00, 0x00, 0x00, // mov eax, 42
        0x05, 0x08, 0x00, 0x00, 0x00, // add eax, 8
        0x90,                         // nop
        0xC3                          // ret
    };
}

int main(int argc, char* argv[]) {
    std::vector<uint8_t> code;

    if (argc >= 2) {
        const std::string path = argv[1];
        if (!readBinaryFile(path, code)) {
            std::cerr << "Failed to read binary file: " << path << '\n';
            return 1;
        }
        std::cout << "Disassembly from file: " << path << " (" << code.size() << " bytes)\n";
    } else {
        code = builtInDemoCode();
        std::cout << "Disassembly output (built-in demo bytes):\n";
        std::cout << "Usage: ./disassembler_demo <binary_file>\n";
    }

    disassemble(code);
    return 0;
}