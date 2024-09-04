#!/bin/bash

cd rpc/src
rm -rf *.pb.h
rm -rf *.pb.cc
protoc --cpp_out=./ *.proto

cd ../
cd ../
cd examples/rpc/friendproto
rm -rf *.pb.h
rm -rf *.pb.cc
protoc --cpp_out=./ *.proto
cd ../userproto
rm -rf *.pb.h
rm -rf *.pb.cc
protoc --cpp_out=./ *.proto


