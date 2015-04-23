set -x
make
mv a unit-test/ 
cd unit-test
./a train.ch.tree train.en.tree train.align.li 
cat rules.s2t
cat rules.t2s
cd -
