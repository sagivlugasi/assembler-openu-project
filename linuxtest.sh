clear
rm -rf  *.log ./_tests/good_code/code/*.log ./_tests/good_code/code/*.am ./_tests/good_code/code/*.ob ./_tests/good_code/code/*.ent ./_tests/good_code/code/*.ext ./_tests/good_code/code/*.warnings ./_tests/good_code/code/*.warnings.warnings ./_tests/good_code/code/*.errors ./_tests/good_code/code/*.errors.errors  
make 
./compiler  test-with-macros2
rm -f compiler
