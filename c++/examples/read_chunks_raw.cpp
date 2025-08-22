#include <iostream>
#include <fstream>
#include <vector>
#include <unistd.h>
#include <fcntl.h>

const char* FILE_NAME = "SDS.h5";

// These are the same offsets printed by your HDF5 reader
std::vector<off_t> chunk_offsets = {4016, 4040, 4064, 4088, 4112, 4136};
const size_t chunk_size = 24; // bytes

int main() {
    int fd = open(FILE_NAME, O_RDONLY);
    if (fd < 0) {
        perror("Failed to open file");
        return 1;
    }

    std::cout << "Reading raw chunks from file:\n";
    for (size_t i = 0; i < chunk_offsets.size(); ++i) {
        off_t offset = chunk_offsets[i];
        if (lseek(fd, offset, SEEK_SET) < 0) {
            perror("lseek failed");
            continue;
        }

        char buffer[chunk_size];
        ssize_t bytes_read = read(fd, buffer, chunk_size);
        if (bytes_read != chunk_size) {
            std::cerr << "Failed to read chunk " << i << " at offset " << offset << "\n";
            continue;
        }

        std::cout << "Chunk " << i << " at offset " << offset << ": ";
        for (int j = 0; j < chunk_size; ++j)
            std::cout << std::hex << ((unsigned int)(unsigned char)buffer[j]) << " ";
        std::cout << std::dec << "\n";
    }

    close(fd);
    return 0;
}
