clear && rm -f compiler && rm -f *.am *.ob *.ent *.ext *.warnings *.errors && make && ./compiler test test2 test3 && rm compiler.exe
