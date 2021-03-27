
#include "round_buffer.h"

#include <stdexcept>

using namespace std;

round_buffer::round_buffer(size_t size, int mode) {
    // TODO: catch bad_alloc
    buffer = new byte[size];

    this->size = size;
    readOffset = 0;
    writeOffset = 0;
    this->mode = mode;
}


byte* round_buffer::rb(size_t offset) {
    return buffer + ( (readOffset + offset) % size );
}

byte* round_buffer::wb(size_t offset) {
    return buffer + ( (writeOffset + offset) % size );
}


byte* round_buffer::read(size_t length) {
    if (length == 0) return nullptr;
    if ((mode & SAFE_READ) && ( (readOffset + length) > writeOffset)) {
//        throw runtime_error("round_buffer read overflow");
        return nullptr; // TODO: throw an exception?
    }
    readOffset += length; // Mark data as read
    return rb( -length ); // Now export marked data
}

byte round_buffer::read() {
    byte* d = round_buffer::read(1);
    if (d == nullptr) return static_cast<byte>(0);
    return *d;
}


bool round_buffer::write(byte* c, size_t length) {
    if (length == 0) return false;
    if ((mode & SAFE_WRITE) && ( (writeOffset + length) > readOffset)) {
//        throw runtime_error("round_buffer write overflow");
        return false; // TODO: throw an exception?
    }
    for (size_t i = 0; i < length; i++) {
        *wb() = *(c + i);
        writeOffset++;
    }
    return true;
}

bool round_buffer::write(byte c) {
    return round_buffer::write(&c, 1);
}


size_t round_buffer::available() const {
    return writeOffset - readOffset;
}



