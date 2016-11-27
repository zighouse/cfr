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

    -m, --maxden=integer    maximum denominator

        --maxnum=integer    maximum numerator

    -n, --number=integer    maximum number of contined fraction coeffs

    -w, --welformed         welformed style (default)

    -p, --plain             plain style

    -r, --report            report result

    -c, --cont              display continued fraction

    -g, --gcd               display gcd of two integers

    -s, --simple            display approximate simple fraction
                            print the best rational for float in round convention

    -l, --list              list iteration process

    -h, --help              display this help

    --version               output version information

    --                      following minus number not parsed as option(s)

## EXAMPLE

1.

    $ ./cfr -m 200 3.14159

    Status:
        Calculation is not completed.
        GCD is not available.
        Simple fraction = 355 / 113
    
    Continued fraction:
                  1
    3 + ---------------------
                     1
         7 + ----------------
                        1
              15 + ----------
                          1
                    1 + -----
                         ...
    
    Calculating iterations: 4
      3 / 1     3  (1/8 < err < 1/7)
     22 / 7     7  (1/791 < err < 1/742)
    333 / 106  15  (1/23214 < err < 1/11978)
    355 / 113   1  (1/343972 < err < 1/331203)

result shows the best fraction with den<=200 approx 3.14159 is 355/113,
and the continued fraction is:

    3 + 1/(7 + 1/(15 + 1/(1 + ...))).

2.

    $ ./cfr -w 1920 1080

    Status:
        Calculation is completed.
        GCD = 120
        Simple fraction = 16 / 9

    Continued fraction:
              1
    1 + -------------
                 1
         1 + --------
                   1
              3 + ---
                   2

    Calculating iterations: 4
     1 / 1  1  (1/2 < err < 1/1)
     2 / 1  1  (1/5 < err < 1/4)
     7 / 4  3  (1/52 < err < 1/36)
    16 / 9  2  (gcd = 120)

result shows: 1920/1080 = 16/9, and the gcd of 1920 and 1080 is 120, and
the continued fraction is:

    1 + 1/(1 + 1/(3 + 1/2)).


## LIBRARY

The library libcf.a implements more functions related to continued fractions.


## AUTHOR

Written by Xie Zhigang.

The algorithm of arithmethics of continued fractions is invented by Gosper.
