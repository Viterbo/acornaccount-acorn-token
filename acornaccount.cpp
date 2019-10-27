#include "acornaccount.hpp"

#include <cmath>


void acornaccount::create( name   issuer,
                    asset  maximum_supply )
{
    require_auth( _self );

    auto sym = maximum_supply.symbol;
    eosio_assert( sym.is_valid(), "invalid symbol name" );
    eosio_assert( maximum_supply.is_valid(), "invalid supply");
    eosio_assert( maximum_supply.amount > 0, "max-supply must be positive");

    stats statstable( _self, sym.code().raw() );
    auto existing = statstable.find( sym.code().raw() );
    eosio_assert( existing == statstable.end(), "token with symbol already exists" );

    statstable.emplace( _self, [&]( auto& s ) {
        s.supply.symbol = maximum_supply.symbol;
        s.max_supply    = maximum_supply;
        s.issuer        = issuer;
    });
}


void acornaccount::issue( name to, asset quantity, string memo )
{
    auto sym = quantity.symbol;
    eosio_assert( sym.is_valid(), "invalid symbol name" );
    eosio_assert( memo.size() <= 256, "memo has more than 256 bytes" );

    stats statstable( _self, sym.code().raw() );
    auto existing = statstable.find( sym.code().raw() );
    eosio_assert( existing != statstable.end(), "token with symbol does not exist, create token before issue" );
    const auto& st = *existing;

    require_auth( get_self() );
    eosio_assert( quantity.is_valid(), "invalid quantity" );
    eosio_assert( quantity.amount > 0, "must issue positive quantity" );

    eosio_assert( quantity.symbol == st.supply.symbol, "symbol precision mismatch" );
    eosio_assert( quantity.amount <= st.max_supply.amount - st.supply.amount, "quantity exceeds available supply");

    statstable.modify( st, same_payer, [&]( auto& s ) {
        s.supply += quantity;
    });

    add_balance( get_self(), quantity, get_self() );

    if( to != get_self() ) {
        SEND_INLINE_ACTION( *this, transfer, { {get_self(), "active"_n} },
                          { get_self(), to, quantity, memo }
        );
    }
}

void acornaccount::retire( asset quantity, string memo )
{
    auto sym = quantity.symbol;
    eosio_assert( sym.is_valid(), "invalid symbol name" );
    eosio_assert( memo.size() <= 256, "memo has more than 256 bytes" );

    stats statstable( _self, sym.code().raw() );
    auto existing = statstable.find( sym.code().raw() );
    eosio_assert( existing != statstable.end(), "token with symbol does not exist" );
    const auto& st = *existing;

    require_auth( st.issuer );
    eosio_assert( quantity.is_valid(), "invalid quantity" );
    eosio_assert( quantity.amount > 0, "must retire positive quantity" );

    eosio_assert( quantity.symbol == st.supply.symbol, "symbol precision mismatch" );

    statstable.modify( st, same_payer, [&]( auto& s ) {
       s.supply -= quantity;
    });

    sub_balance( st.issuer, quantity );
}

void acornaccount::transfer( name    from,
                      name    to,
                      asset   quantity,
                      string  memo )
{
    eosio_assert( from != to, "cannot transfer to self" );
    require_auth( from );
    eosio_assert( is_account( to ), "to account does not exist");
    auto sym = quantity.symbol.code();
    stats statstable( _self, sym.raw() );
    const auto& st = statstable.get( sym.raw() );

    require_recipient( from );
    require_recipient( to );

    eosio_assert( quantity.is_valid(), "invalid quantity" );
    eosio_assert( quantity.amount > 0, "must transfer positive quantity" );
    eosio_assert( quantity.symbol == st.supply.symbol, "symbol precision mismatch" );
    eosio_assert( memo.size() <= 256, "memo has more than 256 bytes" );

    name payer = has_auth( to ) ? to : from;

    try_ubi_claim(from, payer, quantity.symbol);

    sub_balance( from, quantity );
    add_balance( to, quantity, payer );
}

void acornaccount::sub_balance( name owner, asset value ) {
    accounts from_acnts( _self, owner.value );

    const auto& from = from_acnts.get( value.symbol.code().raw(), "no balance object found" );
    eosio_assert( from.balance.amount >= value.amount, "overdrawn balance" );

    from_acnts.modify( from, owner, [&]( auto& a ) {
        a.balance -= value;
    });
}

