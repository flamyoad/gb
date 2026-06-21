cmake -S . -B build

cd build

cmake --build .

./gb ../tests/blargg/cpu_instrs/cpu_instrs.gb

todo:
test 02-2 failing at EI instruction
https://github.com/retrio/gb-test-roms/blob/master/cpu_instrs/source/02-interrupts.s
