set -x
make
mv a unit-test/ 
cd unit-test
./a train.en.tree train.ch train.align.li 
cat rules.c2e
cat rules.e2c
cd -
