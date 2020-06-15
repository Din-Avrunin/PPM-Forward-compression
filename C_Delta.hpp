/* 
 * Reference C_Delta coding
 * Copyright (c) Project Nayuki
 * 
 * https://www.nayuki.io/page/reference-C_Delta-coding
 * https://github.com/nayuki/Reference-C_Delta-coding
 */

#pragma once
#include <math.h>
#include "BitIoStream.hpp"


/* 
 * Reads from a C_Delta-coded bit stream and decodes symbols.
 */
class C_DeltaDecoder final {
	
	/*---- Fields ----*/
	
	// The underlying bit input stream.
	private: BitInputStream &input;

	/*---- Constructor ----*/
	
	// Constructs a C_Delta decoder based on the given bit input stream.
	public: explicit C_DeltaDecoder(BitInputStream &in);
	
	
	/*---- Method ----*/
	
	// Reads from the input stream to decode the next C_Delta-coded symbol.
	//public: int read();

    //public: std::uint32_t readASCII();

    public: std::uint32_t decode();
	
};


class C_DeltaEncoder final {
	
	/*---- Fields ----*/
	
	// The underlying bit output stream.
	private: BitOutputStream &output;

	/*---- Constructor ----*/
	
	public: explicit C_DeltaEncoder(BitOutputStream &out);
	
	/*---- Method ----*/
	
    public: void encode(uint32_t x);
	
};