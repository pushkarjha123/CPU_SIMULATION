#ifndef _MEM
#define _MEM
#include "Apex_Pipeline.hpp"
#include <cstdio>
namespace MEM{
    /*****************************************************
Function for reading the value at any given memory location.
******************************************************/
    int readMem(int location) {
        write_log(_PRI,"in readMem stage\n");
        write_log(_PRI,"%d,%d\n",location,memoryMap[location/4]);
        return memoryMap[location/4];
    }


    /*****************************************************
Function responsible performing write operation.
******************************************************/
    void writeMem(int location, int value) {
        write_log(_PRI,"in writeMem stage\n");
        memoryMap[location/4] = value;
    }

    void display(int a, int b) {
        int count=0;
	
        for(int i =a; i<=b; i++)
        {
            if((i+1)%10==0){
               write_log(PRIO_ALL,"%d,%d ",i,memoryMap[i]);
		write_log(PRIO_ALL,"\n");
               } else {
		write_log(PRIO_ALL,"%d,%d ",i,memoryMap[i]);
                count++;
            }
        }
      write_log(PRIO_ALL,"\n");
    }
    
     void display() {
        display(0,_MEM_SIZE-1);
    }
};

#endif
