#include <fstream>
#include <iostream>
#include <memory>
#include <map>
#include <sstream>
#include <iomanip>
#include <math.h>

#include "analyzerReducer.h"

double round(double val)
{
    if (val < 0)
        return ceil(val - 0.5);
    return floor(val + 0.5);
}

//////////////////////////////////////////////////////////////////////////
// Description: Get the next available item from the Mapper / Batch
// Return: The next available item.
//////////////////////////////////////////////////////////////////////////
CAnalyzeItem CAnalyzerReducer::MapperBatchData::getNextItem()
{
    if (!m_items.empty())
    {
        // If the queue has items, pop it out of the m_items queue.
        CAnalyzeItem result = m_items.front();
        m_items.pop();
        return result;
    }
    else
    {
        // If the queue is empty, load the current chunk file into the queue.
        // Then pop out the top item and return.
        if (readChunkFile())
        {
            m_chunkIndex++;
            CAnalyzeItem result = m_items.front();
            m_items.pop();
            return result;
        }
        else
        {
            m_noMoreData = true;
            return CAnalyzeItem();
        }
    }
}

//////////////////////////////////////////////////////////////////////////
// Description: read the chunk specified by the current mapper ID, the batch index
//              and the chunk index.
// Return: Indicate if the chunk file is read successfully.
//////////////////////////////////////////////////////////////////////////
bool CAnalyzerReducer::MapperBatchData::readChunkFile()
{
    ifstream chunkFile(getChunkFileName(m_mapperId, m_batchIndex, m_chunkIndex));
    if (chunkFile.good())
    {
        // Preset the size of the contents based on the size of chunk file.
        string contents;
        chunkFile.seekg(0, std::ios::end);
        contents.reserve(chunkFile.tellg());
        chunkFile.seekg(0, std::ios::beg);

        contents.assign((std::istreambuf_iterator<char>(chunkFile)),
                        std::istreambuf_iterator<char>());

        // Read every line from the contents and load CAnalyzeItem.
        istringstream iss(contents);
        string line;
        while (std::getline(iss, line))
        {
            CAnalyzeItem item(line, true);
            m_items.push(item);
        }
        chunkFile.close();
        return true;
    }
    else
    {
        return false;
    }
}

string CAnalyzerReducer::MapperBatchData::getChunkFileName(int mapperID, int batchNumber, int chunkIndex)
{
    return m_workingPath + CAnalyzerMapper::CHUNK_FILE_NAME_BASE + "_" + to_string(mapperID) + "_" + to_string(batchNumber) + "_" +
           to_string(chunkIndex) + ".csv";
}

//////////////////////////////////////////////////////////////////////////
// Description: The routine function for the reducer task thread.
//////////////////////////////////////////////////////////////////////////
void CAnalyzerReducer::reducerTask()
{
    while (true)
    {
        // If done, the thread can quit.
        if (m_bDone.load())
        {
            break;
        }

        // If the reducer is not ready to process, just wait.
        if (!isReadyForReducer())
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
        else
        {
            // Load the first chunk file of every mapper and batch into the map and minimal priority queue.
            map<pair<int, int>, MapperBatchData> mapMapperBatchIndexToData;
            priority_queue<pair<CAnalyzeItem, pair<int, int>>, vector<pair<CAnalyzeItem, pair<int, int>>>, std::greater<pair<CAnalyzeItem, pair<int, int>>>> minPQ;
            for (auto &mapper : m_mappers)
            {
                for (int batchIndex = 0; batchIndex < mapper->getTotalBatchCount(); batchIndex++)
                {
                    cout << "Add the data of mapper " << mapper->getIndex() << " batch " << batchIndex << " to the reducer minPQ.\n";
                    auto it = mapMapperBatchIndexToData.emplace(pair<int, int>(mapper->getIndex(), batchIndex),
                                                                MapperBatchData(mapper->getIndex(), batchIndex, m_workingPath));
                    CAnalyzeItem item = it.first->second.getNextItem();
                    if (!it.first->second.IsNoMoreData())
                    {
                        pair<int, int> mapperBatchIndex(mapper->getIndex(), batchIndex);
                        minPQ.push(pair<CAnalyzeItem, pair<int, int>>(item, mapperBatchIndex));
                    }
                }
            }

            // If the priority queue has no content, it means no content to write to output file. Skip the current process.
            if (minPQ.empty())
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(200));
                continue;
            }

            string result;
            ofstream fsResultJsonFile;

            try
            {
                cout << "Reducer start working!\n";

                // Start to write to the output JSON file
                fsResultJsonFile.open(m_outputFileName, ios::out | ios::app);
                fsResultJsonFile << "{\n";
                fsResultJsonFile << "\"array\":[\n";

                while (!minPQ.empty())
                {
                    auto itemAndIndex = minPQ.top();
                    minPQ.pop();

                    // Set it into the result string
                    result += itemAndIndex.first.formatJSON();
                    m_sumProperty2 += itemAndIndex.first.GetProperty2();
                    m_numItems += 1;

                    // Add to the priority queue the next item from the same mapper/batch that the item is popped out of the priority queue.
                    auto itMap = mapMapperBatchIndexToData.find(itemAndIndex.second);
                    if (itMap != mapMapperBatchIndexToData.end())
                    {
                        if (!itMap->second.IsNoMoreData())
                        {
                            CAnalyzeItem item = itMap->second.getNextItem();
                            if (!itMap->second.IsNoMoreData())
                            {
                                minPQ.push(pair<CAnalyzeItem, pair<int, int>>(item, itemAndIndex.second));
                            }
                        }
                    }

                    // Write the cached result to file if it's bigger than 100MB
                    if (result.size() > 100000000 || minPQ.empty())
                    {
                        cout << "Write data to output. Size: " << result.size() << "\n";
                        fsResultJsonFile << result;
                        fsResultJsonFile.flush();
                        result.clear();
                        cout << "Done writing data to output \n";
                    }
                }

                // Append the final object with average of property 2
                appendFinalAverageObject(fsResultJsonFile);

                fsResultJsonFile << "]\n";
                fsResultJsonFile << "}\n";
                fsResultJsonFile.close();
            }
            catch (const std::exception &e)
            {
                cerr << "Failed to write to middle CSV file. Error: " << e.what() << endl;
            }

            cout << "Reducer finished working!\n";
            m_bDone.store(true);
        }
    }
}

//////////////////////////////////////////////////////////////////////////
// Description: Append the object that contains the average of property2 to the JSON file.
// Parameter: fileStream - The output file stream
//////////////////////////////////////////////////////////////////////////
void CAnalyzerReducer::appendFinalAverageObject(ofstream &fileStream)
{
    fileStream << "{\n";
    fileStream << "\"property2\":" << round(m_sumProperty2 / m_numItems * 1000.0) / 1000.0 << "\n";
    fileStream << "}\n";
}

//////////////////////////////////////////////////////////////////////////
// Description: Start the reducer task thread.
//////////////////////////////////////////////////////////////////////////
void CAnalyzerReducer::startTask()
{
    m_threads.push_back(thread(&CAnalyzerReducer::reducerTask, this));
}

//////////////////////////////////////////////////////////////////////////
// Description: Stop the reducer task thread.
//////////////////////////////////////////////////////////////////////////
void CAnalyzerReducer::stopTask()
{
    for (auto &t : m_threads)
    {
        t.join();
    }
}