#include "ex.hpp"

#include <cmath>
#include <enulib/action.hpp>
#include <enulib/asset.hpp>
#include "enu.token.hpp"

using namespace enumivo;
using namespace std;

void ex::receivedenu(const currency::transfer &transfer) {
  if (transfer.to != _self) {
    return;
  }

  // get ENU balance
  double enu_balance = enumivo::token(N(enu.token)).
	   get_balance(N(enu.eth.mm), enumivo::symbol_type(ENU_SYMBOL).name()).amount;
  
  enu_balance = enu_balance/10000;

  double received = transfer.quantity.amount;
  received = received/10000;

  // get ETH balance
  double eth_balance = enumivo::token(N(iou.coin)).
	   get_balance(N(enu.eth.mm), enumivo::symbol_type(ETH_SYMBOL).name()).amount;

  eth_balance = eth_balance/100000000;

  //deduct fee
  received = received * 0.997;
  
  double product = eth_balance * enu_balance;

  double buy = eth_balance - (product / (received + enu_balance));

  auto to = transfer.from;

  auto quantity = asset(100000000*buy, ETH_SYMBOL);

  action(permission_level{N(enu.eth.mm), N(active)}, N(iou.coin), N(transfer),
         std::make_tuple(N(enu.eth.mm), to, quantity,
                         std::string("Buy ETH with ENU")))
      .send();

  action(permission_level{_self, N(active)}, N(enu.token), N(transfer),
         std::make_tuple(_self, N(enu.eth.mm), transfer.quantity,
                         std::string("Buy ETH with ENU")))
      .send();
}

void ex::receivedenu(const currency::transfer &transfer) {
  if (transfer.to != _self) {
    return;
  }

  // get ETH balance
  double eth_balance = enumivo::token(N(iou.coin)).
	   get_balance(N(enu.eth.mm), enumivo::symbol_type(ETH_SYMBOL).name()).amount;
  
  eth_balance = eth_balance/100000000;

  double received = transfer.quantity.amount;
  received = received/10000;

  // get ENU balance
  double enu_balance = enumivo::token(N(enu.token)).
	   get_balance(N(enu.eth.mm), enumivo::symbol_type(ENU_SYMBOL).name()).amount;

  enu_balance = enu_balance/10000;

  //deduct fee
  received = received * 0.997;

  double product = enu_balance * eth_balance;

  double sell = enu_balance - (product / (received + eth_balance));

  auto to = transfer.from;

  auto quantity = asset(10000*sell, ENU_SYMBOL);

  action(permission_level{N(enu.eth.mm), N(active)}, N(enu.token), N(transfer),
         std::make_tuple(N(enu.eth.mm), to, quantity,
                         std::string("Sell ETH for ENU")))
      .send();

  action(permission_level{_self, N(active)}, N(iou.coin), N(transfer),
         std::make_tuple(_self, N(enu.eth.mm), transfer.quantity,
                         std::string("Sell ETH for ENU")))
      .send();
      
}

void ex::apply(account_name contract, action_name act) {

  if (contract == N(enu.token) && act == N(transfer)) {
    auto transfer = unpack_action_data<currency::transfer>();

    enumivo_assert(transfer.quantity.symbol == ENU_SYMBOL,
                 "Must send ENU");
    receivedenu(transfer);
    return;
  }

  if (contract == N(iou.coin) && act == N(transfer)) {
    auto transfer = unpack_action_data<currency::transfer>();

    enumivo_assert(transfer.quantity.symbol == ETH_SYMBOL,
                 "Must send ETH");
    receivedenu(transfer);
    return;
  }

  if (act == N(transfer)) {
    auto transfer = unpack_action_data<currency::transfer>();
    enumivo_assert(false, "Must send ETH or ENU");
    return;
  }

  if (contract != _self) return;

}

extern "C" {
[[noreturn]] void apply(uint64_t receiver, uint64_t code, uint64_t action) {
  ex enuenu(receiver);
  enuenu.apply(code, action);
  enumivo_exit(0);
}
}
