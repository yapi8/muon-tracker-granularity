@echo off
REM Run all seven configurations, then analyse and plot.
setlocal

if "%BIN%"=="" set BIN=build\Release\muontracker.exe
if "%EVENTS%"=="" set EVENTS=20000

if not exist "%BIN%" (
  echo error: "%BIN%" not found.
  echo build first:  cmake -S . -B build ^&^& cmake --build build --config Release
  exit /b 1
)

if not exist results mkdir results

for %%N in (4 8 16 32 64 128 256) do (
  echo ==============================================
  echo  N = %%N   ^(%EVENTS% events^)
  echo ==============================================
  "%BIN%" %%N %EVENTS%
)

echo.
echo running analysis...
python analysis\analyze.py
python analysis\plot_results.py
echo done -- see results\resolution.csv and results\resolution.png
endlocal
