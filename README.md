# Number-Ten

Any digitized file is, at bottom, just a sequence of digits. Prefix that
sequence with a decimal point and you get a terminating decimal — which is
always a rational number, expressible exactly as one integer divided by
another.

`digit_fraction.py` reads a stream of decimal digit characters from stdin
and prints the numerator and denominator of that number in lowest terms.

This branch is the Python implementation, evaluated alongside a C
implementation (see the `c-version` branch) as a performance candidate
before settling on a final version.

## Requirements

Python 3. No third-party dependencies — Python's built-in integers are
already arbitrary-precision.

## Usage

```sh
python3 digit_fraction.py < input.txt
python3 digit_fraction.py -v < input.txt   # verbose: print stats before the fraction
```

The input must consist solely of the ASCII digits `0`-`9`; a single
trailing newline (`\n` or `\r\n`), as is typical when piping in a text
file, is ignored. Any other non-digit character is a hard error.

### Example

```sh
$ printf '333' | python3 digit_fraction.py
333 / 1000

$ printf '333' | python3 digit_fraction.py -v
digits in output: 7
output digits as % of input digits: 233.33%
time spent computing: 00:00:00
333 / 1000
```

With `-v`, three statistics are printed before the numerator/denominator
line:

- total digit count of the numerator and denominator combined
- that count as a percentage of the number of input digits
- wall-clock time spent computing the result, as `hh:mm:ss`
