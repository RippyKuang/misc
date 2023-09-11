#include "stdio.h"
#include "stdlib.h"
#include "math.h"
#include "string.h"
#include "time.h"

/*

Created on 2023.3.3
@author: Rippy Kuang

*/

typedef struct _city
{
    int x;
    int y;
} city;

float get_distance(city *src, city *dst)
{
    float x = ((float)src->x - (float)dst->x);
    float y = ((float)src->y - (float)dst->y);
    return sqrt(x * x + y * y);
}

float **constructDisArr(city *citylist, int len)
{
    float **distance_arr = (float **)malloc(sizeof(float *) * len);
    for (int a = 0; a < len; a++)
    {
        distance_arr[a] = (float *)malloc(sizeof(float) * len);
        for (int b = 0; b <= a; b++)
        {
            float temp = get_distance(&citylist[a], &citylist[b]);
            distance_arr[a][b] = temp;
            distance_arr[b][a] = temp;
        }
    }
    return distance_arr;
}

float get_fittness(int len, int *route, float **distance_arr)
{
    float cost = 0;
    for (int a = 0; a < len - 1; a++)
        cost += distance_arr[route[a]][route[a + 1]];
    cost += distance_arr[route[0]][route[len - 1]];
    return 1 / cost;
}

int **get_randomRoutePool(int num, int len,int **routePool)
{

    for (int a = 0; a < num; a++)
    {
        for (int b = 0; b < len; b++)
            routePool[a][b] = b;
        for (int b = 0; b < len / 2; b++)
        {
            int r1 = rand() % (len - 1) + 1;
            int r2 = rand() % (len - 1) + 1;
            int temp;
            temp = routePool[a][r1];
            routePool[a][r1] = routePool[a][r2];
            routePool[a][r2] = temp;
        }
    }

    return routePool;
}
void free_Disarr(float **arr, int len)
{
    for (int a = 0; a < len; a++)
        free(arr[a]);
    free(arr);
}

void free_Routearr(int **arr, int num)
{
    for (int a = 0; a < num; a++)
        free(arr[a]);
    free(arr);
}

int **select(float **distance_arr, int **srcPool, int **dstPool, int len, int num, float *bestfit, float *avgFit)
{
    *bestfit = 0;
    *avgFit = 0;
    int bestIndex=0;
    for (int a = 0; a < num; a++)
    {
        int index1 = rand() % num;
        int index2 = rand() % num;

        float fitness1 = get_fittness(len, srcPool[index1], distance_arr);
        float fitness2 = get_fittness(len, srcPool[index2], distance_arr);

        if (fitness1 > fitness2)
        {
            for (int b = 0; b < len; b++)
                dstPool[a][b] = srcPool[index1][b];
            if (fitness1 > *bestfit)
                *bestfit = fitness1;
            *avgFit += fitness1;
            bestIndex=a;
        }
        else
        {
            for (int b = 0; b < len; b++)
                dstPool[a][b] = srcPool[index2][b];
            if (fitness2 > *bestfit)
                *bestfit = fitness2;
            *avgFit += fitness2;
            bestIndex=a;
        }
    }
    *avgFit = *avgFit / num;
    int **temp = srcPool;
    srcPool = dstPool;
    dstPool = temp;
    return srcPool;
}
void cross(int **routePool, float **distance_arr, int len, int num, float crossingRate, float bestfitness, float avgfitness)
{
    for (int d = 0; d < num; d++)
    {
        int t1 = rand() % num;
        int t2 = rand() % num;
        float cost = get_fittness(len, routePool[t1], distance_arr);
        float cost2 = get_fittness(len, routePool[t2], distance_arr);
        if (cost < cost2)
            cost = cost2;

        if (bestfitness - avgfitness < 1)
            crossingRate = crossingRate;
        else
            crossingRate = crossingRate * (bestfitness - cost) / (bestfitness - avgfitness);

        if ((rand() % 100) / 100.0 < crossingRate)
        {
            int crossPoint = rand() % (len - 2) + 1;
            int temp[len];
            memcpy(temp, routePool[t1], sizeof(int) * (len));
            memcpy(routePool[t1] + crossPoint, routePool[t2] + crossPoint, sizeof(int) * (len - crossPoint));
            memcpy(routePool[t2] + crossPoint, temp + crossPoint, sizeof(int) * (len - crossPoint));
            int hash[len];
            for (int a = 0; a < len; a++)
                hash[a] = 0;
            for (int a = 0; a < len; a++)
                hash[routePool[t2][a]] += 1;
            for (int a = crossPoint; a < len; a++)
            {
                if (hash[routePool[t2][a]] >= 2)
                {
                    int x = 0;
                    while (hash[x] != 0)
                        x++;
                    routePool[t2][a] = x;
                    hash[x]++;
                }
            }

            for (int a = 0; a < len; a++)
                hash[a] = 0;
            for (int a = 0; a < len; a++)
                hash[routePool[t1][a]] += 1;
            for (int a = crossPoint; a < len; a++)
            {
                if (hash[routePool[t1][a]] >= 2)
                {
                    int x = 0;
                    while (hash[x] != 0)
                        x++;
                    routePool[t1][a] = x;
                    hash[x]++;
                }
            }
        }
    }
}

