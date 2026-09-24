#pragma once

#include <vector>
#include <cstdint>
#include <algorithm>
#include <cassert>

#include "./entity.h"

namespace ECS
{
    template <typename T>
    class Allocator
    {
    private:
        std::vector<T> dense_components_;
        std::vector<Entity> dense_entities_;
        std::vector<size_t> sparse_indices_;

    public:
        Allocator() = default;
        ~Allocator() = default;

        void insert(Entity entity, T component)
        {
            if (entity >= sparse_indices_.size())
            {
                sparse_indices_.resize(entity + 1, NULL_ENTITY);
            }

            if (has(entity))
            {
                dense_components_[sparse_indices_[entity]] = std::move(component);
                return;
            }

            size_t dense_index = dense_components_.size();
            sparse_indices_[entity] = dense_index;

            dense_components_.push_back(std::move(component));
            dense_entities_.push_back(entity);
        }

        void remove(Entity entity)
        {
            if (!has(entity))
                return;

            size_t idx_to_remove = sparse_indices_[entity];
            size_t last_idx = dense_components_.size() - 1;

            if (idx_to_remove != last_idx)
            {
                dense_components_[idx_to_remove] = std::move(dense_components_[last_idx]);
                dense_entities_[idx_to_remove] = dense_entities_[last_idx];

                sparse_indices_[dense_entities_[idx_to_remove]] = idx_to_remove;
            }

            dense_components_.pop_back();
            dense_entities_.pop_back();

            sparse_indices_[entity] = NULL_ENTITY;
        }

        bool has(Entity entity)
        {
            return entity < sparse_indices_.size() && sparse_indices_[entity] != NULL_ENTITY;
        }

        T *get(Entity entity)
        {
            if (!has(entity))
                return nullptr;
            return &dense_components_[sparse_indices_[entity]];
        }

        const T *get(Entity entity) const
        {
            if (!has(entity))
                return nullptr;
            return &dense_components_[sparse_indices_[entity]];
        }

        size_t size() const { return dense_components_.size(); }

        auto begin() { return dense_components_.begin(); }
        auto end() { return dense_components_.end(); }

        auto begin() const { return dense_components_.begin(); }
        auto end() const { return dense_components_.end(); }

        bool is_empty() const { return dense_components_.empty(); }

        Entity get_entity_at(size_t dense_index) const
        {
            assert(dense_index < dense_entities_.size());
            return dense_entities_[dense_index];
        }
    };
}