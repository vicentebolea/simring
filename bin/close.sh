#!/bin/bash

for i in `seq -w 1 8`; do
  ssh raven0$i killall node
done
