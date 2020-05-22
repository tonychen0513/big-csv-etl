#include <random>
#include <sstream>
#include <iostream>
#include <fstream>

#include "generator.h"

string CGenerator::generateA()
{
    return generateAlphaNumbericString(4) + generateNumericString(5, 0);
}

string CGenerator::generateNumericString(int wholeNumberLength, int maxDecimalPlaces)
{
    stringstream sswholeNumber;
    uniform_int_distribution<int> numberGen(0, 9);
    uniform_int_distribution<int> nonZeroNumberGen(1, 9);
    for (int i = 0; i < wholeNumberLength; i++)
    {
        sswholeNumber << (i == 0 ? nonZeroNumberGen(m_gen) : numberGen(m_gen));
    }

    stringstream ssDecimal;
    if (maxDecimalPlaces > 0)
    {
        uniform_int_distribution<int> decimalPlacesGen(1, maxDecimalPlaces);
        int decimalPlaces = decimalPlacesGen(m_gen);
        for (int i = 0; i < decimalPlaces; i++)
        {
            ssDecimal << numberGen(m_gen);
        }
    }

    return sswholeNumber.str() + (ssDecimal.str().empty() ? "" : ("." + ssDecimal.str()));
}

string CGenerator::generateAlphaNumbericString(int maxLength)
{
    uniform_int_distribution<int> lengthGen(1, maxLength);
    int length = lengthGen(m_gen);

    stringstream ssAlphaNumeric;
    uniform_int_distribution<int> alphaNumericGen(0, ALPHA_NUM_CHARACTERS.size() - 1);
    for (int i = 0; i < length; i++)
    {
        ssAlphaNumeric << ALPHA_NUM_CHARACTERS[alphaNumericGen(m_gen)];
    }

    return ssAlphaNumeric.str();
}

void CGenerator::start()
{
    cout << "Start to generate CSV file data... \n";
    uniform_int_distribution<int> wholeNumberLengthGen(1, 3);

    ofstream fsCSVFile;

    try
    {
        fsCSVFile.open(m_fileName, ios::out | ios::app);
        cout << "File " << m_fileName << " opened for writing. \n";

        int k = 0;
        stringstream ssLineBuilder;
        while (true)
        {
            ssLineBuilder << generateA() << ",";

            for (int i = 0; i < 2; i++)
            {
                ssLineBuilder << generateNumericString(wholeNumberLengthGen(m_gen), 3) << ",";
            }

            for (int i = 0; i < 7; i++)
            {
                ssLineBuilder << generateAlphaNumbericString(3) << (i == 6 ? "" : ",");
            }

            ssLineBuilder << endl;

            if (++k == 1000000)
            {
                fsCSVFile << ssLineBuilder.str();
                fsCSVFile.flush();
                ssLineBuilder.clear();

                k = 0;
                cout << "file size: " << fsCSVFile.tellp() << endl;

                if (fsCSVFile.tellp() / 1024 / 1024 / 1024 > 15)
                {
                    break;
                }
            }
        }

        fsCSVFile.close();
    }
    catch (const std::exception &e)
    {
        cerr << "Failed to write to CSV file. Error: " << e.what() << endl;
    }

    cout << "Finished generating CSV file.\n";
}