#pragma once
#include "Decoder.h"

int Ri = 2147483644;
int imageHeight;
int imageWidth;
int numComponent;
unsigned char *imageDecoded;
unsigned char *imagePadded;
int twoPowers[17] = { 1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096, 8192, 16384, 32768, 65536 };

struct Component{
	int id;
	int sampleRateX;
	int sampleRateY;
	int QTableID;
} component[3];

struct QTable{
	int id;
	int precision;
	int table[64];
} QT[4];

struct HTable{
	int id;
	int hClass;
	int lenTable[16];
	int *valTable;
	int *HTTable;
} HT[8];


void DeZigZag(int *block){
	int DeZZ[64];
	int idx[64] =
	{ 0, 1, 5, 6, 14, 15, 27, 28,
	2, 4, 7, 13, 16, 26, 29, 42,
	3, 8, 12, 17, 25, 30, 41, 43,
	9, 11, 18, 24, 31, 40, 44, 53,
	10, 19, 23, 32, 39, 45, 52, 54,
	20, 22, 33, 38, 46, 51, 55, 60,
	21, 34, 37, 47, 50, 56, 59, 61,
	35, 36, 48, 49, 57, 58, 62, 63,
	};
	for (int i = 0; i < 64; i++)
		DeZZ[i] = block[idx[i]];
  for (int i = 0; i < 64; i++)
    block[i] = DeZZ[i];
}

int *ExpandDHT(int *lenTable, int *valTable, int numCode){

	/* Expand DHT into Huffman Tree */
	int cursorH = 0;
	int cursorV = 0;
	int cursorB = cursorH + 1;
	int *HTTable = (int *)malloc(sizeof(int) * 5 * 500);
	for (int i = 0; i < 5 * 500; i++)
	{
		HTTable[i] = 0;
	}
	int initVal[] =
	{ 0, 1, 2, 0, 0,
	0, 0, 0, 0, 0,
	0, 0, 0, 0, 0
	};
	for (int i = 0; i < 15; i++)
	{
		HTTable[i] = initVal[i];
	}
	cursorH += 3;
	for (int i = 0; i < 16; i++)
	{
		int *preVal = (int *)malloc(sizeof(int) * lenTable[i]);
		for (int j = 0; j < lenTable[i]; j++)
		{
			preVal[j] = valTable[cursorV + j];
		}
		cursorV += lenTable[i];
		for (int j = 0; j < lenTable[i]; j++)
		{
			HTTable[5 * (cursorB + j) + 0] = preVal[j];
			HTTable[5 * (cursorB + j) + 4] = 1;
		}
		if (cursorV >= numCode)
		{
			break;
		}
		for (int j = 0; j < cursorH - cursorB - lenTable[i]; j++)
		{
			HTTable[5 * (cursorB + lenTable[i] + j) + 1] = cursorH + j * 2;
			HTTable[5 * (cursorB + lenTable[i] + j) + 2] = cursorH + j * 2 + 1;
			HTTable[5 * (cursorH + j * 2) + 3] = cursorB + lenTable[i] + j;
			HTTable[5 * (cursorH + j * 2 + 1) + 3] = cursorB + lenTable[i] + j;
		}
		int cursorTemp = cursorB;
		cursorB = cursorH;
		cursorH = cursorH + 2 * (cursorH - cursorTemp - lenTable[i]);
	}
	/* display the huffman tree
	std::cout << "Expand into Huffman Tree..." << std::endl;
	for (int i = 0; i < cursorH; i++)
	{
	std::cout << i << "\t";
	for (int j = 0; j < 5; j++)
	{
	std::cout << HTTable[5 * i + j] << "\t";
	}
	std::cout << std::endl;
	}
	*/
	return HTTable;
}


