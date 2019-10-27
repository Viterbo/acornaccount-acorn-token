# acornaccount-acorn-token
Source code for the ACORN standard eosio token that runs on Telos Network.



----------------
### Notes 

ACORN original code:
https://github.com/fcecin/wubi/blob/master/wubi.cpp
https://github.com/fcecin/wubi/blob/master/wubi.hpp

YANG code:
https://github.com/fcecin/ygc/blob/master/ygc.hpp
https://github.com/fcecin/ygc/blob/master/ygc.cpp

https://t.me/dappstelos/3484

telostest set account permission acornaccount active '{"threshold": 1,"keys": [{"key": "EOS7JYMQL2QE8XLYqoEwHgAhbhdf3nhZFnCpeRrdRyLVTWUcBza9N","weight": 1}],"accounts": [{"permission":{"actor":"acornaccount","permission":"eosio.code"},"weight":1}]}' owner -p acornaccount@active

telosmain set account permission acornaccount active '{"threshold": 1,"keys": [{"key": "EOS894C16yeSgR4ER2FnasjwmZpQWVL2fJGFJQo1uaG5qyjJVfBQC","weight": 1}],"accounts": [{"permission":{"actor":"acornaccount","permission":"eosio.code"},"weight":1}]}' owner -p acornaccount@active