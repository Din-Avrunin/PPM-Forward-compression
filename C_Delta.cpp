#include <iostream>
#include <stdexcept>
#include "C_Delta.hpp"


C_DeltaDecoder::C_DeltaDecoder(BitInputStream &in) : input(in) {}

std::uint32_t C_DeltaDecoder::decode(){
    uint32_t num = 1;
    uint32_t len = 1;
    uint32_t lengthOfLen = 0;
    while (!input.readNoEof())     // potentially dangerous with malformed files.
            lengthOfLen++;

    for (uint32_t i = 0; i < lengthOfLen; i++){
        len <<= 1;
        if (input.readNoEof())
            len |= 1;
    }
    for (uint32_t i = 0; i < len - 1; i++){
        num <<= 1;
        if (input.readNoEof())
            num |= 1;
    }
    return num;
}

C_DeltaEncoder::C_DeltaEncoder(BitOutputStream &out) : output(out){}

void C_DeltaEncoder::encode(uint32_t x){
    uint32_t num = x;
    uint32_t len = 0;
    uint32_t lengthOfLen = 0;
    len = 1 + floor(log2(num));  // calculate 1+floor(log2(num))
    lengthOfLen = 1 + floor(log2(len)); // calculate floor(log2(len))

    for (uint32_t i = lengthOfLen - 1; i > 0; --i){
        output.write(0);
    }
    for (uint32_t i = lengthOfLen - 1; i+1 >= 1; --i){
        output.write((len >> i) & 1); // 11 >> 1 = 3/2, 11 >> 0 = 11 
    }
    for (uint32_t i = len - 2; i+1 >= 1; i--){
        output.write((num >> i) & 1); //111>>1
    }
}