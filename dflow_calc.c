/* 046267 Computer Architecture - Spring 2017 - HW #3 */
/* Implementation (skeleton)  for the dataflow statistics calculator */

#include "dflow_calc.h"
#include <stdio.h>

//======================
//Enums and Typedefs
//======================
enum {
    NA      = 0,
    DST     = 1,
    SRC     = 2,
    SRC_DST = 3,
};

typedef struct {
    int depend1;
    int depend2;
    int latency;
    int depth;
}instStats, *pInstStats;

typedef struct {
    pInstStats instStatsVec;
    int progDepth;
    int numOfInsts;
}ctxDB;

//======================
//Globals & Defines
//======================

ctxDB progDB;

#define TOT_REG_NUM 32
#define FIRST_INST 0
#define ENTRY -1


int checkRootDepth(int inst){
    int src1Lat = (progDB.instStatsVec[inst].depend1 != ENTRY) ? checkRootDepth(progDB.instStatsVec[inst].depend1) : 0;
    int src2Lat = (progDB.instStatsVec[inst].depend2 != ENTRY) ? checkRootDepth(progDB.instStatsVec[inst].depend2) : 0;
    int pathLat = (src1Lat>src2Lat) ? src1Lat : src2Lat; 
    return pathLat+progDB.instStatsVec[inst].latency;  
}

ProgCtx analyzeProg(const unsigned int opsLatency[], const InstInfo progTrace[], unsigned int numOfInsts) {
    //allocate and reset variables
    int dependMat[TOT_REG_NUM][numOfInsts];
    progDB.instStatsVec = (pInstStats)malloc(sizeof(instStats)*numOfInsts);
    if (progDB.instStatsVec == NULL) return PROG_CTX_NULL;
    int i, j;
    for (i=0; i <TOT_REG_NUM; i++)
        for(j=0; j<numOfInsts; j++)
            dependMat[i][j] = NA;
    progDB.progDepth = 0;
    progDB.numOfInsts = numOfInsts;
    int curInst, backScanInst;
    for (curInst =0; curInst<numOfInsts; curInst++){
        //reset with start values
        progDB.instStatsVec[curInst].depend1 = ENTRY;
        progDB.instStatsVec[curInst].depend2 = ENTRY;
        progDB.instStatsVec[curInst].latency = opsLatency[progTrace[curInst].opcode];
        progDB.instStatsVec[curInst].depth   = 0;
        // fill matrix with parameters deduced from inst info
        dependMat[progTrace[curInst].dstIdx][curInst] = DST;
        dependMat[progTrace[curInst].src1Idx][curInst] = (dependMat[progTrace[curInst].src1Idx][curInst]==DST) ? SRC_DST : SRC;
        dependMat[progTrace[curInst].src2Idx][curInst] = (dependMat[progTrace[curInst].src2Idx][curInst]==DST) ? SRC_DST : SRC;
        //calc depth and latency recursively
        if (curInst == FIRST_INST) continue;//TODO: check if neccessary
        for (backScanInst = curInst-1; backScanInst >-1; backScanInst--){
            if((progDB.instStatsVec[curInst].depend1 == ENTRY) && 
              ((dependMat[progTrace[curInst].src1Idx][backScanInst] == SRC_DST) || (dependMat[progTrace[curInst].src1Idx][backScanInst] == DST)))  
                    progDB.instStatsVec[curInst].depend1 = backScanInst;
            if((progDB.instStatsVec[curInst].depend2 == ENTRY) && 
              ((dependMat[progTrace[curInst].src2Idx][backScanInst] == SRC_DST) || (dependMat[progTrace[curInst].src2Idx][backScanInst] == DST))) 
                progDB.instStatsVec[curInst].depend2 = backScanInst;
        }
        progDB.instStatsVec[curInst].depth = checkRootDepth(curInst) - progDB.instStatsVec[curInst].latency;
        int newDepth = progDB.instStatsVec[curInst].latency + progDB.instStatsVec[curInst].depth; 
        progDB.progDepth = (progDB.progDepth > newDepth) ? progDB.progDepth : newDepth;
        //printf("Inst: %d depend 1: %d depend 2: %d dst: %d \n", curInst,dependMat[progTrace[curInst].src1Idx][curInst], dependMat[progTrace[curInst].src1Idx][curInst], dependMat[progTrace[curInst].dstIdx][curInst]);
    }
    return &progDB;
}

void freeProgCtx(ProgCtx ctx) {
  free(progDB.instStatsVec);
}

int getInstDepth(ProgCtx ctx, unsigned int theInst) {
    if (ctx == NULL || theInst>=progDB.numOfInsts) return -1; 
    return progDB.instStatsVec[theInst].depth;
}

int getInstDeps(ProgCtx ctx, unsigned int theInst, int *src1DepInst, int *src2DepInst) {
   if (ctx == NULL || src1DepInst == NULL || src2DepInst == NULL || theInst>=progDB.numOfInsts) return -1;   
   *src1DepInst = progDB.instStatsVec[theInst].depend1;
   *src2DepInst = progDB.instStatsVec[theInst].depend2; 
    return 0;
}

int getProgDepth(ProgCtx ctx) {
    return progDB.progDepth;
}



