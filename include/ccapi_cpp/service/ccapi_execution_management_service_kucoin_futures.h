#ifndef INCLUDE_CCAPI_CPP_SERVICE_CCAPI_EXECUTION_MANAGEMENT_SERVICE_KUCOIN_FUTURES_H_
#define INCLUDE_CCAPI_CPP_SERVICE_CCAPI_EXECUTION_MANAGEMENT_SERVICE_KUCOIN_FUTURES_H_
#ifdef CCAPI_ENABLE_SERVICE_EXECUTION_MANAGEMENT
#ifdef CCAPI_ENABLE_EXCHANGE_KUCOIN_FUTURES
#include "ccapi_cpp/service/ccapi_execution_management_service_kucoin_base.h"

namespace ccapi {

class ExecutionManagementServiceKucoinFutures : public ExecutionManagementServiceKucoinBase {
 public:
  ExecutionManagementServiceKucoinFutures(std::function<void(Event&, Queue<Event>*)> eventHandler, SessionOptions sessionOptions, SessionConfigs sessionConfigs,
                                          ServiceContextPtr serviceContextPtr)
      : ExecutionManagementServiceKucoinBase(eventHandler, sessionOptions, sessionConfigs, serviceContextPtr) {
    this->exchangeName = CCAPI_EXCHANGE_NAME_KUCOIN_FUTURES;
    this->baseUrlRest = sessionConfigs.getUrlRestBase().at(this->exchangeName);
    this->setHostRestFromUrlRest(this->baseUrlRest);
    this->apiKeyName = CCAPI_KUCOIN_FUTURES_API_KEY;
    this->apiSecretName = CCAPI_KUCOIN_FUTURES_API_SECRET;
    this->apiPassphraseName = CCAPI_KUCOIN_FUTURES_API_PASSPHRASE;
    this->setupCredential({this->apiKeyName, this->apiSecretName, this->apiPassphraseName});
    this->createOrderTarget = "/api/v1/orders";
    this->cancelOrderTarget = "/api/v1/orders/<id>";
    this->getOrderTarget = "/api/v1/orders/<id>";
    this->getOrderByClientOrderIdTarget = "/api/v1/orders/byClientOid?clientOid=<id>";
    this->getOpenOrdersTarget = "/api/v1/orders";
    this->cancelOpenOrdersTarget = "/api/v1/orders";
    this->getAccountBalancesTarget = "/api/v1/account-overview";
    this->getAccountPositionsTarget = "/api/v1/positions";
    this->topicTradeOrders = "/contractMarket/tradeOrders";
    this->isDerivatives = true;
  }

  virtual ~ExecutionManagementServiceKucoinFutures() {}
#ifndef CCAPI_EXPOSE_INTERNAL

 protected:
#endif
  void extractAccountInfoFromRequest(std::vector<Element>& elementList, const Request& request, const Request::Operation operation,
                                     const rj::Document& document) override {
    const auto& data = document["data"];
    switch (request.getOperation()) {
      case Request::Operation::GET_ACCOUNT_BALANCES: {
        Element element;
        element.insert(CCAPI_EM_ASSET, data["currency"].GetString());
        element.insert(CCAPI_EM_QUANTITY_TOTAL, data["accountEquity"].GetString());
        element.insert(CCAPI_EM_QUANTITY_AVAILABLE_FOR_TRADING, data["availableBalance"].GetString());
        elementList.emplace_back(std::move(element));
      } break;
      case Request::Operation::GET_ACCOUNT_POSITIONS: {
        for (const auto& x : data.GetArray()) {
          Element element;
          element.insert(CCAPI_INSTRUMENT, x["symbol"].GetString());
          element.insert(CCAPI_SETTLE_ASSET, x["settleCurrency"].GetString());
          element.insert(CCAPI_EM_POSITION_QUANTITY, x["currentQty"].GetString());
          element.insert(CCAPI_EM_POSITION_COST, x["posCost"].GetString());
          element.insert(CCAPI_EM_POSITION_ENTRY_PRICE, x["avgEntryPrice"].GetString());
          element.insert(CCAPI_EM_POSITION_LEVERAGE, x["realLeverage"].GetString());
          elementList.emplace_back(std::move(element));
        }
      } break;
      default:
        CCAPI_LOGGER_FATAL(CCAPI_UNSUPPORTED_VALUE);
    }
  }
  void extractOrderInfo(Element& element, const rj::Value& x,
                        const std::map<std::string_view, std::pair<std::string_view, JsonDataType>>& extractionFieldNameMap,
                        const std::map<std::string_view, std::function<std::string(const std::string&)>> conversionMap = {}) override {
    ExecutionManagementServiceKucoinBase::extractOrderInfo(element, x, extractionFieldNameMap);
    {
      auto it1 = x.FindMember("value");
      auto it2 = x.FindMember("price");
      auto it3 = x.FindMember("size");
      auto it4 = x.FindMember("dealValue");
      if (it1 != x.MemberEnd() && it2 != x.MemberEnd() && it3 != x.MemberEnd() && it4 != x.MemberEnd()) {
        double value = std::stod(it1->value.GetString());
        double price = std::stod(it2->value.GetString());
        double size = std::stod(it3->value.GetString());
        double dealValue = std::stod(it4->value.GetString());
        double multiplier = value / price / size;
        element.insert(CCAPI_EM_ORDER_CUMULATIVE_FILLED_QUOTE_QUANTITY, std::to_string(dealValue / multiplier));
      }
    }
  }
};

} /* namespace ccapi */
#endif
#endif
#endif  // INCLUDE_CCAPI_CPP_SERVICE_CCAPI_EXECUTION_MANAGEMENT_SERVICE_KUCOIN_FUTURES_H_
