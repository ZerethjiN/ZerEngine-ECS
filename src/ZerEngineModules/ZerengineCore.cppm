module;

export module ZerengineCore;
export import :ZerEngine;
export import :World;
export import :Utils;

///////////////////////////////////////////////////////////////////////////////////

// class Comp {
// public:
//     constexpr Comp() noexcept:
//         manager(nullptr) {
//     }

//     Comp(const Comp& othComp) {
//         if (!othComp.has_value()) {
//             manager = nullptr;
//         } else {
//             Arg arg;
//             arg.comp = this;
//             othComp.manager(OP_CLONE, &othComp, &arg);
//         }
//     }

//     Comp(Comp&& othComp) {
//         if (!othComp.has_value()) {
//             manager = nullptr;
//         } else {
//             Arg arg;
//             arg.comp = this;
//             othComp.manager(OP_TRANSFERT, &othComp, &arg);
//         }
//     }

//     template <typename T> requires (std::copy_constructible<T> && !std::same_as<T, Comp>)
//     Comp(T&& newComp):
//         data(new T(std::forward<T>(newComp))),
//         manager(managerFunc<T>) {
//     }

//     template <typename T, typename... Args>
//     Comp(std::in_place_type_t<T>, Args&&... args):
//         data(new T(std::forward<Args>(args)...)),
//         manager(managerFunc<T>) {
//     }

//     ~Comp() {
//         reset();
//     }

//     Comp& operator =(const Comp& othComp) {
//         *this = Comp(othComp);
//         return *this;
//     }

//     Comp& operator =(Comp&& othComp) {
//         if (!othComp.has_value()) {
//             reset();
//         } else if (this != &othComp) {
//             reset();
//             Arg arg;
//             arg.comp = this;
//             othComp.manager(OP_TRANSFERT, &othComp, &arg);
//         }
//         return *this;
//     }

//     template <typename T>
//     Comp& operator =(T&& othComp) {
//         *this = Comp(std::forward<T>(othComp));
//         return *this;
//     }

// public:
//     void reset() noexcept {
//         if (has_value()) {
//             manager(OP_DESTROY, this, nullptr);
//             manager = nullptr;
//         }
//     }

//     const std::type_info& type() const noexcept {
//         if (!has_value()) {
//             return typeid(void);
//         }
//         Arg arg;
//         manager(OP_GET_TYPE, this, &arg);
//         return *arg.typeInfo;
//     }

//     [[nodiscard]] bool has_value() const noexcept {
//         return manager != nullptr;
//     }

// public:
//     template <typename T>
//     friend T* compCast(const Comp* comp) noexcept {
//         if (comp && comp->type() == typeid(T)) {
//             return static_cast<T*>(comp->data);
//         }
//         printf("CompCast Impossible: Comp<%s> voulu en %s", comp->type().name(), typeid(T).name());
//         return nullptr;
//     }

// private:
//     enum Op {
//         OP_ACCESS, OP_GET_TYPE, OP_CLONE, OP_DESTROY, OP_TRANSFERT
//     };

//     union Arg {
//         void* obj;
//         const std::type_info* typeInfo;
//         Comp* comp;
//     };

// private:
//     template <typename T>
//     static void managerFunc(Op op, const Comp* comp, Arg* arg) {
//         const T* ptr = static_cast<const T*>(comp->data);
//         switch (op) {
//             case OP_ACCESS:
//                 arg->obj = const_cast<T*>(ptr);
//                 break;
//             case OP_GET_TYPE:
//                 arg->typeInfo = &typeid(T);
//                 break;
//             case OP_CLONE:
//                 arg->comp->data = new T(*ptr);
//                 arg->comp->manager = comp->manager;
//                 break;
//             case OP_DESTROY:
//                 delete ptr;
//                 break;
//             case OP_TRANSFERT:
//                 arg->comp->data = comp->data;
//                 arg->comp->manager = comp->manager;
//                 const_cast<Comp*>(comp)->manager = nullptr;
//                 break;
//         }
//     }

// private:
//     void* data;
//     void(*manager)(Op, const Comp*, Arg*);
// };
