#include <functional>
#include <map>
#include <string>
#include <iostream>
#include <variant>

class VibeCastingInterface;
using SendDeviceInfo = std::string;
using ErrorCode = uint64_t;

namespace {
enum class EventType {
  EVENT_STOP_SHARE,
  EVENT_ON_ERROR,
};

struct default_msg {};
struct on_device_connect {};
struct remote_device_info {
  SendDeviceInfo device_info;
};
struct on_error {
  ErrorCode error_code;
  std::string error_info;
};

using DisptchMessage =
    std::variant<on_device_connect, remote_device_info, on_error, default_msg>;

template <typename... Ts>
class VibeCastingEventDispatcherImp;

typedef std::function<void(VibeCastingInterface*, DisptchMessage message)>
    EventDispatcherCallback;

template <typename T, typename... Ts>
class VibeCastingEventDispatcherImp<T, Ts...>
    : public VibeCastingEventDispatcherImp<Ts...> {
 private:
  std::map<EventType, std::vector<EventDispatcherCallback>> callback_maps;
  std::function<void(VibeCastingInterface*, const T&)> msg_callback;

 public:
  void registerCallback(EventType eventType, EventDispatcherCallback callback) {
    callback_maps[eventType].push_back(callback);
  }

  void dispatchEvent(VibeCastingInterface* instance,
                     EventType eventType,
                     std::string info = {}) {
    for (auto callback : callback_maps[eventType]) {
      callback(instance, default_msg{});
    }
  }

  using VibeCastingEventDispatcherImp<Ts...>::registerHandler;
  void registerHandler(
      std::function<void(VibeCastingInterface*, const T&)> callback) {
    msg_callback = std::move(callback);
  }

  using VibeCastingEventDispatcherImp<Ts...>::OnMessage;
  void OnMessage(VibeCastingInterface* instance, T message) {
    msg_callback(instance, std::forward<T>(message));
  }
};

template <>
class VibeCastingEventDispatcherImp<> {
 protected:
  void registerCallback(EventType eventType,
                        std::function<void(VibeCastingInterface*)> callback) {}
  void dispatchEvent(VibeCastingInterface* instance,
                     EventType eventType,
                     std::string info = {}) {}
  void registerHandler(std::function<void(VibeCastingInterface*)> callback) {}
  void OnMessage() {}
};

using EventDispatcher =
    VibeCastingEventDispatcherImp<on_device_connect,
                                  remote_device_info,
                                  on_error,
                                  default_msg>;
}

int main() {
  EventDispatcher eventDispatcher_;
  eventDispatcher_.registerHandler([] (VibeCastingInterface* instance, const on_device_connect&) {
    std::cout << "Recv on_device_connect from interface: " << instance << std::endl;
  });

  eventDispatcher_.registerHandler([] (VibeCastingInterface* instance, const remote_device_info& remote_device_info) {
    std::cout << "Recv remote_device_info from interface: " << instance << std::endl;
  });

  eventDispatcher_.registerHandler([] (VibeCastingInterface* instance, const on_error& message) {
    std::cout << "Recv on_error from interface: " << instance << std::endl;
  });

  return 0;
}
