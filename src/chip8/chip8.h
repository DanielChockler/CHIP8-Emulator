#include <cstdint>

class Chip8 {
private:
    uint8_t memory[4096];
    uint8_t V[16]; // registers
    uint16_t stack[16];
    uint16_t sp; // stack pointer
    uint16_t I; // 12-bit address register
    uint16_t pc; // 12-bit program counter
    uint16_t opcode;

    // timer registers
    uint8_t delayTimer;
    uint8_t soundTimer;
    
public:
    void initialise();
    void emulateCycle();
    bool loadGame(const char* filename);
    void setKeys();
};
