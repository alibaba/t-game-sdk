:: Copyright (C) 2021-2014 Alibaba Group Holding Limited

@setlocal
@set EM_PY=%EMSDK_PYTHON%
@if "%EM_PY%"=="" (
  set EM_PY=python
)

@"%EM_PY%" "%~dp0\%~n0.py" %*
