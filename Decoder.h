#pragma once
#include <iostream>
#include "stdio.h"
#include "stdlib.h"
#include "time.h"

#define MARK	255
#define	SOI		216
#define	EOI		217
#define DQT		219
#define DRI		221
#define APP0	224
#define APP15	239
#define SOF0	192
#define SOF15	207
#define DHT		196
#define SOS		218

#define c1 0.980785280403230
#define c2 0.923879532511287
#define c3 0.831469612302545
#define c4 0.707106781186548
#define s1 0.195090322016128
#define s2 0.382683432365090
#define s3 0.555570233019602
#define s4 0.707106781186547 

#define sqrt2 1.414213562373095



struct InScanComponent{
	int selector;
	int idDC;
	int idAC;
	int *HTTableDC;
	int *HTTableAC;
	int *QTable;
	int sampleRateX;
	int sampleRateY;
  int sampleH;
  int sampleV;
	int Hi;
	int Vi;
};

int *DeZigZag(int);
int *ExpandDHT(int *lenTable, int *valTable, int numCode);
void InterpretMarkers(int marker, unsigned char *buffer, int &cursor);
int *UpSample(int *block, int sampleH, int sampleV);
int *char2Binary(unsigned char a);
int getNextBit(unsigned char *buffer, int &cursor, int &bitPos, int *bitBuffer);
int getValue(unsigned char *buffer, int &cursor, int &bitPos, int *bitBuffer, int codeLength);
void IDCT(double *F);
void IDCT2(int *block);
int *DecodeBlock(unsigned char *buffer, int &cursor, int &bitPos, int *bitBuffer, int &predDC, int *HTTableDC, int *HTTableAC, int *QTable);
void DecodeRestartInterval(unsigned char *buffer, int &cursor, InScanComponent *InScanComp, int numInScanComp);
void DecodeScan_Baseline(unsigned char *buffer, int &cursor);
void DecodeFrame_Baseline(unsigned char *buffer, int &cursor);
unsigned char *DecodeImage(unsigned char *buffer, int &cursor);