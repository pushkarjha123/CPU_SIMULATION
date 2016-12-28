#ifndef _URF
#define _URF
#include<iostream>
#include<vector>
const int RAT_SIZE = 18;
int static debug;    
namespace RAT{
    //0-15 + X + Z
    int registerAliasTable[RAT_SIZE];
    int retirementRegisterAliasTable[RAT_SIZE];
    int VALID = 64;
    void display(){
        write_log(_PRI,"in RAT:display\n");
        write_log(PRIO_ALL,"RAT:\n");
        for(int j=0;j<RAT_SIZE;j++) {
            write_log(PRIO_ALL,"%d ", j);
        }      
        write_log(PRIO_ALL,"\n");
        for(int j=0;j<RAT_SIZE;j++) {
            write_log(PRIO_ALL,"%d ", registerAliasTable[j]);
        }        
        write_log(PRIO_ALL,"\n");
        write_log(PRIO_ALL,"RRAT:\n");
        for(int j=0;j<RAT_SIZE;j++) {
            write_log(PRIO_ALL,"%d ", j);
        }      
        write_log(PRIO_ALL,"\n");
        for(int j=0;j<RAT_SIZE;j++) {
            write_log(PRIO_ALL,"%d ", retirementRegisterAliasTable[j]);
        }        
        write_log(PRIO_ALL,".\n");
    }
    int get(int Table,int ar_regester) {
        if (Table == FRAT) {
            if (registerAliasTable[ar_regester] == -1){                
                write_log(PRIO_ALL, "Dumping The Value of MAP_TABLE Before Exiting:  %d\n",ar_regester);
                display();
                abort();
            } else {
                return (registerAliasTable[ar_regester]%VALID);
            }
        } else if (Table == RRAT){
            //if (retirementRegisterAliasTable[ar_regester] == -1){                
              //  write_log(PRIO_ALL, "Dumping The Value of MAP_TABLE Before Exiting: %d\n", ar_regester);
                //display();
                //abort();
            //} else {
                return retirementRegisterAliasTable[ar_regester];
            //}            
        }
    }
    
    void set(int Table,int ar_regester,int phy_regester) {
        if (Table == FRAT) {
            registerAliasTable[ar_regester] = phy_regester;
        } else if (Table == RRAT){
            retirementRegisterAliasTable[ar_regester] = phy_regester;            
        }
    }
};

namespace URF{
    int SIZE=32;
    struct reg{
        int value;
        int status;
    };
    
    std::vector<reg> TABLE(SIZE);
    
    void init(){
        for(int i=0;i<SIZE;i++)
        {
            TABLE[i].value=-1;
            TABLE[i].status=-1;
        }
        
        for(int j=0;j<RAT_SIZE;j++)
        {
            RAT::registerAliasTable[j]=-1;
            RAT::retirementRegisterAliasTable[j]=-1;			
        }
    }

    void setSize(int a){
        SIZE=a;     	
        TABLE.resize(SIZE);
        init();
        
    }
    
    int getFirstFree() {
        for(int i=0;i<SIZE;i++)
        {
            if (TABLE.at(i).status==-1) {                
                return i;                
            }
        }
        return -1;
    }
    
    int isFull() {
        if(getFirstFree() == -1) {
            return 1;
        } else {    
            return 0;
        }
    }
    
    int get(int index) {
            return TABLE.at(index).value;
    }
    
    void set(int index,int value,int ar_regester) {
        TABLE.at(index).value=value;
        TABLE.at(index).status=ar_regester;   
	//if(debug)
	//abort();     
    }
    
    
    int isValid (int phy_regester) {
		
        return(TABLE[phy_regester].status > 0);
    }
    
    void display(){
        write_log(_PRI,"in URF display\n");
        write_log(PRIO_ALL,"URF Value\n");
        for(int i=0;i<TABLE.size();i++) { 
            write_log(PRIO_ALL,"%d ", i);
        }
        write_log(PRIO_ALL,"\n");        
        for(int i=0;i<TABLE.size();i++) { 
            write_log(PRIO_ALL,"%d ", TABLE.at(i).value);
        }
        write_log(PRIO_ALL,"\nURF Status\n");
        for(int i=0;i<TABLE.size();i++) { 
            write_log(PRIO_ALL,"%d ", i);
        }
        write_log(PRIO_ALL,"\nin URF: Status \n");
        for(int i=0;i<TABLE.size();i++) { 
            write_log(PRIO_ALL,"%d ", TABLE.at(i).status);
        }
        write_log(PRIO_ALL,"\n");
    }

};

    void maintainPreciseState () {
		
		write_log(_PRI,"in precise state: Value\n");
		int phy_reg;
        for(int j=0;j<RAT_SIZE;j++) {
		
		phy_reg=RAT::retirementRegisterAliasTable[j];
            	
		RAT::registerAliasTable[j]=phy_reg;
	        
		if(phy_reg>=0)		
		URF::set(phy_reg,URF::get(phy_reg),1);
				
	}	
		
		for(int j=0;j<URF::SIZE;j++){
			write_log(_PRI,"J : Status : %d %d \n", j,URF::isValid(j));
			if(URF::TABLE.at(j).status!=1){
				URF::set(j,-1,-1);			
			}
		}			
		debug=1;
	    
}
#endif


