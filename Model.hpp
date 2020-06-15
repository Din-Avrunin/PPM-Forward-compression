#include <iostream>
#include <vector>
#include <string.h>
#include <bits/stdc++.h>

#pragma once
using namespace std;
static constexpr uint32_t ESCAPE_SYMBOL = 256;

class myNode;

class Model{

public:

    //// variables
    uint32_t total;
    uint32_t model_order;
    uint32_t num_of_escapeSymbol;
    std::vector <myNode> nodeVector;

    //// methods
    Model(uint32_t model_order);
    ~Model();
    int32_t findLetter(uint32_t l);
    void updateContexts(uint32_t symbol, vector <uint32_t> &history, bool isFirst);
    void increment(uint32_t l);
    void decreaseLetter(uint32_t l, vector <uint32_t> &history);
    bool remove(uint32_t l,bool isMainModel);
    friend std::ostream& operator<<(std::ostream &strm, const Model &model); 

private:
    void addNode(uint32_t l);
    void removeRec(uint32_t l, uint32_t currOrder,bool isMainModel);

};

class myNode {
public:
    //// variables
    uint32_t letter;
    uint32_t freq;
    Model model;

    //// methods
    myNode(uint32_t l ,uint32_t k );
    myNode(uint32_t l ,uint32_t k ,uint32_t num);
    bool operator<(const myNode &other) const { return letter < other.letter; }

};