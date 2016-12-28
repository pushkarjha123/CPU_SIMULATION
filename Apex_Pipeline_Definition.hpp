#ifndef APEX_PIPELINE_CPP
#define APEX_PIPELINE_CPP
#include "Apex_Pipeline.hpp"
/*****************************************************
The starting stage of simulator. 
******************************************************/
void initialize () {
    write_log(_PRI,"in initialize\n");
    resetState();
    populateIcache(InstructionCodeFile);
    pcValueUpdate();
    init_done=1;
}

/*****************************************************
This is the Function which calls all the stages of pipe
line. The pre-condition to this function is that Simulator
must be initialized.
******************************************************/
void simulate (int cycles) {
    write_log(_PRI,"in simulate with %d cycles\n",cycles);
    if(init_done==0)
    {write_log(_PRI,"Please initialize\n"); return;}
    for (int i=0;i<cycles;i++) {
        simSteps++;
        resetForwarding();
        commit();
        wb();
        vfu2();
        vfu1();
        issue();
        drf2();
        drf1();
        fetch();
        pcUpdate();
    }
    if (_PRI >= LOGPRIORITY) {
        display();
    }    
}

/*****************************************************
This is the Function which displays the content of Archi-
tectural register, memory value, instructions in pipelineStages
program Counter Value.
******************************************************/
void display () {
    if(init_done==0){
        write_log(_PRI,"Please initialize\n"); 
        return;
    }
    write_log(_PRI,"in display\n");
    displayForwardingDataPaths();
    displayNormalDataPaths();
    displayVFUStatus();
    URF::display();
    RAT::display();
    ROB::display();
    IQ::display();
    MEM::display(0,100);    
    
    write_log(PRIO_ALL,"Steps:%d\n",simSteps);
    for(int i=0;i<3;i++){
        if (i == 0) {
            write_log(PRIO_ALL,"%s:-PcValue: %d InstructionRegister: %d\n", pipelineStageNames[i],pcValue[i], instructionRegister[i]);
            
        } else {
            write_log(PRIO_ALL,"%s:-PcValue: %d InstructionNames: %s  instructionRegister %d\n", pipelineStageNames[i],pcValue[i], instructionNames[decoder[i][0]],  instructionRegister[i]);
        }

        for(int j=0;j<4;j++){
            write_log(PRIO_ALL, "Decoder %d ",decoder[i][j]);
            
        }
        write_log(PRIO_ALL,"\n");
    }
}
/*****************************************************
This is the Helper function to give the user shell for
providing input value to Simulator.
******************************************************/
void shell () {
    char *line = NULL;
    size_t bufsize = 0; // have getline allocate a buffer for us
    int status=1;

    do {
        write_log(PRIO_LOG,"> ");
        getline(&line, &bufsize, stdin);
        if(line[0]=='i' || line[0]=='s' || line[0]=='P'|| line[0]=='q' || line[0]=='d' || line[0]=='S'){
            status = execute(line);
        }else
        cout<<"Enter Valid option"<<endl;
    } while (status);
}

/*****************************************************
Helper Function to deduce the shell input.
******************************************************/
int execute(char *line) {

    int status = 1;
    int pos = 0, buffSize = 64;
    char* TOK_DELIM = (char *)" \r\n";
    char* token;
    char* tokens[20];
    
    if(strcmp(line,"Print_map_tables\n")==0)
    line[0]='m';  

    if(strcmp(line,"Print_IQ\n")==0)
    {line[0]='Z';} 

    if(strcmp(line,"Print_ROB\n")==0)
    line[0]='R'; 

    if(strcmp(line,"Print_URF\n")==0)
    line[0]='U'; 

    if(line[7]=='e')
    line[0]='O';

    if(strcmp(line, "Print_Stats\n")==0)
    line[0]='T';
    
    
    token = strtok(line, TOK_DELIM);
    while (token != NULL) {
        //write_log(_PRI,"<%s>",token);
        tokens[pos] = token;
        pos++;
        token = strtok(NULL, TOK_DELIM);
    }
    tokens[pos] = NULL;

    switch (tokens[0][0]) {
    case 'i' :
        initialize();
        break;
    case 'd':
        display();
        break;
    case 's' :
        if (tokens[1] != NULL) {
            simulate(atoi( tokens[1]));
        } else {
            simulate(1);
        }
        break;
    case 'S':
        if (tokens[1] != NULL) {
            URF::setSize(atoi( tokens[1]));
        }
        break;
    case 'm':	
        RAT::display();
        break;
    case 'Z' :
        IQ::display();
        break;
    case 'R' :
        ROB::display();
        break;
    case 'U' :
        URF::display();
        break;
    case 'O' :
        if (tokens[1] != NULL && tokens[2] != NULL) {
            MEM::display(atoi(tokens[1]), atoi(tokens[2]) );
          }else {
		MEM::display(0,100);
	 } 
        break;
    case 'T' :
	Print_Stats();
        break;

    case 'q' :
        status = 0;
        break;

        default :
        write_log(_PRI,"Following are the supported commands\n");
        write_log(_PRI,"initialize|i\n");
        write_log(_PRI,"simulate|s <n>\n");
        write_log(_PRI,"display|d\n");
        write_log(_PRI,"quit|q\n");

    }
    return status;
    
}

//void vfuLSWB();

