cd "C:\Users\Nirmal\SYSC4006\Group_4_Bloc"
touch "./script/test_result.txt"
make clean
make test
cd bin
./tests >&1 | tee "../script/test_result.txt"
