cd SDK/include
for /f %%f in ('dir /b /s *.cpp') do clang-format -i %%f
for /f %%f in ('dir /b /s *.hpp') do clang-format -i %%f
for /f %%f in ('dir /b /s *.c') do clang-format -i %%f
for /f %%f in ('dir /b /s *.h') do clang-format -i %%f
cd ../..

cd Server
for /f %%f in ('dir /b /s *.cpp') do clang-format -i %%f
for /f %%f in ('dir /b /s *.hpp') do clang-format -i %%f
for /f %%f in ('dir /b /s *.c') do clang-format -i %%f
for /f %%f in ('dir /b /s *.h') do clang-format -i %%f
cd ..

cd Shared/NetCode
for /f %%f in ('dir /b /s *.cpp') do clang-format -i %%f
for /f %%f in ('dir /b /s *.hpp') do clang-format -i %%f
for /f %%f in ('dir /b /s *.c') do clang-format -i %%f
for /f %%f in ('dir /b /s *.h') do clang-format -i %%f
cd ../..