void commit() {
    write_log(_PRI,"In commit stage\n");
    
    if (ROB::readEntry(ROB::head,ROB::STATUS)) {        
	    robDeleteCount++;
        int retireAddress;
        int cRetireAddress;
        int arAddress = ROB::readEntry(ROB::head,ROB::AR_ADDRESS);
        int phyAddress = ROB::readEntry(ROB::head,ROB::PHY_ADDRESS);
        int cPhyAddress = ROB::readEntry(ROB::head,ROB::C_PHY_ADDRESS);        
        int instruction=ROB::readEntry(ROB::head,ROB::TYPE);
        if(instruction== HALT){
            while (!ROB::isEmpty()) {                
                ROB::deleteEntry();
            }
            display();
            Print_Stats();
            write_log(PRIO_ALL,"HALT...\n");
            exit(0);
        }
        if((instruction== LOAD) || (instruction== STORE)){

            if(instruction== LOAD){
                
                //URF::set(phyAddress,memoryMap[latchLS1LS2[1]],arAddress);
                write_log(_PRI,"in Load %d %d\n", latchLS1LS2[1], MEM::readMem(latchLS1LS2[1]));
                int data = MEM::readMem(latchLS1LS2[1]);
                URF::set(phyAddress,data,0);
                vfuLsStatus2=VFU_FREE;
                latchLSIssue[1] = data;
                latchLSIssue[0] = phyAddress;	
                latchLSDrf[0]=data;	
                latchLSDrf[1]= phyAddress;
		loadCount++;
            }
            else{
                MEM::writeMem( latchLS1LS2[1], latchLS1LS2[0]);
                //memoryMap[latchLS1LS2[1]] = latchLS1LS2[0];
                vfuLsStatus2=VFU_FREE;
                ROB::deleteEntry();
		storeCount++;
                return;
      
            }
        } else if((instruction== BZ) || (instruction== BNZ) || (instruction== JUMP) || (instruction== BAL)){    
            
            int to_branch = ROB::readEntry(ROB::head,ROB::RESULT);
            if (to_branch>= 0) {
                if (instruction== BAL) {
                    RAT::set(RRAT,arAddress,phyAddress);
                    URF::set(phyAddress,ROB::readEntry(ROB::head,ROB::PC_VALUE) + 4,1);
                }
                ROB::deleteEntry();                    
                while (!ROB::isEmpty()) {
                    retireAddress = ROB::readEntry(ROB::head,ROB::PHY_ADDRESS);
                        cRetireAddress = ROB::readEntry(ROB::head,ROB::C_PHY_ADDRESS);
                        if (cRetireAddress >=0) {
                            URF::set(cRetireAddress,URF::get(cRetireAddress),-1);
                        }
                    if (retireAddress >=0) {
                        URF::set(retireAddress,URF::get(retireAddress),-1);
                    }
                    ROB::deleteEntry();
                }                                
                maintainPreciseState();
                vfuBranchStatus = VFU_FREE;
                flush(to_branch);
                stalled = 0;
                return;
            }else {
                if (instruction== BAL) {
                    URF::set(phyAddress,URF::get(phyAddress),-1);
                }				
                ROB::deleteEntry(); 
                return;}
        }
        write_log(_PRI,"c:%d",cPhyAddress);
        if (cPhyAddress >=0) {
            int stale_entry = RAT::get(RRAT,17);
            
            if(stale_entry>=0){write_log(_PRI,"In Stale stage %d \n",stale_entry);
                URF::set(stale_entry,-1,-1);
                
            }
            RAT::set(RRAT,17,cPhyAddress);
            if (RAT::get(FRAT,17) == cPhyAddress)
            URF::set(cPhyAddress,URF::get(cPhyAddress),1);
                        
        }
        int stale_entry = RAT::get(RRAT,arAddress);
        write_log(_PRI,"stale :%d\n\n",stale_entry);            
        if(stale_entry>=0){write_log(_PRI,"In stale_entry stage stale_entry %d\n",stale_entry);
            URF::set(stale_entry,-1,-1);
            
        }
        RAT::set(RRAT,arAddress,phyAddress);
        //write_log(_PRI,"Frat address :%d  Phyaddress  %d\n\n",RAT::get(FRAT,arAddress),phyAddress ); 
        if (RAT::get(FRAT,arAddress) == phyAddress)
        URF::set(phyAddress,URF::get(phyAddress),1);
        // else {
            // write_log(_PRI,"Frat address :%d  Phyaddress  %d\n\n",RAT::get(FRAT,arAddress),phyAddress );
            // URF::set(phyAddress,-1,-1);
        // }
        ROB::deleteEntry();
    }
    
}


//void vfuBranchWB();
void vfuMULWB() {
    write_log(_PRI,"in MUL wb stage\n");
    if(latchWBMULVFU[1] != -1) {
        int phyAddress = ROB::readEntry(latchWBMULVFU[1],ROB::PHY_ADDRESS);
        int cPhyAddress = ROB::readEntry(latchWBMULVFU[1],ROB::C_PHY_ADDRESS);
        ROB::writeEntry(latchWBMULVFU[1],ROB::RESULT,latchWBMULVFU[0]);
        ROB::writeEntry(latchWBMULVFU[1],ROB::CONDITIONCODE,latchWBMULVFU[2]);
        ROB::writeEntry(latchWBMULVFU[1],ROB::STATUS,true);
        URF::set(phyAddress,latchWBMULVFU[0],0);
        if (latchWBMULVFU[2] >=0 ) {
            URF::set(cPhyAddress,latchWBMULVFU[2],0);
        }
        //forwarding
        latchMULWBDrf2[LATCH_TAG] = phyAddress;
        latchMULWBDrf2[LATCH_VALUE] = latchWBMULVFU[0];
        latchMULWBIssue[LATCH_TAG] = phyAddress;
        latchMULWBIssue[LATCH_VALUE] = latchWBMULVFU[0];
        latchMULWBZIssue[LATCH_VALUE] = latchWBMULVFU[2];
        latchMULWBZIssue[LATCH_TAG] = cPhyAddress;
        latchMULWBZDrf2[LATCH_VALUE] = latchWBMULVFU[2];
        latchMULWBZDrf2[LATCH_TAG] = cPhyAddress;
        latchWBMULVFU[1] = -1;
    }
}
void vfuALUWB() {
    write_log(_PRI,"in ALU wb stage\n");
    if(latchWBALUVFU[1] != -1) {        
        int phyAddress = ROB::readEntry(latchWBALUVFU[1],ROB::PHY_ADDRESS);
        int cPhyAddress = ROB::readEntry(latchWBALUVFU[1],ROB::C_PHY_ADDRESS);
        ROB::writeEntry(latchWBALUVFU[1],ROB::RESULT,latchWBALUVFU[0]);
        ROB::writeEntry(latchWBALUVFU[1],ROB::CONDITIONCODE,latchWBALUVFU[2]);
        ROB::writeEntry(latchWBALUVFU[1],ROB::STATUS,true);
        URF::set(phyAddress,latchWBALUVFU[0],0);        
        if (latchWBALUVFU[2] >=0 ) {
            URF::set(cPhyAddress,latchWBALUVFU[2],0);
        }
        //forwarding
        latchALUWBDrf2[LATCH_TAG] = phyAddress;
        latchALUWBDrf2[LATCH_VALUE] = latchWBALUVFU[0];
        latchALUWBIssue[LATCH_TAG] = phyAddress;
        latchALUWBIssue[LATCH_VALUE] = latchWBALUVFU[0];    
        latchALUWBZIssue[LATCH_VALUE] = latchWBALUVFU[2];
        latchALUWBZIssue[LATCH_TAG] = cPhyAddress;
        latchALUWBZDrf2[LATCH_VALUE] = latchWBALUVFU[2];
        latchALUWBZDrf2[LATCH_TAG] = cPhyAddress;
        latchWBALUVFU[1] = -1;
    }
}
void wb() {
    write_log(_PRI,"in wb stage\n");
    vfuMULWB();
    vfuALUWB();
}
void vfuLS2() {
    write_log(_PRI,"in LS2 stage\n");
    if(latchLS1LS2[2]==-1)
    return;
    else{
        ROB::writeEntry(latchLS1LS2[2],ROB::STATUS,true);
        latchLS1LS2[2]=-1;
        vfuLsStatus2=VFU_BUSY;
    }
}


