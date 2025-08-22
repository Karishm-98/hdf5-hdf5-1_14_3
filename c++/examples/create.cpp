#include "H5Cpp.h"
#include <iostream>
#include <vector>

using namespace H5;

const H5std_string FILE_NAME("test_cs1_2d.h5");
const H5std_string DATASET_NAME("dset");

const int DIM0 = 1000;
const int DIM1 = 1000;
const int CHUNK0 = 100;
const int CHUNK1 = 100;

int main() {
    try {
        H5File file(FILE_NAME, H5F_ACC_TRUNC);

        hsize_t dims[2] = {DIM0, DIM1};
        hsize_t chunk_dims[2] = {CHUNK0, CHUNK1};

        DataSpace dataspace(2, dims);

        DSetCreatPropList prop;
        prop.setChunk(2, chunk_dims);

        DataSet dataset = file.createDataSet(DATASET_NAME, PredType::NATIVE_DOUBLE, dataspace, prop);

        std::vector<double> data(DIM0 * DIM1);
        for (int i = 0; i < DIM0 * DIM1; ++i)
            data[i] = static_cast<double>(i);

        dataset.write(data.data(), PredType::NATIVE_DOUBLE);

        std::cout << "✅ Created dataset '" << DATASET_NAME << "' in file '" << FILE_NAME << "'\n";
    } catch (FileIException &e) {
        e.printErrorStack();
        return -1;
    } catch (DataSetIException &e) {
        e.printErrorStack();
        return -1;
    } catch (DataSpaceIException &e) {
        e.printErrorStack();
        return -1;
    }

    return 0;
}