void InterpretMarkers(int marker, unsigned char *buffer, int &cursor){
	if (marker == DQT)
	{
		int offset = cursor - 1;
		cursor++;
		int length = buffer[cursor] * 256 + buffer[cursor + 1];

		std::cout << "================ MARKER: DQT =============================" << std::endl;
		std::cout << "Offset: " << offset << std::endl;
		std::cout << "Length: " << length << std::endl;
    
		int cursorDQT = cursor + 2;
		while (cursorDQT != offset + length + 2)
		{
			int QTInfo = buffer[cursorDQT];
			int QTID = QTInfo % 16;
			int QTPrecision = QTInfo / 16;
			int idx = QTID;
			QT[idx].id = QTID;
			if (QTPrecision == 0)
				QT[idx].precision = 1;
			else
				QT[idx].precision = 2;
			if (QT[idx].precision == 1)
			{
				for (int i = 0; i < 64; i++)
					QT[idx].table[i] = buffer[cursorDQT + 1 + i];
				cursorDQT += 65;
			}
			else
			{
				for (int i = 0; i < 64; i++)
					QT[idx].table[i] = buffer[cursorDQT + i * 2 + 1] * 256 + buffer[cursorDQT + i * 2 + 2];
				cursorDQT += 64 * 2 + 1;
			}
      /* print
			std::cout << "---" << std::endl;
			if (QT[idx].id == 0)
				std::cout << "Table ID: " << QT[idx].id << " (luminance)" << std::endl;
			else
				std::cout << "Table ID: " << QT[idx].id << " (chrominance)" << std::endl;
			std::cout << "Precision: " << QT[idx].precision * 8 << " bits" << std::endl;
			std::cout << "Quantization Table: (not applicable)" << std::endl;
      int *QTDeZZ = QT[idx].table;
      DeZigZag(QT[idx]);
			for (int i = 0; i < 8; i++)
			{
				for (int j = 0; j < 8; j++)
					std::cout << QTDeZZ[i * 8 + j] << "\t";
				std::cout << std::endl;
			}
      */
		}
		cursor += length;
	}
	else if (marker == DRI)
	{
		int offset = cursor - 1;
		cursor++;
		int length = buffer[cursor] * 256 + buffer[cursor + 1];
		cursor += 2;
		Ri = buffer[cursor] * 256 + buffer[cursor + 1];
		cursor += 2;
		std::cout << "================ MARKER: DRI =============================" << std::endl;
		std::cout << "Offset: " << offset << std::endl;
		std::cout << "Length: " << length << std::endl;
		std::cout << "Interval: " << Ri << std::endl;
	}
	else if (marker == DHT)
	{
		int offset = cursor - 1;
		cursor++;
		int length = buffer[cursor] * 256 + buffer[cursor + 1];

		std::cout << "================ MARKER: DHT =============================" << std::endl;
		std::cout << "Offset: " << offset << std::endl;
		std::cout << "Length: " << length << std::endl;

		int cursorDHT = cursor + 2;
		while (cursorDHT != offset + length + 2)
		{
			int HTInfo = buffer[cursorDHT];
			int id = HTInfo % 16;
			int hClass = HTInfo / 16;
			int idx = id * 2 + hClass;
			HT[idx].id = id;
			HT[idx].hClass = hClass;
			int numCode = 0;
			for (int i = 0; i < 16; i++)
			{
				HT[idx].lenTable[i] = buffer[cursorDHT + 1 + i];
				numCode += buffer[cursorDHT + 1 + i];
			}
			HT[idx].valTable = (int *)malloc(sizeof(int) * numCode);
			for (int i = 0; i < numCode; i++)
			{
				HT[idx].valTable[i] = buffer[cursorDHT + 17 + i];
			}
			HT[idx].HTTable = ExpandDHT(HT[idx].lenTable, HT[idx].valTable, numCode);
			cursorDHT += 16 + numCode + 1;

      /* print HT
			std::cout << "---" << std::endl;
			std::cout << "Table ID: " << HT[idx].id << std::endl;
			char *hClassName;
			switch (HT[idx].hClass)
			{
			case 0:
				hClassName = " (DC Table)";
				break;
			case 1:
				hClassName = " (AC Table)";
				break;
			default:
				break;
			}
			std::cout << "Table Class: " << HT[idx].hClass << hClassName << std::endl;
			int cursorVal = 0;
			for (int i = 0; i < 16; i++)
			{
				std::cout << "Codes of length " << (i > 9 ? "" : "0") << i <<
					" bits (" << (HT[idx].lenTable[i] > 99 ? "" : (HT[idx].lenTable[i] > 9 ? "0" : "00")) << HT[idx].lenTable[i] <<
					" total): ";
				for (int j = 0; j < HT[idx].lenTable[i]; j++)
				{
					int high = HT[idx].valTable[cursorVal + j] / 16;
					int low = HT[idx].valTable[cursorVal + j] % 16;
					char highHex, lowHex;
					switch (low)
					{
					case 15:
						lowHex = 'F';
						break;
					case 14:
						lowHex = 'E';
						break;
					case 13:
						lowHex = 'D';
						break;
					case 12:
						lowHex = 'C';
						break;
					case 11:
						lowHex = 'B';
						break;
					case 10:
						lowHex = 'A';
						break;
					default:
						lowHex = low + '0';
					}
					switch (high)
					{
					case 15:
						highHex = 'F';
						break;
					case 14:
						highHex = 'E';
						break;
					case 13:
						highHex = 'D';
						break;
					case 12:
						highHex = 'C';
						break;
					case 11:
						highHex = 'B';
						break;
					case 10:
						highHex = 'A';
						break;
					default:
						highHex = high + '0';
					}
					std::cout << highHex << lowHex << " ";
				}
				std::cout << std::endl;
				cursorVal += HT[idx].lenTable[i];
			}
      */
		}
		cursor += length;
	}
}

