import datetime

import pytest

import cron
from   cron import *

import data

#-------------------------------------------------------------------------------

def test_date_add():
    d = 1973/Dec/3
    assert(d + -720595).invalid
    assert d + -720594 ==    1/Jan/ 1
    assert d + -100000 == 1700/Feb/17
    assert d +  -10000 == 1946/Jul/18
    assert d +   -1000 == 1971/Mar/ 9
    assert d +    -100 == 1973/Aug/25
    assert d +     -10 == 1973/Nov/23
    assert d +      -1 == 1973/Dec/ 2
    assert d +       0 == 1973/Dec/ 3
    assert d +       1 == 1973/Dec/ 4
    assert d +       2 == 1973/Dec/ 5
    assert d +       7 == 1973/Dec/10
    assert d +      10 == 1973/Dec/13
    assert d +      28 == 1973/Dec/31
    assert d +      29 == 1974/Jan/ 1
    assert d +     100 == 1974/Mar/13
    assert d +    1000 == 1976/Aug/29
    assert d +   10000 == 2001/Apr/20
    assert d +  100000 == 2247/Sep/18
    assert d + 1000000 == 4711/Oct/31
    assert d + 2931464 == 9999/Dec/31
    assert(d + 2931465).invalid

    assert(-720595 + d).invalid
    assert -720594 + d ==    1/Jan/ 1
    assert -100000 + d == 1700/Feb/17
    assert      -1 + d == 1973/Dec/ 2
    assert       0 + d == 1973/Dec/ 3
    assert       1 + d == 1973/Dec/ 4
    assert 1000000 + d == 4711/Oct/31
    assert 2931464 + d == 9999/Dec/31
    assert(2931465 + d).invalid

    assert (d + 100) + -100 == d
    assert -100 + (d + 100) == d
    assert 100 + (-100 + d) == d


def test_date_sub():
    d = 1973/Dec/3
    assert(d -  720595).invalid
    assert d -  720594 ==    1/Jan/ 1
    assert d -  100000 == 1700/Feb/17
    assert d -   10000 == 1946/Jul/18
    assert d -    1000 == 1971/Mar/ 9
    assert d -     100 == 1973/Aug/25
    assert d -      10 == 1973/Nov/23
    assert d -       1 == 1973/Dec/ 2
    assert d -       0 == 1973/Dec/ 3
    assert d -      -1 == 1973/Dec/ 4
    assert d -      -2 == 1973/Dec/ 5
    assert d -      -7 == 1973/Dec/10
    assert d -     -10 == 1973/Dec/13
    assert d -     -28 == 1973/Dec/31
    assert d -     -29 == 1974/Jan/ 1
    assert d -    -100 == 1974/Mar/13
    assert d -   -1000 == 1976/Aug/29
    assert d -  -10000 == 2001/Apr/20
    assert d - -100000 == 2247/Sep/18
    assert d - -2931464 == 9999/Dec/31
    assert(d - -2931465).invalid
    assert (d - 100) - -100 == d

    with pytest.raises(TypeError):
        100 - d


def test_date_date_sub():
    assert 1973/Dec/ 3 - 1973/Dec/ 3 ==        0
    assert 1973/Dec/ 4 - 1973/Dec/ 3 ==        1
    assert 1973/Dec/ 2 - 1973/Dec/ 3 ==       -1
    assert 1973/Dec/ 3 - 1973/Nov/23 ==       10
    assert 1973/Aug/25 - 1973/Dec/ 3 ==     -100
    assert 1973/Dec/ 3 - 1976/Aug/29 ==    -1000
    assert 2001/Apr/20 - 1973/Dec/ 3 ==    10000
    assert 1973/Dec/ 3 -    1/Jan/ 1 == (1973/Dec/3).datenum
    assert 9999/Dec/31 - 1973/Dec/ 3 ==  2931464
    assert 9999/Dec/31 -    1/Jan/ 1 ==  3652058
    assert    1/Jan/ 1 - 9999/Dec/31 == -3652058


def test_date_math_invalid():
    assert (Date.INVALID +        100).invalid
    assert (Date.MISSING +        100).missing
    assert (Date.INVALID -          1).invalid
    assert (Date.MISSING -          1).missing
    assert (Date.INVALID - 4294967294).invalid
    assert (Date.MISSING - 4294967294).missing

    assert Date.INVALID - 1973/Dec/3   is None
    assert 1973/Dec/3   - Date.MISSING is None
    assert Date.INVALID - Date.MISSING is None


