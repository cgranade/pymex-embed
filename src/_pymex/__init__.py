from _pymex.mx_array import mxArray
import _pymex.redirect_io as _redirect_io
from _pymex.mat_funcs import matfunc
from . import mtypes

def init():
    _redirect_io.redirect_io()
