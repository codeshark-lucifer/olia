use ash::vk;
use crate::utils::mathf::{Mat4, Vec2, Vec3, Vec4};

/// Vertex struct containing 3D Position, Normal vector, and UV texture coordinates
#[repr(C)]
#[derive(Copy, Clone, Debug)]
pub struct Vertex {
    pub position: Vec3,
    pub normal: Vec3,
    pub uv: Vec2,
}

impl Vertex {
    pub fn get_binding_description() -> vk::VertexInputBindingDescription {
        vk::VertexInputBindingDescription::default()
            .binding(0)
            .stride(std::mem::size_of::<Self>() as u32)
            .input_rate(vk::VertexInputRate::VERTEX)
    }

    pub fn get_attribute_descriptions() -> [vk::VertexInputAttributeDescription; 3] {
        [
            // Location 0: Position (Vec3 -> RGB32_SFLOAT)
            vk::VertexInputAttributeDescription::default()
                .binding(0)
                .location(0)
                .format(vk::Format::R32G32B32_SFLOAT)
                .offset(0),
            // Location 1: Normal (Vec3 -> RGB32_SFLOAT)
            vk::VertexInputAttributeDescription::default()
                .binding(0)
                .location(1)
                .format(vk::Format::R32G32B32_SFLOAT)
                .offset(std::mem::size_of::<Vec3>() as u32),
            // Location 2: UV Coordinates (Vec2 -> RG32_SFLOAT)
            vk::VertexInputAttributeDescription::default()
                .binding(0)
                .location(2)
                .format(vk::Format::R32G32_SFLOAT)
                .offset((std::mem::size_of::<Vec3>() * 2) as u32),
        ]
    }
}

/// Push Constants for Model matrix
#[repr(C)]
#[derive(Copy, Clone, Debug)]
pub struct PushConstants {
    pub model_matrix: Mat4,
    pub tint_color: Vec4,
}

/// Camera Uniform Buffer Object
#[repr(C)]
#[derive(Copy, Clone, Debug)]
pub struct CameraUbo {
    pub view: Mat4,
    pub projection: Mat4,
}

/// Storage Buffer Object
#[repr(C)]
#[derive(Copy, Clone, Debug)]
pub struct ObjectDataSsbo {
    pub world_matrix: Mat4,
    pub color_override: Vec4,
}