void mutation(int **routePool, int len, int num, float mutatingRate, float **distance_arr, float bestfitness, float avgfitness)
{
    for (int a = 0; a < num; a++)
    {
        float cost = get_fittness(len, routePool[a], distance_arr);
        if (bestfitness - avgfitness < 1)
            mutatingRate = mutatingRate;
        else
            mutatingRate = mutatingRate * (bestfitness - cost) / (bestfitness - avgfitness);

        if ((rand() % 100) / 100.0 < mutatingRate)
        {
            int mutationPoint1 = (rand() % (len - 1)) + 1;
            int mutationPoint2 = (rand() % (len - 1)) + 1;
            int temp1 = routePool[a][mutationPoint1];
            float cost_ori = get_fittness(len, routePool[a], distance_arr);
            int temp = routePool[a][mutationPoint1];
            routePool[a][mutationPoint1] = routePool[a][mutationPoint2];
            routePool[a][mutationPoint2] = temp;
            float cost_mut = get_fittness(len, routePool[a], distance_arr);
            if (cost_mut < cost_ori)
            {
                temp = routePool[a][mutationPoint1];
                routePool[a][mutationPoint1] = routePool[a][mutationPoint2];
                routePool[a][mutationPoint2] = temp;
            }
        }
    }
}

int *GAtrain(city *citylist, int len, int num, int epoch, float crossingRate, float mutatingRate)
{
    float **distance_arr = constructDisArr(citylist, len);

    int **SecPool = (int **)malloc(sizeof(int *) * num);
    for (int a = 0; a < num; a++)
        SecPool[a] = (int *)malloc(sizeof(int) * len);
    int **FirPool = (int **)malloc(sizeof(int *) * num);
    for (int a = 0; a < num; a++)
        FirPool[a] = (int *)malloc(sizeof(int) * len);
    int *bestroute = (int *)malloc(sizeof(int) * len);
    float bestfitness = 0;
    float _bestfitness=0;
    float avgfitness = 0;
    int bestIndex = 0;
for(int q=0;q<5;q++){
     get_randomRoutePool(num, len,FirPool);
    for (int b = 0; b < epoch; b++)
    {
        FirPool = select(distance_arr, FirPool, SecPool, len, num, &bestfitness, &avgfitness);
        cross(FirPool, distance_arr, len, num, crossingRate, bestfitness, avgfitness);
        mutation(FirPool, len, num, mutatingRate, distance_arr, bestfitness, avgfitness);
    }

    for (int a = 0; a < num; a++)
    {
        float cost = get_fittness(len, FirPool[a], distance_arr);
         if (cost > _bestfitness)
        {
            _bestfitness = cost;
            for (int r = 0; r < len; r++)
                bestroute[r] = FirPool[bestIndex][r];
        }
    }

}
    printf("%f \n",1/_bestfitness);
    free_Routearr(FirPool,num);
    free_Routearr(SecPool, num);
    free_Disarr(distance_arr, len);

    return bestroute;
}
int main()
{
    srand((unsigned)time(NULL));
    city c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12, c13, c14, c15, c16, c17, c18, c19, c20;
    c1.x = 60;
    c1.y = 200;
    c2.x = 180;
    c2.y = 200;
    c3.x = 80;
    c3.y = 180;
    c4.x = 140;
    c4.y = 180;
    c5.x = 20;
    c5.y = 160;
    c6.x = 100;
    c6.y = 160;
    c7.x = 200;
    c7.y = 160;
    c8.x = 140;
    c8.y = 140;
    c9.x = 40;
    c9.y = 120;
    c10.x = 100;
    c10.y = 120;
    c11.x = 180;
    c11.y = 100;
    c12.x = 60;
    c12.y = 80;
    c13.x = 120;
    c13.y = 80;
    c14.x = 180;
    c14.y = 60;
    c15.x = 20;
    c15.y = 40;
    c16.x = 100;
    c16.y = 40;
    c17.x = 200;
    c17.y = 40;
    c18.x = 20;
    c18.y = 20;
    c19.x = 60;
    c19.y = 20;
    c20.x = 160;
    c20.y = 20;

    city city_list[20] = {c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12, c13, c14, c15, c16, c17, c18, c19, c20};
    int *route = GAtrain(city_list, 20, 80, 30, 0.8, 0.8);
    for(int a=0;a<20;a++){
        printf("%d  ",route[a]);
    }


    getchar();
    return 0;
}
