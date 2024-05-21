#!/usr/bin/env bash
set -euo pipefail

make

# deploy glsl
cp -vr glsl/gl2 bin/data
cp -vr glsl/gl3 bin/data

# run
cd bin
./amUrtayantra
