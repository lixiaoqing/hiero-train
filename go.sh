set -x
make
mv a unit-test/ 
cd unit-test
./a train.en.tree train.ch train.align.li > log
cat log
cd -
