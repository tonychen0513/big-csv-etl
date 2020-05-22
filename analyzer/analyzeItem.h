#include <string>

using namespace std;

#ifndef ANALYZEITEM_H
#define ANALYZEITEM_H

class CAnalyzeItem
{
public:
    CAnalyzeItem() = default;
    CAnalyzeItem(const string &line, bool bFromChunk);

    bool operator>(const CAnalyzeItem &that) const;
    bool operator<(const CAnalyzeItem &that) const;

    string serialize();
    string formatJSON();

    double GetProperty2() { return m_property2; }

private:
    int m_property1;
    double m_property2;
    string m_property3;
    string m_property4;
    string m_property5;
};

#endif