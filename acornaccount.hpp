#pragma once

#include <eosiolib/asset.hpp>
#include <eosiolib/eosio.hpp>
#include <eosiolib/time.hpp>

#include <string>

using namespace eosio;
using namespace std;

namespace acorn {

   using std::string;

   CONTRACT token : public eosio::contract {
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

         TABLE claim_table {
            asset          clamed;
            time_point_sec last_claim;
	    
            uint64_t primary_key() const { return clamed.symbol.code().raw(); }
         };
   };

} /// namespace eosio
