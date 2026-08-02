use std::ops::{Add, AddAssign, Div, Mul, Neg, Sub, SubAssign};

// ============================================================================
// VEC2
// ============================================================================
#[derive(Copy, Clone, Debug, PartialEq)]
pub struct Vec2 {
    pub x: f32,
    pub y: f32,
}

#[allow(dead_code)]
impl Vec2 {
    pub const ZERO: Self = Self { x: 0.0, y: 0.0 };
    pub const ONE: Self = Self { x: 1.0, y: 1.0 };

    pub fn new(x: f32, y: f32) -> Self {
        Self { x, y }
    }

    pub fn dot(self, other: Self) -> f32 {
        self.x * other.x + self.y * other.y
    }

    pub fn length(self) -> f32 {
        self.dot(self).sqrt()
    }

    pub fn normalize(self) -> Self {
        let len = self.length();
        if len > 0.0 { self / len } else { Self::ZERO }
    }
}

// Vec2 Operators
impl Add for Vec2 { type Output = Self; fn add(self, rhs: Self) -> Self { Self::new(self.x + rhs.x, self.y + rhs.y) } }
impl Sub for Vec2 { type Output = Self; fn sub(self, rhs: Self) -> Self { Self::new(self.x - rhs.x, self.y - rhs.y) } }
impl Mul<f32> for Vec2 { type Output = Self; fn mul(self, rhs: f32) -> Self { Self::new(self.x * rhs, self.y * rhs) } }
impl Div<f32> for Vec2 { type Output = Self; fn div(self, rhs: f32) -> Self { Self::new(self.x / rhs, self.y / rhs) } }
impl Neg for Vec2 { type Output = Self; fn neg(self) -> Self { Self::new(-self.x, -self.y) } }
impl AddAssign for Vec2 { fn add_assign(&mut self, rhs: Self) { self.x += rhs.x; self.y += rhs.y; } }
impl SubAssign for Vec2 { fn sub_assign(&mut self, rhs: Self) { self.x -= rhs.x; self.y -= rhs.y; } }

// ============================================================================
// VEC3
// ============================================================================
#[derive(Copy, Clone, Debug, PartialEq)]
pub struct Vec3 {
    pub x: f32,
    pub y: f32,
    pub z: f32,
}

#[allow(dead_code)]
impl Vec3 {
    pub const ZERO: Self = Self { x: 0.0, y: 0.0, z: 0.0 };
    pub const ONE: Self = Self { x: 1.0, y: 1.0, z: 1.0 };
    pub const UP: Self = Self { x: 0.0, y: 1.0, z: 0.0 };
    pub const DOWN: Self = Self { x: 0.0, y: -1.0, z: 0.0 };
    pub const FORWARD: Self = Self { x: 0.0, y: 0.0, z: -1.0 };
    pub const BACK: Self = Self { x: 0.0, y: 0.0, z: 1.0 };

    pub fn new(x: f32, y: f32, z: f32) -> Self {
        Self { x, y, z }
    }

    pub fn dot(self, other: Self) -> f32 {
        self.x * other.x + self.y * other.y + self.z * other.z
    }

    pub fn cross(self, other: Self) -> Self {
        Self {
            x: self.y * other.z - self.z * other.y,
            y: self.z * other.x - self.x * other.z,
            z: self.x * other.y - self.y * other.x,
        }
    }

    pub fn length(self) -> f32 {
        self.dot(self).sqrt()
    }

    pub fn normalize(self) -> Self {
        let len = self.length();
        if len > 0.0 { self / len } else { Self::ZERO }
    }
}

