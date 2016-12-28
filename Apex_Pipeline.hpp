#ifndef APEX_PIPELINE
#define APEX_PIPELINE
#include <stdio.h>
#include <stdlib.h>
#include <cstring>
#include <string>
#include <vector>
#include <algorithm>
#include<iostream>
#include<fstream>
#include<sstream>
#include <stdarg.h>

#define _LOAD_BUB 2
#define _REST_BUB 1
#define _SEARCH_SPACE 3
#define _STARTPCVALUE 4000
#define _MEM_SIZE 1000
#define LOGPRIORITY 1
using namespace std;
static int init_done=0;
static int ini_flag=0;
static int simSteps=0;
enum fuType {
    ALU,
    MULT,
    BRANCH,
    LS
};

enum instructions {
    NOP,
    ADD,
    SUB,
    MOVC,
    MUL,
    AND,
    OR,
    EXOR,
    LOAD,
    STORE,
    BZ,
    BNZ,
    JUMP,
    BAL,
    HALT
};

const char* instructionNames[] = {
    "NOP",
    "ADD",
    "SUB",
    "MOVC",
    "MUL",
    "AND",
    "OR",
    "EXOR",
    "LOAD",
    "STORE",
    "BZ",
    "BNZ",
    "JUMP",
    "BAL",
    "HALT"
};

enum pipelineStages {
    Fetch,
    DRF1,
    DRF2,
    Issue,
    VFU1,
    VFU2,
    VFU3,
    VFU4,
    WB,
    Commit
};

const char* pipelineStageNames[] = {
    "Fetch",
    "DRF1",
    "DRF2",
    "ISSUE",
    "VFU1",
    "VFU2",
    "VFU3",
    "VFU4",
    "WB",
    "Commit"
};

enum forwardingOpertunity {
    STALL,
    FRWRD,
    NODEP
};

enum regStructure {
    REG_VALUE,
    REG_STATUS
};

enum vfuStatus {
    VFU_FREE,
    VFU_BUSY
};

enum ForwardLatchStructure {
    LATCH_TAG,
    LATCH_VALUE
};

enum regEnum {
    Z,
    S,
    D,
    L,
    X
};

enum MAP_TABLE {
    FRAT,
    RRAT
};

//Data structures
int pcValue[3];
int decoder[3][4];
static std::vector< std::vector<int> > iCache;  // Two dimensional vector
std::vector<int> temp;   // single dimensional vector
int instructionSet[15][3] = {
    {Z,Z,Z},
    {D,S,S},
    {D,S,S},
    {D,L,Z},
    {D,S,S},
    {D,S,S},
    {D,S,S},
    {D,S,S},
    {D,S,L},
    {S,S,L},
    {S,L,Z},
    {S,L,Z},
    {S,L,Z},
    {D,S,L},
    {Z,Z,Z}
};
int regX = 16;
static int instructionRegister[3] = {-1,-1,-1};
int emptDestReg = NOP;
int stallBranchDispatch = 0;
int total_count = 0;
//normal data path
int latchWBMULVFU[3] = {0,-1,0};
int latchWBALUVFU[3] = {0,-1,0};
int latchALU12[2] = {0,-1};
int latchMULMUL[2] = {0,-1};
int latchAluIssueVFU[4] = {0,0,0,-1};
int latchBranchIssueVFU[4] = {0,0,0,-1};
int latchMulIssueVFU[4] = {0,0,0,-1};
int latchLsIssueVFU[4] = {0,0,0,-1};
int latchLS1LS2[3] = {0,0,-1};

//forwarding path
static int dispatchStalled=0;
static int issueStalled=0;
static int loadCount=0;
static int storeCount=0;
static int robDeleteCount=0;
int latchALUWBIssue[2] = {-1,0};
int latchMULWBIssue[2] = {-1,0};
int latchALUWBDrf2[2] = {-1,0};
int latchMULWBDrf2[2] = {-1,0};
int latchALUIssue[2] = {-1,0};
int latchMULIssue[2] = {-1,0};
int latchALUDrf2[2] = {-1,0};
int latchMULDrf2[2] = {-1,0};
int latchALUWBZIssue[2] = {-1,0};
int latchMULWBZIssue[2] = {-1,0};
int latchALUWBZDrf2[2] = {-1,0};
int latchMULWBZDrf2[2] = {-1,0};
int latchALUZIssue[2] = {-1,0};
int latchMULZIssue[2] = {-1,0};
int latchALUZDrf2[2] = {-1,0};
int latchMULZDrf2[2] = {-1,0};
int latchLSIssue[2] = {-1,0};
int latchLSDrf[2]={-1,0};

bool FlagEx2ex1 = 0;
//VFU status
bool vfuMulStatus = VFU_FREE;
bool vfuBranchStatus = VFU_FREE;
bool vfuAluStatus = VFU_FREE;
bool vfuAlu2Status = VFU_FREE;
bool vfuLsStatus = VFU_FREE;
bool vfuLsStatus2 = VFU_FREE;
int vfuMULState = 0;
//stage status

int branch = 0;
int stalled = 0;
//Memory
int memoryMap[_MEM_SIZE];

#include "Util.hpp"
#include "ROB.cpp"
#include "IQ.cpp"
#include "urf.hpp"
#include "mem.hpp"

string InstructionCodeFile;
//private functions
void commit();
void vfuLSWB();
void vfuMULWB();
void vfuALUWB();
void wb();
void vfuMUL(); //implement a 4 stage counter
void vfu4();
void vfuMUL(); //implement a 4 stage counter
void vfu3();
void vfuLS2();
void vfuMUL(); //implement a 4 stage counter
void vfuALU2();
void vfu2();
void vfuLS1();
void vfuBRANCH();        
void vfuMUL(); //implement a 4 stage counter
void vfuALU1();
void vfu1();
void wakeupAndSelection();
void forwardAndMatch();
void issue();
void drf1();
void drf2();
void fetch();
void pcUpdate();
void resetState();
int instructionWriteReg(int);
std::vector<int> instructionReadReg(int);
int resolveDependenciesOverFlag ();
int resolveDependenciesInDRF(int, int, int);
void pcValueUpdate();
int populateIcache(string);
int readMem(int);
void writeMem(int, int);
void flush(int);
//public functions
void initialize ();
void simulate ();
//part of display
void displayRegTables();
//to be removed
void displayIQ();
void displayROB();
void displayURF();
void displayMem();
void displayStats();
void displayForwardingDataPaths();
void displayNormalDataPaths();
void displayVFUStatus();
void Print_Stats();
void display ();
void setURF();
void shell();
int execute(char *line);
void write_log(int , const char *, ...);
void initURF();
int fuOperMap(int);
void resetForwarding();
#endif
