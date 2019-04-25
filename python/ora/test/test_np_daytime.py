import numpy as np
import pytest

import ora
from   ora import DAYTIME_TYPES

#-------------------------------------------------------------------------------

def get_array(Daytime):
    return np.array(
        [Daytime(0, 0), Daytime(14,31, 25), Daytime.INVALID, Daytime.MISSING])


@pytest.mark.parametrize("Daytime", DAYTIME_TYPES)
def test_daytime_array(Daytime):
    arr = get_array(Daytime)
    assert arr.dtype == Daytime.dtype


@pytest.mark.parametrize("Daytime", DAYTIME_TYPES)
def test_is_valid(Daytime):
    arr = get_array(Daytime)
    assert (ora.np.is_valid(arr).astype(int) == [1, 1, 0, 0]).all()


@pytest.mark.xfail
def test_convert_invalid():
    assert (np.array([
        "",
        None,
        "2019-04-16",
        ora.Date(2019, 4, 16),
        ora.now(),
        "2019-04-16T12:30:45+00:00",
        "12:3",
        "00:00:99",
        "0:61:00",
        "25:12:15",
    ], dtype=ora.Daytime) == ora.Daytime.INVALID).all()


@pytest.mark.xfail
@pytest.mark.parametrize("Daytime", DAYTIME_TYPES)
def test_to_offset(Daytime):
    daytimes = get_array(Daytime)
    offsets = ora.np.to_offset(daytimes)
    assert (offsets == [ y.offset for y in daytimes ]).all()


@pytest.mark.xfail
@pytest.mark.parametrize("Daytime", DAYTIME_TYPES)
def test_daytime_from_offset(Daytime):
    daytimes = get_array(Daytime)
    offsets = np.array([ y.offset for y in daytimes ])
    assert (ora.np.daytime_from_offset(offsets) == daytimes).all()


@pytest.mark.parametrize(
    "Daytime0, Daytime1",
    [
        (ora.Daytime, ora.UsecDaytime),
        (ora.Daytime, ora.Daytime32),
        (ora.Daytime32, ora.UsecDaytime),
    ]
)
def test_cast(Daytime0, Daytime1):
    """
    Tests that casts between `Daytime0` and `Daytime1` work.
    """
    arr = get_array(Daytime0)
    assert (arr.astype(Daytime1).astype(Daytime0) == arr).all()


@pytest.mark.xfail
@pytest.mark.parametrize(
    "Daytime0, Daytime1",
    [
        (ora.Daytime, ora.UsecDaytime),
        (ora.Daytime, ora.Daytime32),
        (ora.Daytime32, ora.UsecDaytime),
    ]
)
def test_compare(Daytime0, Daytime1):
    """
    Tests that comparisons between `Daytime0` and `Daytime1` work.
    """
    arr0 = get_array(Daytime0)
    arr1 = arr0.astype(Daytime1)
    assert (arr0 == arr1).all()
    assert (arr0 <  arr1 + 1).all()
    assert (arr0 <= arr1).all()
    assert (arr0 >  arr1 - 1).all()
    assert (arr0 >= arr1).all()
    assert (arr0 != arr1 + 1).all()