void vfuALU2() {
    write_log(_PRI,"in ALU2 stage\n");
    if (latchALU12[1] != -1) {
        latchWBALUVFU[0] = latchALU12[0];
        latchWBALUVFU[1] = latchALU12[1];
        //forwarding
        int phyAddress = ROB::readEntry(latchWBALUVFU[1],ROB::PHY_ADDRESS);
        int cphyAddress = ROB::readEntry(latchWBALUVFU[1],ROB::C_PHY_ADDRESS);
        int instruction = ROB::readEntry(latchWBALUVFU[1],ROB::TYPE);
        int zReg = -1;
        int zVal = -1;
        latchALUIssue[LATCH_TAG] = phyAddress;
        latchALUIssue[LATCH_VALUE] = latchWBALUVFU[0];
        latchALUDrf2[LATCH_TAG] = phyAddress;
        latchALUDrf2[LATCH_VALUE] = latchWBALUVFU[0];
        
        if (instruction != MOVC) {
            zReg = cphyAddress;
            zVal = !latchALU12[0];
            write_log(_PRI,"Z Value %d \n\n", zVal);
        }
        if(latchALU12[0] == 0) {
            //zVal=true;
            latchWBALUVFU[2] = zVal;
            latchALUZIssue[LATCH_VALUE] = zVal;
            latchALUZIssue[LATCH_TAG] = zReg;
            latchALUZDrf2[LATCH_VALUE] = zVal;
            latchALUZDrf2[LATCH_TAG] = zReg;
        } else {
            //zVal=false;
            latchWBALUVFU[2] = zVal;
            latchALUZIssue[LATCH_VALUE] = zVal;
            latchALUZIssue[LATCH_TAG] = zReg;
            latchALUZDrf2[LATCH_VALUE] = zVal;
            latchALUZDrf2[LATCH_TAG] = zReg;
        }
        latchALU12[1] = -1;
    }
    vfuAlu2Status = VFU_FREE;
}
void vfu2() {
    write_log(_PRI,"in vfu2 stage\n");
    vfuALU2();
    vfuLS2();
}

void vfuBRANCH() {
    int ZFLAG=0;
    write_log(_PRI,"in MUL stage\n");

    int branch = -1;
    if (latchBranchIssueVFU[3] == -1) {
        return;
    }
    int instruction = ROB::readEntry(latchBranchIssueVFU[3],ROB::TYPE);
    vfuBranchStatus = VFU_BUSY;
    
    int  pc= ROB::readEntry(latchBranchIssueVFU[3],ROB::PC_VALUE);
    
    ZFLAG=latchBranchIssueVFU[1];

    switch (instruction) {
    case BZ:
        if(ZFLAG == 1) {
            branch = latchBranchIssueVFU[0] + pc;
        }
        break;            
    case BNZ:
        if(ZFLAG != 1) {
            branch = latchBranchIssueVFU[0] + pc;
        }
        break;
    case JUMP:
        branch = latchBranchIssueVFU[0] + latchBranchIssueVFU[1];
        break;
    case BAL:
        branch = latchBranchIssueVFU[0] + latchBranchIssueVFU[1];
        break;
    }
    ROB::writeEntry(latchBranchIssueVFU[3],ROB::STATUS,true);
    ROB::writeEntry(latchBranchIssueVFU[3],ROB::RESULT,branch);        
    
    if (branch < 0) {
        vfuBranchStatus = VFU_FREE;
    }
    latchBranchIssueVFU[3] = -1;
}       
void vfuMUL() {
    write_log(_PRI,"in MUL stage\n");
    int instruction;
    if (latchMulIssueVFU[3] == -1 && vfuMULState == 0) {
        return;
    }
    
    if (vfuMULState == 0 ) {   
        instruction = ROB::readEntry(latchMulIssueVFU[3],ROB::TYPE);
        switch (instruction) {
        case MUL:
            latchMULMUL[0] = latchMulIssueVFU[1] * latchMulIssueVFU[2];
            latchMULMUL[1] = latchMulIssueVFU[3];
            latchMulIssueVFU[3] = -1;
            break;            
        default:
            latchMULMUL[1] = -1;
        };            
        vfuMULState++;
        vfuMulStatus = VFU_BUSY;
        latchWBMULVFU[2] = -1;
    } else if (vfuMULState == 3) {
        int cphyAddress = ROB::readEntry(latchMULMUL[1],ROB::C_PHY_ADDRESS);
        int phyAddress = ROB::readEntry(latchMULMUL[1],ROB::PHY_ADDRESS);
    
        latchWBMULVFU[0] = latchMULMUL[0];
        latchWBMULVFU[1] = latchMULMUL[1];
        //forwarding
        latchMULIssue[LATCH_VALUE] = latchMULMUL[0];
        latchMULIssue[LATCH_TAG] = phyAddress;
        latchMULDrf2[LATCH_VALUE] = latchMULMUL[0];
        latchMULDrf2[LATCH_TAG] = phyAddress;
        if(latchMULMUL[0] == 0) {
            latchWBMULVFU[2] = 1;            
            latchMULZIssue[LATCH_VALUE] = 1;
            latchMULZIssue[LATCH_TAG] = cphyAddress;
            latchMULZDrf2[LATCH_VALUE] = 1;
            latchMULZDrf2[LATCH_TAG] = cphyAddress;
        } else {
            latchWBMULVFU[2] = 0;           
            latchMULZIssue[LATCH_VALUE] = 0;
            latchMULZIssue[LATCH_TAG] = cphyAddress;
            latchMULZDrf2[LATCH_VALUE] = 0;
            latchMULZDrf2[LATCH_TAG] = cphyAddress;
        }
        vfuMULState = 0;
        vfuMulStatus = VFU_FREE;
    } else {
        vfuMULState++;
    }
}
void vfuALU1() {
    write_log(_PRI,"in ALU1 stage\n");
    int instruction;
    if (latchAluIssueVFU[3] == -1) {
        return;
    }
    instruction = ROB::readEntry(latchAluIssueVFU[3],ROB::TYPE);
    switch (instruction) {
    case ADD:
        latchALU12[0] = latchAluIssueVFU[1] + latchAluIssueVFU[2];
        latchALU12[1] = latchAluIssueVFU[3];
        break;
    case SUB:
        latchALU12[0] = latchAluIssueVFU[1] - latchAluIssueVFU[2];
        latchALU12[1] = latchAluIssueVFU[3];
        break;
    case MOVC:
        latchALU12[0] = latchAluIssueVFU[0];
        latchALU12[1] = latchAluIssueVFU[3];
        break;
    case AND:
        latchALU12[0] = latchAluIssueVFU[1] & latchAluIssueVFU[2];
        latchALU12[1] = latchAluIssueVFU[3];
        break;
    case OR:
        latchALU12[0] = latchAluIssueVFU[1] | latchAluIssueVFU[2];
        latchALU12[1] = latchAluIssueVFU[3];
        break;
    case EXOR:
        latchALU12[0] = latchAluIssueVFU[1] ^ latchAluIssueVFU[2];
        latchALU12[1] = latchAluIssueVFU[3];
        break;
    default:
        latchALU12[1] = -1;
    };            
    latchAluIssueVFU[3] = -1;
}

