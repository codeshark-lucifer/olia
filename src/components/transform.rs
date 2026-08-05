use crate::utils::mathf::{Mat4, Vec3};

#[allow(dead_code)]
#[derive(Debug, Clone, Copy)]
pub struct Transform {
    pub position: Vec3,
    pub rotation: Vec3, // Euler angles in radians (X, Y, Z)
    pub scale: Vec3,
}

#[allow(dead_code)]
impl Transform {
    /// Creates a Transform with custom position, rotation, and scale
    pub fn new(position: Vec3, rotation: Vec3, scale: Vec3) -> Self {
        Self {
            position,
            rotation,
            scale,
        }
    }

    /// Creates a default Transform (pos: 0, rot: 0, scale: 1)
    pub fn identity() -> Self {
        Self {
            position: Vec3::ZERO,
            rotation: Vec3::ZERO,
            scale: Vec3::ONE,
        }
    }

    /// Calculates the 4x4 Model Transformation Matrix (Translation * Rotation * Scale)
    pub fn get_matrix(&self) -> Mat4 {
        let translation = Mat4::translate(self.position);

        // Individual rotations around X, Y, and Z axes
        let rot_x = Mat4::rotate(self.rotation.x, Vec3::new(1.0, 0.0, 0.0));
        let rot_y = Mat4::rotate(self.rotation.y, Vec3::new(0.0, 1.0, 0.0));
        let rot_z = Mat4::rotate(self.rotation.z, Vec3::new(0.0, 0.0, 1.0));

        // Combined Rotation Matrix (Z * Y * X)
        let rotation = rot_z * rot_y * rot_x;

        let scale = Mat4::scale(self.scale);

        // Model Matrix = T * R * S
        translation * rotation * scale
    }
}

// Implement standard Rust Default trait
impl Default for Transform {
    fn default() -> Self {
        Self::identity()
    }
}