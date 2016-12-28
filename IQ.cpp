#ifndef _IQ
#define _IQ
#include "Util.hpp"
#include<vector>
#include<algorithm>
#include<iostream>
using namespace std;

namespace IQ {
    static int currentSize=0;
    static int maxSize=12;

    struct STRUCTURE{
        bool status; 
        int typeFU;
        int lit;
        bool statusS1;
        int tagS1;
        int valueS1;
        bool statusS2;
        int tagS2;  
        int valueS2;
        int dest;
    };
    enum Element{
        STATUS,
        TYPE_FU,
        LIT,
        STATUS_S1,
        TAG_S1,
        VALUE_S1,
        STATUS_S2,
        TAG_S2,
        VALUE_S2,
        DEST
    };
    vector<STRUCTURE> TABLE;
    void display();

    bool isEmpty(){
        return currentSize==0;
    }

    int isFull(){
        return currentSize==maxSize;
    }

    void addEntry(bool status,int typeFU, int lit,bool statusS1,int tagS1, int valueS1,bool statusS2,int tagS2, int valueS2,int dest){
        
        
        if(!isFull()){
            TABLE.push_back(STRUCTURE());
            
            TABLE[currentSize].status=status;
            TABLE[currentSize].typeFU=typeFU;          
            TABLE[currentSize].lit=lit;
            TABLE[currentSize].statusS1=statusS1;
            TABLE[currentSize].tagS1=tagS1;
            TABLE[currentSize].valueS1=valueS1;
            TABLE[currentSize].statusS2=statusS2;
            TABLE[currentSize].tagS2=tagS2;
            TABLE[currentSize].valueS2=valueS2;
            TABLE[currentSize].dest=dest;
            
            currentSize=currentSize+1;
        }else{
            write_log(PRIO_ALL,"Maximum Size of IQ Exceeded %d \n", currentSize);
            write_log(PRIO_ALL, "Dumping The Value of IQ Before Exiting\n");
            display();
            abort();
        }
        
    }


    void writeEntry(int index, int element, int value){
        if(index>TABLE.size()){
            std::cout<<"Accessing Out Of Size Element"<<std::endl;
        } else {
            switch (element) {	
                
            case STATUS : 
                TABLE.at(index).status=value;
                break;
       
            case TYPE_FU : 
                TABLE.at(index).typeFU=value;
                break;
                
            case LIT : 
                TABLE.at(index).lit=value;
                break;

            case STATUS_S1 : 
                TABLE.at(index).statusS1=value;
                break;
            
            case TAG_S1 : 
                TABLE.at(index).tagS1=value;
                break;
                
            case VALUE_S1 : 
                TABLE.at(index).valueS1=value;
                break;
                
            case STATUS_S2 : 
                TABLE.at(index).statusS2=value;
                break;
            
            case TAG_S2 : 
                TABLE.at(index).tagS2=value;
                break;
                
            case VALUE_S2 : 
                TABLE.at(index).valueS2=value;
                break;
                
            case DEST : 
                TABLE.at(index).dest=value;
                break;		
                
            default :
                write_log(PRIO_ALL,"Element does not exist %d \n", element);
                abort();
            }
            
        }
        
    }

    void deleteEntry(int index){
        if(index>TABLE.size()){
            std::cout<<"Accessing Out Of Size Element"<<std::endl;
        } else {
            TABLE.erase(TABLE.begin() + (index));
            currentSize--;
        }
        
        
    }

    int readEntry(int index, int element){
        
        if(index>TABLE.size()){
            std::cout<<"Accessing Out Of Size Element"<<std::endl;
        } else {
            switch (element) {	
                
            case STATUS : 
                return TABLE.at(index).status;
       
            case TYPE_FU : 
                return TABLE.at(index).typeFU;
                
            case LIT : 
                return TABLE.at(index).lit;

            case STATUS_S1 : 
                return TABLE.at(index).statusS1;
            
            case TAG_S1 : 
                return TABLE.at(index).tagS1;
                
            case VALUE_S1 : 
                return TABLE.at(index).valueS1;
            
            case STATUS_S2 : 
                return TABLE.at(index).statusS2;
            
            case TAG_S2 : 
                return TABLE.at(index).tagS2;
                
            case VALUE_S2 : 
                return TABLE.at(index).valueS2;
                
            case DEST : 
                return TABLE.at(index).dest;
                
                default :
                write_log(PRIO_ALL,"Element does not exist %d \n", element);
                abort();	
            }            
        }        
        return 0;
    }
    
    int tagMatchAssign(int TAG,int value) {
        for(int it = 0; it< TABLE.size(); ++it){
            if (TAG == TABLE[it].tagS1) {
                TABLE[it].statusS1 = true;
                TABLE[it].valueS1 = value;
            }
            if (TAG == TABLE[it].tagS2) {
                TABLE[it].statusS2 = true;
                TABLE[it].valueS2 = value;
            }
        }
    }
    
