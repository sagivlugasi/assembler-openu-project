clear
rm -rf  *.log ./_tests/good_code/code/*.log ./_tests/good_code/code/*.am ./_tests/good_code/code/*.ob ./_tests/good_code/code/*.ent ./_tests/good_code/code/*.ext ./_tests/good_code/code/*.warnings ./_tests/good_code/code/*.warnings.warnings ./_tests/good_code/code/*.errors ./_tests/good_code/code/*.errors.errors  
rm -rf  *.log *.am *.ob *.ent *.ext *.warnings *.warnings *.errors *.errors.errors  
make 
./compiler ./_test/good_code/code/test3  ./_test/good_code/code/test-with-macrosm test
rm -f compiler 
# wrongNumOfOperands wrongUseOfInstructions wrongAddressingMethodTest
# test test1 test-with-macros2