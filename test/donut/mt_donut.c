#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <pthread.h>
#include <unistd.h>
#include <sys/time.h>

#define THREADS    4

#define R(mul, shift, x, y) \
	do { \
		int _; \
		_ = x; \
		x -= (mul * y) >> shift; \
		y += (mul * _) >> shift; \
		_ = (3145728 - x * x - y * y) >> 11; \
		x = (x * _) >> 10; \
		y = (y * _) >> 10; \
	} while (0)

// text buffer
int8_t bb[THREADS][1760];
// z buffer
int8_t zz[THREADS][1760];

pthread_barrier_t barrier;

void *thread_main(void *arg)
{
	int n = (intptr_t)arg;
	int sA = 1024, cA = 0, sB = 1024, cB = 0;
	struct timeval tv_s, tv_e, tv_v, tv_d;
	int8_t *b = bb[n];
	int8_t *z = zz[n];

	for (;;) {
		int sj = 0, cj = 1024;
		int st, ed;

		gettimeofday(&tv_s, NULL);

		memset(b, 32, 1760);
		memset(z, 127, 1760);

		if (THREADS == 1 && n == 0) {
			st = 0;
			ed = 90;
		} else if (n == 0) {
			st = 0;
			ed = 10;
		} else if (n == THREADS - 1) {
			st = 10 + 80 / (THREADS - 1) * (n - 1);
			ed = 90;
		} else {
			st = 10 + 80 / (THREADS - 1) * (n - 1);
			ed = 10 + 80 / (THREADS - 1) * n;
		}

		for (int j = 0; j < st; j++) {
			R(9, 7, cj, sj);
		}

		for (int j = st; j < ed; j++) {
			// sine and cosine of angle i
			int si = 0, ci = 1024;

			for (int i = 0; i < 324; i++) {
				int R1 = 1, R2 = 2048, K2 = 5120 * 1024;

				int x0 = R1 * cj + R2;
				int x1 = (ci * x0) >> 10;
				int x2 = (cA * sj) >> 10;
				int x3 = (si * x0) >> 10;
				int x4 = R1 * x2 - ((sA * x3) >> 10);
				int x5 = (sA * sj) >> 10;
				int x6 = K2 + R1 * 1024 * x5 + cA * x3;
				int x7 = (cj * si) >> 10;
				int x = 40 + 30 * (cB * x1 - sB * x4) / x6;
				int y = 12 + 15 * (cB * x4 + sB * x1) / x6;
				int N = (((-cA * x7 - cB * ((-sA * x7 >> 10) + x2) - ci * (cj * sB >> 10)) >> 10) - x5) >> 7;

				int o = x + 80 * y;
				int8_t zz = (x6 - K2) >> 15;
				if (22 > y && y > 0 && x > 0 && 80 > x && zz < z[o]) {
					z[o] = zz;
					b[o] = ".,-~:;=!*#$@"[N > 0 ? N : 0];
				}
				// rotate i
				R(5, 8, ci, si);
			}
			// rotate j
			R(9, 7, cj, sj);
		}

		R(5, 7, cA, sA);
		R(5, 8, cB, sB);

		pthread_barrier_wait(&barrier);

		if (THREADS == 1 && n == 0) {
			st = 0;
			ed = 1760;
		} else if (n == THREADS - 1) {
			st = 1760 / THREADS * n;
			ed = 1760;
		} else {
			st = 1760 / THREADS * n;
			ed = 1760 / THREADS * (n + 1);
		}

		for (int i = st; i < ed; i++) {
			int z_min = zz[0][i];
			for (int j = 0; j < THREADS; j++) {
				if (zz[j][i] < z_min) {
					z_min = zz[j][i];
					zz[0][i] = z_min;
					bb[0][i] = bb[j][i];
				}
			}
		}

		pthread_barrier_wait(&barrier);

		gettimeofday(&tv_e, NULL);
		timersub(&tv_e, &tv_s, &tv_v);

		if (n == 0) {
			for (int k = 0; k < 1760; k++) {
				putchar(k % 80 ? b[k] : 10);
			}
			putchar('\n');

			gettimeofday(&tv_s, NULL);
			timersub(&tv_s, &tv_e, &tv_d);

			uint64_t tt = ((tv_v.tv_sec + tv_d.tv_sec) * 1000000
				+ tv_v.tv_usec + tv_d.tv_usec) / 1000;
			uint64_t fps = 0;
			if (tt != 0) {
				fps = 1000000 / tt;
			}

			printf("calc:%d.%06ds, draw:%d.%06ds, fps:%d.%03d\n",
				(int)tv_v.tv_sec, (int)tv_v.tv_usec,
				(int)tv_d.tv_sec, (int)tv_d.tv_usec,
				(int)(fps / 1000), (int)(fps % 1000));
			printf("\x1b[24A");
		}
	}

	return NULL;
}

int main(int argc, char *argv[])
{
	pthread_t th[THREADS];
	void *res[THREADS];
	int r;

	// hide cursor
	printf("\x1b[?25l");

	pthread_barrier_init(&barrier, NULL, 4);

	for (int i = 1; i < THREADS; i++) {
		r = pthread_create(&th[i], NULL, thread_main, (void *)(intptr_t)i);
		if (r != 0) {
			perror("pthread_create");
			return 1;
		}
	}

	thread_main((void *)(intptr_t)0);

	for (int i = 1; i < THREADS; i++) {
		r = pthread_join(th[i], &res[i]);
		if (r != 0) {
			perror("pthread_create");
			return 1;
		}
	}

	return 0;
}
