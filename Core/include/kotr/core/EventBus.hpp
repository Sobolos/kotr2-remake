#pragma once
#include <functional>
#include <memory>
#include <typeindex>
#include <unordered_map>
#include <vector>
#include <algorithm>

namespace kotr::core {

/// Центральная шина событий Core Layer.
/// Модули подписываются на события и публикуют их без прямых зависимостей.
class EventBus {
public:
    using HandlerId = uint64_t;

    /// Подписаться на событие типа EventType.
    /// Возвращает ID подписки для последующей отписки.
    template<typename EventType>
    HandlerId subscribe(std::function<void(const EventType&)> handler) {
        auto id = nextHandlerId_++;
        auto key = std::type_index(typeid(EventType));
        handlers_[key].push_back(
            HandlerEntry{id, std::make_shared<HandlerImpl<EventType>>(std::move(handler))}
        );
        return id;
    }

    /// Отписаться по ID подписки.
    template<typename EventType>
    void unsubscribe(HandlerId id) {
        auto key = std::type_index(typeid(EventType));
        auto it = handlers_.find(key);
        if (it == handlers_.end()) return;

        auto& vec = it->second;
        vec.erase(
            std::remove_if(vec.begin(), vec.end(),
                [id](const HandlerEntry& e) { return e.id == id; }),
            vec.end()
        );
    }

    /// Опубликовать событие. Все подписчики получат его синхронно.
    template<typename EventType>
    void publish(const EventType& event) {
        auto key = std::type_index(typeid(EventType));
        auto it = handlers_.find(key);
        if (it == handlers_.end()) return;

        // Копируем список на случай, если handler отпишется во время вызова
        auto snapshot = it->second;
        for (auto& entry : snapshot) {
            static_cast<HandlerImpl<EventType>*>(entry.handler.get())->call(event);
        }
    }

    /// Проверить, есть ли подписчики на тип.
    template<typename EventType>
    [[nodiscard]] bool hasSubscribers() const {
        auto key = std::type_index(typeid(EventType));
        auto it = handlers_.find(key);
        return it != handlers_.end() && !it->second.empty();
    }

private:
    struct IHandler {
        virtual ~IHandler() = default;
    };

    template<typename EventType>
    struct HandlerImpl : IHandler {
        std::function<void(const EventType&)> func;
        explicit HandlerImpl(std::function<void(const EventType&)> f)
            : func(std::move(f)) {}
        void call(const EventType& e) { if (func) func(e); }
    };

    struct HandlerEntry {
        HandlerId id;
        std::shared_ptr<IHandler> handler;
    };

    std::unordered_map<std::type_index, std::vector<HandlerEntry>> handlers_;
    HandlerId nextHandlerId_ = 1;
};

} // namespace kotr::core