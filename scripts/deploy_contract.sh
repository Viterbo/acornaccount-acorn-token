#!/bin/bash

force=false
NET=

if [ "$1" == "prod" ]; then
   NET='--url https://telos.eos.barcelona'
fi

if [ "$1" == "test" ]; then
   NET='--url https://testnet.telos.caleos.io'
fi

acornaccount_HOME=../
echo "-------- acornaccount ---------"
cd $acornaccount_HOME
eosio-cpp -o acornaccount.wasm acornaccount.cpp --abigen
echo "$ cleos $NET set contract acornaccount $PWD -p acornaccount@active"
cleos $NET set contract acornaccount $PWD -p acornaccount@active




