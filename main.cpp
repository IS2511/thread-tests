#include <iostream>
#include <string>
#include <fstream>
#include <chrono>
#include <queue>
#include <thread>
#include <mutex>
//#include <atomic>
#include <condition_variable>

#include "round_buffer.h"

using namespace std;


#define FILENAME "TMP_thread-tests.bin"

#define ONE_MB (1024u*1024u) // unsigned int to prevent overflow

const size_t CHUNK_SIZE = ONE_MB;
const size_t BUFFER_SIZE = 256 * CHUNK_SIZE; // 256 MB
const size_t DATA_SIZE = 2 * 1024 * ONE_MB; // 2 GB
//const size_t DATA_SIZE = 512 * ONE_MB;

uint32_t chunkInboundSpeedUpper = 2000;
uint32_t chunkInboundSpeedLower = 0;
uint32_t chunkInboundSpeed = (chunkInboundSpeedUpper + chunkInboundSpeedLower)/2;
void cisLower() {
    chunkInboundSpeedUpper = chunkInboundSpeed;
    chunkInboundSpeed = (chunkInboundSpeedUpper + chunkInboundSpeedLower)/2;
}
void cisHigher() {
    chunkInboundSpeedLower = chunkInboundSpeed;
    chunkInboundSpeed = (chunkInboundSpeedUpper + chunkInboundSpeedLower)/2;
}


void fileError(ofstream::failure& err, const string& custom) {
    throw runtime_error(custom + " | " + err.what() + "\n");
}


// Clang-Tidy is complaining about initialisation in global scope,
// but I think these are safe enough
round_buffer chunkBuffer(BUFFER_SIZE);         // NOLINT(cert-err58-cpp)
//queue<chrono::duration<double>> tmrWriteQueue; // NOLINT(cert-err58-cpp)
//queue<size_t> timerQueueSizes;                 // NOLINT(cert-err58-cpp)


mutex mtxListener, mtxCounter, mtxMain;
condition_variable cvCounter, cvStart;
// atomic is optional bc timing is not strict, volatile is probably enough
volatile bool listenerReady = false, counterReady = false, outOfChunks = false;




thread counterThread;
void counterFunc(const char* DATA) {
    unique_lock<mutex> lck(mtxCounter);

    chrono::high_resolution_clock::time_point tmrStart;

    counterReady = true; // I'm ready
    cvStart.wait(lck); // Wait for equal start
    for (size_t offset = 0; offset < DATA_SIZE; offset += CHUNK_SIZE) {
        tmrStart = chrono::high_resolution_clock::now();

        // TODO: Is this needed? listenerThread should take care of this
        if ( !chunkBuffer.write( (byte*) (DATA + offset), CHUNK_SIZE) ) {
            cisLower(); // Speed down inbound if catching up to read pointer
        }

//        cout << ".";  cout.flush(); // TODO: Needed?

        // Slows down a lot, but only on very small time scales
        cvCounter.wait_until(lck, tmrStart + chrono::microseconds(1000000 / chunkInboundSpeed) );
    }

    outOfChunks = true;
}




thread listenerThread;
void listenerFunc() {
    unique_lock<mutex> lck(mtxListener);

    ofstream file;
    try {
        file.open(FILENAME);
    } catch (ofstream::failure& err) {
        fileError(err, "Error opening file for writing!");
        return;
    }

//    chrono::steady_clock::time_point tmrWriteStart, tmrWriteEnd;

    listenerReady = true; // I'm ready
    cvStart.wait(lck); // Wait for equal start
    while ( (!outOfChunks) || (chunkBuffer.available()) ) {
//        timerQueueSizes.push(chunkQueue.size());
        size_t dataLength = chunkBuffer.available();
        char* data = reinterpret_cast<char*>(chunkBuffer.read(dataLength));

        if (dataLength > CHUNK_SIZE*2) cisLower(); // Speed down inbound if >2 chunks available
        if (dataLength < CHUNK_SIZE) cisHigher(); // Speed up inbound if <1 chunks available

//        tmrWriteStart = chrono::steady_clock::now();
        try {
            file.write(data, dataLength);
            file.flush();
        } catch (ofstream::failure& err) {
            fileError(err, "Error writing file!");
            return;
        }
//        tmrWriteEnd = chrono::steady_clock::now();

//        tmrWriteQueue.push(chrono::duration_cast<chrono::duration<double>>(tmrWriteEnd - tmrWriteStart) );
    }
    try {
        file.close();
    } catch (ofstream::failure& err) {
        fileError(err, "Error closing file!");
        return;
    }
}




int main(int argc, char* argv[]) {
    unique_lock<mutex> lck(mtxMain);

    cout << "Allocation memory... "; cout.flush();

    // Can be big, static (global) initialisation could cause trouble
    char* DATA = new char[DATA_SIZE];

    cout << "Done!" << endl;
    cout << "Filling DATA... "; cout.flush();

    // Pre-filling
    for (size_t i = 0; i < DATA_SIZE; i++) {
        DATA[i] = static_cast<char>((i%255)+1);
    }

    cout << "Done!" << endl;
    cout << "Writing to disk..."; cout.flush();


    listenerThread = thread(listenerFunc);
    counterThread = thread(counterFunc, DATA);

    // Wait for threads to be ready
    // timeout for guaranteed main thread continuation, better crash then hang
    cvStart.wait_until(lck, chrono::steady_clock::now() + chrono::seconds(1),
                      [](){ return listenerReady && counterReady; });

    cvStart.notify_all(); // Start!


    // Wait for threads to exit
    counterThread.join();
    listenerThread.join();


    cout << "\nProgram finished!\n";
    cout << "Final chunk inbound speed (chunk/s): " << chunkInboundSpeed << endl;

}