# only execute this if you're sure there are no errors
cd skprx/build
cmake ..
make
make send
cd ../..
cd vpk/build
cmake ..
make
make vpksend