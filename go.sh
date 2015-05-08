set -x
make
mv extract-rules unit-test/ 
cd unit-test
./extract-rules train.ch train.en train.align.li 
cat rules.s2t
cat rules.t2s
cd -
