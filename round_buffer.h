
#ifndef THREAD_TESTS_ROUND_BUFFER_H
#define THREAD_TESTS_ROUND_BUFFER_H

#include <cstddef>

using namespace std;

/** @brief  Safe round FIFO buffer
 *
 * A lazy implementation of a "FIFO stack"-styled buffer
 * with some protection from reading garbage and overwriting
 */
class round_buffer {
private:
    byte* buffer;
    size_t size;
    // Since size_t is AFAIK guaranteed to be unsigned
    // overflow is possible, but very far. Still, TODO: fix overflow
    // Also: volatile to keep up to date, we can read while writing
    volatile size_t readOffset;
    volatile size_t writeOffset;
    int mode;

    // Helper functions to get pointers with read/write offsets + offset
    byte* rb(size_t offset = 0);
    byte* wb(size_t offset = 0);
public:
    /** @brief  Bitmask const
     * @param SAFE_READ  throws error when reading old or unwritten data<br>
     * @param SAFE_WRITE  throws error when writing over unread data
     */
    static constexpr int SAFE_READ =  1,
                         SAFE_WRITE = 2;

    /** @brief  round_buffer constructor
     *
     * @param size  Size of buffer in bytes
     * @param mode  Bitmask, default: SAFE_READ | SAFE_WRITE
     */
    explicit round_buffer(size_t size, int mode = SAFE_READ | SAFE_WRITE);

    /** @brief  Try read specified length of bytes from buffer
     *
     * @warning Reading more bytes than requested is undefined behavior
     *
     * @param length  Length of bytes to read
     * @return Pointer to beginning of read section
     *
     * @see available()
     */
    byte* read(size_t length);
    /** @brief  Try read a byte from buffer
     *
     * @return Value of read byte, 0 if none available
     *
     * @see read(size_t size)
     * @see available()
     */
    byte read();

    /** @brief  Try writing a byte array of specified length to buffer
     *
     * @param c  Pointer to byte array to write
     * @param length  Length of provided array
     * @return true if successful, false otherwise
     */
    bool write(byte* c, size_t length);
    /** @brief  Try writing a byte to buffer
     *
     * @param c  Byte to write
     * @return true if successful, false otherwise
     */
    bool write(byte c);

    /** @brief  Get how many bytes are available to read
     *
     * @return Count of available bytes to read
     */
    [[nodiscard]] size_t available() const;

};


#endif //THREAD_TESTS_ROUND_BUFFER_H
