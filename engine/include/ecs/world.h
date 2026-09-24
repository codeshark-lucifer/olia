#pragma once

#include <vector>
#include <memory>
#include <unordered_map>
#include <typeindex>
#include <cstdint>

#include "./entity.h"
#include "./allocator.h"

namespace ECS
{
    class World
    {
    private:
        Entity next_entity_id_{0};
        std::vector<Entity> free_entities_; // Simple recycling list for destroyed entities

        // Type-erased base interface for component pools
        struct IPool
        {
            virtual ~IPool() = default;
            virtual void remove(Entity entity) = 0;
        };

        // Concrete implementation holding the allocator for type T
        template <typename T>
        struct PoolWrapper : public IPool
        {
            Allocator<T> allocator;

            void remove(Entity entity) override
            {
                allocator.remove(entity);
            }
        };

        // Map containing all active component pools, keyed by their type_index
        std::unordered_map<std::type_index, std::unique_ptr<IPool>> pools_;

        // Helper to fetch or automatically create a pool for type T
        template <typename T>
        PoolWrapper<T>* get_or_create_pool()
        {
            std::type_index type_idx(typeid(T));
            auto it = pools_.find(type_idx);
            
            if (it == pools_.end())
            {
                auto pool = std::make_unique<PoolWrapper<T>>();
                auto* ptr = pool.get();
                pools_[type_idx] = std::move(pool);
                return ptr;
            }
            
            return static_cast<PoolWrapper<T>*>(it->second.get());
        }

        // Helper to fetch a pool without creating it (returns nullptr if it doesn't exist)
        template <typename T>
        PoolWrapper<T>* get_pool_internal() const
        {
            std::type_index type_idx(typeid(T));
            auto it = pools_.find(type_idx);
            
            if (it == pools_.end())
            {
                return nullptr;
            }
            
            return static_cast<PoolWrapper<T>*>(it->second.get());
        }

    public:
        World() = default;
        ~World() = default;

        // --- Entity Management ---

        Entity create_entity()
        {
            if (!free_entities_.empty())
            {
                Entity entity = free_entities_.back();
                free_entities_.pop_back();
                return entity;
            }
            return next_entity_id_++;
        }

        void destroy_entity(Entity entity)
        {
            // Automatically clean up this entity from ALL component pools
            for (auto& pair : pools_)
            {
                pair.second->remove(entity);
            }
            
            // Recycle the entity ID
            free_entities_.push_back(entity);
        }

        // --- Component Management ---

        template <typename T, typename... Args>
        T& add_component(Entity entity, Args&&... args)
        {
            auto* pool_wrapper = get_or_create_pool<T>();
            pool_wrapper->allocator.insert(entity, T(std::forward<Args>(args)...));
            return *pool_wrapper->allocator.get(entity);
        }

        template <typename T>
        void remove_component(Entity entity)
        {
            auto* pool_wrapper = get_pool_internal<T>();
            if (pool_wrapper)
            {
                pool_wrapper->allocator.remove(entity);
            }
        }

        template <typename T>
        bool has_component(Entity entity) const
        {
            auto* pool_wrapper = get_pool_internal<T>();
            if (!pool_wrapper) return false;
            return pool_wrapper->allocator.has(entity);
        }

        template <typename T>
        T* get_component(Entity entity)
        {
            auto* pool_wrapper = get_pool_internal<T>();
            if (!pool_wrapper) return nullptr;
            return pool_wrapper->allocator.get(entity);
        }

        template <typename T>
        const T* get_component(Entity entity) const
        {
            auto* pool_wrapper = get_pool_internal<T>();
            if (!pool_wrapper) return nullptr;
            return pool_wrapper->allocator.get(entity);
        }

        template <typename T>
        Entity find_component(Entity entity)
        {
            return has_component<T>(entity) ? entity : NULL_ENTITY;
        }

        template <typename T>
        Entity find_component(Entity entity) const
        {
            return has_component<T>(entity) ? entity : NULL_ENTITY;
        }

        template <typename T>
        Entity find_component() const
        {
            const auto entities = query<T>();
            return entities.empty() ? NULL_ENTITY : entities.back();
        }

        template <typename T>
        std::vector<Entity> query() const
        {
            const auto* pool_wrapper = get_pool_internal<T>();
            if (!pool_wrapper)
            {
                return {};
            }

            std::vector<Entity> entities;
            entities.reserve(pool_wrapper->allocator.size());
            for (size_t index = 0; index < pool_wrapper->allocator.size(); ++index)
            {
                entities.push_back(pool_wrapper->allocator.get_entity_at(index));
            }
            return entities;
        }

        // Direct access to a component allocator (useful for fast system loops)
        template <typename T>
        Allocator<T>& get_allocator()
        {
            return get_or_create_pool<T>()->allocator;
        }

        
    };
}

extern ECS::World* g_world;