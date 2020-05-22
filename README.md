# Overview
The project consists of generator and analyzer. 
The generator is to generate the huge input.csv based on pseudo random data.

The analyzer is to analyze the huge input.csv and generate the output.json, which is even huger. 

# Design
The analyzer follows the map-reduce framework but put all the components into one application.

### CAnalyzerMaster 
* This class is responsible to read the huge file, break down the data from input into many batches and send batches to CAnalyzerMappers for processing. 

### CAnalyzerMapper 
* This class is responsible to process the smaller batch data, sort the data and further break down the data to smaller chunk and write into multiple chunk files. 
* Each CAnalyzerMapper instance has a task thread. 
* The generated chunk files are named "chunk_<mapperID>_<batchIndex>_<chunkIndex>". Basically the files with the same mapper ID and batch Index are sorted based on the chunk index. And every chunk file itself is also sorted. 


### CAnalyzerReducer
* This class is responsible to process all the chunk files based on the mapperID, batchIndex and chunkIndex. It loads all chunk files with chunk index 0 from all mappers and batch index. Only in this way, the result would be globally sorted across all files. 
* The CAnalyzerReducer also has a task thread. 
* The reducer uses a priority queue to sort the top node from each chunk files serials from mapper ID and batch index. Once the top node is found and popped, the next top node from the same mapper ID and batch index is filled into the reducer's priority queue. It's generally the K sorted list merging algorithm. the output is orderedly written into the output JSON file. 

### CAnalyzeItem
* This class encapsulate an individual item derived a row in the input file. All the required the conversion is implemented in this class. 
* This class also generate the JSON object string from its member data.


# How to Build
## Prerequisites
* GNU C++
* CMake

## Build
### Generator
* cd ./generator
* cmake CMakeLists.txt
* make
  
### Analyzor
* cd ./analyzer
* cmake CMakeLists.txt
* make

#Execution
### Generator
* cd ./generator    
* ./generator

### Analyzor
* cd ./analyzer
* ./analyzer

# Configuration
All configurations are in the main.cpp.

    constexpr char CSV_FILE_NAME[] = "/mnt/hgfs/Devel/input.csv";           // Input file
    constexpr char OUTPUT_JSON_FILE_NAME[] = "/mnt/hgfs/Devel/output.json"; // Output file
    constexpr char WORKING_FOLDER[] = "/mnt/hgfs/Devel/";                   // The folder to store all chunk files
    constexpr int NUMBER_OF_MAPPER_TASKS = 10;                              // Number of mapper task threads
    constexpr int CHUNK_FILE_SIZE = 2 * 1024 * 1024;                        // The max chunk file size. 

Notes that the chunk file size is set to 2MB because the application crashes in the reducing stage if it's set bigger than that. In another more powerful machine, it could be tuned big to achieve better performance.