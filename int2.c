// int2.c - вычисление двойного интеграла с использованием процессов.

#include <stdio.h>
#include <unistd.h>
#include <math.h>
#include <time.h>

double f(double x, double y);
double integrate(double a, double b, double c, double d, int N, int M);

int main(int argc, char *argv[])
{
	if(argc == 1) {
		puts("Usage: int2 [a] [b] [c] [d] [N] [M] [processes number]");
		return 0;
	}

	double a; sscanf(argv[1], "%lf", &a);
	double b; sscanf(argv[2], "%lf", &b);
	double c; sscanf(argv[3], "%lf", &c);
	double d; sscanf(argv[4], "%lf", &d);
	int N = atoi(argv[5]);
	int M = atoi(argv[6]);
	unsigned procnum = atoi(argv[7]);     // количество процессов
	double result;
	time_t begtime, endtime, restime;			// время вычисления
	int channel[2];
	unsigned j, i;
	int flg_parent = 0;										// флаг родителя. 1 - родитель
	double piece;													// кусочек интегралла
	double buf;

	begtime = time(NULL);
	if(pipe(channel)) {
		printf("Failed to create channel.\n");
		return 1;
	}

	// запуск процессов
	for(j = 0; j < procnum - 1; j++)
		if(!fork())
			break;

	if(j == procnum - 1)
		flg_parent = 1;

	// всем детям поровну погрысть интеграл!
	N /= procnum;
	piece = (b - a)/procnum;
	a+= j * piece;
	b = a + piece;  
	result = integrate(a, b, c, d, N, M);

	printf("Piece result: %f\n", result);
	if(flg_parent)		// пора собирать урожай!
	{
		for(i = 0; i < j; i++)
		{
			read(channel[0], &buf, sizeof(double));
			result+= buf;
		}

		endtime = time(NULL);
		restime = endtime - begtime;
		printf("Result = %f\nCalculating time: %d sec\n", result, restime);
	}
	else
		write(channel[1], &result, sizeof(double));

	return 0;
}

double f(double x, double y)
{
	return sin(x);
}

double integrate(double a, double b, double c, double d, int N, int M)
{
	int i, j;
	double hx, hy;
	double x, x1, y1, f_x_y, f_xh_y, f_x_yh, f_xh_yh;
	double res = 0;

	hx = (b - a)/N;
	hy = (d - c)/M;

	for(i = 0; i < N; i++)
	{		
		x = a + i * hx;
		x1 = a + (i + 1) * hx;
		f_x_y = f(x, c);
		f_xh_y = f(x, c);

		for(j = 0; j < M; j++)
		{		
			y1 = c + (j + 1) * hy;
			f_x_yh = f(x, y1);
			f_xh_yh = f(x1, y1);
			res+= (1.0/4) * hx * hy * (f_x_y + f_xh_y + f_x_yh + f_xh_yh);
			f_x_y = f_x_yh;
			f_xh_y = f_xh_yh;
		}
	}

	return res;
}
