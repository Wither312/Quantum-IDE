#pragma once

#include <unordered_map>
#include <functional>
#include <vector>
#include <typeindex>
#include <memory>

namespace core {

    class EventBus {
    public:
        template<typename EventType>
        void subscribe(std::function<void(const EventType&)> handler);

        template<typename EventType>
        void publish(const EventType& event) const;

    private:
        using HandlerBase = std::function<void(const void*)>;
        std::unordered_map<std::type_index, std::vector<HandlerBase>> m_handlers;
    };

} // namespace core
