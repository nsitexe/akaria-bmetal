#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <pthread.h>
#include <unistd.h>
#include <sys/time.h>

#define MAX_THREADS    4
#define REPEAT         1
#define DEMO_SEC       6

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
int8_t bb[MAX_THREADS][1760];
// z buffer
int8_t zz[MAX_THREADS][1760];

struct thread_info {
	int threads;
	int id;
};

struct thread_info th_info[MAX_THREADS];
pthread_t th[MAX_THREADS];
void *th_res[MAX_THREADS];

pthread_barrier_t barrier;
struct timeval demo_init, demo_start;
int term;

void *thread_main(void *arg)
{
	const struct thread_info *thi = arg;
	int sA = 1024, cA = 0, sB = 1024, cB = 0;
	struct timeval tv_s, tv_e, tv_calc, tv_draw, tv_demo, tv_wall;
	int threads = thi->threads;
	int n = thi->id;
	int8_t *b = bb[n];
	int8_t *z = zz[n];


	for (;;) {
		int st, ed;

		gettimeofday(&tv_s, NULL);

		for (int rep = 0; rep < REPEAT; rep++) {
			int sj = 0, cj = 1024;

			memset(b, 32, 1760);
			memset(z, 127, 1760);

			if (threads == 1 && n == 0) {
				st = 0;
				ed = 90;
			} else if (n == threads - 1) {
				st = 90 / threads * n;
				ed = 90;
			} else {
				st = 90 / threads * n;
				ed = 90 / threads * (n + 1);
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
		}

		R(5, 7, cA, sA);
		R(5, 8, cB, sB);

		pthread_barrier_wait(&barrier);

		if (term) {
			break;
		}

		if (threads == 1 && n == 0) {
			st = 0;
			ed = 1760;
		} else if (n == threads - 1) {
			st = 1760 / threads * n;
			ed = 1760;
		} else {
			st = 1760 / threads * n;
			ed = 1760 / threads * (n + 1);
		}

		for (int i = st; i < ed; i++) {
			int z_min = zz[0][i];
			for (int j = 0; j < threads; j++) {
				if (zz[j][i] < z_min) {
					z_min = zz[j][i];
					zz[0][i] = z_min;
					bb[0][i] = bb[j][i];
				}
			}
		}

		pthread_barrier_wait(&barrier);

		gettimeofday(&tv_e, NULL);
		timersub(&tv_e, &tv_s, &tv_calc);
		tv_s = tv_e;

		if (n == 0) {
			for (int k = 0; k < 1760; k++) {
				putchar(k % 80 ? b[k] : 10);
			}
			putchar('\n');
			fflush(stdout);

			gettimeofday(&tv_e, NULL);
			timersub(&tv_e, &tv_s, &tv_draw);

			uint64_t tt = ((tv_calc.tv_sec + tv_draw.tv_sec) * 1000000
				+ tv_calc.tv_usec + tv_draw.tv_usec);
			uint64_t fps = 0;
			if (tt != 0) {
				fps = 1000000000 / tt;
			}

			timersub(&tv_e, &demo_init, &tv_wall);
			printf("%02d:%02d:%02d) thread:%d calc:%d.%06ds draw:%d.%06ds "
				"fps:%d.%03d w:%d  \n",
				(int)(tv_wall.tv_sec / 3600),
				(int)(tv_wall.tv_sec / 60) % 60,
				(int)(tv_wall.tv_sec % 60),
				threads,
				(int)tv_calc.tv_sec, (int)tv_calc.tv_usec,
				(int)tv_draw.tv_sec, (int)tv_draw.tv_usec,
				(int)(fps / 1000), (int)(fps % 1000),
				REPEAT);
			printf("\x1b[24A");

			timersub(&tv_e, &demo_start, &tv_demo);
			if (tv_demo.tv_sec >= DEMO_SEC) {
				term = 1;
			}
		}

		pthread_barrier_wait(&barrier);
	}

	return NULL;
}

int demo_main(int threads)
{
	int r;

	if (threads > MAX_THREADS) {
		fprintf(stderr, "exceed number of threads:%d (max:%d).\n", threads, MAX_THREADS);
		return -1;
	}

	gettimeofday(&demo_start, NULL);
	term = 0;

	pthread_barrier_init(&barrier, NULL, threads);

	for (int i = 1; i < threads; i++) {
		th_info[i].threads = threads;
		th_info[i].id = i;
		r = pthread_create(&th[i], NULL, thread_main, &th_info[i]);
		if (r != 0) {
			perror("pthread_create");
			return 1;
		}
	}

	th_info[0].threads = threads;
	th_info[0].id = 0;
	thread_main(&th_info[0]);

	for (int i = 1; i < threads; i++) {
		r = pthread_join(th[i], &th_res[i]);
		if (r != 0) {
			perror("pthread_create");
			return 1;
		}
	}

	return 0;
}

int main(int argc, char *argv[])
{
	int scenario[] = {
		1, 2, 3, 4, 1, 4,
	};
	int len, r;

	// hide cursor
	printf("\x1b[?25l");

	gettimeofday(&demo_init, NULL);

	len = sizeof(scenario) / sizeof(scenario[0]);
	for (int i = 0; i < 1000000; i++) {
		int j = i % len;

		r = demo_main(scenario[j]);
		if (r != 0) {
			return r;
		}
	}

	return 0;
}