void UpSample(int *block, int *upBlock, int sampleH, int sampleV){
	for (int i = 0; i < 8; i++)
		for (int j = 0; j < 8; j++)
			for (int v = 0; v < sampleV; v++)
				for (int h = 0; h < sampleH; h++)
					upBlock[(i * sampleV + v) * sampleH * 8 + (j * sampleH + h)] = block[i * 8 + j];
}

void char2Binary(unsigned char a, int *bit){
  /*
  int b = 128;
	for (int i = 0; i < 8; i++)
	{
		bit[i] = a / b;
		a = a % b;
		b = b >> 1;
	} 
  */
  for (int i = 7; i >= 0; i--)
  {
    bit[i] = a & 1;
    a = a >> 1;
  }
}

int getNextBit(unsigned char *buffer, int &cursor, int &bitPos, int *bitBuffer){
	if (bitPos < 8)
	{
		int ret = bitBuffer[bitPos];
		bitPos++;
		return ret;
	}
	else
	{
		bitPos = 0;
		int ret;
		if (buffer[cursor] != MARK)
			cursor++;
		else
			cursor += 2;
		char2Binary(buffer[cursor], bitBuffer);
		ret = bitBuffer[bitPos];
		bitPos++;
		return ret;
	}
}

int getValue(unsigned char *buffer, int &cursor, int &bitPos, int *bitBuffer, int codeLength){
	if (codeLength)
	{
		int value = 0;
    int midPoint = twoPowers[codeLength - 1]; // pow(2, codeLength - 1);
    int lowPoint = -twoPowers[codeLength] + 1; // -pow(2, codeLength) + 1;
		int mul = midPoint;
		for (int i = 0; i < codeLength; i++)
		{
			value += getNextBit(buffer, cursor, bitPos, bitBuffer) * mul;
			mul = mul >> 1;
		}
		if (value >= midPoint)
			return value;
		else
			return value + lowPoint;
	}
	else
		return 0;

}