void acornaccount::add_balance( name owner, asset value, name ram_payer )
{
    accounts to_acnts( _self, owner.value );
    auto to = to_acnts.find( value.symbol.code().raw() );
    if( to == to_acnts.end() ) {
        to_acnts.emplace( ram_payer, [&]( auto& a ){
            a.balance = value;
        });
    } else {
        to_acnts.modify( to, same_payer, [&]( auto& a ) {
            a.balance += value;
        });
    }
}

void acornaccount::open( name owner, const symbol& symbol, name ram_payer )
{
    require_auth( ram_payer );

    auto sym_code_raw = symbol.code().raw();

    stats statstable( _self, sym_code_raw );
    const auto& st = statstable.get( sym_code_raw, "symbol does not exist" );
    eosio_assert( st.supply.symbol == symbol, "symbol precision mismatch" );

    accounts acnts( _self, owner.value );
    auto it = acnts.find( sym_code_raw );
    if( it == acnts.end() ) {
        acnts.emplace( ram_payer, [&]( auto& a ){
            a.balance = asset{0, symbol};
        });
    }

    try_ubi_claim(owner, ram_payer, symbol);
}

void acornaccount::close( name owner, const symbol& symbol )
{
    require_auth( owner );
    accounts acnts( _self, owner.value );
    auto it = acnts.find( symbol.code().raw() );
    eosio_assert( it != acnts.end(), "Balance row already deleted or never existed. Action won't have any effect." );
    eosio_assert( it->balance.amount == 0, "Cannot close because the balance is not zero." );
    acnts.erase( it );
}

// -----------------------------------


void acornaccount::try_ubi_claim( name owner, name payer, symbol sym ) {
    // Check if the "owner" account is authorized to receive it.
    PRINT("acornaccount::try_ubi_claim()\n");
    if (! can_claim_UBI( owner ))
        return;

    // The token contract account is NOT eligible for an UBI.
    if (owner == _self)
        return;

    const time_point_sec _now = time_point_sec(now());

    users users_table(_self, _self.value);
    auto usr = users_table.find(owner.value);

    claims claims_table(_self, owner.value);
    auto claim = claims_table.find(sym.code().raw());

    if (usr == users_table.end()) {
        eosio_assert(claim == claims_table.end(), "ERROR: inconsistency error. claims_table exist but users_table does not");
        users_table.emplace(payer, [&]( auto& a ) {
            a.account = owner;
        });
        claims_table.emplace(payer, [&]( auto& a ) {
            a.claimed = asset(0, sym);
            a.last_claim = time_point_sec(now() + two_days);
        });
    } else {
        eosio_assert(claim != claims_table.end(), "ERROR: inconsistency error. users_table exist but claims_table does not");
        uint32_t last = claim->last_claim.sec_since_epoch();
        uint32_t today = now();
        PRINT("today: ", std::to_string((unsigned) today), "\n");
        PRINT("last:  ", std::to_string((unsigned) last), "\n");
        if (today > last) {
            uint32_t diff = today - last;

            PRINT("diff:  ", std::to_string((unsigned) diff), "\n");
            uint32_t mod = diff % one_day; // fraction of last day
            diff -= mod;                   // if we have 2 days and a half, we take out the "half"
            PRINT("mod:   ", std::to_string((unsigned) mod), "\n");
            PRINT("diff:  ", std::to_string((unsigned) diff), "\n");
            if (diff > 0) {
                uint32_t days = diff / one_day;
                uint32_t lost = 0;
                PRINT("days:  ", std::to_string((unsigned) days), " <- antes de procesar\n");
                if (days > max_days) {
                    lost = days - max_days;
                    days -= lost;
                }

                PRINT("lost:  ", std::to_string((unsigned) lost), "\n");
                PRINT("days:  ", std::to_string((unsigned) days), "\n");

                uint64_t amount = days * pow(10, sym.precision());
                asset quantity = asset(amount, sym);
                claims_table.modify(*claim, same_payer, [&]( auto& a ) {
                    a.claimed = quantity;
                    a.last_claim = time_point_sec(now());
                });
                // name to, asset quantity, string memo

                string memo = string("UBI: ") + quantity.to_string();
                if (lost > 0) {
                    memo += " you lost " + std::to_string((unsigned) lost) + " days of income";
                } 

                action(
                    permission_level{get_self(), name("active")},
                    get_self(),
                    name("issue"),
                    std::make_tuple(owner, quantity, memo)
                ).send();
            } 
        }
        
    }
    PRINT("acornaccount::try_ubi_claim()...\n");
}


