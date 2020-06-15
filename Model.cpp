#include "model.hpp"

using namespace std; 

Model::Model(uint32_t model_order){
    total=0;
    num_of_escapeSymbol=0;
    this->model_order=model_order;
}

Model::~Model(){
}

int32_t Model::findLetter(uint32_t l){
    for (uint32_t i = 0; i < nodeVector.size(); i++){
        if(nodeVector.at(i).letter==l)
            return i;
    }
    return -1;
}

void Model::updateContexts(uint32_t l , vector <uint32_t> &history, bool isFirst) {
	if (history.size() > model_order || l >= 256)
		throw std::invalid_argument("Illegal argument");
    for (size_t i = 0; i < history.size(); i++){
        int32_t j=findLetter(history.at(i));
        if (j==-1){
            throw logic_error("history error");
        }
        else{
            vector <uint32_t> tempHistory;
            for (size_t vSize = i+1; vSize < history.size(); vSize++){
                tempHistory.push_back(history.at(vSize));            
                //cout<<char(history.at(vSize));
            }
            //if (i+1< history.size())
                //cout<<endl;
            nodeVector.at(j).model.updateContexts(l , tempHistory, false);
        }
    }
    if (isFirst)
    	decreaseLetter(l,history);
    else if(history.size()==0)
        increment(l);
}

void Model::increment(uint32_t l){
    int32_t i=findLetter(l);
    if (i==-1)
        addNode(l);
    else
        nodeVector.at(i).freq++; 
    total++;
}

void Model::addNode(uint32_t l){
    nodeVector.push_back(myNode(l, model_order - 1));
    num_of_escapeSymbol++;
    total++;
}

bool Model::remove(uint32_t l,bool isMainModel){
    int32_t i=findLetter(l);
    if(i==-1)
        return false;
    total-=nodeVector.at(i).freq;
    if(!isMainModel)
        num_of_escapeSymbol--;
    nodeVector.erase(nodeVector.begin()+i);
    return true;
}

void Model::decreaseLetter(uint32_t l, vector <uint32_t> &history){
    int32_t i=findLetter(l);
    if (i==-1){
        throw std::logic_error("decrease error");
    }
    nodeVector.at(i).freq--;     
    total--;
    //remove unused letters 
    if(nodeVector.at(i).freq==0){
        removeRec(l, model_order-1 ,true);
        num_of_escapeSymbol--;
        total--;
    }
    for (uint32_t j = 0; j < history.size(); j++){
        i=findLetter(history.at(j));
        if(nodeVector.at(i).freq==0 && nodeVector.at(i).letter!=l){
            bool inHistory=false;
            for(uint32_t index=j+1; index<history.size() ; index++)
                if(history.at(j)==history.at(index))
                    inHistory=true;
            if(!inHistory){
                uint32_t diff=model_order-1-history.size();
                removeRec(history.at(j), j+diff, true);                
            }
        }
    }       
}

void Model::removeRec(uint32_t l,uint32_t currOrder,bool isMainModel){
    if (currOrder==0)
        remove(l,isMainModel);
    else {
        for (size_t i = 0; i < nodeVector.size(); i++)
            nodeVector.at(i).model.removeRec(l,currOrder-1,false);
    }
}

std::ostream& operator<<(std::ostream &strm, const Model &model){
    strm <<"model order: "<<model.model_order<<" (";
    for (size_t i = 0; i < model.nodeVector.size(); i++){
        strm <<"("<<static_cast<char>(model.nodeVector.at(i).letter)<<","<<model.nodeVector.at(i).freq<<")"<<" ";
    }
    strm <<"("<<"$,"<<model.num_of_escapeSymbol<<")";
    strm <<"), total is "<<model.total<<"\n";

    return strm;    
}

myNode::myNode(uint32_t l, uint32_t k ): model(k){
    letter=l;
    freq=1;
}
