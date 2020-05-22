#include <thread>

#include "analyzerMapper.h"

#ifndef ANALYZERREDUCER_H
#define ANALYZERREDUCER_H

class CAnalyzerReducer
{
public:
    CAnalyzerReducer(string workingPath, string outputFileName, vector<unique_ptr<CAnalyzerMapper>> &mappers)
        : m_sumProperty2(0.0), m_numItems(0L), m_workingPath(workingPath), m_outputFileName(outputFileName),
          m_mappers(mappers)
    {
        m_bReadyForReducer.store(false);
        m_bDone.store(false);
    }

    void reducerTask();
    void startTask();
    void stopTask();

    bool isReadyForReducer() { return m_bReadyForReducer.load(); }
    void setReadbyForReducer(bool ready) { m_bReadyForReducer.store(ready); }

    bool isDone() { return m_bDone.load(); }

private:
    double m_sumProperty2;
    long long m_numItems;

    string m_workingPath;
    string m_outputFileName;

    // Atomic flag to indicate the reducer can start processing chunk files
    atomic_bool m_bReadyForReducer;
    // Atomic flag to indicate the reducer has finished the job
    atomic_bool m_bDone;

    vector<thread> m_threads;
    vector<unique_ptr<CAnalyzerMapper>> &m_mappers;

    // Embedded class that holds the chunk file and is responsible to read the chunk and load the data into items.
    class MapperBatchData
    {
    public:
        MapperBatchData(int mapperId, int batchIndex, string workingPath)
            : m_mapperId(mapperId), m_batchIndex(batchIndex), m_chunkIndex(0), m_workingPath(workingPath) {}
        CAnalyzeItem getNextItem();

        bool IsNoMoreData() { return m_noMoreData; }

    private:
        int m_mapperId;
        int m_batchIndex;
        int m_chunkIndex;
        string m_workingPath;
        queue<CAnalyzeItem> m_items;
        bool m_noMoreData;

        bool readChunkFile();
        string getChunkFileName(int mapperID, int batchNumber, int chunkIndex);
    };

    void appendFinalAverageObject(ofstream &fileStream);
};

#endif