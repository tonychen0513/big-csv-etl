#include "generator.h"

int main()
{
    constexpr char CSV_FILE_NAME[] = "/mnt/hgfs/Devel/input.csv";

    CGenerator generator(CSV_FILE_NAME);
    generator.start();
    return 0;
}