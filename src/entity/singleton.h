#ifndef ENTITY_SINGLETON_H_
#define ENTITY_SINGLETON_H_

namespace entity {

template<typename T, typename ErrT>
class Singleton {
public:
  static auto Init() {
    static T* instance_ptr = nullptr;
    if (instance_ptr == nullptr) {
      instance_ptr = new T;
      return typename T::Handle(instance_ptr);
    }
    throw ErrT("instance already created");
  }
};

} // namespace entity

#endif // ENTITY_SINGLETON_H_
