#include <vector>
#include <stdio.h>
#include <malloc.h>
#include <string.h>

typedef int (*CALCULATE_FN)(int, int);

int sub(int a, int b)
{
	return a - b;
}

int add(int a, int b)
{
	return a + b;
}

__attribute__((used)) void test_threadcall1()
{
    int a = 0;
    for (int i = 0; i < 10000000; i++)
    {
        a += 2;
        printf("thread1 call %d\n", a);
    }
}

__attribute__((used)) void test_threadcall2()
{
    int a = 1000;
    for (int i = 0; i < 10000000; i++)
    {
        a += 1;
        printf("thread2 call %d\n", a);
    }
}

extern int execute(CALCULATE_FN fn, int p, int q);

int main(int argc, char* argv[])
{
    std::vector<int> vec;
    vec.resize(100);
    for (int i = 0; i < vec.size(); i++)
    {
        printf("vec random value[%d]: %d\n", i, vec[i]);
    }
	char* content = (char*)malloc(100);
	memset(content, 0, 100);
	for (int i = 0; i < 99; i++)
	{
	    content[i] = i;
	}
	printf("%s\n", content);

	FILE* f = fopen("./test.c", "rb");
	if (f != NULL)
	{
	    fseek(f, 0, SEEK_END);
	    long size = ftell(f);
	    fseek(f, 0, SEEK_SET);
	    fclose(f);
	    printf("file size %ld\n", size);
	}

	int result = execute(add, 5, 5);
	result = execute(sub, 5, 5);

	return 0;
}