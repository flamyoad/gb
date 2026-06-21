cmake -S . -B build

cd build

cmake --build .

./gb ../tests/blargg/cpu_instrs/cpu_instrs.gb
