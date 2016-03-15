#!/bin/bash

make clean && make && qemu-system-x86_64 -kernel kernel -serial stdio -s -m 4G
