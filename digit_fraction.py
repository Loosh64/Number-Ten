#!/usr/bin/env python3
"""Reads a stream of decimal digit characters from stdin and prints the
numerator and denominator of the equivalent reduced fraction.

Usage: digit_fraction.py [-v] < input
"""

import sys
import time
from math import gcd

PROGNAME = "digit_fraction.py"


def die(message):
    sys.stderr.write(f"{PROGNAME}: error: {message}\n")
    sys.exit(1)


def usage(out, code):
    out.write(f"Usage: {PROGNAME} [-v] < input\n")
    out.write(
        "Reads a stream of decimal digits from stdin and prints the\n"
        "numerator and denominator of the equivalent reduced fraction.\n"
        "  -v   verbose: also print digit-count and timing statistics\n"
    )
    sys.exit(code)


def read_all_stdin():
    data = sys.stdin.buffer.read()
    return data.decode("latin-1")


def format_hhmmss(seconds):
    if seconds < 0:
        seconds = 0
    total = int(seconds + 0.5)  # round to nearest second
    hh, rem = divmod(total, 3600)
    mm, ss = divmod(rem, 60)
    return f"{hh:02d}:{mm:02d}:{ss:02d}"


def main(argv):
    global PROGNAME
    if argv and argv[0]:
        PROGNAME = argv[0]

    # Digit streams of arbitrary size are the whole point of this program;
    # lift Python's default int<->str conversion digit-count guard (PEP 683 /
    # bpo-95778), which otherwise raises on inputs over 4300 digits.
    if hasattr(sys, "set_int_max_str_digits"):
        sys.set_int_max_str_digits(0)

    verbose = False
    for arg in argv[1:]:
        if arg == "-v":
            verbose = True
        elif arg in ("-h", "--help"):
            usage(sys.stdout, 0)
        else:
            sys.stderr.write(f"{PROGNAME}: error: unrecognized argument '{arg}'\n")
            usage(sys.stderr, 1)

    t_start = time.monotonic()

    buf = read_all_stdin()

    # Ignore a single trailing newline (or CRLF), which is common when
    # piping in the contents of a text file.
    if buf.endswith("\r\n"):
        buf = buf[:-2]
    elif buf.endswith("\n"):
        buf = buf[:-1]

    if len(buf) == 0:
        die("no digit characters found on standard input")

    for i, c in enumerate(buf):
        if not ("0" <= c <= "9"):
            code = ord(c)
            if 0x20 <= code < 0x7F:
                die(
                    f"invalid character '{c}' at input position {i + 1}: "
                    "input must contain only decimal digits (0-9)"
                )
            else:
                die(
                    f"invalid byte 0x{code:02x} at input position {i + 1}: "
                    "input must contain only decimal digits (0-9)"
                )

    n_input_digits = len(buf)

    d = int(buf)
    denom = 10 ** n_input_digits

    if d == 0:
        numerator, denominator = 0, 1
    else:
        g = gcd(d, denom)
        numerator, denominator = d // g, denom // g

    t_end = time.monotonic()
    elapsed = t_end - t_start

    if verbose:
        out_digits = len(str(numerator)) + len(str(denominator))
        pct = 100.0 * out_digits / n_input_digits
        print(f"digits in output: {out_digits}")
        print(f"output digits as % of input digits: {pct:.2f}%")
        print(f"time spent computing: {format_hhmmss(elapsed)}")

    print(f"{numerator} / {denominator}")


if __name__ == "__main__":
    main(sys.argv)
