#ifndef _ROB
#define _ROB
#include "Util.hpp"
#include<vector>
#include<deque>
#include<algorithm>
#include<iostream>

using namespace std;

namespace ROB {
    static int currentSize=0;
    static int maxSize=40;
    void display();
    struct STRUCTURE{
        //int robEntryvalid; for implementation purpose     0- invalid and 1 -valid   
        int result;
        bool status;
        int pcValue;
        int arAddress;
        int type;
        int phyAddress;
        int conditioncode;
        int cPhyAddress;
    };


    enum Element{
        RESULT,
        STATUS,
        PC_VALUE,
        AR_ADDRESS,
        PHY_ADDRESS,
        TYPE,
        CONDITIONCODE,
        C_PHY_ADDRESS
    };
    
    STRUCTURE TABLE[40];
    static int head=0;
    static int tail=0;
    static int robindex=0;

    bool isEmpty(){
        return currentSize==0;
    }

    int isFull(){
        //;
        if(head==0 && tail==0)
        return false;
        else
        return currentSize==maxSize;
    }


    void addEntry(){
        
        if(!isFull()){
            TABLE[head].result=-1;
            TABLE[head].status=false;
            TABLE[head].pcValue=0;
            TABLE[head].arAddress=-1;
            TABLE[head].type=-1;
            TABLE[head].conditioncode=-1;
            TABLE[head].cPhyAddress=-1;
            //TABLE[head].robEntryvalid=0;
            
            //currentSize=currentSize+1;
        }
    }
    int addEntry(int result, bool status, int pcValue, int arAddress, int phyAddress, int type,int conditioncode){
        
        int index;
        
        if(isFull())
        {write_log(PRIO_ALL,"cannot Insert. Maximum size exceeded\n");}	
        else{
            
            TABLE[tail].result=result;
            TABLE[tail].status=status;
            TABLE[tail].pcValue=pcValue;
            TABLE[tail].arAddress=arAddress;
            TABLE[tail].phyAddress=phyAddress;
            TABLE[tail].type=type;
            TABLE[tail].conditioncode=conditioncode;
            TABLE[tail].cPhyAddress=-1;
            
            currentSize=currentSize+1;

            robindex++;  index=tail;

            
            if(tail+1==maxSize && (tail+1)%maxSize!=head)
            tail=(tail+1)%maxSize;
            else if(tail+1==maxSize)
            {tail=(tail+1)%maxSize;}
            else 
            tail++; 
        }
        return index;    
    }


    void writeEntry(int index, int element, int value){
        if(index>maxSize){
            write_log(_PRI,"Accessing Out Of Size Element\n");
        } else { 
            switch (element) {	
                
            case STATUS : 
                TABLE[index].status=value;
                break;
                
            case PC_VALUE : 
                TABLE[index].pcValue=value;
                break;

            case AR_ADDRESS : 
                TABLE[index].arAddress=value;
                break;
                
            case TYPE : 
                TABLE[index].type=value;
                break;
                
            case RESULT : 
                TABLE[index].result=value;
                break;	
            case CONDITIONCODE : 
                TABLE[index].conditioncode=value;
                break;	
            case PHY_ADDRESS : 
                TABLE[index].phyAddress=value;
                break;
            case C_PHY_ADDRESS : 
                TABLE[index].cPhyAddress=value;
                break;
                
                default :
                write_log(PRIO_ALL,"Element does not exist %d\n", element);
                abort();
            }
            
        }
        
    }

    void deleteEntry(){
        if(isEmpty()){
            write_log(_PRI,"Accessing Out Of Size Element\n");
        } else {
            if((head+1)!=maxSize)
            head++;
            else if(head==maxSize-1)
            head=(head+1)%maxSize;
            currentSize--;
        }
	
    }

