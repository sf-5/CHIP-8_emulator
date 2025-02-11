#include "decode.h"

void decode(instruction toDecode, byte registers[REGISTER_SIZE], address *PC, address *I, address stack[STACK_SIZE + 1], byte memory[MEMORY_SIZE], SDL_Renderer *renderer, bool pixels[SCREEN_HEIGHT][SCREEN_WIDTH]) {
    byte N;
    byte NN;
    byte NNN;
    byte X;
    byte Y;
    byte placeholder;
    switch (toDecode / FIRST_DIGIT_DIVISOR) {
    case 0x0:
        switch (toDecode) {
        case 0xE0:
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
            SDL_RenderClear(renderer);
            SDL_RenderPresent(renderer);
            break;
        case 0xEE:
            *PC = pop(stack);
            break;
        }
        break;

    case 0x1:
        *PC = toDecode % FIRST_DIGIT_DIVISOR;
        break;

    case 0x2:
        push(*PC, stack);
        *PC = toDecode % FIRST_DIGIT_DIVISOR;

        break;

    case 0x3:
        NN = toDecode % SECOND_DIGIT_DIVISOR;
        X = (toDecode / SECOND_DIGIT_DIVISOR) % 0x10;

        if (registers[X] == NN) {
            *PC += 0x2;
        }

        break;

    case 0x4:
        NN = toDecode % SECOND_DIGIT_DIVISOR;
        X = (toDecode / SECOND_DIGIT_DIVISOR) % 0x10;

        if (registers[X] != NN) {
            *PC += 0x2;
        }

        break;

    case 0x5:
        X = (toDecode / SECOND_DIGIT_DIVISOR) % 0x10;
        Y = (toDecode / 0x10) % 0x10;

        if (registers[X] == registers[Y]) {
            *PC += 0x2;
        }

        break;

    case 0x6:
        registers[(toDecode / SECOND_DIGIT_DIVISOR) % 0x10] = toDecode % SECOND_DIGIT_DIVISOR;
        break;

    case 0x7:
        registers[(toDecode / SECOND_DIGIT_DIVISOR) % 0x10] += toDecode % SECOND_DIGIT_DIVISOR;
        break;

    case 0x8:
        X = (toDecode / SECOND_DIGIT_DIVISOR) % 0x10;
        Y = (toDecode / 0x10) % 0x10;
        switch (toDecode % 0x10) {
        case 0x0:
            registers[X] = registers[Y];
            break;
        case 0x1:
            registers[X] = registers[X] | registers[Y];
            break;
        case 0x2:
            registers[X] = registers[X] & registers[Y];
            break;
        case 0x3:
            registers[X] = registers[X] ^ registers[Y];
            break;
        case 0x4:
            registers[X] += registers[Y];
            if (registers[X] < registers[Y]) {
                registers[VF] = 1;
            } else {
                registers[VF] = 0;
            }
            break;
        case 0x5:
            placeholder = registers[X];
            registers[X] -= registers[Y];

            if (placeholder >= registers[Y]) {
                registers[VF] = 1;
            } else {
                registers[VF] = 0;
            }
            break;
        case 0x6:
            placeholder = registers[X] & 0x1;
            registers[X] = registers[X] >> 1;
            registers[VF] = placeholder;
            break;
        case 0x7:
            placeholder = registers[X];
            registers[X] = registers[Y] - registers[X];

            if (registers[Y] >= placeholder) {
                registers[VF] = 1;
            } else {
                registers[VF] = 0;
            }
            break;
        case 0xE:
            placeholder = (registers[X] >> 7) & 0x1;
            registers[X] = (registers[X] << 1) & 0xFF;
            registers[VF] = placeholder;
            break;
        }

        break;

    case 0x9:
        X = (toDecode / SECOND_DIGIT_DIVISOR) % 0x10;
        Y = (toDecode / 0x10) % 0x10;

        if (registers[X] != registers[Y]) {
            *PC += 0x2;
        }
        break;

    case 0xA:
        *I = toDecode % FIRST_DIGIT_DIVISOR;
        break;

    case 0xB:
        NNN = toDecode % FIRST_DIGIT_DIVISOR;
        *PC = registers[V0] + NNN;
        break;

    case 0xC:
        X = (toDecode / SECOND_DIGIT_DIVISOR) % 0x10;
        NN = toDecode % SECOND_DIGIT_DIVISOR;

        registers[X] = (rand() % 0xff) & NN;
        break;

    case 0xD:
        X = (toDecode / SECOND_DIGIT_DIVISOR) % 0x10;
        Y = (toDecode / 0x10) % 0x10;
        N = toDecode % 0x10;

        byte xCoord = registers[X] % SCREEN_WIDTH;
        byte yCoord= registers[Y] % SCREEN_HEIGHT;
        registers[VF] = 0;

        for (int i = 0; i < N; i++) {
            byte data = memory[*I + i];
            byte currentX = xCoord;

            for (int j = 7; j >= 0; j--) {
                byte pixel = (data >> j) & 1;
    
                if (pixel != 0) {
                    if (pixels[yCoord][currentX]) {
                        SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
                        pixels[yCoord][currentX] = false;
                        registers[VF] = 1;
                    } else {
                        SDL_SetRenderDrawColor(renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);
                        pixels[yCoord][currentX] = true;
                    }
                    SDL_FRect rect = {currentX * PIXEL_RESIZE, yCoord * PIXEL_RESIZE, PIXEL_RESIZE, PIXEL_RESIZE};
                    SDL_RenderFillRect(renderer, &rect);
                }

                currentX++;
                if (currentX >= SCREEN_WIDTH) {
                    break;
                }
            }

            yCoord++;
            if (yCoord >= SCREEN_HEIGHT) {
                break;
            }

        }

        SDL_RenderPresent(renderer);
        
        break;

    case 0xE:
        printf("%x\n", toDecode);
        break;

    case 0xF:
        X = (toDecode / SECOND_DIGIT_DIVISOR) % 0x10;
        printf("%x\n", toDecode % SECOND_DIGIT_DIVISOR);

        switch (toDecode % SECOND_DIGIT_DIVISOR) {
        case 0x1E:
            if (*I + registers[X] > 0x0FFF || *I + registers[X] < *I) {
                registers[VF] = 0x1;
            }
            *I += registers[X];
            break;
        case 0x55:
            for (int i = 0; i <= X; i++) {
                memory[*I + i] = registers[i];
            }
            break;
        case 0x65:
            for (int i = 0; i <= X; i++) {
                registers[i] = memory[*I + i];
            }
            break;
        }

        break;
    }
}
