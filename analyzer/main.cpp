#include "analyzerMaster.h"

int main()
{
    constexpr char CSV_FILE_NAME[] = "/mnt/hgfs/Devel/input.csv";           // Input file
    constexpr char OUTPUT_JSON_FILE_NAME[] = "/mnt/hgfs/Devel/output.json"; // Output file
    constexpr char WORKING_FOLDER[] = "/mnt/hgfs/Devel/";                   // The folder to store all chunk files
    constexpr int NUMBER_OF_MAPPER_TASKS = 10;                              // Number of mapper task threads
    constexpr int CHUNK_FILE_SIZE = 2 * 1024 * 1024;                        // The max chunk file size.
    constexpr int BATCH_SIZE = 512 * 1024 * 1024;                           // The batch data size sent to mappers for processing

    CAnalyzerMaster master(CSV_FILE_NAME, WORKING_FOLDER, OUTPUT_JSON_FILE_NAME, NUMBER_OF_MAPPER_TASKS, CHUNK_FILE_SIZE, BATCH_SIZE);
    master.start();
    master.end();

    return 0;
}