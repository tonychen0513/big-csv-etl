#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>

#include "analyzerMaster.h"

CAnalyzerMaster::CAnalyzerMaster(string inputFileName, string workingFolder, string outputFileName, int taskNumber, int chunkFileSize, int batchSize)
    : m_inputFileName(inputFileName), m_taskNumber(taskNumber), m_reducer(workingFolder, outputFileName, m_mappers), m_batchSize(batchSize)
{
    // Initialize the multiple mapper objects and task threads
    for (int i = 0; i < taskNumber; i++)
    {
        unique_ptr<CAnalyzerMapper> pMapper(new CAnalyzerMapper(i, workingFolder, chunkFileSize));
        m_mappers.push_back(std::move(pMapper));
    }

    // Initialize the reducer task thread.
    m_reducer.startTask();
}

void CAnalyzerMaster::start()
{
    ifstream csvFile(m_inputFileName);
    try
    {
        // Get the input file length
        csvFile.seekg(0, csvFile.end);
        streampos csvFilelength = csvFile.tellg();
        csvFile.seekg(0, csvFile.beg);

        unique_ptr<char[]> buffer(new char[m_batchSize]);
        string leftOverMsg;
        while (csvFile)
        {
            cout << "About to read the input file... " << endl;
            streampos prevPos = csvFile.tellg();
            csvFile.read(buffer.get(), m_batchSize);
            streampos currPos = csvFile.tellg();

            int nBatchSize = (csvFile.eof() ? csvFilelength - prevPos : currPos - prevPos);
            cout << "The current batch size: " << nBatchSize << endl;

            dispatchItems(buffer, nBatchSize, leftOverMsg);

            // Wait for all mappers to finish processing this batch and read in the next
            // This is necessary to prevent the out of memory exception.
            while (!isAllMapperBatchDone())
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            }
        }

        csvFile.close();

        cout << "Move to Reducer process... " << endl;

        m_reducer.setReadbyForReducer(true);
        while (!m_reducer.isDone())
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        }
    }
    catch (const std::exception &e)
    {
        cerr << "Failed to analyze the CSV file. Error: " << e.what() << endl;
    }
}

//////////////////////////////////////////////////////////////////////////
// Description: End the threads that are initialized from the master.
//////////////////////////////////////////////////////////////////////////
void CAnalyzerMaster::end()
{
    for (auto &mapper : m_mappers)
    {
        mapper->stopTask();
    }

    m_reducer.stopTask();
}

/////////////////////////////////////////////////////////////////////////////////////////
// Description: Break down the chunk data read from the input file and align each
//              packets to the endline character and dispatch to each mappers
// Parameter:   buffer: the buffer data read from the input file
//              batchSize: length of the buffer in this batch
//              leftOverMsg: store the message cut out due to aligning to the endline characters
//                           The last batch's leftOverMsg will be applied to the beginning
//                           of the next batch and allocate to mappers.
/////////////////////////////////////////////////////////////////////////////////////////
void CAnalyzerMaster::dispatchItems(const unique_ptr<char[]> &buffer, int batchSize, string &leftOverMsg)
{
    char *p = buffer.get();
    int increment = batchSize / m_taskNumber;
    int index = 0;
    int bufferIndex = 0;
    cout << "The individual mapper data size: " << increment << endl;
    while (true)
    {
        // Found the clostest "\n" from the current cutting point
        int i = 0;
        while (*(p + bufferIndex + increment - i) != '\n')
            i++;

        // Send the packet ending on the "\n"
        string messagesToTask = (index == 0) ? leftOverMsg : "";
        messagesToTask += string(p + bufferIndex, increment - i);
        m_mappers[index]->addMessages(messagesToTask);

        index++;
        bufferIndex += increment - i + 1;

        // Calculate the left over string once the dispatching for the current batch is done.
        if (index == m_taskNumber)
        {
            leftOverMsg = string(p + bufferIndex, batchSize - bufferIndex + 1);
            break;
        }
    }

    cout << "Dispatch end!" << endl;
}

///////////////////////////////////////////////////////////////////////////////////
// Description: Check if all mappers has done the processing for the batch data.
///////////////////////////////////////////////////////////////////////////////////
bool CAnalyzerMaster::isAllMapperBatchDone()
{
    bool bAllMapperBatchDone = true;
    for (auto &mapper : m_mappers)
    {
        if (!mapper->IsBatchDone())
        {
            bAllMapperBatchDone = false;
            break;
        }
    }
    return bAllMapperBatchDone;
}