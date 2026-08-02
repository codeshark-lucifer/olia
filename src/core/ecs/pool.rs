use crate::core::ecs::entity::Entity;

#[allow(dead_code)]
pub struct ComponentPool<T> {
    dense: Vec<Entity>,
    data: Vec<T>,
    sparse: Vec<Option<usize>>,
}

#[allow(dead_code)]
impl<T> ComponentPool<T> {
    pub fn new() -> Self {
        Self {
            dense: Vec::new(),
            data: Vec::new(),
            sparse: Vec::new(),
        }
    }

    pub fn insert(&mut self, entity: Entity, component: T) {
        let idx = entity.index as usize;
        if idx >= self.sparse.len() {
            self.sparse.resize(idx + 1, None);
        }

        if let Some(dense_idx) = self.sparse[idx] {
            self.data[dense_idx] = component;
        } else {
            let dense_idx = self.data.len();
            self.sparse[idx] = Some(dense_idx);
            self.dense.push(entity);
            self.data.push(component);
        }
    }

    pub fn remove(&mut self, entity: Entity) -> Option<T> {
        let idx = entity.index as usize;
        let dense_idx = (*self.sparse.get(idx)?)?;

        // Swap with last element for O(1) removal
        let last_dense_idx = self.data.len() - 1;
        self.data.swap(dense_idx, last_dense_idx);
        self.dense.swap(dense_idx, last_dense_idx);

        // Update sparse index of the swapped entity
        let swapped_entity = self.dense[dense_idx];
        self.sparse[swapped_entity.index as usize] = Some(dense_idx);

        // Clear sparse entry for removed entity
        self.sparse[idx] = None;

        self.dense.pop();
        self.data.pop()
    }

    pub fn get(&self, entity: Entity) -> Option<&T> {
        let idx = entity.index as usize;
        let dense_idx = (*self.sparse.get(idx)?)?;
        Some(&self.data[dense_idx])
    }

    pub fn get_mut(&mut self, entity: Entity) -> Option<&mut T> {
        let idx = entity.index as usize;
        let dense_idx = (*self.sparse.get(idx)?)?;
        Some(&mut self.data[dense_idx])
    }

    pub fn iter_mut(&mut self) -> impl Iterator<Item = (&Entity, &mut T)> {
        self.dense.iter().zip(self.data.iter_mut())
    }

    pub fn len(&self) -> usize {
        self.data.len()
    }

    pub fn is_empty(&self) -> bool {
        self.data.is_empty()
    }
}