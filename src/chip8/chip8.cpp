#include "chip8.h"
#include <cstdint>
#include <cstddef>
#include <cstdlib>
#include <iostream>

constexpr uint8_t fontset[80] = {
	0xF0, 0x90, 0x90, 0x90, 0xF0,		// 0
	0x20, 0x60, 0x20, 0x20, 0x70,		// 1
	0xF0, 0x10, 0xF0, 0x80, 0xF0,		// 2
	0xF0, 0x10, 0xF0, 0x10, 0xF0,		// 3
	0x90, 0x90, 0xF0, 0x10, 0x10,		// 4
	0xF0, 0x80, 0xF0, 0x10, 0xF0,		// 5
	0xF0, 0x80, 0xF0, 0x90, 0xF0,		// 6
	0xF0, 0x10, 0x20, 0x40, 0x40,		// 7
	0xF0, 0x90, 0xF0, 0x90, 0xF0,		// 8
	0xF0, 0x90, 0xF0, 0x10, 0xF0,		// 9
	0xF0, 0x90, 0xF0, 0x90, 0x90,		// A
	0xE0, 0x90, 0xE0, 0x90, 0xE0,		// B
	0xF0, 0x80, 0x80, 0x80, 0xF0,		// C
	0xE0, 0x90, 0x90, 0x90, 0xE0,		// D
	0xF0, 0x80, 0xF0, 0x80, 0xF0,		// E
	0xF0, 0x80, 0xF0, 0x80, 0x80		// F
};

void Chip8::initialise() {
    pc = 0x200;
    opcode = 0;
    I = 0;
    sp = 0;

    for (auto& i : stack) i = 0;
    for (auto& i : V) i = 0;
    for (auto& i : memory) i = 0;

    for (auto i {0uz}; i < 80; ++i) memory[i] = fontset[i];

    delayTimer = 0;
    soundTimer = 0;
}

