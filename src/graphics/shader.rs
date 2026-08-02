use std::cell::RefCell;
use std::collections::HashMap;
use std::ffi::CString;
use std::fs;
use std::ptr;

use crate::utils::mathf::{Mat4, Vec2, Vec3, Vec4};

#[allow(dead_code)]
pub struct Shader {
    pub id: u32,
    // HashMap cache to store uniform name -> location mapping
    uniform_locations: RefCell<HashMap<String, i32>>,
}

#[allow(dead_code)]
impl Shader {
    /// Load shader program directly from hardcoded GLSL source strings
    pub fn from_source(vertex_code: &str, fragment_code: &str) -> Result<Self, String> {
        let vertex_shader = compile_shader(vertex_code, gl::VERTEX_SHADER)?;
        let fragment_shader = compile_shader(fragment_code, gl::FRAGMENT_SHADER)?;

        let id = unsafe { gl::CreateProgram() };
        unsafe {
            gl::AttachShader(id, vertex_shader);
            gl::AttachShader(id, fragment_shader);
            gl::LinkProgram(id);
        }

        // Check for linking errors
        let mut success = gl::FALSE as i32;
        unsafe {
            gl::GetProgramiv(id, gl::LINK_STATUS, &mut success);
        }

        if success == (gl::FALSE as i32) {
            let mut len = 0;
            unsafe { gl::GetProgramiv(id, gl::INFO_LOG_LENGTH, &mut len); }
            let mut buffer = vec![0u8; len as usize];
            unsafe {
                gl::GetProgramInfoLog(id, len, ptr::null_mut(), buffer.as_mut_ptr() as *mut i8);
            }
            let log = String::from_utf8_lossy(&buffer);
            return Err(format!("Shader Program Link Error:\n{}", log));
        }

        // Clean up individual shaders once linked
        unsafe {
            gl::DeleteShader(vertex_shader);
            gl::DeleteShader(fragment_shader);
        }

        Ok(Self {
            id,
            uniform_locations: RefCell::new(HashMap::new()),
        })
    }

    /// Load shader program from GLSL files on disk
    pub fn from_file(vertex_path: &str, fragment_path: &str) -> Result<Self, String> {
        let vertex_code = fs::read_to_string(vertex_path)
            .map_err(|e| format!("Failed to read vertex shader at '{vertex_path}': {e}"))?;

        let fragment_code = fs::read_to_string(fragment_path)
            .map_err(|e| format!("Failed to read fragment shader at '{fragment_path}': {e}"))?;

        Self::from_source(&vertex_code, &fragment_code)
    }

    /// Activate this shader program
    pub fn use_program(&self) {
        unsafe {
            gl::UseProgram(self.id);
        }
    }

    /// Unbind the active shader program
    pub fn unbind(&self) {
        unsafe {
            gl::UseProgram(0);
        }
    }

    // ========================================================================
    // UNIFORM LOCATION CACHING (HASHMAP)
    // ========================================================================

    fn get_uniform_location(&self, name: &str) -> Option<i32> {
        let mut locations = self.uniform_locations.borrow_mut();

        // 1. Return from HashMap cache if already queried
        if let Some(&location) = locations.get(name) {
            if location == -1 {
                return None;
            }
            return Some(location);
        }

        // 2. Query OpenGL driver if not in HashMap
        let c_name = match CString::new(name) {
            Ok(c_str) => c_str,
            Err(_) => return None,
        };

        let location = unsafe { gl::GetUniformLocation(self.id, c_name.as_ptr()) };

        // 3. Cache location in HashMap (even if -1, to avoid asking driver again for missing names)
        locations.insert(name.to_string(), location);

        if location == -1 {
            None
        } else {
            Some(location)
        }
    }

    // ========================================================================
    // UNIFORM SETTERS
    // ========================================================================

    pub fn set_bool(&self, name: &str, value: bool) {
        if let Some(loc) = self.get_uniform_location(name) {
            unsafe { gl::Uniform1i(loc, value as i32); }
        }
    }

    pub fn set_int(&self, name: &str, value: i32) {
        if let Some(loc) = self.get_uniform_location(name) {
            unsafe { gl::Uniform1i(loc, value); }
        }
    }

    pub fn set_float(&self, name: &str, value: f32) {
        if let Some(loc) = self.get_uniform_location(name) {
            unsafe { gl::Uniform1f(loc, value); }
        }
    }

    pub fn set_vec2(&self, name: &str, value: &Vec2) {
        if let Some(loc) = self.get_uniform_location(name) {
            unsafe { gl::Uniform2f(loc, value.x, value.y); }
        }
    }

    pub fn set_vec3(&self, name: &str, value: &Vec3) {
        if let Some(loc) = self.get_uniform_location(name) {
            unsafe { gl::Uniform3f(loc, value.x, value.y, value.z); }
        }
    }

    pub fn set_vec4(&self, name: &str, value: &Vec4) {
        if let Some(loc) = self.get_uniform_location(name) {
            unsafe { gl::Uniform4f(loc, value.x, value.y, value.z, value.w); }
        }
    }

    pub fn set_mat4(&self, name: &str, mat: &Mat4) {
        if let Some(loc) = self.get_uniform_location(name) {
            unsafe {
                gl::UniformMatrix4fv(loc, 1, gl::FALSE, mat.as_ptr());
            }
        }
    }
}

// Automatically cleanup OpenGL Shader Program when dropped
impl Drop for Shader {
    fn drop(&mut self) {
        unsafe {
            gl::DeleteProgram(self.id);
        }
    }
}

// Private helper to compile individual shaders
fn compile_shader(source: &str, shader_type: u32) -> Result<u32, String> {
    let shader = unsafe { gl::CreateShader(shader_type) };
    let c_str = CString::new(source).map_err(|e| e.to_string())?;

    unsafe {
        gl::ShaderSource(shader, 1, &c_str.as_ptr(), ptr::null());
        gl::CompileShader(shader);
    }

    let mut success = gl::FALSE as i32;
    unsafe {
        gl::GetShaderiv(shader, gl::COMPILE_STATUS, &mut success);
    }

    if success == (gl::FALSE as i32) {
        let mut len = 0;
        unsafe { gl::GetShaderiv(shader, gl::INFO_LOG_LENGTH, &mut len); }
        let mut buffer = vec![0u8; len as usize];
        unsafe {
            gl::GetShaderInfoLog(shader, len, ptr::null_mut(), buffer.as_mut_ptr() as *mut i8);
        }
        let log = String::from_utf8_lossy(&buffer);
        let kind = if shader_type == gl::VERTEX_SHADER { "Vertex" } else { "Fragment" };
        return Err(format!("{} Shader Compilation Error:\n{}", kind, log));
    }

    Ok(shader)
}