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
            ~LateUpgrade() noexcept {
                for (auto* chunk: transfertChunk) {
                    free(chunk);
                }
            }

            template <typename... Args>
            inline void newEnt(const Args&... args) noexcept {
                const lock_guard<std::mutex> lock(mtx);
                infosAddEnt.emplace_back();
                size_t size = rowSize<Args...>();
                if (transfertChunk.empty() || transfertOffset + size > pageSize) {
                    transfertOffset = 0;
                    transfertChunk.emplace_back(aligned_alloc(pageSize, pageSize));
                }
                newEntRec(infosAddEnt.back(), transfertChunk.back(), transfertOffset, std::forward<const Args>(args)...);
                transfertOffset += size;
            }

            template <typename Arg>
            inline void add(Ent id, const Arg& arg) noexcept {
                const lock_guard<std::mutex> lock(mtx);
                if (transfertChunk.empty() || transfertOffset + sizeof(Arg) > pageSize) {
                    transfertOffset = 0;
                    transfertChunk.emplace_back(aligned_alloc(pageSize, pageSize));
                }
                void* buffer = transfertChunk.back();
                memcpy(
                    static_cast<byte*>(buffer) + transfertOffset,
                    static_cast<const void*>(&arg),
                    sizeof(Arg)
                );
                infosAddComp.emplace_back(
                    id,
                    std::vector<LateUpgradeData>{
                        {
                            typeid(Arg).hash_code(),
                            sizeof(Arg),
                            buffer,
                            transfertOffset
                        }
                    }
                );
                transfertOffset += sizeof(Arg);
            }

            template <typename Arg>
            inline void del(Ent id) noexcept {
                const lock_guard<std::mutex> lock(mtx);
                infosDelComp.emplace_back(
                    id,
                    std::vector<LateUpgradeDelCompData>{
                        {
                            typeid(Arg).hash_code(),
                            sizeof(Arg)
                        }
                    }
                );
            }

            inline void destroy(Ent id) noexcept {
                const lock_guard<std::mutex> lock(mtx);
                infosDelEnt.emplace_back(
                    id
                );
            }

            inline void upgrade(Registry& reg) noexcept {
                for (auto& info: infosAddEnt) {
                    reg.newEnt(info.args);
                }

                for (auto& info: infosAddComp) {
                    reg.add(info.ent, info.args);
                }

                for (auto& info: infosDelComp) {
                    reg.del(info.ent, info.args);
                }

                for (auto& info: infosDelEnt) {
                    reg.destroy(info.ent);
                }
                infosAddEnt.clear();
                infosAddComp.clear();
                infosDelComp.clear();
                infosDelEnt.clear();

                for (auto* chunk: transfertChunk) {
                    free(chunk);
                }
                transfertChunk.clear();
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
            constexpr void newEntRec(LateUpgradeInfo& info, void* buffer, size_t curOffset, const Arg& arg, const Args&... args) {
                memcpy(
                    static_cast<byte*>(buffer) + curOffset,
                    static_cast<const void*>(&arg),
                    sizeof(Arg)
                );
                info.args.emplace_back(
                    typeid(Arg).hash_code(),
                    sizeof(Arg),
                    buffer,
                    curOffset
                );
                if constexpr (sizeof...(Args) > 0) {
                    newEntRec(info, buffer, curOffset + sizeof(Arg), std::forward<const Args>(args)...);
                }
            }

        private:
            std::vector<LateUpgradeInfo> infosAddEnt;
            std::vector<LateUpgradeInfo> infosAddComp;
            std::vector<LateUpgradeDelEntInfo> infosDelEnt;
            std::vector<LateUpgradeDelCompInfo> infosDelComp;
            std::vector<void*> transfertChunk;
            std::mutex mtx;
            size_t transfertOffset;
        };
    }
}