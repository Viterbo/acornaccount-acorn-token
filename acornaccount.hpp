#pragma once

#include <eosiolib/eosio.hpp>
#include <eosiolib/symbol.hpp>
#include <eosiolib/asset.hpp>
#include <eosiolib/print.hpp>
#include <eosiolib/transaction.hpp>
#include <eosiolib/singleton.hpp>

// #include <eosiolib/asset.hpp>
// #include <eosiolib/eosio.hpp>
// #include <eosiolib/time.hpp>

#include <string>

using namespace eosio;
using namespace std;


/* <-- erase just one slash bar to switch those codes (commented/actual-code)
#define PRINT(...) print(__VA_ARGS__)
#define AUX_DEBUG_ACTIONS(...) __VA_ARGS__
/*/
#define PRINT(...)
#define AUX_DEBUG_ACTIONS(...)
//*/

using std::string;

CONTRACT acornaccount : public eosio::contract {
    public:
        using contract::contract;

        // standard token API
        ACTION create(name issuer, asset maximum_supply);
        ACTION issue(name to, asset quantity, string memo);
        ACTION retire(asset quantity, string memo);
        ACTION transfer(name from, name to, asset quantity, string memo);
        ACTION open(name owner, const symbol& symbol, name ram_payer);
        ACTION close(name owner, const symbol& symbol);

    private:
        // standard token table structures
        TABLE account {
            asset    balance;
            uint64_t primary_key()const { return balance.symbol.code().raw(); }
        };

        TABLE currency_stats {
            asset    supply;
            asset    max_supply;
            name     issuer;
            uint64_t primary_key()const { return supply.symbol.code().raw(); }
        };

        // standard token table names
        typedef eosio::multi_index< "accounts"_n, account > accounts;
        typedef eosio::multi_index< "stat"_n, currency_stats > stats;

        // aux functions 
        void sub_balance( name owner, asset value );
        void add_balance( name owner, asset value, name ram_payer );

        // -----------------------------------------------------------

        // this table holds claim information. Last clame date and how much
        TABLE claim_table {
            asset          claimed;
            time_point_sec last_claim;
            uint64_t primary_key() const { return claimed.symbol.code().raw(); }
        };
        typedef eosio::multi_index< "claims"_n, claim_table > claims;

        // this table holds a lista of users of acorn. This cuold be usefull for batch procecssing in the future
        TABLE user_table {
            name account;
            uint64_t primary_key() const { return account.value; }
        };
        typedef eosio::multi_index< "users"_n, user_table > users;


        void try_ubi_claim(name from, name payer, symbol sym);         

        bool can_claim_UBI( name claimant ) { return true; }
        const uint32_t two_days = 172800;
        const uint32_t one_day = 86400; 
        const uint32_t max_days = 360; 

    public:
        AUX_DEBUG_ACTIONS(
            ACTION addclaim(name owner, uint32_t sec);
            ACTION subclaim(name owner, uint32_t sec);
            ACTION setclaim(name owner, uint32_t sec);
        )
};
