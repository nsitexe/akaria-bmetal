#include <stdio.h>
#include <stdint.h>

#include <sys/time.h>
#include <openssl/evp.h>

/* Length of test block, number of test blocks. */
#define TEST_BLOCK_LEN 4096
#define TEST_BLOCK_COUNT 8000

static uint8_t key[32];
static uint8_t iv[16];
static uint8_t g_block_in[TEST_BLOCK_LEN];
static uint8_t g_block_out[TEST_BLOCK_LEN + 16];

int bench(EVP_CIPHER_CTX *ctx, const EVP_CIPHER *cipher)
{
	struct timeval end, start, elapse;
	uint8_t *block_in = g_block_in;
	uint8_t *block_out = g_block_out;
	int len;
	size_t i;
	long long bytes, mills;

	printf("AES-256-CBC: %d x %d-byte blocks\n",
		TEST_BLOCK_LEN, TEST_BLOCK_COUNT);
	fflush(stdout);

	for (i = 0; i < TEST_BLOCK_LEN; i++) {
		block_in[i] = (uint8_t)(i & 0xff);
	}

	gettimeofday(&start, NULL);

	EVP_EncryptInit(ctx, cipher, key, iv);
	for (i = 0; i < TEST_BLOCK_COUNT; i++) {
		EVP_EncryptUpdate(ctx, block_out, &len, block_in, TEST_BLOCK_LEN);
	}
	EVP_EncryptFinal(ctx, block_out, &len);

	gettimeofday(&end, NULL);
	timersub(&end, &start, &elapse);

	bytes = (long long)TEST_BLOCK_LEN * (long long)TEST_BLOCK_COUNT;
	mills = (long long)elapse.tv_sec * 1000 + elapse.tv_usec / 1000;

	printf("time: %d.%03d [s]\n",
		(int)(mills / 1000), (int)(mills % 1000));
	printf("speed: %d bytes/s\n",
		(int)(bytes * 1000 / mills));

	return 0;
}

int main(int argc, char *argv[], char *envp[])
{
	EVP_CIPHER_CTX *ctx;
	const EVP_CIPHER *cipher;

	printf("hello OpenSSL!\n");

	ctx = EVP_CIPHER_CTX_new();
	cipher = EVP_aes_256_cbc();

	bench(ctx, cipher);

	EVP_CIPHER_CTX_free(ctx);

	printf("good bye OpenSSL!\n");

	return 0;
}