// Vec3 Operators
impl Add for Vec3 { type Output = Self; fn add(self, rhs: Self) -> Self { Self::new(self.x + rhs.x, self.y + rhs.y, self.z + rhs.z) } }
impl Sub for Vec3 { type Output = Self; fn sub(self, rhs: Self) -> Self { Self::new(self.x - rhs.x, self.y - rhs.y, self.z - rhs.z) } }
impl Mul<f32> for Vec3 { type Output = Self; fn mul(self, rhs: f32) -> Self { Self::new(self.x * rhs, self.y * rhs, self.z * rhs) } }
impl Div<f32> for Vec3 { type Output = Self; fn div(self, rhs: f32) -> Self { Self::new(self.x / rhs, self.y / rhs, self.z / rhs) } }
impl Neg for Vec3 { type Output = Self; fn neg(self) -> Self { Self::new(-self.x, -self.y, -self.z) } }
impl AddAssign for Vec3 { fn add_assign(&mut self, rhs: Self) { self.x += rhs.x; self.y += rhs.y; self.z += rhs.z; } }
impl SubAssign for Vec3 { fn sub_assign(&mut self, rhs: Self) { self.x -= rhs.x; self.y -= rhs.y; self.z -= rhs.z; } }

// ============================================================================
// VEC4
// ============================================================================
#[derive(Copy, Clone, Debug, PartialEq)]
pub struct Vec4 {
    pub x: f32,
    pub y: f32,
    pub z: f32,
    pub w: f32,
}

#[allow(dead_code)]
impl Vec4 {
    pub const ZERO: Self = Self { x: 0.0, y: 0.0, z: 0.0, w: 0.0 };

    pub fn new(x: f32, y: f32, z: f32, w: f32) -> Self {
        Self { x, y, z, w }
    }

    pub fn from_vec3(v: Vec3, w: f32) -> Self {
        Self { x: v.x, y: v.y, z: v.z, w }
    }

    pub fn dot(self, other: Self) -> f32 {
        self.x * other.x + self.y * other.y + self.z * other.z + self.w * other.w
    }
}

// Vec4 Operators
impl Add for Vec4 { type Output = Self; fn add(self, rhs: Self) -> Self { Self::new(self.x + rhs.x, self.y + rhs.y, self.z + rhs.z, self.w + rhs.w) } }
impl Sub for Vec4 { type Output = Self; fn sub(self, rhs: Self) -> Self { Self::new(self.x - rhs.x, self.y - rhs.y, self.z - rhs.z, self.w - rhs.w) } }
impl Mul<f32> for Vec4 { type Output = Self; fn mul(self, rhs: f32) -> Self { Self::new(self.x * rhs, self.y * rhs, self.z * rhs, self.w * rhs) } }

// ============================================================================
// MAT2 & MAT3
// ============================================================================

#[allow(dead_code)]
#[derive(Copy, Clone, Debug, PartialEq)]
pub struct Mat2 { pub data: [f32; 4] }

#[allow(dead_code)]
impl Mat2 {
    pub fn identity() -> Self {
        Self { data: [1.0, 0.0, 0.0, 1.0] }
    }
}

#[allow(dead_code)]
#[derive(Copy, Clone, Debug, PartialEq)]
pub struct Mat3 { pub data: [f32; 9] }

#[allow(dead_code)]
impl Mat3 {
    pub fn identity() -> Self {
        Self {
            data: [
                1.0, 0.0, 0.0,
                0.0, 1.0, 0.0,
                0.0, 0.0, 1.0,
            ],
        }
    }
}

// ============================================================================
// MAT4 (Column-Major 4x4 Matrix - OpenGL / Vulkan Standard)
// ============================================================================
#[allow(dead_code)]
#[derive(Copy, Clone, Debug, PartialEq)]
pub struct Mat4 {
    pub data: [f32; 16], // Column-major order
}

#[allow(dead_code)]
impl Mat4 {
    pub fn identity() -> Self {
        Self {
            data: [
                1.0, 0.0, 0.0, 0.0, // Col 0
                0.0, 1.0, 0.0, 0.0, // Col 1
                0.0, 0.0, 1.0, 0.0, // Col 2
                0.0, 0.0, 0.0, 1.0, // Col 3
            ],
        }
    }

    /// Returns a raw pointer to pass directly to Shader Uniforms (`glUniformMatrix4fv`)
    pub fn as_ptr(&self) -> *const f32 {
        self.data.as_ptr()
    }

    /// Translation matrix
    pub fn translate(v: Vec3) -> Self {
        let mut m = Self::identity();
        m.data[12] = v.x;
        m.data[13] = v.y;
        m.data[14] = v.z;
        m
    }