    int select(int VFU) {
        if(isEmpty()) {
            return -1;
        }
        for(int it = 0; it< TABLE.size(); ++it){
            if (VFU == TABLE[it].typeFU && TABLE[it].status && TABLE[it].statusS1 && TABLE[it].statusS2) {
                return it;
            }
        }
        return -1;
    }
    void display(){
        write_log(_PRI,"in IQ:display\n");
        write_log(PRIO_ALL,"\n...IQ...\nID:\tstatus\ttype\tlit\tstaS1\ttagS1\tvalS1\tstaS2\ttagS2\tvalS2\tdest\n");
        for(int it = 0; it< TABLE.size(); ++it){
            write_log(PRIO_ALL,"%d:\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\n",it,
            TABLE[it].status,
            TABLE[it].typeFU,
            TABLE[it].lit,
            TABLE[it].statusS1,
            TABLE[it].tagS1,
            TABLE[it].valueS1,
            TABLE[it].statusS2,
            TABLE[it].tagS2,
            TABLE[it].valueS2,
            TABLE[it].dest
            );
        }
        write_log(PRIO_ALL,"\n");
    }
    void display1(){
        write_log(_PRI,"in IQ:display\n");
        write_log(PRIO_ALL,"Status:");
        for(int it = 0; it< TABLE.size(); ++it){
            write_log(PRIO_ALL,"%d ", TABLE[it].status);
        }
        write_log(PRIO_ALL,"\n");
        write_log(PRIO_ALL,"typeFU:");
        for(int it = 0; it< TABLE.size(); ++it){
            write_log(PRIO_ALL,"%d ", TABLE[it].typeFU);
        }
        write_log(PRIO_ALL,"\n");
        write_log(PRIO_ALL,"lit:");
        for(int it = 0; it< TABLE.size(); ++it){
            write_log(PRIO_ALL,"%d ", TABLE[it].lit);
        }
        write_log(PRIO_ALL,"\n");
        write_log(PRIO_ALL,"StatusS1:");
        for(int it = 0; it< TABLE.size(); ++it){
            write_log(PRIO_ALL,"%d ", TABLE[it].statusS1);
        }
        write_log(PRIO_ALL,"\n");
        write_log(PRIO_ALL,"TagS1:");
        for(int it = 0; it< TABLE.size(); ++it){
            write_log(PRIO_ALL,"%d ", TABLE[it].tagS1);
        }
        write_log(PRIO_ALL,"\n");
        write_log(PRIO_ALL,"VALUES1:");
        for(int it = 0; it< TABLE.size(); ++it){
            write_log(PRIO_ALL,"%d ", TABLE[it].valueS1);
        }        
        write_log(PRIO_ALL,"\n");
        write_log(PRIO_ALL,"StatusS2:");
        for(int it = 0; it< TABLE.size(); ++it){
            write_log(PRIO_ALL,"%d ", TABLE[it].statusS2);
        }
        write_log(PRIO_ALL,"\n");
        write_log(PRIO_ALL,"TagS2:");
        for(int it = 0; it< TABLE.size(); ++it){
            write_log(PRIO_ALL,"%d ",TABLE[it].tagS2);
        }
        write_log(PRIO_ALL,"\n");
        write_log(PRIO_ALL,"ValueS2:");
        for(int it = 0; it< TABLE.size(); ++it){
            write_log(PRIO_ALL,"%d ", TABLE[it].valueS2);
        }        
        write_log(PRIO_ALL,"\n");
        write_log(PRIO_ALL,"dest:");
        for(int it = 0; it< TABLE.size(); ++it){
            write_log(PRIO_ALL,"%d ",TABLE[it].dest);
        }
        write_log(PRIO_ALL,"\n");
    }
};

// int test(){
    // using namespace IQ;
    // _PRI=1;
    // write_log(_PRI,"is Empty? %d \n",IQ::isEmpty());
    // IQ::addEntry();	
    // IQ::addEntry(false,3,4000,false,67,false,2,1);
    // IQ::addEntry(false,3,4000,false,67,false,2,2);
    // IQ::addEntry(false,3,4000,false,67,false,2,3);
    // IQ::addEntry(false,3,4000,false,67,false,2,4);
    // write_log(_PRI,"is Full? %d \n",IQ::isFull());
    
    // IQ::display();	
    
    // IQ::writeEntry(3,STATUS, true);
    // IQ::writeEntry(3,DEST, 16798);
    // IQ::writeEntry(3,STATUS_S1, true);
    // IQ::writeEntry(3,VALUE_S1, 16798);
    // IQ::writeEntry(3,VALUE_S2, 16798);
    // IQ::writeEntry(3,TYPE_FU, 16798);

    // write_log(_PRI,"\n");
    
    // IQ::display();	

    // IQ::deleteEntry(0);
    
    // write_log(_PRI,"\n");
    
    // IQ::display();
    
    
    // return 0;  
// }
#endif