void IDCT(double *F){
	double S[8];
	double E[8];

	S[0] = F[0] / (2 * sqrt2);
	S[1] = F[4] / (2 * sqrt2);
	S[2] = F[2] / 2;
	S[3] = F[6] / 2;
	S[4] = F[1] / 2;
	S[5] = F[7] / 2;
	S[6] = F[3] / 2;
	S[7] = F[5] / 2;

	E[0] = S[0] + S[1];
	E[1] = S[0] - S[1];
	E[2] = s2 * S[2] - c2 * S[3];
	E[3] = c2 * S[2] + s2 * S[3];
	E[4] = c1 * S[4] + s1 * S[5];
	E[5] = s1 * S[4] - c1 * S[5];
	E[6] = c3 * S[6] + s3 * S[7];
	E[7] = s3 * S[6] - c3 * S[7];
	for (int i = 0; i < 8; i++)
		S[i] = E[i];

	E[0] = S[0] + S[3];
	E[1] = S[1] + S[2];
	E[2] = S[1] - S[2];
	E[3] = S[0] - S[3];
	E[4] = S[4] + S[6];
	E[5] = S[5] + S[7];
	E[6] = S[4] - S[6];
	E[7] = S[5] - S[7];
	for (int i = 0; i < 8; i++)
		S[i] = E[i];

	E[5] = c4 * S[5] + c4 * S[6];
	E[6] = c4 * S[5] - c4 * S[6];
	for (int i = 5; i < 7; i++)
		S[i] = E[i];

	for (int i = 0; i < 4; i++)
	{
		E[i] = S[i] + S[4 + i];
		E[i + 4] = S[i] - S[4 + i];
	}

	F[0] = E[0];
	F[1] = E[1];
	F[2] = E[6];
	F[3] = E[3];
	F[4] = E[7];
	F[5] = E[2];
	F[6] = E[5];
	F[7] = E[4];
}

void IDCT2(int *block){
	double F[64];
	double temp[64];

	for (int i = 0; i < 64; i++)
		F[i] = (double)block[i];

	for (int i = 0; i < 8; i++)
	{

		for (int j = 0; j < 8; j++)
		{
			temp[j] = F[i * 8 + j];
		}
		IDCT(temp);
		for (int j = 0; j < 8; j++)
		{
			F[i * 8 + j] = temp[j];
		}
	}

	for (int i = 0; i < 8; i++)
	{
		for (int j = 0; j < 8; j++)
		{
			temp[j] = F[j * 8 + i];
		}
		IDCT(temp);
		for (int j = 0; j < 8; j++)
		{
			F[j * 8 + i] = temp[j];
		}
	}

	for (int i = 0; i < 64; i++)
		block[i] = (int)F[i];

}

void DecodeBlock(int *block, unsigned char *buffer, int &cursor, int &bitPos, int *bitBuffer, int &predDC, int *HTTableDC, int *HTTableAC, int *QTable){
	for (int i = 0; i < 64; i++)
		block[i] = 0;
	int cursorSeq = 0;
	int cursorHT = 0;
	while (!HTTableDC[cursorHT * 5 + 4])
	{
		int bit = getNextBit(buffer, cursor, bitPos, bitBuffer);
		if (bit)
			cursorHT = HTTableDC[cursorHT * 5 + 2];
		else
			cursorHT = HTTableDC[cursorHT * 5 + 1];
	}
	int DCCodeWord = HTTableDC[cursorHT * 5];
	int DCCodeLength = DCCodeWord % 16;
	int DCValue = getValue(buffer, cursor, bitPos, bitBuffer, DCCodeLength);
	DCValue = DCValue + predDC;
	predDC = DCValue;
	block[cursorSeq++] = DCValue;

	int ACCodeWord = 1;
	while (ACCodeWord && cursorSeq < 64)
	{
		cursorHT = 0;
		while (!HTTableAC[cursorHT * 5 + 4])
		{
			int bit = getNextBit(buffer, cursor, bitPos, bitBuffer);
			if (bit)
				cursorHT = HTTableAC[cursorHT * 5 + 2];
			else
				cursorHT = HTTableAC[cursorHT * 5 + 1];
		}
		ACCodeWord = HTTableAC[cursorHT * 5];
		int ACCodeLength = ACCodeWord % 16;
		int ACRunLength = ACCodeWord >> 4;
		int ACValue = getValue(buffer, cursor, bitPos, bitBuffer, ACCodeLength);
		block[cursorSeq + ACRunLength] = ACValue;
		cursorSeq += ACRunLength + 1;
	}

	for (int i = 0; i < 64; i++)
		block[i] = block[i] * QTable[i];
	DeZigZag(block);
	//double *blockDouble = IDCT2(block);
	IDCT2(block);
	for (int i = 0; i < 64; i++)
		block[i] += 128;
}

