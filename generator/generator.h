#include <string>
#include <random>

using namespace std;

class CGenerator
{
public:
    CGenerator(string fileName) : m_fileName(fileName), m_gen(m_rd()), m_gen1(m_rd()) {}
    void start();

private:
    string m_fileName;
    string m_cacheContent;

    const string ALPHA_NUM_CHARACTERS = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

    // Members to generate random numbers
    random_device m_rd;
    mt19937 m_gen;
    mt19937 m_gen1;

    string generateA();
    string generateNumericString(int wholeNumberLength, int maxDecimalPlaces);
    string generateAlphaNumbericString(int maxLength);
};