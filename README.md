# cfr

## NAME

cfr - calculate continued fraction, approximate simple fractions, gcd.

## SYNOPSIS

    cfr [OPTIONS] NUMERATOR [[/] DENOMINATOR]

## DESCRIPTION

Process an iteration of continued fraction calculation. Print
approximate simple fractions, continued fractioin coeffecients, errors
and or gcd.

As numerator and denominator of a fraction, integers and floats are all
aceptable. Negative numbers does not affect continued fraction
coeffecients. Gcd for float is unsupported.

Continued fraction can be obtained after an iteration of representing a
number as the sum of its integer part and the reciprocal of another
number, then writing this other number as the sum of its integer part
and another reciprocal, and so on (see wikipedia).

Every rational number can be expressed as a finite continued fraction,
while irrational numbers can only be expressed as infinite ones.  The
process of iteration to obtain a continued fraction is related to the
well known Euclidean algorithm, and for rational number it gives the
Greatest Common Divisor.

Example of continued fraction of a rational number:

      1920              1          16 x 120
     ------ = 1 + ------------- = ---------- (gcd = 120)
      1080                1         9 x 120
              :    1 + --------
              :              1
              :    :    3 + ---
              :    :         2   (finished)
              :    :    :     
              :    :    :    :
     approx 
       as     1    2    7   16 
     simple  ---  ---  --- ----
    fraction  1    1    4    9

Example of continued fraction of an irrational number:

                            1
        pi  = 3 + ---------------------
                               1
              :    7 + ----------------
              :                   1
              :    :    15 + ----------
              :    :                1
              :    :    :     1 + -----
              :    :    :          ... (infinite) 
              :    :    :     :       
     approx 
       as     3   22   333   355
     simple  --- ---   ---   ---  ...
    fraction  1   7    106   113

## OPTIONS

    num    - is real number , or numerator of a fraction

    den    - [optional] is the denominator, integer or another real

    -m integer|-minteger|--maxden=integer
                      specify the maximum denominator

    --maxnum=integer  specify the maximum numerator

    -w|--welformed    print in welformed style

    -p|--plain        print in plain style

    -v|--verbose      show detailed iteration process (default)

    -c|--cont         show the continued fraction

    -g|--gcd          show the gcd of two integer numbers

    -s|--simple       show the approximation of simple fraction

    -h|--help         show help

    --version         show version


## EXAMPLE

    $ ./cfr -m 200 -w 3.14159

       3 / 1     3  err = 1.415900e-01
      22 / 7     7  err = -1.267143e-03
     333 / 106  15  err = 8.056604e-05
     355 / 113   1  err = -2.920354e-06

result shows the best fraction with den<=200 approx 3.14159 is 355/113,
and the continued fraction is: 3 + 1/(7 + 1/(15 + 1/(1 + ...))).

    $ ./cfr -w 1920 1080

      1 / 1  1  err = 7.777778e-01
      2 / 1  1  err = -2.222222e-01
      7 / 4  3  err = 2.777778e-02
     16 / 9  2  gcd = 120

result shows: 1920/1080 = 16/9, and the gcd of 1920 and 1080 is 120.

## AUTHOR

Written by Xie Zhigang based on work of Arno Formella (May 2008)
and David Eppstein (8 Aug 1993).
http://www.ics.uci.edu/~eppstein/numth/frap.c
