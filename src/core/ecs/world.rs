use std::any::{Any, TypeId};
use std::cell::{Ref, RefCell, RefMut, UnsafeCell};
use std::collections::HashMap;

use crate::core::ecs::entity::{Entity, EntityAllocator};
use crate::core::ecs::pool::ComponentPool;

pub struct World {
    pub entities: EntityAllocator,
    // UnsafeCell allows us to fetch/insert pools via &self
    pools: UnsafeCell<HashMap<TypeId, Box<RefCell<Box<dyn Any>>>>>,
}

#[allow(dead_code)]
impl World {
    pub fn new() -> Self {
        Self {
            entities: EntityAllocator::new(),
            pools: UnsafeCell::new(HashMap::new()),
        }
    }

    /// Takes `&self` so you can borrow multiple pools simultaneously
    pub fn pool_mut<T: 'static>(&self) -> RefMut<'_, ComponentPool<T>> {
        let type_id = TypeId::of::<T>();

        // SAFETY: Each pool is heap-allocated in a Box. Inserting new pools into
        // the HashMap will not move existing Box addresses in memory.
        let pools = unsafe { &mut *self.pools.get() };

        let pool_box = pools
            .entry(type_id)
            .or_insert_with(|| Box::new(RefCell::new(Box::new(ComponentPool::<T>::new()))));

        RefMut::map(pool_box.borrow_mut(), |boxed| {
            boxed
                .downcast_mut::<ComponentPool<T>>()
                .expect("Failed to downcast component pool")
        })
    }

    /// Read-only pool access
    pub fn pool<T: 'static>(&self) -> Option<Ref<'_, ComponentPool<T>>> {
        let type_id = TypeId::of::<T>();
        let pools = unsafe { &*self.pools.get() };
        let pool_box = pools.get(&type_id)?;

        Some(Ref::map(pool_box.borrow(), |boxed| {
            boxed
                .downcast_ref::<ComponentPool<T>>()
                .expect("Failed to downcast component pool")
        }))
    }

    /// Add a component using &self
    pub fn add_component<T: 'static>(&self, entity: Entity, component: T) {
        self.pool_mut::<T>().insert(entity, component);
    }

    /// Remove a component using &self
    pub fn remove_component<T: 'static>(&self, entity: Entity) -> Option<T> {
        self.pool_mut::<T>().remove(entity)
    }

    /// Despawn an entity
    pub fn despawn(&mut self, entity: Entity) -> bool {
        self.entities.despawn(entity)
    }
}