//working here
void vfuLS1() {
    write_log(_PRI,"in LS stage\n");
    if(latchLsIssueVFU[3]==-1)
    return;
    if(vfuLsStatus2==VFU_FREE) {
        int instruction = ROB::readEntry(latchLsIssueVFU[3],ROB::TYPE);
        if(instruction==STORE) {
            latchLS1LS2[0] = latchLsIssueVFU[1];
            latchLS1LS2[1] = latchLsIssueVFU[0] + latchLsIssueVFU[2]; 
            
        } else {
            latchLS1LS2[0]=-1;
            latchLS1LS2[1]=	 latchLsIssueVFU[0] + latchLsIssueVFU[1];
        }
        latchLS1LS2[2] = latchLsIssueVFU[3];
        latchLsIssueVFU[3]==-1;

        vfuLsStatus = VFU_FREE;
    }else{
        vfuLsStatus=VFU_BUSY;
    }
}
void vfu1() {
    write_log(_PRI,"in VFU1 stage\n");    
    vfuLS1();
    vfuBRANCH();        
    vfuMUL();
    vfuALU1();
}
void wakeupAndSelection() {
    write_log(_PRI,"in wakeupAndSelection stage\n");
    int selectedInstruction;
    int temp=0;
    if (vfuAluStatus == VFU_FREE) {
        selectedInstruction = IQ::select(ALU);
        if(selectedInstruction!=-1) {
            latchAluIssueVFU[0] = IQ::readEntry(selectedInstruction,IQ::LIT);
            latchAluIssueVFU[1] = IQ::readEntry(selectedInstruction,IQ::VALUE_S1);
            latchAluIssueVFU[2] = IQ::readEntry(selectedInstruction,IQ::VALUE_S2);
            latchAluIssueVFU[3] = IQ::readEntry(selectedInstruction,IQ::DEST);
            IQ::deleteEntry(selectedInstruction);
	    temp++;
        } else {
            latchAluIssueVFU[3] = -1;
        }
    }
    if (vfuMulStatus == VFU_FREE) {
        selectedInstruction = IQ::select(MULT);        
        if(selectedInstruction!=-1) {
            latchMulIssueVFU[0] = IQ::readEntry(selectedInstruction,IQ::LIT);
            latchMulIssueVFU[1] = IQ::readEntry(selectedInstruction,IQ::VALUE_S1);
            latchMulIssueVFU[2] = IQ::readEntry(selectedInstruction,IQ::VALUE_S2);
            latchMulIssueVFU[3] = IQ::readEntry(selectedInstruction,IQ::DEST);
            IQ::deleteEntry(selectedInstruction);
	    temp++;
        } else {
            latchMulIssueVFU[3] = -1;
        }
    }
    if (vfuBranchStatus == VFU_FREE) {
        selectedInstruction = IQ::select(BRANCH);        
        if(selectedInstruction!=-1) {
            latchBranchIssueVFU[0] = IQ::readEntry(selectedInstruction,IQ::LIT);
            latchBranchIssueVFU[1] = IQ::readEntry(selectedInstruction,IQ::VALUE_S1);
            latchBranchIssueVFU[2] = IQ::readEntry(selectedInstruction,IQ::VALUE_S2);
            latchBranchIssueVFU[3] = IQ::readEntry(selectedInstruction,IQ::DEST);
            IQ::deleteEntry(selectedInstruction);
            stallBranchDispatch = 0;
            stalled = 0;
	    temp++;
        } else {
            latchBranchIssueVFU[3] = -1;
        }
    }
    if (vfuLsStatus == VFU_FREE) {
        selectedInstruction = IQ::select(LS);        
        if(selectedInstruction!=-1) {
            latchLsIssueVFU[0] = IQ::readEntry(selectedInstruction,IQ::LIT);
            latchLsIssueVFU[1] = IQ::readEntry(selectedInstruction,IQ::VALUE_S1);
            latchLsIssueVFU[2] = IQ::readEntry(selectedInstruction,IQ::VALUE_S2);
            latchLsIssueVFU[3] = IQ::readEntry(selectedInstruction,IQ::DEST);
            IQ::deleteEntry(selectedInstruction);
	    temp++;
        } else {
            latchLsIssueVFU[3] = -1;
        }
    }
	if(temp==0)
	 issueStalled++;
}
void forwardAndMatch() {
    write_log(_PRI,"in forwardAndMatch stage\n");        
    IQ::tagMatchAssign(latchALUIssue[LATCH_TAG],latchALUIssue[LATCH_VALUE]);
    IQ::tagMatchAssign(latchMULIssue[LATCH_TAG],latchMULIssue[LATCH_VALUE]);
    IQ::tagMatchAssign(latchLSIssue[LATCH_TAG],latchLSIssue[LATCH_VALUE]);
    IQ::tagMatchAssign(latchALUWBIssue[LATCH_TAG],latchALUWBIssue[LATCH_VALUE]);
    IQ::tagMatchAssign(latchMULWBIssue[LATCH_TAG],latchMULWBIssue[LATCH_VALUE]);
    IQ::tagMatchAssign(latchALUZIssue[LATCH_TAG],latchALUZIssue[LATCH_VALUE]);
    IQ::tagMatchAssign(latchMULZIssue[LATCH_TAG],latchMULZIssue[LATCH_VALUE]);
    IQ::tagMatchAssign(latchALUWBZIssue[LATCH_TAG],latchALUWBZIssue[LATCH_VALUE]);
    IQ::tagMatchAssign(latchMULWBZIssue[LATCH_TAG],latchMULWBZIssue[LATCH_VALUE]);
}
void issue() {  
    write_log(_PRI,"in issue stage\n");
    forwardAndMatch();
    wakeupAndSelection();
}

