makejpeg: JPEG.cpp Decoder.cpp
	g++ -o JPEG JPEG.cpp Decoder.cpp -I.
