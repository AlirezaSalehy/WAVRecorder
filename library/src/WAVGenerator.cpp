#include "WAVGenerator.h"

unsigned char WAVTemplate[] = {
	0x52, 0x49, 0x46, 0x46, // Chunk ID (RIFF) 'R', 'I', 'F', 'F',
	0x24, 0x71, 0x02, 0x00, // Chunk payload => size 36 + subchunk2size 160036 
	0x57, 0x41, 0x56, 0x45, // RIFF resource format type 'W', 'A', 'V', 'E'
	
	0x66, 0x6D, 0x74, 0x20, // Chunk ID (fmt)  'f' , 'm' , 't' , ' ' , 
	0x10, 0x00, 0x00, 0x00, // Chunk payload size (0x10 = 16 bytes)
	0x01, 0x00,       		// Format Tag (IMA ADPCM)
	0x01, 0x00,       		// Channels (1)  
	0x40, 0x1F, 0x00, 0x00, // Sample Rate, 0x3e80 = 16.0kHz // 1F40
	0x80, 0x3E, 0x00, 0x00, // Average Bytes Per Second
	0x02, 0x00,       		// Data Block Size (2 bytes) 
	0x10, 0x00,       		// ADPCM encoded bits per sample (16 bits)
	
	0x64, 0x61, 0x74, 0x61, // Chunk ID (data) 'd' , 'a' , 't' , 'a'
	0x00, 0x71, 0x02, 0x00 	// Chunk payload size (calculate after rec!) 160000 0x27100
};

WAVGenerator::WAVGenerator(HardwareSerial* serial) {
	this->serial = serial;
	this->sampleChunkSize = 0;
	this->index = 0;
	//this->chunk = new uint8_t(512);
	
}

WAVGenerator::WAVGenerator(audio_t audio, HardwareSerial* serial) :
WAVGenerator::WAVGenerator(serial) {
	if (audio.numberOfChannels > 2)
		serial->println("Audio wave format has at most 2 channels");
	this->audio = audio;
	
	writeHeader();
}

void WAVGenerator::writeHeader() {
	uint32_t samplingRate = audio.samplingRate;
	uint16_t sampleLength = audio.sampleLength;
	
	// Num Channels :  //num of channels is set to 1 by default 
	uint16_t nch = audio.numberOfChannels; //1;
	putInArray((uint8_t*)&WAVTemplate[22], nch, sizeof(nch));
	
	// Sample Rate
	putInArray((uint8_t*)&WAVTemplate[24], samplingRate, sizeof(samplingRate)); 

	// Average bytes per second : samplingRate * NumChannels * BitsPerSample/8
	uint32_t bps = audio.samplingRate * nch * sampleLength/8;
	putInArray((uint8_t*)&WAVTemplate[28], bps, sizeof(bps)); 
	
	// Block Align : NumChannels * BitsPerSample/8
	uint16_t blockAlign = nch * sampleLength / 2;
	putInArray((uint8_t*)&WAVTemplate[32], blockAlign, sizeof(blockAlign)); 
	
	// Bits per sample
	putInArray((uint8_t*)&WAVTemplate[34], sampleLength, sizeof(sampleLength));
	
	audio.dataFile->write((uint8_t*)WAVTemplate, sizeof(WAVTemplate));
}

void WAVGenerator::setAudio(audio_t audio) {
	this->audio = audio;

	writeHeader();
	chunk = buffer;
	index = 0;
	sampleChunkSize = 0;
	toWriteChunk = 0;
	for (int i = 0; i < NUM_CHUNK; i++) {
		toMarkFill[i] = 0;
		isFilled[i] = 0;
	}	
}

/* If the next buffer is full the new data is not buffered so they are lost. in this case stopped is set then in write buffers
	This condition is checked and pull the buffering out of stopped mode.
	
	buffer is treated as circular. data is never overwritten but thrown away.
 */