void drf2() {
    write_log(_PRI,"in drf2 stage\n");
    if (decoder[DRF2][0] == -1 || decoder[DRF2][0] == 0) {
	dispatchStalled++;
        return;
    }
    if ((decoder[DRF2][0] == BZ || 
        decoder[DRF2][0] == BNZ || 
        decoder[DRF2][0] == BAL || 
        decoder[DRF2][0] == JUMP) && 
        stallBranchDispatch == 1) {
        stalled = 1;
        dispatchStalled++;
        return;
    }
    
    if (vfuBranchStatus == VFU_BUSY) {
        stalled = 1;
	dispatchStalled++;
        return;
    }
    int i,d,pd,ROB_ID,s[2],src_count;
    s[0] = s[1] = -1;
    d = -1;
    pd = -1;
    ROB_ID = -1;
    src_count = 0;
    int instructionArrType[3];
    instructionArrType[0] = instructionSet[iCache.at((pcValue[DRF2] - _STARTPCVALUE)/4).at(0)][0];
    instructionArrType[1] = instructionSet[iCache.at((pcValue[DRF2] - _STARTPCVALUE)/4).at(0)][1];
    instructionArrType[2] = instructionSet[iCache.at((pcValue[DRF2] - _STARTPCVALUE)/4).at(0)][2];
    
    //IQ entry being setup
    bool status =true; 
    int typeFU;
    int lit;
    bool statusS1 = true;
    int tagS1;
    int valueTagS1;
    bool statusS2 = true;
    int tagS2;
    int valueTagS2;
    int dest = -1;
    int breakIt = 0;
    if (decoder[DRF2][0] == HALT) {
        dest = ROB::addEntry(0,
            true,
            pcValue[DRF2],
            -1,
            -1,
            decoder[DRF2][0],
            -1);            
        i = 0;
        decoder[DRF2][i]=NOP; i++;
        decoder[DRF2][i]=0; i++;
        decoder[DRF2][i]=0; i++;
        decoder[DRF2][i]=0;
        return;
    }
    for (int i = 0; i < 3; i++) {
        switch (instructionArrType[i]) {
        case D:
            d=decoder[DRF2][i+1];
            break;
        case S:            
            s[src_count]=decoder[DRF2][i+1];
            src_count++;
            break;
        case L:
            lit=decoder[DRF2][i+1]; //literal
            breakIt = 1;
            break;
        case Z:
            breakIt = 1;
            break;
        };        
        if (breakIt) break;
    }
    switch (src_count) {
    case 2:
        statusS1 = URF::isValid(s[0]);
        tagS1 = s[0];
        if (statusS1){
            valueTagS1 = URF::get(tagS1);
            write_log(_PRI,"found from URF:%d",valueTagS1);
        } else {
            if (tagS1 == latchALUDrf2[0]) {
                valueTagS1 = latchALUDrf2[1];
                statusS1 =true;
            }
            if (tagS1 == latchMULDrf2[0]) {
                valueTagS1 = latchMULDrf2[1];
                statusS1 =true;
            }
            if (tagS1 == latchALUWBDrf2[0]){
                valueTagS1 = latchALUWBDrf2[1];
                statusS1 =true;
            }
            if (tagS1 == latchMULWBDrf2[0]){
                valueTagS1 = latchMULWBDrf2[1];
                statusS1 =true;
            }
            if (tagS1 == latchLSDrf[0]) {
                valueTagS1 = latchLSDrf[1];
                statusS1 =true;
            }
            if (tagS1 == latchMULZDrf2[0]){
                valueTagS1 = latchMULWBDrf2[1];
                statusS1 =true;
            }
            if (tagS1 == latchALUZDrf2[0]){
                valueTagS1 = latchALUWBDrf2[1];
                statusS1 =true;
            }
            if (tagS1 == latchMULWBZDrf2[0]){
                valueTagS1 = latchMULWBDrf2[1];
                statusS1 =true;
            }
            if (tagS1 == latchALUWBZDrf2[0]){
                valueTagS1 = latchALUWBDrf2[1];
                statusS1 =true;
            }
        }
        statusS2 = URF::isValid(s[1]);
        tagS2 = s[1];
        if (statusS2){
            valueTagS2 = URF::get(tagS2);
            write_log(_PRI,"found from URF:%d",valueTagS2);
        } else {
            if (tagS2 == latchALUDrf2[0]){
                valueTagS2 = latchALUDrf2[1];
                statusS2 =true;
            }
            if (tagS2 == latchMULDrf2[0]){
                valueTagS2 = latchMULDrf2[1];
                statusS2 =true;
            }
            if (tagS2 == latchALUWBDrf2[0]){
                valueTagS2 = latchALUWBDrf2[1];
                statusS2 =true;
            }
            if (tagS2 == latchMULWBDrf2[0]){
                valueTagS2 = latchMULWBDrf2[1];
                statusS2 =true;
            }
            if (tagS2 == latchLSDrf[0]) {
                valueTagS2 = latchLSDrf[1];
                statusS2 =true;
            }
            if (tagS2 == latchMULZDrf2[0]){
                valueTagS2 = latchMULWBDrf2[1];
                statusS2 =true;
            }
            if (tagS2 == latchALUZDrf2[0]){
                valueTagS2 = latchALUWBDrf2[1];
                statusS2 =true;
            }
            if (tagS2 == latchMULWBZDrf2[0]){
                valueTagS2 = latchMULWBDrf2[1];
                statusS2 =true;
            }
            if (tagS2 == latchALUWBZDrf2[0]){
                valueTagS2 = latchALUWBDrf2[1];
                statusS2 =true;
            }
        }
        break;
    case 1:                        
        statusS1 = URF::isValid(s[0]);
        tagS1 = s[0];
        if (statusS1) {
            valueTagS1 = URF::get(tagS1);
            write_log(_PRI,"found from URF:%d",valueTagS1);
        } else {
            if (tagS1 == latchALUDrf2[0]) {
                valueTagS1 = latchALUDrf2[1];
                statusS1 =true;
            }
            if (tagS1 == latchMULDrf2[0]) {
                valueTagS1 = latchMULDrf2[1];
                statusS1 =true;
            }
            if (tagS1 == latchALUWBDrf2[0]){
                valueTagS1 = latchALUWBDrf2[1];
                statusS1 =true;
            }
            if (tagS1 == latchMULWBDrf2[0]){
                valueTagS1 = latchMULWBDrf2[1];
                statusS1 =true;
            }
            if (tagS1 == latchLSDrf[0]) {
                valueTagS1 = latchLSDrf[1];
                statusS1 =true;
            }
            if (tagS1 == latchMULZDrf2[0]){
                valueTagS1 = latchMULWBDrf2[1];
                statusS1 =true;
            }
            if (tagS1 == latchALUZDrf2[0]){
                valueTagS1 = latchALUWBDrf2[1];
                statusS1 =true;
            }
            if (tagS1 == latchMULWBZDrf2[0]){
                valueTagS1 = latchMULWBDrf2[1];
                statusS1 =true;
            }
            if (tagS1 == latchALUWBZDrf2[0]){
                valueTagS1 = latchALUWBDrf2[1];
                statusS1 =true;
            }
        }
        break;
    case 0:
        break;
    }
    typeFU = fuOperMap(decoder[DRF2][0]);
    write_log(_PRI,"dest:%d",d);
    if (d!=-1) {
        pd = URF::getFirstFree();
        URF::set(pd,0,0);
        RAT::set(FRAT,d,pd);
    }    
    
    dest = ROB::addEntry(0,
    false,
    pcValue[DRF2],
    d,
    pd,
    decoder[DRF2][0],
    -1);
    IQ::addEntry(status, 
    typeFU,
    lit,
    statusS1,
    tagS1,
    valueTagS1,
    statusS2,
    tagS2,
    valueTagS2,
    dest);
    //handling Z flag
    if (decoder[DRF2][0] == ADD || 
            decoder[DRF2][0] == SUB || 
            decoder[DRF2][0] == AND || 
            decoder[DRF2][0] == OR ||
            decoder[DRF2][0] == EXOR ||
            decoder[DRF2][0] == MUL) {
        int pd1 = URF::getFirstFree();
        URF::set(pd1,-1,0);
        RAT::set(FRAT,17,pd1);
        ROB::writeEntry(dest,ROB::C_PHY_ADDRESS,pd1);
    } else {
        ROB::writeEntry(dest,ROB::C_PHY_ADDRESS,-1);
    }
    if (decoder[DRF2][0] == BZ || 
        decoder[DRF2][0] == BNZ || 
        decoder[DRF2][0] == BAL || 
        decoder[DRF2][0] == JUMP == 1) {
        stallBranchDispatch = 1;
    }
    i = 0;
    decoder[DRF2][i]=NOP; i++;
    decoder[DRF2][i]=0; i++;
    decoder[DRF2][i]=0; i++;
    decoder[DRF2][i]=0;
}