void DecodeRestartInterval(unsigned char *buffer, int &cursor, InScanComponent *InScanComp, int numInScanComp){
	int HMax = 0;
	int VMax = 0;
	for (int i = 0; i < numInScanComp; i++)
	{
		if (HMax < InScanComp[i].Hi)
			HMax = InScanComp[i].Hi;
		if (VMax < InScanComp[i].Vi)
			VMax = InScanComp[i].Vi;
	}

	int sampleRateXMax = 0;
	int sampleRateYMax = 0;
	for (int i = 0; i < numInScanComp; i++)
	{
		if (sampleRateXMax < InScanComp[i].sampleRateX)
			sampleRateXMax = InScanComp[i].sampleRateX;
		if (sampleRateYMax < InScanComp[i].sampleRateY)
			sampleRateYMax = InScanComp[i].sampleRateY;
	}
  for (int k = 0; k < numInScanComp; k++)
  {
    InScanComp[k].sampleH = sampleRateXMax / InScanComp[k].sampleRateX;
    InScanComp[k].sampleV = sampleRateYMax / InScanComp[k].sampleRateY;
  }

	int MCUWidth = 8 * HMax;
	int MCUHeight = 8 * VMax;
	int MCUHk = imageWidth / MCUWidth;
	int MCUVk = imageHeight / MCUHeight;
	imagePadded = (unsigned char *)malloc(sizeof(unsigned char) * MCUWidth * MCUVk * MCUHeight * MCUHk * numComponent);
	int *predDC = (int *)malloc(sizeof(int) * numInScanComp);
	for (int i = 0; i < numInScanComp; i++)
		predDC[i] = 0;
	int countMCU = 0;
	int bitPos = 0; /* cursor within a byte */
	int *bitBuffer = (int *)malloc(sizeof(int) * 8);
	char2Binary(buffer[cursor], bitBuffer);
  int *MCU = (int *)malloc(sizeof(int) * MCUHeight * MCUWidth * numInScanComp);
  int blockUpSampled[256];
  int block[64];

  clock_t launch = clock();
 
	for (int i = 0; i < MCUVk; i++)
	{
		for (int j = 0; j < MCUHk; j++)
		{
			for (int k = 0; k < numInScanComp; k++)
			{
				for (int v = 0; v < InScanComp[k].Vi; v++)
				{
					for (int h = 0; h < InScanComp[k].Hi; h++)
					{
						DecodeBlock(block, buffer, cursor, bitPos, bitBuffer, predDC[k], InScanComp[k].HTTableDC, InScanComp[k].HTTableAC, InScanComp[k].QTable);
            UpSample(block, blockUpSampled, InScanComp[k].sampleH, InScanComp[k].sampleV);
            for (int bv = 0; bv < 8 * InScanComp[k].sampleV; bv++)
              for (int bh = 0; bh < 8 * InScanComp[k].sampleH; bh++)
                MCU[k * MCUHeight * MCUWidth + (v * InScanComp[k].sampleV * 8 + bv) * MCUWidth + (h * InScanComp[k].sampleH * 8 + bh)] = block[bv * InScanComp[k].sampleV * 8 + bh];
					}
				}
			}
			for (int mk = 0; mk < numInScanComp; mk++)
				for (int mv = 0; mv < MCUHeight; mv++)
					for (int mh = 0; mh < MCUWidth; mh++)
						imagePadded[mk * MCUWidth * MCUHk * MCUHeight * MCUVk + (i * MCUHeight + mv) * MCUWidth * MCUHk + (j * MCUWidth + mh)] = MCU[mk * MCUWidth * MCUHeight + mv * MCUWidth + mh];
			
			countMCU++;
			if (countMCU == Ri)
			{
				countMCU = 0;
				for (int i = 0; i < numInScanComp; i++)
					predDC[i] = 0;
				if (buffer[cursor] == MARK)
					cursor += 4;
				else
					cursor += 3;
				char2Binary(buffer[cursor], bitBuffer);
				bitPos = 0;
			}
		}
  }
  double decodeTime = (double)(clock() - launch) / 1000;
  std::cout << "Decode bit stream elapsed time is: " << decodeTime << " secs." << std::endl;

  free(MCU);
  free(bitBuffer);
	for (int k = 0; k < numComponent; k++)
		for (int i = 0; i < imageHeight; i++)
			for (int j = 0; j < imageWidth; j++)
				imageDecoded[k * imageWidth * imageHeight + i * imageWidth + j] = imagePadded[k * MCUWidth * MCUHk * MCUHeight * MCUVk + i * MCUWidth * MCUHk + j];
	free(imagePadded);
}