void WAVGenerator::appendBuffer(uint8_t* data, uint16_t len, uint8_t isMarked) {
	uint16_t remainder = len;
	uint16_t endIndex = 0;
	
	// If the chunk is marked once it could not be cancelled by isMarked
	int chunkIndex = ((int)(chunk - buffer)) / CHUNK_SIZE;
	toMarkFill[chunkIndex] |= isMarked;
	
	// An improvement here is that we can write data to file directly from data pointer if 
	// it has a size larger than 512
	do {
		if (CHUNK_SIZE - index > remainder) {
			endIndex = remainder;
		}
		else if (index == 0){
			dataFile->write(data, CHUNK_SIZE);
			remainder -= CHUNK_SIZE;
			
			continue;
		}
		else {
			endIndex = CHUNK_SIZE - index;
		}
		
		for (int i = 0; i < endIndex; i++) {
			chunk[index++] = data[i];
		}
		
		if (index >= CHUNK_SIZE) {
			if (toMarkFill[chunkIndex]) {
				isFilled[chunkIndex] = 1;
				toMarkFill[chunkIndex] = 0;
			}
			
			chunkIndex++;
			if (chunkIndex >= NUM_CHUNK) {
				chunkIndex = 0;
			
			}
			
			if (!isFilled[chunkIndex]) {
				chunk = buffer + (chunkIndex * CHUNK_SIZE);
				index = 0;
			} else {
				stopped = 1;	
			}
			
			/*
			for (size_t i = 0; i < NUM_CHUNK; i++) {
				if (isFilled[i] == 0) {
					chunk = buffer + (i * CHUNK_SIZE);
					index = 0;

					break;
				}
			}			
			*/
		}
			
		remainder -= endIndex; 
	}
	while (remainder != 0);

}

/* because of never overwriting the data so we are sure if stopped happened and 
	This method called then the chunk should be equal to the next chunk, chunks are
	filled sequentially.
 */
void WAVGenerator::writeChunks() {
	int fillingChunk = ((int)(chunk - buffer)) / CHUNK_SIZE;

	while (toWriteChunk != fillingChunk) {
		if (isFilled[toWriteChunk]) {
			//serial->print("This chunk: ");
			//serial->println(toWriteChunk);
			
			audio.dataFile->write(buffer + (CHUNK_SIZE * toWriteChunk), CHUNK_SIZE);			
			isFilled[toWriteChunk] = 0;
			sampleChunkSize += CHUNK_SIZE;	
		}
		else { // This should only happen when stopped is no happened
			break;
		}
		
		toWriteChunk++;
		if (toWriteChunk >= NUM_CHUNK) {
			toWriteChunk = 0;
		}
	} 
	
	if (stopped) {
		stopped = 0;
		index = 0;
		
		fillingChunk++;
		if (fillingChunk >= NUM_CHUNK) {
			fillingChunk = 0;
		}
		
		chunk = buffer + (fillingChunk * CHUNK_SIZE);
	}
}

bool WAVGenerator::isBufferFull() {
	return index >= CHUNK_SIZE;
}

void WAVGenerator::flush() {
	writeChunks();
	
	int fillingChunk = ((int)(chunk - buffer)) / CHUNK_SIZE;
	if (index != 0 && toMarkFill[fillingChunk]) {
		audio.dataFile->write(chunk, index);
	}
	
	index = 0;
}

void WAVGenerator::create() {
	flush();
	
	appenedFileSize();
	
	
	closeFile();
}

void WAVGenerator::appenedFileSize() {
	// 36 + SubChunk2Size, or more precisely: 4 + (8 + SubChunk1Size) + (8 + SubChunk2Size) @4
	uint32_t chunkSizeTotal = sampleChunkSize + 36;
	audio.dataFile->seek(4);
	audio.dataFile->write((uint8_t*) &chunkSizeTotal, sizeof(chunkSizeTotal));
	
	// Subchunk2Size @40
	uint32_t sub2ChunkSize = sampleChunkSize;
	audio.dataFile->seek(40);
	audio.dataFile->write((uint8_t*) &sub2ChunkSize, sizeof(sub2ChunkSize));
}

