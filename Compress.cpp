#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <vector>
#include "ArithmeticCoder.hpp"
#include "BitIoStream.hpp"
#include "Model.hpp"
#include "C_Delta.hpp"
#include "FrequencyTable.hpp"

using std::uint32_t;
using std::vector;


// Must be at least -1 and match PpmDecompress. Warning: Exponential memory usage at O(257^n).
static constexpr int MODEL_ORDER = 3;

static void compress(std::ifstream &in, BitOutputStream &out);
static void encodeSymbol(Model &model, vector <uint32_t> &history, uint32_t symbol, ArithmeticEncoder &enc);
static void encodePrelude(std::ifstream &in, BitOutputStream &out, Model &model);
static void write(Model &model, uint32_t symbol, ArithmeticEncoder &enc, vector <uint32_t> &EPVec);
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
	
	// Perform file compression
	std::ifstream in(inputFile, std::ios::binary);
	std::ofstream out(outputFile, std::ios::binary);
	BitOutputStream bout(out);
	try {
		compress(in, bout);
		bout.finish();
		return EXIT_SUCCESS;
	} catch (const char *msg) {
		std::cerr << msg << std::endl;
		return EXIT_FAILURE;
	}
}


static void compress(std::ifstream &in, BitOutputStream &out) {
	ArithmeticEncoder enc(32, out);

	Model model(MODEL_ORDER);
	vector <uint32_t> history;

	encodePrelude(in, out, model);
	if (model.num_of_escapeSymbol!=1) {
		// Read input file again, compress with arithmetic coding, and write output file
		while (true) {
			if (model.num_of_escapeSymbol==1)
				break;
			// Read and encode one byte
			int symbol = in.get();
			if (symbol == EOF)
				break;
			if (symbol < 0 || symbol > 255)
				throw std::logic_error("Assertion error");

			uint32_t sym = static_cast<uint32_t>(symbol);
			encodeSymbol(model, history, sym, enc);
			model.updateContexts(sym, history, true);
			if (model.model_order >= 1) {
				if (history.size() >= model.model_order - 1){
					history.erase(history.begin());
				}
				history.push_back(symbol);
			}
		}
		// encode EOF
		enc.finish();  // Flush remaining code bits
	}
}

static void encodePrelude(std::ifstream &in, BitOutputStream &out, Model &model) {
	C_DeltaEncoder C_DeltaEnc(out);

	// Read input file once to compute symbol frequencies
	while (true) {
	    int symbol = in.get();
    	if (symbol == EOF)
            break;
    	if (symbol < 0 || symbol > 255)
    	    throw std::logic_error("Assertion error");
		model.increment(static_cast<uint32_t>(symbol));
	}
	// encode a frequency for each symbol from ascii 
    for (uint32_t i = 0; i < 256; i++){
        int32_t j=model.findLetter(i);
        if(j!=-1)
		    C_DeltaEnc.encode(model.nodeVector.at(j).freq+1);
		else
		    C_DeltaEnc.encode(1);
    }        
	// set in to the start of the file
	in.clear();
	in.seekg(0);
}


static void encodeSymbol( Model &model, vector <uint32_t> &history, uint32_t symbol, ArithmeticEncoder &enc){
	// Try to use highest order context that exists based on the history
	// When symbol 256 is produced at a context at any order
	// it means escape to a lower order
	vector <uint32_t> EPVec;
	bool found=false;
	for (uint32_t order = 0 ; !found && order < history.size(); order++) {
		Model *modelContext=&model;
		for (uint32_t curr=order ; curr<history.size(); curr++){
			int32_t i= modelContext->findLetter(history.at(curr));
			if (i==-1){
				throw std::logic_error("no context found empty model");					
			}
			modelContext=&(modelContext->nodeVector.at(i).model);
		}
		int32_t i= modelContext->findLetter(symbol);
		if ( i!=-1){
			write(*modelContext, symbol, enc, EPVec);
			found=true;
		}
		else{
			if(modelContext->num_of_escapeSymbol>0){
				write(*modelContext, ESCAPE_SYMBOL, enc, EPVec);	
				for (uint32_t i = 0; i < modelContext->nodeVector.size(); i++)
					if(!isInVec(EPVec, modelContext->nodeVector.at(i).letter))
						EPVec.push_back(modelContext->nodeVector.at(i).letter);
			}
		}
	}
	if(!found){ 
		int32_t i= model.findLetter(symbol);
		if (i==-1)
			throw std::logic_error("no context found");
		write(model, symbol,enc, EPVec);
	}
}

static void write(Model &model, uint32_t symbol, ArithmeticEncoder &enc, vector <uint32_t> &EPVec){
	vector <uint32_t> freqVector(257);
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
	if(!isPropOne){
		/*if(symbol==ESCAPE_SYMBOL)
			cout<<"encode: $ on model: ";
		else
			cout<<"encode: "<<char(symbol)<<"="<<symbol<<" on model: ";
		cout<<model;
		*/
		enc.write(freqTable,symbol);

	}
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