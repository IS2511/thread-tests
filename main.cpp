#include <iostream>
#include <string>
#include <fstream>
#include <chrono>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>

#define PREFILL_DATA

using namespace std;

const int64_t COUNTER_HZ = 1000;
const int32_t ONE_MB = 1024*1024;
const int32_t counterThreadDataSize = 256*ONE_MB;

struct dataChunk {
    char* pointer;
    size_t size;
};


void fileError(ofstream::failure& err, string custom) { // NOLINT(performance-unnecessary-value-param)
    throw runtime_error(custom + " | " + err.what() + "\n");
}


mutex mtxListener, mtxCounter, mtxMain;
condition_variable cvListener, cvCounter, cvMain, cvStart;
bool threadFinish = false;
// Why the warning? Very interesting...
// Clang-Tidy: Initialization of 'threadQueue' with static storage duration may throw an exception that cannot be caught
queue<dataChunk> threadQueue;
queue<chrono::duration<double>> threadTimerQueue;
queue<size_t> timerQueueSizes;

thread counterThread;
void counterFunc() {
    unique_lock<mutex> lck(mtxCounter);
    char* data = new char[counterThreadDataSize];
//    fill(data, data + counterThreadDataSize, 'A'); // Testing
#if defined(PREFILL_DATA)
    for (size_t i = 0; i < ONE_MB; i++) { // Pre-filling is much better
        data[i] = static_cast<char>((i%255)+1);
    }
#endif
    cvMain.notify_all(); // I'm ready
    cvStart.wait(lck); // Wait for equal start
    for (size_t i = 0; i < counterThreadDataSize/ONE_MB; i++) {
        chrono::high_resolution_clock::time_point timerStart = chrono::high_resolution_clock::now();
#if !defined(PREFILL_DATA)
        for (size_t j = 0; j < ONE_MB; j++) { // Slows down A LOT
            data[i*ONE_MB + j] = static_cast<char>((j%255)+1);
        }
#endif
        dataChunk d = {
            .pointer = data + (i * ONE_MB),
            .size = ONE_MB
        };
        threadQueue.push(d);
        if (threadQueue.size() > 256) throw runtime_error("threadQueue buffer overflow (>256)");
        if (i == (counterThreadDataSize/ONE_MB - 1)) threadFinish = true;
        cvListener.notify_all();

        cout << "."; cout.flush();

        // Slows down a lot, but only on smaller time scales
        cvCounter.wait_until(lck, timerStart + chrono::microseconds(1000000 / COUNTER_HZ));
    }

    while (!threadQueue.empty()) cvListener.notify_all();

}

thread listenerThread;
void listenerFunc() {
    unique_lock<mutex> lck(mtxListener);
    ofstream file;

    try {
        file.open("thread-tests.tmp");
    } catch (ofstream::failure& err) {
        fileError(err, "Error opening file for writing!");
        return;
    }
    chrono::steady_clock::time_point timer1, timer2;
    cvMain.notify_all(); // I'm ready
    cvStart.wait(lck); // Wait for equal start
    while ( !(threadQueue.empty() && threadFinish) ) {
        cvListener.wait(lck);
        timerQueueSizes.push(threadQueue.size());
        dataChunk d = threadQueue.front();
        threadQueue.pop();
        timer1 = chrono::steady_clock::now();
        try {
            file.write(d.pointer, d.size);
            file.flush();
        } catch (ofstream::failure& err) {
            fileError(err, "Error writing file!");
            return;
        }
        timer2 = chrono::steady_clock::now();
        threadTimerQueue.push( chrono::duration_cast<chrono::duration<double>>(timer2 - timer1) );
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

    threadQueue = queue<dataChunk>();

    cout << "Starting threads..." << endl;
    cout << "Doing the thing"; cout.flush();


    listenerThread = thread(listenerFunc);
    counterThread = thread(counterFunc);

    cvMain.wait_for(lck, chrono::seconds(1)); // Wait for threads to be ready
    cvMain.wait_for(lck, chrono::seconds(1)); // wait_for() to guarantee a start
    cvStart.notify_all(); // Start!

    counterThread.join();
    listenerThread.join();


    chrono::duration<double> total = chrono::seconds(0);
    size_t threadTimerQueueSize = threadTimerQueue.size();
    while (!threadTimerQueue.empty()) {
        chrono::duration<double> duration = threadTimerQueue.front();
        total += duration;
        threadTimerQueue.pop();
    }


    cout << "\nProgram finished!\n";
    cout << "Queue sizes on each write: ";
    while (!timerQueueSizes.empty()) {
        cout << timerQueueSizes.front() << " ";
        timerQueueSizes.pop();
    } cout << endl;
    cout << "Total time taken: " << total.count() << endl;
    cout << "Average time per write: " << (total/threadTimerQueueSize).count() << endl;
    cout << "Average write MB/s: " << (1/(total/threadTimerQueueSize).count()) << endl;

}