#include <iostream>
#include <string>
#include <fstream>
#include <chrono>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>

using namespace std;

const int32_t ONE_MB = 1024*1024;
const int32_t counterThreadDataSize = 256*ONE_MB;

struct dataChunk {
    char* pointer;
    size_t size;
};

mutex mtx;
condition_variable cv;
bool threadFinish = false;
queue<dataChunk> threadQueue;


thread counterThread;
void counterFunc() {
    char* data = new char[counterThreadDataSize];
    fill(data, data + counterThreadDataSize, 'A');
    for (size_t i = 0; i < counterThreadDataSize/ONE_MB; i++) {
        dataChunk d = {
            .pointer = data + (i * ONE_MB),
            .size = ONE_MB
        };
        threadQueue.push(d);
        if (threadQueue.size() > 256) throw runtime_error("threadQueue buffer overflow (>256)");
        cv.notify_all();
    }
    threadFinish = true;
}

thread listenerThread;
void listenerFunc() {
    unique_lock<mutex> lck(mtx);
    ofstream file;
    file.open("thread-tests.tmp");
    while (!threadFinish) {
        cv.wait(lck);
        dataChunk d = threadQueue.front();
        threadQueue.pop();
        file.write(d.pointer, d.size);
        file.flush();
    }
    file.close();
}


int main(int argc, char* argv[]) {

    char* counter1MB = new char[ONE_MB];
    threadQueue = queue<dataChunk>();

    cout << "Starting threads..." << endl;

    listenerThread = thread(listenerFunc);
    counterThread = thread(counterFunc);

    chrono::steady_clock::time_point timer1 = chrono::steady_clock::now();

    if (!flag_dryRun) {
        for (int i = 0; i < filecount; ++i) {
            try {
                file.open("chrono-tests_"+to_string(i)+".tmp");
                file.write(data, filesize);
                file.close();
            } catch (ofstream::failure& err) {
                cerr << "Error when writing file! | ";
                cerr << err.what() << endl;
                return 1;
            }
        }
    }

    chrono::steady_clock::time_point timer2 = chrono::steady_clock::now();

    chrono::duration<double> timer = chrono::duration_cast<chrono::duration<double>>(timer2 - timer1);

    cout << "Test ended! Results:" << endl;
    cout << "Total time: " << timer.count() << " seconds." << endl;
    cout << "Average time per file: " << (timer.count()/filecount) << " seconds." << endl;

}