AUX_DEBUG_ACTIONS(
void acornaccount::addclaim( name owner, uint32_t sec ) {
    // Check if the "owner" account is authorized to receive it.
    PRINT("acornaccount::addclaim()\n");

    symbol sym = symbol(symbol_code("ACORN"), 4);
    name payer = get_self();

    users users_table(_self, _self.value);
    auto usr = users_table.find(owner.value);

    claims claims_table(_self, owner.value);
    auto claim = claims_table.find(sym.code().raw());

    if (usr == users_table.end()) {
        eosio_assert(claim == claims_table.end(), "ERROR: inconsistency error. claims_table exist but users_table does not");
        users_table.emplace(payer, [&]( auto& a ) {
            a.account = owner;
        });
        claims_table.emplace(payer, [&]( auto& a ) {
            a.claimed = asset(0, sym);
            a.last_claim = time_point_sec(now() + sec);
        });
    } else {
        eosio_assert(claim != claims_table.end(), "ERROR: inconsistency error. users_table exist but claims_table does not");
        claims_table.modify(*claim, same_payer, [&]( auto& a ) {
            a.last_claim = time_point_sec(now() + sec);
        });
    }
}


void acornaccount::subclaim( name owner, uint32_t sec ) {
    // Check if the "owner" account is authorized to receive it.
    PRINT("acornaccount::subclaim()\n");

    symbol sym = symbol(symbol_code("ACORN"), 4);
    name payer = get_self();

    users users_table(_self, _self.value);
    auto usr = users_table.find(owner.value);

    claims claims_table(_self, owner.value);
    auto claim = claims_table.find(sym.code().raw());

    if (usr == users_table.end()) {
        eosio_assert(claim == claims_table.end(), "ERROR: inconsistency error. claims_table exist but users_table does not");
        users_table.emplace(payer, [&]( auto& a ) {
            a.account = owner;
        });
        claims_table.emplace(payer, [&]( auto& a ) {
            a.claimed = asset(0, sym);
            a.last_claim = time_point_sec(now() - sec);
        });
    } else {
        eosio_assert(claim != claims_table.end(), "ERROR: inconsistency error. users_table exist but claims_table does not");
        claims_table.modify(*claim, same_payer, [&]( auto& a ) {
            a.last_claim = time_point_sec(now() - sec);
        });
    }
}

void acornaccount::setclaim( name owner, uint32_t sec ) {
    // Check if the "owner" account is authorized to receive it.
    PRINT("acornaccount::subclaim()\n");

    symbol sym = symbol(symbol_code("ACORN"), 4);
    name payer = get_self();

    users users_table(_self, _self.value);
    auto usr = users_table.find(owner.value);

    claims claims_table(_self, owner.value);
    auto claim = claims_table.find(sym.code().raw());

    if (usr == users_table.end()) {
        eosio_assert(claim == claims_table.end(), "ERROR: inconsistency error. claims_table exist but users_table does not");
        users_table.emplace(payer, [&]( auto& a ) {
            a.account = owner;
        });
        claims_table.emplace(payer, [&]( auto& a ) {
            a.claimed = asset(0, sym);
            a.last_claim = time_point_sec(sec);
        });
    } else {
        eosio_assert(claim != claims_table.end(), "ERROR: inconsistency error. users_table exist but claims_table does not");
        claims_table.modify(*claim, same_payer, [&]( auto& a ) {
            a.last_claim = time_point_sec(sec);
        });
    }
}

) // AUX_DEBUG_ACTIONS

EOSIO_DISPATCH( acornaccount,
    (create)(issue)(transfer)(open)(close)(retire)
    AUX_DEBUG_ACTIONS((addclaim)(subclaim)(setclaim))
)

