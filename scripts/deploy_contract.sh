#!/bin/bash
# cleos create account eosio acornaccount EOS8RoCAXxWYUW2v4xkG19F57BDVBzpt9NN2iDsD1ouQNyV2BkiNc

force=false
NET=

if [ "$1" == "prod" ]; then
   NET='--url https://telos.eos.barcelona'
fi

if [ "$1" == "test" ]; then
   NET='--url https://testnet.telos.caleos.io'
fi

if [ "$1" == "create" ]; then
    echo "$ cleos $NET create account eosio acornaccount EOS8RoCAXxWYUW2v4xkG19F57BDVBzpt9NN2iDsD1ouQNyV2BkiNc"
    cleos $NET create account eosio acornaccount EOS8RoCAXxWYUW2v4xkG19F57BDVBzpt9NN2iDsD1ouQNyV2BkiNc
fi



acornaccount_HOME=../
echo "-------- acornaccount ---------"
cd $acornaccount_HOME
eosio-cpp -o acornaccount.wasm acornaccount.cpp --abigen
echo "$ cleos $NET set contract acornaccount $PWD -p acornaccount@active"
cleos $NET set contract acornaccount $PWD -p acornaccount@active




