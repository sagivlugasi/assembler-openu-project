clear
rm -rf ./_tests/good_code/code/*.log ./_tests/good_code/code/*.am ./_tests/good_code/code/*.ob ./_tests/good_code/code/*.ent ./_tests/good_code/code/*.ext ./_tests/good_code/code/*.warnings ./_tests/good_code/code/*.warnings.warnings ./_tests/good_code/code/*.errors ./_tests/good_code/code/*.errors.errors  
make 
./compiler ./_tests/good_code/code/test1 ./_tests/good_code/code/test2 ./_tests/good_code/code/test3 
rm -f compiler
