#!/bin/bash

make
./mapper.o "input.txt"
./reducer.o "Mapper_Output.txt"
./combiner.o "input.txt"

