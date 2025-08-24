#!/bin/sh

make -C lib
for plugin in plugins/*; do 
	make -C "$plugin"
done
make
