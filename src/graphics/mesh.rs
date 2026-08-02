use std::mem::size_of;
use std::ptr;

use crate::utils::mathf::{Vec2, Vec3};

/// Memory layout must be #[repr(C)] so OpenGL matches the Rust struct byte layout
    
#[allow(dead_code)]
#[repr(C)]
#[derive(Copy, Clone, Debug, PartialEq)]
pub struct Vertex {
    pub position: Vec3,
    pub normal: Vec3,
    pub uv: Vec2,
}

#[allow(dead_code)]
pub struct Mesh {
    pub vertices: Vec<Vertex>,
    pub indices: Vec<u32>,
}

#[allow(dead_code)]
pub struct MeshRenderer {
    pub vao: u32,
    pub vbo: u32,
    pub ebo: u32,
    pub index_count: i32,
}

#[allow(dead_code)]
impl MeshRenderer {
    /// Uploads mesh data to GPU VBO/VAO/EBO buffers
    pub fn new(mesh: &Mesh) -> Self {
        let mut vao = 0;
        let mut vbo = 0;
        let mut ebo = 0;

        unsafe {
            gl::GenVertexArrays(1, &mut vao);
            gl::GenBuffers(1, &mut vbo);
            gl::GenBuffers(1, &mut ebo);

            gl::BindVertexArray(vao);

            // Upload Vertex Buffer (VBO)
            gl::BindBuffer(gl::ARRAY_BUFFER, vbo);
            gl::BufferData(
                gl::ARRAY_BUFFER,
                (mesh.vertices.len() * size_of::<Vertex>()) as isize,
                mesh.vertices.as_ptr() as *const _,
                gl::STATIC_DRAW,
            );

            // Upload Element Index Buffer (EBO)
            gl::BindBuffer(gl::ELEMENT_ARRAY_BUFFER, ebo);
            gl::BufferData(
                gl::ELEMENT_ARRAY_BUFFER,
                (mesh.indices.len() * size_of::<u32>()) as isize,
                mesh.indices.as_ptr() as *const _,
                gl::STATIC_DRAW,
            );

            let stride = size_of::<Vertex>() as i32;

            // Attribute 0: Position (Vec3 = 3 floats)
            gl::EnableVertexAttribArray(0);
            gl::VertexAttribPointer(0, 3, gl::FLOAT, gl::FALSE, stride, ptr::null());

            // Attribute 1: Normal (Vec3 = 3 floats)
            gl::EnableVertexAttribArray(1);
            gl::VertexAttribPointer(
                1,
                3,
                gl::FLOAT,
                gl::FALSE,
                stride,
                (3 * size_of::<f32>()) as *const _,
            );

            // Attribute 2: UV (Vec2 = 2 floats)
            gl::EnableVertexAttribArray(2);
            gl::VertexAttribPointer(
                2,
                2,
                gl::FLOAT,
                gl::FALSE,
                stride,
                (6 * size_of::<f32>()) as *const _,
            );

            gl::BindVertexArray(0);
        }

        Self {
            vao,
            vbo,
            ebo,
            index_count: mesh.indices.len() as i32,
        }
    }

    /// Renders the mesh using indexed drawing
    pub fn draw(&self) {
        unsafe {
            gl::BindVertexArray(self.vao);
            gl::DrawElements(
                gl::TRIANGLES,
                self.index_count,
                gl::UNSIGNED_INT,
                ptr::null(),
            );
            gl::BindVertexArray(0);
        }
    }
}

#[allow(dead_code)]
// Automatically free GPU buffers when dropped
impl Drop for MeshRenderer {
    fn drop(&mut self) {
        unsafe {
            gl::DeleteVertexArrays(1, &self.vao);
            gl::DeleteBuffers(1, &self.vbo);
            gl::DeleteBuffers(1, &self.ebo);
        }
    }
}