    /// Scale matrix
    pub fn scale(v: Vec3) -> Self {
        let mut m = Self::identity();
        m.data[0] = v.x;
        m.data[5] = v.y;
        m.data[10] = v.z;
        m
    }

    /// Rotation matrix around axis (angle in radians)
    pub fn rotate(angle_rad: f32, axis: Vec3) -> Self {
        let mut m = Self::identity();
        let axis = axis.normalize();
        let c = angle_rad.cos();
        let s = angle_rad.sin();
        let omc = 1.0 - c;

        m.data[0] = c + axis.x * axis.x * omc;
        m.data[1] = axis.y * axis.x * omc + axis.z * s;
        m.data[2] = axis.z * axis.x * omc - axis.y * s;

        m.data[4] = axis.x * axis.y * omc - axis.z * s;
        m.data[5] = c + axis.y * axis.y * omc;
        m.data[6] = axis.z * axis.y * omc + axis.x * s;

        m.data[8] = axis.x * axis.z * omc + axis.y * s;
        m.data[9] = axis.y * axis.z * omc - axis.x * s;
        m.data[10] = c + axis.z * axis.z * omc;

        m
    }

    /// Perspective Projection Matrix (OpenGL standard)
    pub fn perspective(fovy_rad: f32, aspect: f32, near: f32, far: f32) -> Self {
        let f = 1.0 / (fovy_rad * 0.5).tan();
        let mut m = Self { data: [0.0; 16] };

        m.data[0] = f / aspect;
        m.data[5] = f;
        m.data[10] = (far + near) / (near - far);
        m.data[11] = -1.0;
        m.data[14] = (2.0 * far * near) / (near - far);

        m
    }

    /// Orthographic Projection Matrix
    pub fn ortho(left: f32, right: f32, bottom: f32, top: f32, near: f32, far: f32) -> Self {
        let mut m = Self::identity();

        m.data[0] = 2.0 / (right - left);
        m.data[5] = 2.0 / (top - bottom);
        m.data[10] = -2.0 / (far - near);

        m.data[12] = -(right + left) / (right - left);
        m.data[13] = -(top + bottom) / (top - bottom);
        m.data[14] = -(far + near) / (far - near);

        m
    }

    /// LookAt Camera Matrix
    pub fn look_at(eye: Vec3, center: Vec3, up: Vec3) -> Self {
        let f = (center - eye).normalize();
        let s = f.cross(up).normalize();
        let u = s.cross(f);

        let mut m = Self::identity();

        m.data[0] = s.x;
        m.data[4] = s.y;
        m.data[8] = s.z;

        m.data[1] = u.x;
        m.data[5] = u.y;
        m.data[9] = u.z;

        m.data[2] = -f.x;
        m.data[6] = -f.y;
        m.data[10] = -f.z;

        m.data[12] = -s.dot(eye);
        m.data[13] = -u.dot(eye);
        m.data[14] = f.dot(eye);

        m
    }
}

#[allow(dead_code)]
// Mat4 * Mat4 Multiplication
impl Mul for Mat4 {
    type Output = Self;

    fn mul(self, rhs: Self) -> Self {
        let mut result = Self { data: [0.0; 16] };
        for col in 0..4 {
            for row in 0..4 {
                let mut sum = 0.0;
                for i in 0..4 {
                    sum += self.data[i * 4 + row] * rhs.data[col * 4 + i];
                }
                result.data[col * 4 + row] = sum;
            }
        }
        result
    }
}

// Mat4 * Vec4 Multiplication
impl Mul<Vec4> for Mat4 {
    type Output = Vec4;

    fn mul(self, v: Vec4) -> Vec4 {
        Vec4 {
            x: self.data[0] * v.x + self.data[4] * v.y + self.data[8] * v.z + self.data[12] * v.w,
            y: self.data[1] * v.x + self.data[5] * v.y + self.data[9] * v.z + self.data[13] * v.w,
            z: self.data[2] * v.x + self.data[6] * v.y + self.data[10] * v.z + self.data[14] * v.w,
            w: self.data[3] * v.x + self.data[7] * v.y + self.data[11] * v.z + self.data[15] * v.w,
        }
    }
}