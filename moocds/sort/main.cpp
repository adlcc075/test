#include <iostream>
#include <ctime>
#include <cstdio>
//#include "sortHelper.h"

using namespace std;

extern int* genRandom(int n, int left, int right);
//extern int* a;
int* selectSort(int n, int* a)
{
    for(int i = 0; i < n - 1; i++)
    {
        for(int j = i + 1; j < n; j++)
        {
            if(a[j] < a[i])
                swap(a[i], a[j]);
        }
    }
    return a;
}

int* insertSort(int n, int* a)
{
    for(int i = 1; i < n; i++)
    {
        for(int j = i ; j > 0; j--)
        {
            if(a[j] > a[j - 1])
                break;
            else
                swap(a[j], a[j-1]);
        }
    }
    return a;
}

int main()
{
    printf("CLOCKS_PER_SEC is %d\n", (int)CLOCKS_PER_SEC);
    int n = 100000;
    int* p = genRandom(n, 1, 1000);
    time_t start = clock();
    p = selectSort(n, p);
    time_t endt = clock();
    printf("the running time is : %f\n", double(endt -start)/CLOCKS_PER_SEC);
    start = clock();
    p = insertSort(n, p);
    endt = clock();
    printf("the running time is : %f\n", double(endt -start)/CLOCKS_PER_SEC);
    return 0;
}
