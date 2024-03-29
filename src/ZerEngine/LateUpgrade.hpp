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
        for (auto& info: infosAddEnt) {
            for (auto& arg: info.args) {
                free(arg.data);
            }
        }
        for (auto& info: infosAddComp) {
            free(info.args.data);
        }
    }

    template <typename... Args>
    inline Ent newEnt(Registry& reg, const Args&... args) noexcept {
        const std::lock_guard<std::mutex> lock(mtx);
        Ent ent = reg.getEntToken();
        auto& infoAddEnt = infosAddEnt.emplace_back(ent);
        if constexpr (sizeof...(Args) > 0) {
            reg.fillDestructorsRec<Args...>();
            (new (infoAddEnt.args.emplace_back(
                typeid(Args).hash_code(),
                sizeof(Args),
                malloc(sizeof(Args))
            ).data) Args(args), ...);
        }
        return ent;
    }

    template <typename Arg>
    inline void add(Registry& reg, const Ent ent, const Arg& arg) noexcept {
        const std::lock_guard<std::mutex> lock(mtx);
        reg.fillDestructorsRec<Arg>();
        new (infosAddComp.emplace_back(
            ent,
            LateUpgradeAddData{
                typeid(Arg).hash_code(),
                sizeof(Arg),
                malloc(sizeof(Arg))
            }
        ).args.data) Arg(arg);
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
            reg.newEnt(info.ent, info.args);
            for (auto& arg: info.args) {
                free(arg.data);
            }
        }

        for (auto& info: infosDelComp) {
            reg.del(info.ent, info.args);
        }
        
        for (auto& info: infosAddComp) {
            reg.add(info.ent, info.args);
            free(info.args.data);
        }
 
        infosAddEnt.clear();
        infosAddComp.clear();
        infosDelComp.clear();
        infosDelEnt.clear();
    }

private:
    std::vector<LateUpgradeAddEntInfo> infosAddEnt;
    std::vector<LateUpgradeAddInfo> infosAddComp;
    std::vector<LateUpgradeDelEntInfo> infosDelEnt;
    std::vector<LateUpgradeDelCompInfo> infosDelComp;
    std::mutex mtx;
};