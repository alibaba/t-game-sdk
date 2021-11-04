#include <vector>
#include <stdio.h>
#include <malloc.h>
#include <exception>
#include <string.h>
int main(int argc, char* argv[])
{
    try {
        std::vector<int> vec;
        vec.resize(100);
        for (int i = 0; i < vec.size(); i++)
        {
            int toAddSize = rand() / 202;
            *(vec.data() + toAddSize) = 100;
            printf("vec random value[%d]: %d\n", i, vec[i]);
        }
    } catch (std::exception e)
    {
        printf(e.what());
    }
	return 0;
}