void DecodeScan_Baseline(unsigned char *buffer, int &cursor){

	/* Interpret Scan Header */
	int offset = cursor - 1;
	cursor++;
	int length = buffer[cursor] * 256 + buffer[cursor + 1];
	cursor += 2;
	int numInScanComp = buffer[cursor];
	cursor++;
	InScanComponent *InScanComp = (InScanComponent *)malloc(sizeof(InScanComponent) * numInScanComp);

	int sampleRateXInScanMin = 100;
	int sampleRateYInScanMin = 100;
	for (int i = 0; i < numInScanComp; i++)
	{
		InScanComp[i].selector = buffer[cursor];
		cursor++;
		int HTTableIDs = buffer[cursor];
		cursor++;
		InScanComp[i].idDC = (HTTableIDs / 16);
		InScanComp[i].idAC = (HTTableIDs % 16);
		int HTTableIdxDC = (HTTableIDs / 16) * 2;
		int HTTableIdxAC = (HTTableIDs % 16) * 2 + 1;
		InScanComp[i].HTTableDC = HT[HTTableIdxDC].HTTable;
		InScanComp[i].HTTableAC = HT[HTTableIdxAC].HTTable;
		InScanComp[i].QTable = QT[component[InScanComp[i].selector - 1].QTableID].table;
		InScanComp[i].sampleRateX = component[InScanComp[i].selector - 1].sampleRateX;
		InScanComp[i].sampleRateY = component[InScanComp[i].selector - 1].sampleRateY;
		if (InScanComp[i].sampleRateX < sampleRateXInScanMin)
			sampleRateXInScanMin = InScanComp[i].sampleRateX;
		if (InScanComp[i].sampleRateY < sampleRateYInScanMin)
			sampleRateYInScanMin = InScanComp[i].sampleRateY;
	}
	cursor += 3;
	for (int i = 0; i < numInScanComp; i++)
	{
		InScanComp[i].Hi = InScanComp[i].sampleRateX / sampleRateXInScanMin;
		InScanComp[i].Vi = InScanComp[i].sampleRateY / sampleRateYInScanMin;
	}

	std::cout << "================ MARKER: SOS =============================" << std::endl;
	std::cout << "Offset: " << offset << std::endl;
	std::cout << "Length: " << length << std::endl;
  /* print 
	std::cout << "Number of Component in scan: " << numInScanComp << std::endl;
	for (int i = 0; i < numInScanComp; i++)
	{
		std::cout << "  Component[" << i <<
			"]: selector = " << InScanComp[i].selector <<
			", table = " << InScanComp[i].idDC <<
			"(DC), " << InScanComp[i].idAC <<
			"(AC)" << std::endl;
	}
  */

	DecodeRestartInterval(buffer, cursor, InScanComp, numInScanComp);
}