void Chip8::emulateCycle() {
    opcode = (memory[pc] << 8) | (memory[pc + 1]);

    switch (opcode & 0xF000) {
    case (0x0000):
        switch(opcode & 0x00FF) {
        case 0x00E0: // opcode 00E0
            // clear screen
            for (auto& i : gfx) i =0;
            drawFlag = true;
            pc += 2;
            break;

        case 0x00EE: // opcode 00EE
            --sp;
            pc = stack[sp];
            pc += 2;
            break;

        default:
            std::cerr << "Unknown opcode 0x0000: " << opcode << '\n';
        }
        break;
    
    case 0x1000: // opcode 0x1NNN
        pc = opcode & 0x0FFF;
        break;

    case 0x2000:  // opcode 0x2NNN
        stack[sp] = pc;
        pc = opcode & 0x0FFF;
        ++sp;
        break;

    case 0x3000: // opcode 0x3XNN
        if (V[(opcode & 0x0F00) >> 8] == (opcode & 0x00FF)) pc += 4;
        else pc += 2;
        break;

    case 0x4000: // opcode 0x4XNN
        if (V[(opcode & 0x0F00) >> 8] != (opcode & 0x00FF)) pc += 4;
        else pc += 2;
        break;

    case 0x5000: // opcode 0x5XY0
        if (V[(opcode & 0x0F00) >> 8] == V[(opcode & 0x00F0) >> 4]) pc += 4;
        else pc += 2;
        break;

    case 0x6000: // opcode 0x6XNN
        V[(opcode & 0x0F00) >> 8] = (opcode & 0x00FF);
        pc += 2;
        break;

    case 0x7000: // 0x7XNN
        V[(opcode & 0x0F00) >> 8] += (opcode & 0x00FF);
        pc += 2;
        break;

    case 0x8000: 
        switch (opcode & 0x000F) {
        case 0x0000: // opcode 0x8XY0
            V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x00F0) >> 4];
            pc += 2;
            break;

        case 0x0001: // opcode 0x8XY1
            V[(opcode & 0x0F00) >> 8] |= V[(opcode & 0x00F0) >> 4];
            pc += 2;
            break;

        case 0x0002: // opcode 0x8XY2
            V[(opcode & 0x0F00) >> 8] &= V[(opcode & 0x00F0) >> 4];
            pc += 2;
            break;
        

        case 0x0003: // opcode 0x8XY3
            V[(opcode & 0x0F00) >> 8] ^= V[(opcode & 0x00F0) >> 4];
            pc += 2;
            break;

        case 0x0004: // opcode 0x8XY4
            if(V[(opcode & 0x00F0) >> 4] > (0xFF - V[(opcode & 0x0F00) >> 8])) V[0xF] = 1;
            else V[0xF] = 0;
            V[(opcode & 0x0F00) >> 8] += V[(opcode & 0x00F0) >> 4];
            pc += 2;
            break;

        case 0x0005: // opcode 0x8XY5
            if(V[(opcode & 0x00F0) >> 4] > V[(opcode & 0x0F00) >> 8]) V[0xF] = 0;
            else V[0xF] = 1;
            V[(opcode & 0x0F00) >> 8] -= V[(opcode & 0x00F0) >> 4];
            pc += 2;
            break;
        
        case 0x0006: // opcode 0x8XY6
            V[0xF] = V[(opcode & 0x0F00) >> 8] & 0x1;
            V[(opcode & 0x0F00) >> 8] >>= 1;
            pc += 2;
            break;

        case 0x0007: // opcode 0x8XY7
            if (V[(opcode & 0x00F0) >> 4] >= V[(opcode & 0x0F00) >> 8])
                V[0xF] = 1;
            else
                V[0xF] = 0;

            V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x00F0) >> 4] - V[(opcode & 0x0F00) >> 8];
            pc += 2;
            break;

        case 0x000E: // opcode 0x8XYE
            V[0xF] = V[(opcode & 0x0F00) >> 8] >> 7;
            V[(opcode & 0x0F00) >> 8] <<= 1;
            pc += 2;
            break;

        default:
            std::cerr << "Unknown opcode 0x8000: " << opcode << '\n';
        }

        break;

    case 0x9000: // opcode 0x9XY0
        if (V[(opcode & 0x0F00) >> 8] != V[(opcode & 0x00F0) >> 4]) pc += 4;
        else pc += 2;
        break;

    case 0xA000: // opcode 0xANNN
        I = opcode & 0x0FFF;
        pc += 2;
        break;

    case 0xB000: // opcode 0xBNNN
        pc = V[0] + (opcode & 0x0FFF);
        break;

    case 0xC000: // opcode 0xCXNN
        V[(opcode & 0x0F00) >> 8] = (rand() % 256) & (opcode & 0x00FF);
        pc += 2;
        break;

    case 0xD000: { // opcode 0xDXYN
        uint8_t x = V[(opcode & 0x0F00) >> 8];
        uint8_t y = V[(opcode & 0x0F00) >> 4];
        uint8_t height = opcode & 0x000F;
        uint8_t pixel;

        V[0xF] = 0;
        for (uint8_t i {0}; i < height; ++i) {
            pixel = memory[I + i];
            for (uint8_t j {0}; j < 8; ++j) {
                if ((pixel & (0x80 >> j)) != 0) {
                    if (gfx[x + j + ((y + i) * 64)] == 1) V[0xF] = 1;
                    gfx[x + j + ((y + i) * 64)] ^= 1;
                }
            }
        }
        drawFlag = true;
        pc += 2;
        break;
    }
    
    case 0xE000: 
        switch (opcode & 0x00FF) {
        case 0x009E: // opcode 0xEX9E
            if (key[V[(opcode & 0x0F00) >> 8]]) pc += 4;
            else pc += 2;
            break;
        
        case 0x00A1: // opcode 0xEXA1
            if (!key[V[(opcode & 0x0F00) >> 8]]) pc += 4;
            else pc += 2;
            break;

        default:
            std::cerr << "Unknown opcode 0xE000: " << opcode << '\n';
        }

        break;

    }


    if (delayTimer > 0) --delayTimer;
    if (soundTimer > 0) {
        if (soundTimer == 1); // play sound
        --soundTimer;
    }
}
