#!/usr/bin/env bash

#function print_to_file {
#} 
  for i in {1..1000000}; do
    printf "%i %s\n" $i "yoyoyoyoyoyooy" >> file
  done
