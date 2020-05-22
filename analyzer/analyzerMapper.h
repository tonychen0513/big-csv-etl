#include <queue>
#include <mutex>
#include <thread>
#include <atomic>

#include "analyzeItem.h"

using namespace std;

#ifndef ANALYZERMAPPER_H
#define ANALYZERMAPPER_H

class CAnalyzerMapper
{
public:
    CAnalyzerMapper(int index, string workingPath, int chunkFileSize);

    void addMessages(const string &messages);

    queue<string> &getQueue() { return m_queueForMessages; }
    priority_queue<CAnalyzeItem, vector<CAnalyzeItem>, std::greater<CAnalyzeItem>> &getPQ() { return m_minPQ; }

    void mapTask();
    void stopTask();

    bool IsBatchDone() { return m_bBatchDone.load(); }
    void setBatchDone(bool bDone) { m_bBatchDone.store(bDone); }

    int getTotalBatchCount() { return m_totalBatchCount; }
    int getIndex() { return m_index; }

    const static string CHUNK_FILE_NAME_BASE;
    const static int CHUNK_FILE_SIZE;

private:
    int m_index;
    int m_totalBatchCount;
    string m_workingPath;
    int m_chunkFileSize;

    queue<string> m_queueForMessages;
    mutex m_mutexQueue;

    priority_queue<CAnalyzeItem, vector<CAnalyzeItem>, std::greater<CAnalyzeItem>> m_minPQ;

    thread m_thread;

    // Atomic flag to indicate a batch is processed
    atomic<bool> m_bBatchDone;
    // Atomic flag ot indicate the mapper task thread has done the job
    atomic<bool> m_bClose;

    string getChunkFileName(int batchIndex, int chunkIndex);
};

#endif