void DecodeFrame_Baseline(unsigned char *buffer, int &cursor){

	/* Interpret Frame Header */
	int offset = cursor - 1;
	cursor++;
	int length = buffer[cursor] * 256 + buffer[cursor + 1];
	cursor += 2;

	std::cout << "================ MARKER: SOF0 ============================" << std::endl;
	std::cout << "Offset: " << offset << std::endl;
	std::cout << "Length: " << length << std::endl;

	int samplePrecision = buffer[cursor];
	cursor++;
	imageHeight = buffer[cursor] * 256 + buffer[cursor + 1];
	cursor += 2;
	imageWidth = buffer[cursor] * 256 + buffer[cursor + 1];
	cursor += 2;
	numComponent = buffer[cursor];
	cursor++;
	for (int i = 0; i < numComponent; i++)
	{
		int id = buffer[cursor];
		cursor++;
		int idx = id - 1;
		component[idx].id = id;
		int sampleRate = buffer[cursor];
		cursor++;
		component[idx].sampleRateX = sampleRate / 16;
		component[idx].sampleRateY = sampleRate % 16;
		component[idx].QTableID = buffer[cursor];
		cursor++;
	}
	int sampleRateXMax = 0;
	int sampleRateYMax = 0;
	for (int i = 0; i < numComponent; i++)
	{
		if (component[i].sampleRateX >= sampleRateXMax)
			sampleRateXMax = component[i].sampleRateX;
		if (component[i].sampleRateY >= sampleRateYMax)
			sampleRateYMax = component[i].sampleRateY;
	}
  /* print 
	std::cout << "Sample Precision: " << samplePrecision << " bits" << std::endl;
	std::cout << "Image size: " << imageWidth << "*" << imageHeight << std::endl;
	std::cout << "Number of Components: " << numComponent << std::endl;
	for (int i = 0; i < numComponent; i++)
	{
		char *componentType;
		switch (component[i].id)
		{
		case 1:
			componentType = " (Lumin:  Y)";
			break;
		case 2:
			componentType = " (Chrom: Cb)";
			break;
		case 3:
			componentType = " (Chrom: Cr)";
			break;
		default:
			break;
		}
		std::cout << "  Component(#" << i <<
			"): ID = " << component[i].id << componentType <<
			", SubSamp " << sampleRateXMax / component[i].sampleRateX << "*" << sampleRateYMax / component[i].sampleRateY <<
			", Sel QT ID = " << component[i].QTableID << std::endl;
	}
  */
	while (1)
	{
		/* SOS Marker? */
		while (buffer[cursor] != MARK)
			cursor++;
		cursor++;
		if (buffer[cursor] == SOS)
		{
			/* Decode a Scan */
			imageDecoded = (unsigned char *)malloc(sizeof(unsigned char) * imageWidth * imageHeight * numComponent);
			DecodeScan_Baseline(buffer, cursor);
			break;
		}
		/* Interpret Markers */
		else if (buffer[cursor] == DQT)
			InterpretMarkers(DQT, buffer, cursor);
		else if (buffer[cursor] == DRI)
			InterpretMarkers(DRI, buffer, cursor);
		else if (buffer[cursor] == DHT)
			InterpretMarkers(DHT, buffer, cursor);
		else
		{
			std::cout << "Not able to decode. Unexpected marker (" << (int)buffer[cursor] << ") occur.";
			exit(0);
		}
	}

}

unsigned char *DecodeImage(unsigned char *buffer, int &cursor){

	// SOI MARKER? 
	while (buffer[cursor] != MARK)
		cursor++;
	cursor++;

	if (buffer[cursor] != SOI)
	{
		std::cout << "SOI Marker not found. Image file is corrupted." << std::endl;
		exit(0);
		cursor++;
	}

	/* Decoder_setup

	*/

	while (1)
	{
		/* SOFn MARKER? */
		while (buffer[cursor] != MARK)
			cursor++;
		cursor++;
		/* Interpret Markers */
		if (buffer[cursor] >= APP0 && buffer[cursor] <= APP15)
		{
			std::cout << "================ MARKER: APP0" << (int)buffer[cursor] - APP0 << " ===========================" << std::endl;
			std::cout << "Offset: " << cursor - 1 << std::endl;
			cursor++;
			int length = buffer[cursor] * 256 + buffer[cursor + 1];
			cursor = cursor + length;
			std::cout << "Length: " << length << std::endl;
		}
		else if (buffer[cursor] == DQT)
			InterpretMarkers(DQT, buffer, cursor);
		else if (buffer[cursor] == DRI)
			InterpretMarkers(DRI, buffer, cursor);
		else if (buffer[cursor] == DHT)
			InterpretMarkers(DHT, buffer, cursor);
		else if (buffer[cursor] == SOF0)
		{
			/* Decode a Frame */
			DecodeFrame_Baseline(buffer, cursor);
			break;
		}
		else
		{
			std::cout << "Not able to decode. Image is not encoded in Baseline mode.";
			exit(0);
		}
	}


	/* yCbCr2RGB */

	return imageDecoded;
}
