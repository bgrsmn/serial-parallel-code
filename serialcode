#include <stdio.h>
#include <stdlib.h>
#include <ctime>
#include <iostream>
#include <omp.h>

using namespace std;

#define N 1000

float* matris_x[N];
float* matris_y[N];
float* matris_carpim[N];


static void duzenleme(void)
{

	for (long i = 0; i < N; i++) {
		matris_x[i] = (float*)malloc(N * sizeof(float));
		matris_y[i] = (float*)malloc(N * sizeof(float));
		matris_carpim[i] = (float*)malloc(N * sizeof(float));
	}

	for (long i = 0; i < N; i++) {
		for (long j = 0; j < N; j++) {
			matris_x[i][j] = matris_y[i][j] = 1.0f;
		}
	}
}


static void carpim(void)
{

	for (long i = 0; i < N; i++) {
		for (long j = 0; j < N; j++) {
			for (long t = 0; t < N; t++) {
				matris_carpim[i][j] += matris_x[i][t] * matris_y[t][j];
			}
		}
	}

}


static void yazdirma(void)
{
	for (int i = 0; i < N; i++){
		for (int j = 0; j < N; j++) {
			cout << matris_carpim[i][j] << " ";
		}
		cout << endl;
	}
}

static void silme(void)
{
	for (long i = 0; i < N; i++) {
		free(matris_x[i]);
		free(matris_y[i]);
		free(matris_carpim[i]);
	}
}

int main(void)
{

	double st = omp_get_wtime();

	duzenleme();
	carpim();
	//yazdırma();
	silme();

	double en = omp_get_wtime();
	cout<<"Seri Zaman:"<<en - st;


	exit(EXIT_SUCCESS);
}
