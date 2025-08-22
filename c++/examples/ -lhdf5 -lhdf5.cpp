#include <iostream>
#include <string>
#include <H5Cpp.h>
#include <H5Dpublic.h>  // For H5Dget_chunk_info()

using namespace H5;

const H5std_string FILE_NAME("SDS.h5");
const H5std_string DATASET_NAME("IntArray");

int main() {
    try {
        H5File file(FILE_NAME, H5F_ACC_RDONLY);
        DataSet dataset = file.openDataSet(DATASET_NAME);
        DataSpace dataspace = dataset.getSpace();

        // Get dataset dimensions
        hsize_t dims[2];
        dataspace.getSimpleExtentDims(dims);
        std::cout << "Dataset Dimensions: " << dims[0] << "x" << dims[1] << "\n";

        // Get chunk size
        DSetCreatPropList dcpl = dataset.getCreatePlist();
        hsize_t chunk_dims[2];
        dcpl.getChunk(2, chunk_dims);
        std::cout << "Chunk Dimensions: " << chunk_dims[0] << "x" << chunk_dims[1] << "\n";

        // Calculate number of chunks
        hsize_t nchunks[2] = {
            (dims[0] + chunk_dims[0] - 1) / chunk_dims[0],
            (dims[1] + chunk_dims[1] - 1) / chunk_dims[1]
        };

        hid_t dset_id = dataset.getId(); // Raw HDF5 dataset ID

        // Loop over each chunk
        for (hsize_t i = 0; i < nchunks[0]; ++i) {
            for (hsize_t j = 0; j < nchunks[1]; ++j) {
                hsize_t offset[2] = {i * chunk_dims[0], j * chunk_dims[1]};
                hsize_t index = i * nchunks[1] + j;

                hsize_t chunk_offset[2];
                uint32_t filter_mask;
                haddr_t chunk_addr;
                hsize_t chunk_size;

                herr_t status = H5Dget_chunk_info(dset_id, index, chunk_offset, &filter_mask, &chunk_addr, &chunk_size);
                if (status >= 0) {
                    std::cout << "Chunk (" << i << "," << j << ") at logical offset ["
                              << offset[0] << "," << offset[1] << "]:\n";
                    std::cout << " - File byte offset: " << chunk_addr << "\n";
                    std::cout << " - Chunk size (bytes): " << chunk_size << "\n";
                    std::cout << " - Filter mask: " << filter_mask << "\n";
                } else {
                    std::cerr << "Failed to get chunk info at index " << index << "\n";
                }
            }
        }
    } catch (Exception &error) {
        error.printErrorStack();
        return -1;
    }

    return 0;
}
