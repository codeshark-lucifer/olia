pub mod components;
pub mod core;
pub mod graphics;
pub mod utils;

/// Prelude module for convenient imports in your game
pub mod prelude {
    pub use crate::components::camera::*;
    pub use crate::components::transform::*;
    pub use crate::core::ecs::*;
    pub use crate::core::window::*;
    pub use crate::graphics::mesh::*;
    pub use crate::graphics::shader::*;
    pub use crate::utils::mathf::*;
}