#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <vector>
#include "ArithmeticCoder.hpp"
#include "BitIoStream.hpp"
#include "Model.hpp"
#include "C_Delta.hpp"

using std::uint32_t;
using std::vector;


// Must be at least -1 and match PpmDecompress. Warning: Exponential memory usage at O(257^n).
static constexpr int MODEL_ORDER = 3;


static void decompress(BitInputStream &in, std::ostream &out);
static uint32_t decodeSymbol(ArithmeticDecoder &dec, Model &model, vector <uint32_t> &history);
static void decodePrelude(BitInputStream &in, Model &model);
static uint32_t read(Model &model,ArithmeticDecoder &dec, vector <uint32_t> &EPVec);
static bool isInVec(vector <uint32_t> &EPVec,uint32_t letter);
//static void printVec(vector <uint32_t> &vec);

int main(int argc, char *argv[]) {
	// Handle command line arguments
	if (argc != 3) {
		std::cerr << "Usage: " << argv[0] << " InputFile OutputFile" << std::endl;
		return EXIT_FAILURE;
	}
	const char *inputFile  = argv[1];
	const char *outputFile = argv[2];
	
	// Perform file decompression
	std::ifstream in(inputFile, std::ios::binary);
	std::ofstream out(outputFile, std::ios::binary);
	BitInputStream bin(in);
	try {
		decompress(bin, out);
		return EXIT_SUCCESS;
	} catch (const char *msg) {
		std::cerr << msg << std::endl;
		return EXIT_FAILURE;
	}
}


static void decompress(BitInputStream &in, std::ostream &out) {
	// Set up decoder and model. In this PPM model, symbol 256 represents EOF;
	// its frequency is 1 in the order -1 context but its frequency
	// is 0 in all other contexts (which have non-negative order).
	Model model(MODEL_ORDER);
	vector <uint32_t> history;
	decodePrelude(in, model);
	sort(model.nodeVector.begin(),model.nodeVector.end());
	ArithmeticDecoder dec(32, in);

	while (true) {
        if (model.num_of_escapeSymbol==1)
			break;
		// Decode and write one byte
		uint32_t symbol = decodeSymbol(dec, model, history);		
		int b = static_cast<int>(symbol);
		if (std::numeric_limits<char>::is_signed)
			b -= (b >> 7) << 8;
		out.put(static_cast<char>(b));
		model.updateContexts(symbol, history, true);
		if (model.model_order >= 1) {
			if (history.size() >= model.model_order-1){
				history.erase(history.begin());
			}
			history.push_back(symbol);
		}
	}
	for (uint32_t i = 0; i < model.nodeVector.size(); i++){
    	if(model.nodeVector.at(i).freq>0){
			uint32_t b = model.nodeVector.at(i).letter;		
    		for (uint32_t j=0; j<model.nodeVector.at(i).freq; j++){
        		out.put(static_cast<char>(b));
			}    
		
		}
	}
}

static void decodePrelude(BitInputStream &in, Model &model) {
	C_DeltaDecoder C_DeltaDec(in);
	// Read code length table
    uint32_t val = 0;
    for (uint32_t i = 0; i < 256; i++){
        val = C_DeltaDec.decode()-1;
        for (uint32_t j = 0; j < val; j++)
            model.increment(i);
    }
}

static uint32_t decodeSymbol(ArithmeticDecoder &dec, Model &model, vector <uint32_t> &history) {
	vector <uint32_t> EPVec;
    for (uint32_t order = 0 ; order < history.size(); order++) {
        Model *modelContext=&model;
        for (uint32_t curr=order ; curr<history.size(); curr++){
            int32_t i= modelContext->findLetter(history.at(curr));
            if (i==-1)
                throw std::logic_error("no context found");
            modelContext=&(modelContext->nodeVector.at(i).model);
        }      
        if(modelContext->num_of_escapeSymbol>0){
			uint32_t symbol = read(*modelContext, dec, EPVec);
			if (symbol!=256)
				return symbol;
			else
				for (uint32_t i = 0; i < modelContext->nodeVector.size(); i++)
					if(!isInVec(EPVec, modelContext->nodeVector.at(i).letter))
						EPVec.push_back(modelContext->nodeVector.at(i).letter);
		}
	}
	uint32_t symbol = read(model, dec, EPVec);
	if (symbol!=256)
		return symbol;
	else
		throw std::logic_error("symbol error");
}

static uint32_t read(Model &model, ArithmeticDecoder &dec, vector <uint32_t> &EPVec){
	vector<uint32_t> freqVector(257);
	bool isPropOne=true;
	int num=0;
	for (uint32_t i = 0; i < model.nodeVector.size(); i++){
		if(!isInVec(EPVec,model.nodeVector.at(i).letter)){
			freqVector.at(model.nodeVector.at(i).letter)=model.nodeVector.at(i).freq;
			isPropOne=false;
			num++;
		}
	}
	if (MODEL_ORDER!=model.model_order)
		freqVector.at(256)=num;
	SimpleFrequencyTable freqTable(freqVector);
	if(isPropOne)
		return ESCAPE_SYMBOL;
	return dec.read(freqTable);
}

static bool isInVec(vector <uint32_t> &EPVec,uint32_t letter){
	for (uint32_t i = 0; i < EPVec.size(); i++)
		if(EPVec.at(i)==letter)
			return true;
	return false;
}

/*
static void printVec(vector <uint32_t> &vec){
	cout<<"(";
	for (uint32_t i = 0; i < vec.size(); i++){
		cout<<vec.at(i)<<", ";
	}
	cout<<")"<<endl;
}
*/