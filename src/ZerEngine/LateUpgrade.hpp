#pragma once

#include <vector>
#include <mutex>
#include "Utils.hpp"
#include "Registry.hpp"

struct LateUpgradeAddEntInfo final {
    const Ent ent;
    std::vector<LateUpgradeAddData> args;

    inline LateUpgradeAddEntInfo(const Ent newEnt = 0, const std::vector<LateUpgradeAddData>& newArgs = {}) noexcept:
        ent(newEnt),
        args(newArgs) {
    }
};

struct LateUpgradeAddInfo final {
    const Ent ent;
    LateUpgradeAddData args;

    inline LateUpgradeAddInfo(const Ent newEnt, const LateUpgradeAddData& newArgs) noexcept:
        ent(newEnt),
        args(newArgs) {
    }
};
 
struct LateUpgradeDelCompInfo final {
    const Ent ent;
    LateUpgradeDelCompData args;

    inline LateUpgradeDelCompInfo(const Ent newEnt, const LateUpgradeDelCompData& newArgs) noexcept:
        ent(newEnt),
        args(newArgs) {
    }
}; 

struct LateUpgradeDelEntInfo final {
    const Ent ent;

    constexpr LateUpgradeDelEntInfo(const Ent newEnt) noexcept:
        ent(newEnt) {
    }
};

class LateUpgrade final {
public:
    ~LateUpgrade() noexcept {
        for (auto* obj: objTransfert) {
            free(obj);
        }
    }

    template <typename... Args>
    inline Ent newEnt(Registry& reg, const Args&... args) noexcept {
        const std::lock_guard<std::mutex> lock(mtx);
        Ent ent = reg.getEntToken();
        if constexpr (sizeof...(Args) > 0) {
            reg.fillDestructorsRec<Args...>();
        }
        infosAddEnt.emplace_back(ent);
        if constexpr (sizeof...(Args) > 0) {
            std::size_t size = rowSize<Args...>();
            newEntRec(infosAddEnt.back(), args...);
        }
        return ent;
    }

    template <typename Arg>
    inline void add(Registry& reg, const Ent ent, const Arg& arg) noexcept {
        const std::lock_guard<std::mutex> lock(mtx);
        reg.fillDestructorsRec<Arg>();
        void* buffer = malloc(sizeof(Arg));
        objTransfert.push_back(buffer);
        new (buffer) Arg(arg);
        infosAddComp.emplace_back(
            ent,
            LateUpgradeAddData{
                typeid(Arg).hash_code(),
                sizeof(Arg),
                buffer
            }
        );
    }

    template <typename Arg>
    inline void del(const Ent ent) noexcept {
        const std::lock_guard<std::mutex> lock(mtx);
        infosDelComp.emplace_back(
            ent,
            LateUpgradeDelCompData(
                typeid(Arg).hash_code(),
                sizeof(Arg)
            )
        );
    }

    inline void destroy(const Ent ent) noexcept {
        const std::lock_guard<std::mutex> lock(mtx);
        infosDelEnt.emplace_back(
            ent
        );
    }

    inline void upgrade(Registry& reg) noexcept {
        for (auto& info: infosDelEnt) {
            reg.destroy(info.ent);
        }

        for (auto& info: infosAddEnt) {
            reg.newEntLate(info.ent, info.args);
        }

        for (auto& info: infosDelComp) {
            reg.delLate(info.ent, info.args);
        }
        
        for (auto& info: infosAddComp) {
            reg.addLate(info.ent, info.args);
        }

        infosAddEnt.clear();
        infosAddComp.clear();
        infosDelComp.clear();
        infosDelEnt.clear();

        for (auto* obj: objTransfert) {
            free(obj);
        }
        objTransfert.clear();
    }

private:
    template <typename Arg, typename... Args>
    [[nodiscard]] static consteval std::size_t rowSize() {
        if constexpr (sizeof...(Args) > 0) {
            return sizeof(Arg) + rowSize<Args...>();
        }
        return sizeof(Arg);
    }

    template <typename Arg, typename... Args>
    constexpr void newEntRec(LateUpgradeAddEntInfo& info, const Arg& arg, const Args&... args) {
        void* buffer = malloc(sizeof(Arg));
        new (buffer) Arg(arg);
        objTransfert.push_back(buffer);
        info.args.emplace_back(
            typeid(Arg).hash_code(),
            sizeof(Arg),
            buffer
        );
        if constexpr (sizeof...(Args) > 0) {
            newEntRec(info, args...);
        }
    }

private:
    std::vector<LateUpgradeAddEntInfo> infosAddEnt;
    std::vector<LateUpgradeAddInfo> infosAddComp;
    std::vector<LateUpgradeDelEntInfo> infosDelEnt;
    std::vector<LateUpgradeDelCompInfo> infosDelComp;
    std::vector<void*> objTransfert;
    std::mutex mtx;
};