NET=

if [ "$1" == "prod" ]; then
   NET='--url https://telos.eos.barcelona'
fi

if [ "$1" == "test" ]; then
   NET='--url https://testnet.telos.caleos.io'
fi


echo "-- users -------------------------------------"
cleos $NET get table acornaccount acornaccount users

echo "-- accounts -------------------------------------"
echo "- gqydoobuhege -"
cleos $NET get table acornaccount gqydoobuhege accounts
echo "- heztanrrgyge -"
cleos $NET get table acornaccount heztanrrgyge accounts
echo "- fiscalesvote -"
cleos $NET get table acornaccount fiscalesvote accounts
echo "- viterbotelos -"
cleos $NET get table acornaccount viterbotelos accounts

echo "-- claims -------------------------------------"
echo "- gqydoobuhege -"
cleos $NET get table acornaccount gqydoobuhege claims
echo "- heztanrrgyge -"
cleos $NET get table acornaccount heztanrrgyge claims
echo "- fiscalesvote -"
cleos $NET get table acornaccount fiscalesvote claims
echo "- viterbotelos -"
cleos $NET get table acornaccount viterbotelos claims



# echo "-- accounts -------------------------------------"
# echo "- telosdouglas -"
# cleos $NET get table acornaccount telosdouglas accounts
# echo "- islandcrypto -"
# cleos $NET get table acornaccount islandcrypto accounts
# echo "- viterbotelos -"
# cleos $NET get table acornaccount viterbotelos accounts
# 
# echo "-- claims -------------------------------------"
# echo "- telosdouglas -"
# cleos $NET get table acornaccount telosdouglas claims
# echo "- islandcrypto -"
# cleos $NET get table acornaccount islandcrypto claims
# echo "- viterbotelos -"
# cleos $NET get table acornaccount viterbotelos claims


# echo "-- users -------------------------------------"
# cleos $NET get table acorntkntest acorntkntest users
# 
# echo "-- accounts -------------------------------------"
# echo "- gqydoobuhege -"
# cleos $NET get table acorntkntest gqydoobuhege accounts
# echo "- heztanrrgyge -"
# cleos $NET get table acorntkntest heztanrrgyge accounts
# echo "- fiscalesvote -"
# cleos $NET get table acorntkntest fiscalesvote accounts
# 
# echo "-- claims -------------------------------------"
# echo "- gqydoobuhege -"
# cleos $NET get table acorntkntest gqydoobuhege claims
# echo "- heztanrrgyge -"
# cleos $NET get table acorntkntest heztanrrgyge claims
# echo "- fiscalesvote -"
# cleos $NET get table acorntkntest fiscalesvote claims
