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
    int src1Lat = (progDB.instStatsVec[inst].depend1 != ENTRY) ? progDB.instStatsVec[progDB.instStatsVec[inst].depend1].depth + progDB.instStatsVec[progDB.instStatsVec[inst].depend1].latency : 0;
    int src2Lat = (progDB.instStatsVec[inst].depend2 != ENTRY) ? progDB.instStatsVec[progDB.instStatsVec[inst].depend2].depth + progDB.instStatsVec[progDB.instStatsVec[inst].depend2].latency : 0;
    int pathLat = (src1Lat>src2Lat) ? src1Lat : src2Lat; 
    return pathLat+progDB.instStatsVec[inst].latency;  
}

ProgCtx analyzeProg(const unsigned int opsLatency[], const InstInfo progTrace[], unsigned int numOfInsts) {
    //allocate and reset variables
    int dependMat[TOT_REG_NUM][numOfInsts];                                     //****************************************************************************
    progDB.instStatsVec = (pInstStats)malloc(sizeof(instStats)*numOfInsts);     //We created a 2d array by the size of registersSize(rows)xinstructions(cols).
    if (progDB.instStatsVec == NULL) return PROG_CTX_NULL;                      //in each col we fill the rows that consist the instruction sources and destination
    int i, j;                                                                   //with different notation so we could distinguish them
    for (i=0; i <TOT_REG_NUM; i++)                                              //we fill the cols(instruction) by the order they appear in the program
        for(j=0; j<numOfInsts; j++)                                             // the analysis is done by the following algorithm: 
            dependMat[i][j] = NA;                                               //->for each instruction(col) we find the rows that are noted as sources.
    progDB.progDepth = 0;                                                       //->for each source we scan its row backwards till we meet the filrst cell that enrirled "destination".  
    progDB.numOfInsts = numOfInsts;                                             //  this means that the instruction in this col is the correct dependency
    int curInst, backScanInst;                                                  //****************************************************************************
    for (curInst =0; curInst<numOfInsts; curInst++){
        //*************************
        //reset with start values
        //************************
        progDB.instStatsVec[curInst].depend1 = ENTRY;
        progDB.instStatsVec[curInst].depend2 = ENTRY;
        progDB.instStatsVec[curInst].latency = opsLatency[progTrace[curInst].opcode];
        progDB.instStatsVec[curInst].depth   = 0;
        //***************************************************
        // fill matrix with parameters deduced from inst info
        // *************************************************
        dependMat[progTrace[curInst].dstIdx][curInst] = DST; //note: we have 3 marks: DST means destination, SRC means source, SRC_DST means that for that instruction same register is source and dest.
        dependMat[progTrace[curInst].src1Idx][curInst] = (dependMat[progTrace[curInst].src1Idx][curInst]==DST) ? SRC_DST : SRC;
        dependMat[progTrace[curInst].src2Idx][curInst] = ((dependMat[progTrace[curInst].src2Idx][curInst]==DST) ||  (dependMat[progTrace[curInst].src2Idx][curInst]==SRC_DST))? SRC_DST : SRC;
        //**********************************
        //calc depth and latency iteratively
        //*********************************  
        if (curInst == FIRST_INST) continue;                               //first instruction depend merely on the entry.
        for (backScanInst = curInst-1; backScanInst >-1; backScanInst--){  //for each instruction we scan backwards the array to find its dependencies. 
            if((progDB.instStatsVec[curInst].depend1 == ENTRY) &&            
              ((dependMat[progTrace[curInst].src1Idx][backScanInst] == SRC_DST) || (dependMat[progTrace[curInst].src1Idx][backScanInst] == DST)))  
                    progDB.instStatsVec[curInst].depend1 = backScanInst;   //if we find an element in the row of the source of the current instruction that noted as destination take it as depend
            if((progDB.instStatsVec[curInst].depend2 == ENTRY) && 
              ((dependMat[progTrace[curInst].src2Idx][backScanInst] == SRC_DST) || (dependMat[progTrace[curInst].src2Idx][backScanInst] == DST))) 
                progDB.instStatsVec[curInst].depend2 = backScanInst;       //if we find an element in the row of the source of the current instruction that noted as destination take it as depend
        }
        progDB.instStatsVec[curInst].depth = checkRootDepth(curInst) - progDB.instStatsVec[curInst].latency; //the root depth includeing the current instruction latency, here we want it without it
        int newDepth = progDB.instStatsVec[curInst].latency + progDB.instStatsVec[curInst].depth;            //calculating the depth of the current instruction  
        progDB.progDepth = (progDB.progDepth > newDepth) ? progDB.progDepth : newDepth;                      //assign as prog depth only if it's longer than the previous one
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



