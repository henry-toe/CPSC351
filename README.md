Winter 2026 CPSC 351-01 10100, Hai-Duong // henryto@csu.fullerton.edu 
C++ on Windows, but used Linux WSL development environment (https://learn.microsoft.com/en-us/windows/wsl/)
How to execute the program in a Linux environment:
  >make clean
  >make
  Give testfile.txt some sort of data such as:
  >echo "I hope this runs. " > testfile.txt
  On another terminal (receiving)
  >./recv
  On the first terminal (sending)
  >./sender testfile.txt
  Output should be:
  The number of bytes sent is 19
  On the receiving terminal output should be:
  The number of bytes received is: 19