void WAVGenerator::closeFile() {
	audio.dataFile->close();	
	audio.dataFile = 0;
}


void WAVGenerator::putInArray(uint8_t* dest, uint32_t src, uint8_t len) {
	for (int i = 0; i < len; i++) {
		dest[i] = (src >> (i * 8)) & (uint32_t)0x000000FF;
	}
}

/*
void WAVGenerator::markCurrentChunk() {
	int fillingChunk = ((int)(chunk - buffer)) / CHUNK_SIZE;
	toMarkFill[fillingChunk] = 1;
}
*/

// This should be debugged
void WAVGenerator::markIgnoredChunks(uint16_t numSamples) {
	int fillingChunk = ((int)(chunk - buffer)) / CHUNK_SIZE;
	//toMarkFill[fillingChunk] = 1;

	int8_t minChunks = ((numSamples + (numSamples % CHUNK_SIZE)) / CHUNK_SIZE) + 12;
	while (minChunks >= 0) {		
		if (fillingChunk >= minChunks /*&& isFilled[fillingChunk - minChunks]*/) {
			isFilled[fillingChunk - minChunks] = 1;
		}
		else if (fillingChunk < minChunks /*&& isFilled[(NUM_CHUNK + fillingChunk) - minChunks]*/) {
			isFilled[(NUM_CHUNK + fillingChunk) - minChunks] = 1;
		}
		
		minChunks--;
	}
}

void WAVGenerator::markAllChunks() {
	for (int i = 0; i < NUM_CHUNK; i++)
		toMarkFill[i] = 1;
}

/*
inline void WAVGenerator::appendBufferOverwrite(uint8_t* data, uint16_t len, uint8_t stat) {
	uint16_t remainder = len;
	uint16_t endIndex = 0;
	
	// An improvement here is that we can write data to file directly from data pointer if 
	// it has a size larger than 512
	do {
		if (CHUNK_SIZE - index > remainder) {
			endIndex = remainder;
		}
		else if (index == 0){
			audio.dataFile->write(data, CHUNK_SIZE);
			remainder -= CHUNK_SIZE;
			
			continue;
		}
		else {
			endIndex = CHUNK_SIZE - index;
		}
		
		for (int i = 0; i < endIndex; i++) {
			chunk[index++] = data[i];
		}
		
		if (index >= CHUNK_SIZE) {
			int chunkIndex = ((int)(chunk - buffer)) / CHUNK_SIZE;
			if (toMarkFill[chunkIndex]) {
				isFilled[chunkIndex] = 1;				
			}	
					
			chunkIndex++;
			if (chunkIndex >= NUM_CHUNK) {
				chunkIndex = 0;
			}
			
			
			chunk = buffer + (chunkIndex * CHUNK_SIZE);
			index = 0;
		}
			
		remainder -= endIndex; 
	}
	while (remainder != 0);
}
*/

/*
void WAVGenerator::writeMarkedChunks() {
	int fillingChunk = ((int)(chunk - buffer)) / CHUNK_SIZE;

	while (toWriteChunk != fillingChunk) {
		if (isMarked[toWriteChunk]) {
			audio.dataFile->write(buffer + (CHUNK_SIZE * toWriteChunk), CHUNK_SIZE);			
			isMarked[toWriteChunk] = 0;
			isFilled[toWriteChunk] = 0;
			sampleChunkSize += CHUNK_SIZE;	
		}
		else { // This should only happen when stopped is no happened
			break;
		}
		
		toWriteChunk++;
		if (toWriteChunk >= NUM_CHUNK) {
			toWriteChunk = 0;
		}
	}
	
	if (stopped) {
		stopped = 0;
		index = 0;
		
		fillingChunk++;
		if (fillingChunk >= NUM_CHUNK) {
			fillingChunk = 0;
		}
		
		chunk = buffer + (fillingChunk * CHUNK_SIZE);
	}
}
*/