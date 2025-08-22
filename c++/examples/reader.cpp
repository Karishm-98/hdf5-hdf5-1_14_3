#include <iostream>
#include <H5Cpp.h>
#include <hdf5.h>

const std::string FILE_NAME    = "SDS.h5";
const std::string DATASET_NAME = "IntArray";

int
main()
{
    try {
        // Open file and dataset using C API
        hid_t file_id = H5Fopen(FILE_NAME.c_str(), H5F_ACC_RDONLY, H5P_DEFAULT);
        hid_t dset_id = H5Dopen2(file_id, DATASET_NAME.c_str(), H5P_DEFAULT);

        // Get the dataset's creation property list
        hid_t dcpl_id = H5Dget_create_plist(dset_id);

        // Check if it's chunked
        if (H5D_CHUNKED != H5Pget_layout(dcpl_id)) {
            std::cerr << "Dataset is not chunked!" << std::endl;
            return 1;
        }

        // Get chunk dimensions
        int     rank = H5Dget_space(dset_id);
        hsize_t chunk_dims[2];
        H5Pget_chunk(dcpl_id, 2, chunk_dims);
        std::cout << "Chunk dimensions: [" << chunk_dims[0] << ", " << chunk_dims[1] << "]" << std::endl;

        // Get number of chunks
        hsize_t dims[2];
        hid_t   space_id = H5Dget_space(dset_id);
        H5Sget_simple_extent_dims(space_id, dims, nullptr);
        hsize_t nchunks[2] = {(dims[0] + chunk_dims[0] - 1) / chunk_dims[0],
                              (dims[1] + chunk_dims[1] - 1) / chunk_dims[1]};

        std::cout << "Number of chunks: " << nchunks[0] << " x " << nchunks[1] << std::endl;

        hsize_t total_chunks = nchunks[0] * nchunks[1];

        for (hsize_t index = 0; index < total_chunks; ++index) {
            hsize_t  offset[2];
            unsigned filter_mask;
            haddr_t  chunk_addr;
            hsize_t  chunk_size;

            herr_t status = H5Dget_chunk_info(dset_id,      // dataset ID
                                              H5S_ALL,      // dataspace ID
                                              index,        // chunk index
                                              offset,       // chunk logical offset (e.g., [row, col])
                                              &filter_mask, // filter mask (if compression is applied)
                                              &chunk_addr,  // chunk address (byte offset in file)
                                              &chunk_size   // chunk size in bytes
            );

            if (status >= 0) {
                std::cout << "Chunk " << index << " at logical offset [" << offset[0] << ", " << offset[1]
                          << "]:\n";
                std::cout << "  -> File offset (address): " << chunk_addr << "\n";
                std::cout << "  -> Chunk size in file: " << chunk_size << " bytes\n";
                std::cout << "  -> Filter mask: " << filter_mask << "\n\n";
            }
            else {
                std::cerr << "Failed to get chunk info for index " << index << "\n";
            }
        }

        // Close resources
        H5Pclose(dcpl_id);
        H5Sclose(space_id);
        H5Dclose(dset_id);
        H5Fclose(file_id);
    }
    catch (...) {
        std::cerr << "Exception occurred!" << std::endl;
        return 1;
    }

    return 0;
}
