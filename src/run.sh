#!/bin/bash

make clean && make && qemu-system-x86_64 -kernel kernel -m 4G -serial stdio -s
