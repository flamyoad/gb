cmake -S . -B build

cd build

cmake --build .

./gb ../tests/blargg/cpu_instrs/cpu_instrs.gb

todo:
cpu_instrs stil failing
https://github.com/retrio/gb-test-roms/blob/master/cpu_instrs/source/03-op%20sp%2Chl.s