#include <vector>
#include <sstream>

#include "analyzeItem.h"

///////////////////////////////////////////////////////////////////////////////////////////
// Description: Construct the item based on the input string
// Parameter:   item: The input string
//              bFromChunk: a flag to indicate if the item string is from the mapper
//              chunk file or original input file.
//////////////////////////////////////////////////////////////////////////////////////////
CAnalyzeItem::CAnalyzeItem(const string &item, bool bFromChunk)
{
    vector<string> columns;
    string temp;
    stringstream ssItem(item);

    while (std::getline(ssItem, temp, ','))
    {
        columns.push_back(temp);
    }

    if (!bFromChunk)
    {
        m_property1 = atoi(columns[0].substr(columns[0].size() - 5, 5).c_str());
        m_property2 = max(atof(columns[1].c_str()), atof(columns[2].c_str()));

        ostringstream oss;
        oss << m_property1 << columns[9];
        m_property3 = oss.str();
        m_property4 = columns[6];
        m_property5 = columns[7];
    }
    else
    {
        m_property1 = atoi(columns[0].c_str());
        m_property2 = atof(columns[1].c_str());
        m_property3 = columns[2];
        m_property4 = columns[3];
        m_property5 = columns[4];
    }
}

//////////////////////////////////////////////////////////////////////////
// Description: Overload the < operator of the item to set the item as the
//              key in priority queue.
//////////////////////////////////////////////////////////////////////////
bool CAnalyzeItem::operator>(const CAnalyzeItem &that) const
{
    if (this->m_property1 != that.m_property1)
    {
        return this->m_property1 > that.m_property1;
    }
    else
    {
        if (this->m_property4 != that.m_property4)
        {
            return this->m_property4 > that.m_property4;
        }
        else
        {
            return this->m_property5 > that.m_property5;
        }
    }
}

//////////////////////////////////////////////////////////////////////////
// Description: Overload the < operator of the item to set the item as the
//              key in priority queue.
//////////////////////////////////////////////////////////////////////////
bool CAnalyzeItem::operator<(const CAnalyzeItem &that) const
{
    if (this->m_property1 != that.m_property1)
    {
        return this->m_property1 < that.m_property1;
    }
    else
    {
        if (this->m_property4 != that.m_property4)
        {
            return this->m_property4 < that.m_property4;
        }
        else
        {
            return this->m_property5 < that.m_property5;
        }
    }
}

//////////////////////////////////////////////////////////////////////////
// Description: Serialize the item.
//////////////////////////////////////////////////////////////////////////
string CAnalyzeItem::serialize()
{
    stringstream ss;
    ss << m_property1 << "," << m_property2 << "," << m_property3 << "," << m_property4 << "," << m_property5 << endl;

    return ss.str();
}

string CAnalyzeItem::formatJSON()
{
    stringstream ss;
    ss << "{\n";
    ss << "\"property1\":" << m_property1 << ",\n";
    ss << "\"property2\":" << m_property2 << ",\n";
    ss << "\"property3\":\"" << m_property3 << "\",\n";
    ss << "\"property4\":\"" << m_property4 << "\",\n";
    ss << "\"property5\":\"" << m_property5 << "\"\n";
    ss << "},\n";

    return ss.str();
}