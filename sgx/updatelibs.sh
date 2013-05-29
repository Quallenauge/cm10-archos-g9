#!/bin/sh

for a in `find bin lib -type f`; do
cp ${ANDROID_PRODUCT_OUT}/target/`basename $a` $a 
done
