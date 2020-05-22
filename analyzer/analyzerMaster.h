#include <string>
#include <queue>
#include <thread>
#include <mutex>

#include "analyzerMapper.h"
#include "analyzerReducer.h"

using namespace std;

class CAnalyzerMaster
{
public:
    CAnalyzerMaster(string inputFileName, string workingFolder, string outputFileName, int taskNumber, int chunkFileSize, int batchSize);
    void start();
    void end();

private:
    string m_inputFileName;
    int m_taskNumber;
    int m_batchSize;

    vector<unique_ptr<CAnalyzerMapper>> m_mappers;
    CAnalyzerReducer m_reducer;

    void dispatchItems(const unique_ptr<char[]> &buffer, int actualReadSize, string &leftOverMsg);
    bool isAllMapperBatchDone();
};