    int readEntry(int index, int element){
        
        if(index>maxSize || isEmpty()){
            write_log(_PRI,"Accessing Out Of Size Element\n");
            return 0;
        } else {
            switch (element) {	
                
            case STATUS : 
                return TABLE[index].status;
                break;
                
            case PC_VALUE : 
                return TABLE[index].pcValue;
                break;

            case AR_ADDRESS : 
                return TABLE[index].arAddress;
                break;
                
            case TYPE : 
                return TABLE[index].type;
                break;
                
            case RESULT : 
                return TABLE[index].result;
                break;	
            case PHY_ADDRESS : 
                return TABLE[index].phyAddress;
                break;	
            case C_PHY_ADDRESS : 
                return TABLE[index].cPhyAddress;
                break;
            default :
                write_log(PRIO_ALL,"Element does not exist %d\n", element);
                abort();	
            }
            
        }
        
        return 0;
    }
    void display(){
        write_log(_PRI,"in IQ:display\n");
        write_log(PRIO_ALL,"\n...ROB...\nID:\tstatus\tresult\tpcVal\ttype\tarAddr\tphyAddr\tcPhyAddr\tCC\n");
        if(robindex){    
            if(head<=tail) { 
                for(int it = head; it<tail; ++it){
                    write_log(PRIO_ALL,"%d:\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\n",it,
                    TABLE[it].status,
                    TABLE[it].result,
                    TABLE[it].pcValue,
                    TABLE[it].type,
                    TABLE[it].arAddress,
                    TABLE[it].phyAddress,
                    TABLE[it].cPhyAddress,
                    TABLE[it].conditioncode
                    );
                }
            } else  {
                for(int it = head; it<maxSize; ++it){
                    write_log(PRIO_ALL,"%d:\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\n",it,
                    TABLE[it].status,
                    TABLE[it].result,
                    TABLE[it].pcValue,
                    TABLE[it].type,
                    TABLE[it].arAddress,
                    TABLE[it].phyAddress,
                    TABLE[it].cPhyAddress,
                    TABLE[it].conditioncode
                    );
                }
                for(int it =0; it<tail; ++it){
                    write_log(PRIO_ALL,"%d:\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\n",it,
                    TABLE[it].status,
                    TABLE[it].result,
                    TABLE[it].pcValue,
                    TABLE[it].type,
                    TABLE[it].arAddress,
                    TABLE[it].phyAddress,
                    TABLE[it].cPhyAddress,
                    TABLE[it].conditioncode
                    );
                }
            }
        }
        write_log(PRIO_ALL,"\n");
    }
    void display1(){
        write_log(_PRI,"in ROB:display\n");
        write_log(PRIO_ALL,"Result:");   
        if(robindex){    
            if(head<=tail) { 
                for(int it = head; it<tail; ++it){
                    write_log(PRIO_ALL,"%d ", TABLE[it].result);
                }
            }
            else  {
                for(int it = head; it<maxSize; ++it){
                    write_log(PRIO_ALL,"in else %d ", TABLE[it].result);
                }
                for(int it =0; it<tail; ++it){
                    write_log(PRIO_ALL,"%d ", TABLE[it].result);
                }
            }
            write_log(PRIO_ALL,".\n");
            write_log(PRIO_ALL,"Status:");
            if(head<=tail) { 
                for(int it = head; it<tail; ++it){
                    write_log(PRIO_ALL,"%d ",TABLE[it].status);
                }
            }
            else  {
                for(int it = head; it<maxSize; ++it){
                    write_log(PRIO_ALL,"%d ",TABLE[it].status);
                }
                for(int it =0; it<tail; ++it){
                    write_log(PRIO_ALL,"%d ",TABLE[it].status);
                }
            }
            write_log(PRIO_ALL,".\n");
            write_log(PRIO_ALL,"Pc Val:");
            if(head<=tail) { 
                for(int it = head; it<tail; ++it){
                    write_log(PRIO_ALL,"%d ",TABLE[it].pcValue);
                }
            }
            else  {
                for(int it = head; it<maxSize; ++it){
                    write_log(PRIO_ALL,"%d ",TABLE[it].pcValue);
                }
                for(int it =0; it<tail; ++it){
                    write_log(PRIO_ALL,"%d ",TABLE[it].pcValue);
                }
            }
            write_log(PRIO_ALL,".\n");
            write_log(PRIO_ALL,"AAdd:");
            if(head<=tail) { 
                for(int it = head; it<tail; ++it){
                    write_log(PRIO_ALL,"%d ",TABLE[it].arAddress);
                }
            }
            else  {
                for(int it = head; it<maxSize; ++it){
                    write_log(PRIO_ALL,"%d ",TABLE[it].arAddress);
                }
                for(int it =0; it<tail; ++it){
                    write_log(PRIO_ALL,"%d ",TABLE[it].arAddress);
                }
            }
            write_log(PRIO_ALL,".\n");
            write_log(PRIO_ALL,"PAdd:");
            if(head<=tail) { 
                for(int it = head; it<tail; ++it){
                    write_log(PRIO_ALL,"%d ",TABLE[it].phyAddress);
                }
            }
            else  {
                for(int it = head; it<maxSize; ++it){
                    write_log(PRIO_ALL,"%d ",TABLE[it].phyAddress);
                }
                for(int it =0; it<tail; ++it){
                    write_log(PRIO_ALL,"%d ",TABLE[it].phyAddress);
                }
            }
            write_log(PRIO_ALL,".\n");
            write_log(PRIO_ALL,"Type:");
            if(head<=tail) { 
                for(int it = head; it<tail; ++it){
                    write_log(PRIO_ALL,"%d ", TABLE[it].type);
                }
            }
            else  {
                for(int it = head; it<maxSize; ++it){
                    write_log(PRIO_ALL,"%d ", TABLE[it].type);
                }
                for(int it =0; it<tail; ++it){
                    write_log(PRIO_ALL,"%d ", TABLE[it].type);
                }
            }
            write_log(PRIO_ALL,".\n");
            write_log(PRIO_ALL,"CCodes:");
            if(head<=tail) { 
                for(int it = head; it<tail; ++it){
                    write_log(PRIO_ALL,"%d ", TABLE[it].conditioncode);
                }
            }
            else  {
                for(int it = head; it<maxSize; ++it){
                    write_log(PRIO_ALL,"%d ", TABLE[it].conditioncode);
                }
                for(int it =0; it<tail; ++it){
                    write_log(PRIO_ALL,"%d ", TABLE[it].conditioncode);
                }
            }
            write_log(PRIO_ALL,".\n");
            write_log(PRIO_ALL,"CPAddr:");
            if(head<=tail) { 
                for(int it = head; it<tail; ++it){
                    write_log(PRIO_ALL,"%d ", TABLE[it].cPhyAddress);
                }
            }
            else  {
                for(int it = head; it<maxSize; ++it){
                    write_log(PRIO_ALL,"%d ", TABLE[it].cPhyAddress);
                }
                for(int it =0; it<tail; ++it){
                    write_log(PRIO_ALL,"%d ", TABLE[it].cPhyAddress);
                }
            }
        }   write_log(PRIO_ALL,".\n");
    }
};

#endif
