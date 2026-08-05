use std::fmt;

#[derive(Copy, Clone, Debug, PartialEq, Eq, Hash)]
pub struct Entity {
    pub index: u32,
    pub generation: u32,
}

impl fmt::Display for Entity {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        write!(f, "Entity(idx: {}, gen: {})", self.index, self.generation)
    }
}

pub struct EntityAllocator {
    generations: Vec<u32>,
    free_list: Vec<u32>,
}

#[allow(dead_code)]
impl EntityAllocator {
    pub fn new() -> Self {
        Self {
            generations: Vec::new(),
            free_list: Vec::new(),
        }
    }

    pub fn spawn(&mut self) -> Entity {
        if let Some(index) = self.free_list.pop() {
            Entity {
                index,
                generation: self.generations[index as usize],
            }
        } else {
            let index = self.generations.len() as u32;
            self.generations.push(0);
            Entity {
                index,
                generation: 0,
            }
        }
    }

    pub fn despawn(&mut self, entity: Entity) -> bool {
        if !self.is_alive(entity) {
            return false;
        }
        self.generations[entity.index as usize] += 1;
        self.free_list.push(entity.index);
        true
    }

    pub fn is_alive(&self, entity: Entity) -> bool {
        self.generations
            .get(entity.index as usize)
            .map_or(false, |&g| g == entity.generation)
    }
}