void drf1() {
    
    write_log(_PRI,"in drf1 stage\n");
    if(stalled)
    return;        
    int i,d,pd,ROB_ID,s[2],src_count;
    s[0] = s[1] = -1;   
    d = -1;
    pd = -1;
    ROB_ID = -1;
    src_count = 0;
    int instructionArrType[3];
    if (pcValue[DRF1] - _STARTPCVALUE < 0) {
        write_log(_PRI,"returning from here| some issue PC value:%d\n",pcValue[DRF1]);
        return;
    }
    
    if (decoder[DRF1][0]==NOP) {
        return;
    }

    if (decoder[DRF1][0]==HALT) {
        i = 0;
        decoder[DRF2][i]=HALT; i++;
        decoder[DRF2][i]=0; i++;
        decoder[DRF2][i]=0; i++;
        decoder[DRF2][i]=0;
        return;
    }
    if (decoder[DRF1][0]==JUMP && RAT::registerAliasTable[16] == -1) {
        write_log(_PRI,"JUMP before BAL in pipeline:HALT if it reaches commit\n");
        i = 0;
        decoder[DRF2][i]=HALT; i++;
        decoder[DRF2][i]=0; i++;
        decoder[DRF2][i]=0; i++;
        decoder[DRF2][i]=0;
        return;
    }
    
    if (ROB::isFull() || IQ::isFull() || URF::isFull()) {
        return;
    }

    write_log(_PRI,"PCVALUE %d\n",pcValue[DRF1]-_STARTPCVALUE);
    instructionArrType[0] = instructionSet[iCache.at((pcValue[DRF1] - _STARTPCVALUE)/4).at(0)][0];
    instructionArrType[1] = instructionSet[iCache.at((pcValue[DRF1] - _STARTPCVALUE)/4).at(0)][1];
    instructionArrType[2] = instructionSet[iCache.at((pcValue[DRF1] - _STARTPCVALUE)/4).at(0)][2];
    
    decoder[DRF2][0] = -1;
    int breakIt = 0;
    for (int i = 0; i < 3; i++) {
        switch (instructionArrType[i]) {
        case D:
            d=decoder[DRF1][i+1];
            decoder[DRF2][i+1] = d;
            break;
        case S:            
            decoder[DRF2][i+1]=RAT::get(FRAT,decoder[DRF1][i+1]);
            break;
        case L:
            decoder[DRF2][i+1]=decoder[DRF1][i+1]; //literal
            breakIt = 1;
            break;
        case Z:
            breakIt = 1;
            break;
        };        
        if (breakIt) break;
    }
    
    decoder[DRF2][0]=decoder[DRF1][0];
    pcValue[DRF2]=pcValue[DRF1];
    i = 0;
    decoder[DRF1][i]=NOP; i++;
    decoder[DRF1][i]=0; i++;
    decoder[DRF1][i]=0; i++;
    decoder[DRF1][i]=0;
}

/*****************************************************
This function is responsible for reading the instruction
from iCache and forwarding it to next stage of Decode.
******************************************************/
void fetch(){
    write_log(_PRI,"in fetch stage:%d %d\n ",total_count,(pcValue[Fetch]-_STARTPCVALUE)/4);
    int instruction=0,register1=0,register2=0,register3=0,i=0;
    if (stalled) return;
    if(((pcValue[Fetch]-_STARTPCVALUE)/4)>=total_count){
        decoder[DRF1][i]=NOP; i++;
        decoder[DRF1][i]=0; i++;
        decoder[DRF1][i]=0; i++;
        decoder[DRF1][i]=0;
        pcValue[DRF1]=pcValue[Fetch];
    } else if (((pcValue[Fetch]-_STARTPCVALUE)/4) < 0) {
        return;
    } else{
        instruction=iCache.at((pcValue[Fetch]-_STARTPCVALUE)/4).at(i);
        decoder[DRF1][i]=instruction; i++;
        register1=iCache.at((pcValue[Fetch]-_STARTPCVALUE)/4).at(i);
        decoder[DRF1][i]=register1; i++;
        register2=iCache.at((pcValue[Fetch]-_STARTPCVALUE)/4).at(i);
        decoder[DRF1][i]=register2; i++;
        register3=iCache.at((pcValue[Fetch]-_STARTPCVALUE)/4).at(i);
        decoder[DRF1][i]=register3;
        pcValue[DRF1]=pcValue[Fetch];
    }



}

void resetForwarding(){

    latchALUWBIssue[LATCH_TAG] = -1;
    latchMULWBIssue[LATCH_TAG] = -1;
    latchALUWBDrf2[LATCH_TAG] = -1;
    latchMULWBDrf2[LATCH_TAG] = -1;
    latchALUIssue[LATCH_TAG] = -1;
    latchMULIssue[LATCH_TAG] = -1;
    latchALUDrf2[LATCH_TAG] = -1;
    latchMULDrf2[LATCH_TAG] = -1;
    latchALUWBZIssue[LATCH_TAG] = -1;
    latchMULWBZIssue[LATCH_TAG] = -1;
    latchALUWBZDrf2[LATCH_TAG] = -1;
    latchMULWBZDrf2[LATCH_TAG] = -1;
    latchALUZIssue[LATCH_TAG] = -1;
    latchMULZIssue[LATCH_TAG] = -1;
    latchALUZDrf2[LATCH_TAG] = -1;
    latchMULZDrf2[LATCH_TAG] = -1;
    latchLSIssue[LATCH_TAG] = -1;
    latchLSDrf[LATCH_TAG] =-1;
}

