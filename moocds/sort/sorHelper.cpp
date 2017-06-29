#include <iostream>
#include <ctime>
#include <algorithm>

using namespace std;

int* genRandom(int n, int left, int right)
{
    int *a = new int[n];
    srand(time(0));
    for(int i = 0; i < n; i++)
    {
        int num = rand() % (right - left + 1) + left;
        a[i] = num;
    }
    return a;
}
