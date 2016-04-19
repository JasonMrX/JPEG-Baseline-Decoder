#pragma once
#include "Decoder.h"

int main()
{
  clock_t launch = clock();
	FILE *fileptr;
	unsigned char *buffer;
	long fileLen;
	int cursor;
	unsigned char *imageDecoded;

	fileptr = fopen("mom.jpg", "rb");  // Open the file in binary mode
	fseek(fileptr, 0, SEEK_END);          // Jump to the end of the file
	fileLen = ftell(fileptr);             // Get the current byte offset in the file
	rewind(fileptr);                      // Jump back to the beginning of the file

	buffer = (unsigned char *)malloc((fileLen + 1)*sizeof(unsigned char)); // Enough memory for file + \0
	fread(buffer, fileLen, 1, fileptr); // Read in the entire file

	cursor = 0;

  
	imageDecoded = DecodeImage(buffer, cursor);
  double decodeTime = (double)(clock() - launch) / 1000;
  std::cout << "Total elapsed time is: " << decodeTime << " secs." << std::endl;

	fclose(fileptr); // Close the file
}

