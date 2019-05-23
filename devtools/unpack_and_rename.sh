#!/bin/sh

rm -rf main

unzip New\ Project\ source.zip -d main
rm New\ Project\ source.zip

cp jwi_atmos_modified/core.h main/src/atmo
cp jwi_atmos_modified/atmosphere_platform.c main/src/app_src