/*****************************************************
Function responsible for updating the Program counter value.
implements branch MUX + PCUPDATE LOGIC.
******************************************************/
void pcUpdate() {
    write_log(_PRI,"in pcUpdate stage\n");
    if (stalled) return;
    if (branch == 0) {
        pcValue[Fetch]+=4;
    } else {
        pcValue[Fetch] = branch;
        branch = 0;
        decoder[DRF1][0] = NOP;
        decoder[DRF2][0] = NOP;
    }
}
/*****************************************************
This function is responsible for flusing the Decode and
fetch stage if Branching instruction is encountered.
******************************************************/
void flush(int branchAddr) {
    write_log(_PRI,"in flush stage\n");
    branch = branchAddr;  // need to check this statement.. it is exceeding the value
    int i = 0;
    decoder[DRF2][i] =-1; decoder[DRF1][i] =-1; decoder[Fetch][i] = -1;i++;
    decoder[DRF2][i] = decoder[DRF1][i] = decoder[Fetch][i] = -1;i++;
    decoder[DRF2][i] = decoder[DRF1][i] = decoder[Fetch][i] = -1;i++;
    decoder[DRF2][i] = decoder[DRF1][i] = decoder[Fetch][i] = -1;
    pcValue[DRF2] = pcValue[DRF1] = pcValue[Fetch] = _STARTPCVALUE;

    latchWBMULVFU[1] = -1;
    latchWBALUVFU[1] = -1;
    latchALU12[1] = -1;
    latchMULMUL[1] = -1;
    latchAluIssueVFU[3] = -1;
    latchBranchIssueVFU[3] =-1;
    latchMulIssueVFU[3] = -1;
    latchLsIssueVFU[3] = -1;
    latchLS1LS2[2] = -1;

}

/*****************************************************
Function to read instruction from iCache.
******************************************************/
std::vector<int> instructionReadReg (int i) {
    write_log(_PRI,"in instructionReadReg\n");
    std::vector<int> tmpR;
    int instructionArrType[3];
    instructionArrType[0] = instructionSet[iCache.at(i).at(0)][0];
    instructionArrType[1] = instructionSet[iCache.at(i).at(0)][1];
    instructionArrType[2] = instructionSet[iCache.at(i).at(0)][2];

    switch (instructionArrType[0]) {
    case S:
        tmpR.push_back(iCache.at(i).at(1));
        tmpR.push_back(iCache.at(i).at(1));
    };
    switch (instructionArrType[1]) {
    case S:
        tmpR.push_back(iCache.at(i).at(2));
        tmpR.push_back(iCache.at(i).at(2));
    };
    switch (instructionArrType[2]) {
    case S:
        tmpR.push_back(iCache.at(i).at(3));
        tmpR.push_back(iCache.at(i).at(3));
    };
    return tmpR;
}

/*****************************************************
Function to update the current instruction in each stages
of the pipeline.
******************************************************/
int instructionWriteReg (int i) {
    write_log(_PRI,"in instructionWriteReg\n");
    int ret;
    int instructionArrType[3];
    instructionArrType[0] = instructionSet[iCache.at(i).at(0)][0];
    switch (instructionArrType[0]) {
    case D:
        ret = iCache.at(i).at(1);
        break;
    default:
        ret = emptDestReg;
    };
    return ret;
}
/*****************************************************
Helper Function to provide the info about instruction type
******************************************************/
int instructionType(std::string &s){
    if(s=="ADD")
    return ADD;
    if(s=="SUB")
    return SUB;
    if(s=="MOVC")
    return MOVC;
    if(s=="MUL")
    return MUL;
    if(s=="AND")
    return AND;
    if(s=="OR")
    return OR;
    if(s=="EX-OR")
    return EXOR;
    if(s=="LOAD")
    return LOAD;
    if(s=="STORE")
    return STORE;
    if(s=="BZ")
    return BZ;
    if(s=="BNZ")
    return BNZ;
    if(s=="JUMP")
    return JUMP;
    if(s=="BAL")
    return BAL;
    if(s=="HALT")
    return HALT;
}

/*****************************************************
Helper function to find out register number and literal value.
******************************************************/
int registerValue(std::string s, int l){
    int value=0;
    if(s[0]=='R')
    {
        if(s.length()==3 && l==1)
        { return (s[1]-48);}

        if(s.length()==4 && l==1)
        { value = value*10 + s[1]-48;
            value = value*10 + s[2]-48;
            return value;
        }
        if(s.length()==2 && l==0)
        return s[1]-48;
        if(s.length()==3 && l==0)
        { value = value*10 + s[1]-48;
            value = value*10 + s[2]-48;
            return value;
        }

    }
    else{
        if(s[1]=='-'){
            int l=s.length();
            for(int i=2;i<l;i++)
            {   value=value*10 +  s[i] - 48; }
            return -value;
        }
        else {
            int l=s.length();
            for(int i=1;i<l;i++)
            {   value=value*10 +  s[i] - 48; }
            return value;
        }
    }

}

/*****************************************************
Helper Function to update the program counter value
at the starting stage.
******************************************************/
void pcValueUpdate(){
    pcValue[0]=_STARTPCVALUE;
    for(int i=1;i<4;i++)
    pcValue[i]=0;
}

/*****************************************************
Helper Function to update deduce the isntruction from input file.
******************************************************/
int populateIcache(string inputFile){

    std::ifstream infile(inputFile.c_str());
    std::string line;
    static int count=0;
    
    while(std::getline(infile, line))
    {
        int flag=0,j=0,k=0,l=0,flag_jump=0;
        std::istringstream iss(line);
        string a,b,c,d;
        iss>>a;


        if(a=="JUMP"|| a=="MOVC" || a=="BAL" || a=="BZ" || a=="BNZ"){
            iss>>b; iss>>c; d=""; flag=1;
            if(a=="JUMP")
            flag_jump=1;
        }
        else{
            iss>>b; iss>>c; iss>>d;
        }
        int x=instructionType(a);

        if(flag==1 && flag_jump==0){
            j=registerValue(b,1);
            k=registerValue(c,0);
            l=0;
        }
        else if (flag==0){
            j=registerValue(b,1);
            k=registerValue(c,1);
            l=registerValue(d,0);
        }

        if(flag==1 && flag_jump==1)
        { j=16;
            k=registerValue(c,0);
            l=0; }
        //std::cout<<j<<" "<<k<<" "<<l<<endl;
        if(a=="BAL")
        {
            temp.push_back(x);
            temp.push_back(16);
            temp.push_back(j);
            temp.push_back(k);
        } else if (a =="BZ" || a == "BNZ") {
            temp.push_back(x);
            temp.push_back(17);
            temp.push_back(j);
            temp.push_back(k);
        }
        else{
            temp.push_back(x);
            temp.push_back(j);
            temp.push_back(k);
            temp.push_back(l);
        }
        iCache.push_back(temp);
        temp.clear();
        flag=0;
        count++;
    }
    infile.close();
    total_count = count;
    for(int r=0;r<count;r++){
        write_log(_PRI,"%s\t",instructionNames[iCache.at(r).at(0)]);
        for(int t=1;t<4;t++)
        { int k=iCache.at(r).at(t); write_log(_PRI,"%d\t",k);}
        write_log(_PRI,"\n");
    }
    ini_flag=1;
    return 0;
}
/*****************************************************
Helper function for reseting the state of pipeline and registers.
******************************************************/
void resetState () {
    pcValueUpdate();
    for(int i=0;i<6;i++)
    instructionRegister[i]=-1;
    
    for(int i=0;i<3;i++)
    decoder[i][0]=-1;
    //todo
    URF::init();
}
int fuOperMap(int oper){
    switch (oper) {
    case ADD: return ALU;
    case SUB: return ALU;
    case MOVC: return ALU;
    case MUL: return MULT;
    case AND: return ALU;
    case OR: return ALU;
    case EXOR: return ALU;
    case LOAD: return LS;
    case STORE: return LS;
    case BZ: return BRANCH;
    case BNZ: return BRANCH;
    case JUMP: return BRANCH;
    case BAL: return BRANCH;
    default: write_log(PRIO_ALL,"unexpected Oper: %d\n",oper);abort();        
    };
}

