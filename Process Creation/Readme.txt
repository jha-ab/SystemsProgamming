Readme.txt

1.) Unzip and paste the input file at the unpacked location. An input.txt is provided along with the files.
2.) Use the command "make" -> This will create the executables namely mapper.o, reducer.o and combiner.o
	These files can be executed as - 
	a) ./mapper.o "input.txt OR any filename"
	b) ./reducer.o "Mapper_Output.txt"
	c) ./combiner.o "input.txt OR any filename"
    OR
    execute the command ./run.sh; this will sequentially run all commands
3.) output.txt is generated which contains the final output.
