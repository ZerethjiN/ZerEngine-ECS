#pragma once

#include "../Registry/Registry.hpp"
#include <vector>
#include <cstdlib>

namespace zre {
    namespace priv {
        struct LateUpgradeInfo {
            const Ent ent;
            std::vector<LateUpgradeData> args;

            inline LateUpgradeInfo(const Ent newEnt = 0, const std::vector<LateUpgradeData>& newArgs = {}) noexcept:
                ent(newEnt),
                args(newArgs) {
            }
        };

        struct LateUpgradeDelCompInfo {
            const Ent ent;
            std::vector<LateUpgradeDelCompData> args;

            inline LateUpgradeDelCompInfo(const Ent newEnt, const std::vector<LateUpgradeDelCompData>& newArgs) noexcept:
                ent(newEnt),
                args(newArgs) {
            }
        }; 

        struct LateUpgradeDelEntInfo {
            const Ent ent;

            constexpr LateUpgradeDelEntInfo(const Ent newEnt) noexcept:
                ent(newEnt) {
            }
        };

        class LateUpgrade {
        public:
            LateUpgrade(Registry& newReg):
                reg(newReg) {
            }

            ~LateUpgrade() noexcept {
                for (auto* obj: objTransfert) {
                    free(obj);
                }
            }

            template <typename... Args>
            inline Ent newEnt(Args&&... args) noexcept {
                const lock_guard<std::mutex> lock(mtx);
                Ent ent = reg.getEntToken();
                if constexpr (sizeof...(Args) > 0) {
                    reg.fillCompPoolRec<Args...>();
                }
                infosAddEnt.emplace_back(ent);
                if constexpr (sizeof...(Args) > 0) {
                    size_t size = rowSize<Args...>();
                    newEntRec(infosAddEnt.back(), transfertOffset, std::forward<Args>(args)...);
                    transfertOffset += size;
                }
                return ent;
            }

            template <typename Arg>
            inline void add(const Ent ent, Arg&& arg) noexcept {
                const lock_guard<std::mutex> lock(mtx);
                reg.fillCompPoolRec<Arg>();
                void* buffer = malloc(sizeof(Arg));
                objTransfert.push_back(buffer);
                new (buffer) Arg(std::forward<Arg>(arg));
                infosAddComp.emplace_back(
                    ent,
                    std::vector<LateUpgradeData>{
                        {
                            typeid(Arg).hash_code(),
                            sizeof(Arg),
                            buffer,
                            0
                        }
                    }
                );
                transfertOffset += sizeof(Arg);
            }

            template <typename T, typename... Args>
            inline void emplace(const Ent ent, Args&&... args) noexcept {
                const lock_guard<std::mutex> lock(mtx);
                reg.fillCompPoolRec<T>();
                void* buffer = malloc(sizeof(T));
                objTransfert.push_back(buffer);
                new (buffer) T{std::forward<Args>(args)...};
                infosAddComp.emplace_back(
                    ent,
                    std::vector<LateUpgradeData>{
                        {
                            typeid(T).hash_code(),
                            sizeof(T),
                            buffer,
                            0
                        }
                    }
                );
                transfertOffset += sizeof(T);
            }

            template <typename Arg>
            inline void del(const Ent ent) noexcept {
                const lock_guard<std::mutex> lock(mtx);
                infosDelComp.emplace_back(
                    ent,
                    std::vector<LateUpgradeDelCompData>{
                        {
                            typeid(Arg).hash_code(),
                            sizeof(Arg)
                        }
                    }
                );
            }

            inline void destroy(const Ent ent) noexcept {
                const lock_guard<std::mutex> lock(mtx);
                infosDelEnt.emplace_back(
                    ent
                );
            }

            inline void upgrade() noexcept {
                for (auto& info: infosDelEnt) {
                    reg.destroy(info.ent);
                }

                for (auto& info: infosAddEnt) {
                    reg.newEnt(info.ent, info.args);
                }

                for (auto& info: infosDelComp) {
                    reg.del(info.ent, info.args);
                }
                
                for (auto& info: infosAddComp) {
                    reg.add(info.ent, info.args);
                }
                
                infosAddEnt.clear();
                infosAddComp.clear();
                infosDelComp.clear();
                infosDelEnt.clear();

                for (auto* obj: objTransfert) {
                    free(obj);
                }
                objTransfert.clear();
                transfertOffset = 0;
            }

        private:
            template <typename Arg, typename... Args>
            [[nodiscard]] constexpr size_t rowSize() {
                size_t size = sizeof(Arg);
                if constexpr (sizeof...(Args) > 0) {
                    size += rowSize<Args...>();
                }
                return size;
            }

            template <typename Arg, typename... Args>
            constexpr void newEntRec(LateUpgradeInfo& info, size_t curOffset, Arg&& arg, Args&&... args) {
                void* buffer = malloc(sizeof(Arg));
                new (buffer) Arg(std::forward<Arg>(arg));
                objTransfert.push_back(buffer);
                info.args.emplace_back(
                    typeid(Arg).hash_code(),
                    sizeof(Arg),
                    buffer,
                    0
                );
                if constexpr (sizeof...(Args) > 0) {
                    newEntRec(info, curOffset + sizeof(Arg), std::forward<Args>(args)...);
                }
            }

        private:
            std::vector<LateUpgradeInfo> infosAddEnt;
            std::vector<LateUpgradeInfo> infosAddComp;
            std::vector<LateUpgradeDelEntInfo> infosDelEnt;
            std::vector<LateUpgradeDelCompInfo> infosDelComp;
            std::vector<void*> objTransfert;
            std::mutex mtx;
            size_t transfertOffset;
            Registry& reg;
        };
    }
}