void displayForwardingDataPaths() {
    write_log(PRIO_ALL,"Forwarding Data Paths\n");
    write_log(PRIO_ALL,"ALU from WB to Issue: %d of reg %d\n",latchALUWBIssue[1],latchALUWBIssue[0]);
    write_log(PRIO_ALL,"MUL from WB to Issue: %d of reg %d\n",latchMULWBIssue[1],latchMULWBIssue[0]);
    write_log(PRIO_ALL,"ALU from WB to DRF2: %d of reg %d\n",latchALUWBDrf2[1],latchALUWBDrf2[0]);
    write_log(PRIO_ALL,"MUL from WB to DRF2: %d of reg %d\n",latchMULWBDrf2[1],latchMULWBDrf2[0]);
    write_log(PRIO_ALL,"ALU from VFU to Issue: %d of reg %d\n",latchALUIssue[1],latchALUIssue[0]);
    write_log(PRIO_ALL,"MUL from VFU to Issue: %d of reg %d\n",latchMULIssue[1],latchMULIssue[0]);
    write_log(PRIO_ALL,"ALU from VFU to DRF2: %d of reg %d\n",latchALUDrf2[1],latchALUDrf2[0]);
    write_log(PRIO_ALL,"MUL from VFU to DRF2: %d of reg %d\n",latchMULDrf2[1],latchMULDrf2[0]);
    write_log(PRIO_ALL,"LS from Commit to ISSUE: %d of reg %d\n",latchLSIssue[1],latchLSIssue[0]);
    write_log(PRIO_ALL,"LS from Commit to DRF: %d of reg %d\n",latchLSDrf[1],latchLSDrf[0]);
    
    write_log(PRIO_ALL,"ALU from ALU2 to Issue over Z: %d of reg %d\n",latchALUZIssue[1],latchALUZIssue[0]);
    write_log(PRIO_ALL,"ALU from MUL to Issue over Z: %d of reg %d\n",latchMULZIssue[1],latchMULZIssue[0]);
    write_log(PRIO_ALL,"ALU from ALU2 to DRF2 over Z: %d of reg %d\n",latchALUZDrf2[1],latchALUZDrf2[0]);
    write_log(PRIO_ALL,"ALU from MUL to DRF2 over Z: %d of reg %d\n",latchMULZDrf2[1],latchMULZDrf2[0]);
    
    //bool FlagEx2ex1 = 0;
}
void displayNormalDataPaths() {
    write_log(PRIO_ALL,"Normal Data Paths\n");
    write_log(PRIO_ALL,"WB from MUL: %d,%d,%d\n",latchWBMULVFU[0],latchWBMULVFU[1],latchWBMULVFU[2]);
    write_log(PRIO_ALL,"WB from ALU: %d,%d,%d\n",latchWBALUVFU[0],latchWBALUVFU[1],latchWBALUVFU[2]);
    write_log(PRIO_ALL,"ALU1 to ALU2: %d,%d\n",latchALU12[0],latchALU12[1]);
    write_log(PRIO_ALL,"MUL to MUL: %d,%d\n",latchMULMUL[0],latchMULMUL[1]);
    write_log(PRIO_ALL,"ALU from Issue: %d,%d,%d,%d\n",latchAluIssueVFU[0],latchAluIssueVFU[1],latchAluIssueVFU[2],latchAluIssueVFU[3]);
    write_log(PRIO_ALL,"Branch from Issue: %d,%d,%d,%d\n",latchBranchIssueVFU[0],latchBranchIssueVFU[1],latchBranchIssueVFU[2],latchBranchIssueVFU[3]);
    write_log(PRIO_ALL,"Mul from Issue: %d,%d,%d,%d\n",latchMulIssueVFU[0],latchMulIssueVFU[1],latchMulIssueVFU[2],latchMulIssueVFU[3]);
    write_log(PRIO_ALL,"Ls from Issue: %d,%d,%d,%d\n",latchLsIssueVFU[0],latchLsIssueVFU[1],latchLsIssueVFU[2],latchLsIssueVFU[3]);
    write_log(PRIO_ALL,"Ls1Ls2 from Issue: %d,%d,%d\n",latchLS1LS2[0],latchLS1LS2[1],latchLS1LS2[2]);
}
void displayVFUStatus() {
    write_log(PRIO_ALL,"VFU Status\n");
    write_log(PRIO_ALL,"MUL:%d\n",vfuMulStatus);
    write_log(PRIO_ALL,"Branch:%d\n",vfuBranchStatus);
    write_log(PRIO_ALL,"Alu:%d\n",vfuAluStatus);
    write_log(PRIO_ALL,"Alu2:%d\n",vfuAlu2Status);
    write_log(PRIO_ALL,"Ls1:%d\n",vfuLsStatus);
    write_log(PRIO_ALL,"Ls2:%d\n",vfuLsStatus2);
    write_log(PRIO_ALL,"MUL State:%d\n",vfuMULState);
    write_log(PRIO_ALL,"Branch Dispatch State:%d\n",stallBranchDispatch);

}
void Print_Stats(){
	write_log(PRIO_ALL, "Dispatch Stall %d\n", dispatchStalled );
	write_log(PRIO_ALL, "Issue Count %d\n", issueStalled);
	write_log(PRIO_ALL, "Store Count %d\n", storeCount);
	write_log(PRIO_ALL, "Load Count %d\n", loadCount);
	write_log(PRIO_ALL, "IPC %d/%d = %f\n",robDeleteCount,simSteps,(float)robDeleteCount/(float)simSteps);
}
#endif
