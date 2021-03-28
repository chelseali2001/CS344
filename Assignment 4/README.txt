Name: Chelsea Li 
ID: 933294417
Date: 02/22/2021

How to compile program: gcc --std=c99 -pthread lichel_program3.c -o line_processor

How to run program: 
./line_processor
./line_processor < input.txt
./line_processor > output.txt
./line_processor < input.txt > output.txt

Notes: Structure for buffers functions, static variables, and thread functions in the main function are based off of class materials.

Instructions:
1. If there's no input file, user won't be presented with a message, user will just have to start typing.
2. When the user has entered at least 80 characters for an output line, the output line will be produced.
3. If an input file is given but not an output file, the program will print out the output lines in the terminal.
4. If an output file is given but not an input file, the user will enter their characters in the terminal and the 80 character output lines will be printed into the output file.
5. If an input and output file is given, the program will print out the output lines in the output file. 
3. Program terminates when "STOP" is encountered.