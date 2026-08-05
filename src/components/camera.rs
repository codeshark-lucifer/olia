use super::transform::Transform;
use crate::utils::mathf::{Mat4, Vec3};

#[derive(Debug, Clone, Copy)]
pub struct View {
    pub width: i32,
    pub height: i32,
}

#[derive(Debug, Clone)]
pub struct Camera {
    pub view: View,
    pub fov: f32,
    pub near: f32,
    pub far: f32,
}

#[allow(dead_code)]
impl Camera {
    pub fn new(width: i32, height: i32) -> Self {
        Self {
            view: View { width, height },
            fov: 45.0,
            near: 0.1,
            far: 1000.0,
        }
    }

    pub fn with_settings(width: i32, height: i32, fov: f32, near: f32, far: f32) -> Self {
        Self {
            view: View { width, height },
            fov,
            near,
            far,
        }
    }

    pub fn resize(&mut self, width: i32, height: i32) {
        self.view.width = width;
        self.view.height = height;
    }

    pub fn aspect_ratio(&self) -> f32 {
        if self.view.height == 0 {
            return 1.0;
        }
        self.view.width as f32 / self.view.height as f32
    }

    pub fn get_projection_matrix(&self) -> Mat4 {
        Mat4::perspective(self.fov.to_radians(), self.aspect_ratio(), self.near, self.far)
    }

    pub fn get_view_matrix(&self, position: Vec3, forward: Vec3) -> Mat4 {
        let target = position + forward;
        Mat4::look_at(position, target, Vec3::UP)
    }

    pub fn get_matrices(&self, trans: &Transform) -> (Mat4, Mat4) {
        let view = self.get_view_matrix(trans.position, Vec3::FORWARD);
        let proj = self.get_projection_matrix();
        (view, proj)
    }
}