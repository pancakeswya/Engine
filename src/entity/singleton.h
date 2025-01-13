#ifndef ENTITY_SINGLETON_H_
#define ENTITY_SINGLETON_H_

namespace entity {

template<typename T>
class Singleton {
public:
  static T* GetInstance() {
    static T* instance;
    if (instance == nullptr) {
      instance = new T();
    }
    return instance;
  }

  static void Destroy(T* instance) {
    delete instance;
  }
};

} // namespace entity

#endif // ENTITY_SINGLETON_H_
