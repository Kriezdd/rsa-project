#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdint.h>

// -----------------------------------------------------------
// 1) Генерация случайного 64-битного блока
// -----------------------------------------------------------
uint64_t rand64(void) {
    uint64_t r = 0;
    for (int i = 0; i < 4; i++) {
        r = (r << 15) ^ (rand() & 0x7FFF);
    }
    return r;
}

// -----------------------------------------------------------
// 2) Рабин–Миллер (упрощённо)
// -----------------------------------------------------------
static uint64_t mulmod64_naive(uint64_t a, uint64_t b, uint64_t m) {
    // Медленная, но понятная реализация умножения по модулю
    uint64_t res = 0;
    a = a % m;
    while (b > 0) {
        if (b & 1ULL)
            res = (res + a) % m;
        a = (2ULL * a) % m;
        b >>= 1ULL;
    }
    return res;
}

static uint64_t powmod64(uint64_t base, uint64_t exp, uint64_t mod) {
    uint64_t result = 1ULL;
    base = base % mod;
    while (exp > 0) {
        if (exp & 1ULL) {
            result = mulmod64_naive(result, base, mod);
        }
        base = mulmod64_naive(base, base, mod);
        exp >>= 1ULL;
    }
    return result;
}

int rabin_miller_64(uint64_t n, int k) {
    if (n < 2) return 0;
    if (n == 2 || n == 3) return 1;
    if ((n & 1ULL) == 0) return 0;

    // n-1 = d * 2^r
    uint64_t d = n - 1;
    int r = 0;
    while ((d & 1ULL) == 0) {
        d >>= 1ULL;
        r++;
    }

    for (int i = 0; i < k; i++) {
        uint64_t a = 2ULL + (rand64() % (n - 4ULL));
        uint64_t x = powmod64(a, d, n);
        if (x == 1ULL || x == (n - 1ULL)) continue;
        int flag = 1;
        for (int j = 0; j < r - 1; j++) {
            x = (uint64_t)mulmod64_naive(x, x, n);
            if (x == (n - 1ULL)) {
                flag = 0;
                break;
            }
        }
        if (flag) return 0;
    }
    return 1;
}

// -----------------------------------------------------------
// 3) Генерация простого числа нужного размера (в байтах/битах)
// -----------------------------------------------------------
static void fill_random_bytes(unsigned char *buf, int len) {
    for (int i = 0; i < len; i++) {
        buf[i] = (unsigned char)(rand() & 0xFF);
    }
}

// конвертируем массив байт (big-endian) в 64-бит unsigned (только часть)
static uint64_t to_uint64(const unsigned char *buf, int len) {
    uint64_t val = 0ULL;
    for (int i = 0; i < len; i++) {
        val = (val << 8) | buf[i];
    }
    return val;
}

// Эта функция ищет "64-битное простое число" (для демо).
// Но пусть мы хотим что-то больше 32к (16 бит) и т.д.
// В реальности - нужно полноценное big-integer
uint64_t generate_prime_64(void) {
    while (1) {
        // Генерируем 2-байтовый (или 3-байтовый, 8-байтовый) кандидат
        // Ниже - 4-байтовый для ~32-битного диапазона.
        unsigned char buf[4];
        fill_random_bytes(buf, 4);
        // Делать нечётным
        buf[3] |= 0x01;
        // Не даём слишком маленькие значения
        if (buf[0] == 0) buf[0] = 1;

        uint64_t candidate = to_uint64(buf, 4);

        // Тестируем Рабин–Миллером
        if (rabin_miller_64(candidate, 10)) {
            return candidate;
        }
    }
}

// -----------------------------------------------------------
// 4) extended_gcd / modInverse
// -----------------------------------------------------------
uint64_t gcd64(uint64_t a, uint64_t b) {
    while (b != 0) {
        uint64_t t = b;
        b = a % b;
        a = t;
    }
    return a;
}

int64_t extended_gcd(int64_t a, int64_t b, int64_t *x, int64_t *y) {
    if (b == 0) {
        *x = 1; *y = 0;
        return a;
    }
    int64_t x1, y1;
    int64_t g = extended_gcd(b, a % b, &x1, &y1);
    *x = y1;
    *y = x1 - (a / b) * y1;
    return g;
}

uint64_t mod_inverse(uint64_t e, uint64_t phi) {
    int64_t x, y;
    int64_t g = extended_gcd((int64_t)e, (int64_t)phi, &x, &y);
    if (g != 1) {
        return 0ULL;
    }
    int64_t res = x % (int64_t)phi;
    if (res < 0) res += phi;
    return (uint64_t)res;
}

// -----------------------------------------------------------
// 5) Генерация пары ключей (для демонстрации - 2 x ~32 бит)
// -----------------------------------------------------------
int main(void) {
    srand((unsigned)time(NULL));

    // Генерируем два "примерно 32-битных" простых p, q.
    // (В реальном RSA - нужно хотя бы 512, 1024 бит и т.д.)
    uint64_t p = generate_prime_64();
    uint64_t q;
    do {
        q = generate_prime_64();
    } while (q == p);

    uint64_t n = p * q;
    uint64_t phi = (p - 1ULL) * (q - 1ULL);

    // e = 65537
    uint64_t e = 65537ULL;

    // Проверка gcd(e, phi) == 1
    if (gcd64(e, phi) != 1ULL) {
        fprintf(stderr, "Ошибка: gcd(e, phi) != 1\n");
        return 1;
    }

    // d
    uint64_t d = mod_inverse(e, phi);
    if (d == 0ULL) {
        fprintf(stderr, "Ошибка: не можем найти d\n");
        return 1;
    }

    printf("p=%llu\n", (unsigned long long)p);
    printf("q=%llu\n", (unsigned long long)q);
    printf("n=%llu\n", (unsigned long long)n);
    printf("phi=%llu\n", (unsigned long long)phi);
    printf("e=%llu\n", (unsigned long long)e);
    printf("d=%llu\n", (unsigned long long)d);

    return 0;
}
