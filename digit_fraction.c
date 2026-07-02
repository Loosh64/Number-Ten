#define _POSIX_C_SOURCE 200809L

/*
 * digit_fraction.c
 *
 * Reads a stream of decimal digit characters from stdin and treats it as
 * the fractional digits of a number 0.d1 d2 d3 ... dn.  Since that value
 * is a terminating decimal, it is rational: it equals D / 10^n, where D
 * is the integer formed by the digit stream.  This program reduces that
 * fraction to lowest terms using arbitrary-precision arithmetic (GMP)
 * and prints the resulting numerator and denominator.
 *
 * Usage: digit_fraction [-v] < input
 */

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <gmp.h>

static const char *g_progname = "digit_fraction";

static void die(const char *fmt, ...) {
    va_list ap;
    fprintf(stderr, "%s: error: ", g_progname);
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);
    fprintf(stderr, "\n");
    exit(EXIT_FAILURE);
}

static void usage_and_exit(FILE *out, int code) {
    fprintf(out, "Usage: %s [-v] < input\n", g_progname);
    fprintf(out,
            "Reads a stream of decimal digits from stdin and prints the\n"
            "numerator and denominator of the equivalent reduced fraction.\n"
            "  -v   verbose: also print digit-count and timing statistics\n");
    exit(code);
}

/* Reads all of stdin into a NUL-terminated, malloc'd buffer.
 * *out_len is set to the number of bytes read (excluding the NUL). */
static char *read_all_stdin(size_t *out_len) {
    size_t cap = 1 << 16;
    size_t len = 0;
    char *buf = malloc(cap);
    if (!buf) die("out of memory");

    for (;;) {
        if (len == cap) {
            cap *= 2;
            char *nb = realloc(buf, cap);
            if (!nb) die("out of memory");
            buf = nb;
        }
        size_t n = fread(buf + len, 1, cap - len, stdin);
        len += n;
        if (n == 0) break;
    }
    if (ferror(stdin)) die("failed reading standard input");

    char *nb = realloc(buf, len + 1);
    if (nb) buf = nb; /* shrink is best-effort */
    buf[len] = '\0';

    *out_len = len;
    return buf;
}

/* Number of decimal digits needed to print a non-negative mpz_t exactly. */
static size_t decimal_digit_count(const mpz_t v) {
    char *s = mpz_get_str(NULL, 10, v);
    size_t n = strlen(s);
    free(s);
    return n;
}

static void format_hhmmss(double seconds, char *out, size_t out_sz) {
    if (seconds < 0) seconds = 0;
    unsigned long total = (unsigned long)(seconds + 0.5); /* round to nearest second */
    unsigned long hh = total / 3600;
    unsigned long mm = (total % 3600) / 60;
    unsigned long ss = total % 60;
    snprintf(out, out_sz, "%02lu:%02lu:%02lu", hh, mm, ss);
}

int main(int argc, char **argv) {
    if (argc > 0 && argv[0] && argv[0][0]) g_progname = argv[0];

    int verbose = 0;
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-v") == 0) {
            verbose = 1;
        } else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            usage_and_exit(stdout, EXIT_SUCCESS);
        } else {
            fprintf(stderr, "%s: error: unrecognized argument '%s'\n", g_progname, argv[i]);
            usage_and_exit(stderr, EXIT_FAILURE);
        }
    }

    struct timespec t_start, t_end;
    clock_gettime(CLOCK_MONOTONIC, &t_start);

    size_t len;
    char *buf = read_all_stdin(&len);

    /* Ignore a single trailing newline (or CRLF), which is common when
     * piping in the contents of a text file. */
    if (len >= 2 && buf[len - 2] == '\r' && buf[len - 1] == '\n') {
        len -= 2;
    } else if (len >= 1 && buf[len - 1] == '\n') {
        len -= 1;
    }
    buf[len] = '\0';

    if (len == 0) {
        die("no digit characters found on standard input");
    }

    for (size_t i = 0; i < len; i++) {
        unsigned char c = (unsigned char)buf[i];
        if (c < '0' || c > '9') {
            if (c >= 0x20 && c < 0x7f) {
                die("invalid character '%c' at input position %zu: "
                    "input must contain only decimal digits (0-9)",
                    c, i + 1);
            } else {
                die("invalid byte 0x%02x at input position %zu: "
                    "input must contain only decimal digits (0-9)",
                    c, i + 1);
            }
        }
    }

    size_t n_input_digits = len;

    mpz_t D, denom, g, numerator, denominator;
    mpz_inits(D, denom, g, numerator, denominator, NULL);

    if (mpz_set_str(D, buf, 10) != 0) {
        die("internal error: failed to parse validated digit stream");
    }
    free(buf);

    mpz_ui_pow_ui(denom, 10, n_input_digits);

    if (mpz_sgn(D) == 0) {
        mpz_set_ui(numerator, 0);
        mpz_set_ui(denominator, 1);
    } else {
        mpz_gcd(g, D, denom);
        mpz_divexact(numerator, D, g);
        mpz_divexact(denominator, denom, g);
    }

    clock_gettime(CLOCK_MONOTONIC, &t_end);
    double elapsed = (t_end.tv_sec - t_start.tv_sec) +
                      (t_end.tv_nsec - t_start.tv_nsec) / 1e9;

    if (verbose) {
        size_t out_digits = decimal_digit_count(numerator) + decimal_digit_count(denominator);
        double pct = 100.0 * (double)out_digits / (double)n_input_digits;
        char tbuf[32];
        format_hhmmss(elapsed, tbuf, sizeof(tbuf));

        printf("digits in output: %zu\n", out_digits);
        printf("output digits as %% of input digits: %.2f%%\n", pct);
        printf("time spent computing: %s\n", tbuf);
    }

    gmp_printf("%Zd / %Zd\n", numerator, denominator);

    mpz_clears(D, denom, g, numerator, denominator, NULL);
    return EXIT_SUCCESS;
}
