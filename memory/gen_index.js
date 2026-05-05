const fs = require('fs');
const path = require('path');

// ========== Binary Format Spec ==========
// Magic: "RIDX" (4 bytes)
// Version: uint16 LE (2 bytes)
// Section count: uint32 LE (4 bytes)
// Sections[]: each section is:
//   type: uint8 (1 byte)
//   data_len: uint32 LE (4 bytes)
//   data: bytes[data_len]
//
// String encoding: uint16 LE length + UTF-8 bytes
// LineRange: uint32 LE start + uint32 LE end
// Property: String key + String value

const SECTION = {
  META: 0x01,
  FILE_TREE: 0x02,
  MODULE: 0x03,
  DEP_GRAPH: 0x04,
  DATA_FLOW: 0x05,
  STATUS: 0x06,
  CONVENTION: 0x07,
};

class BinaryWriter {
  constructor() {
    this.buf = Buffer.alloc(0);
  }

  writeUint8(v) {
    this.buf = Buffer.concat([this.buf, Buffer.from([v & 0xFF])]);
  }

  writeUint16(v) {
    const b = Buffer.alloc(2);
    b.writeUInt16LE(v, 0);
    this.buf = Buffer.concat([this.buf, b]);
  }

  writeUint32(v) {
    const b = Buffer.alloc(4);
    b.writeUInt32LE(v, 0);
    this.buf = Buffer.concat([this.buf, b]);
  }

  writeString(s) {
    const encoded = Buffer.from(s, 'utf-8');
    this.writeUint16(encoded.length);
    this.buf = Buffer.concat([this.buf, encoded]);
  }

  writeLineRange(start, end) {
    this.writeUint32(start);
    this.writeUint32(end);
  }

  getBuffer() { return this.buf; }
}

function encodeSection(type, writer) {
  const data = writer.getBuffer();
  const header = Buffer.alloc(5);
  header.writeUInt8(type, 0);
  header.writeUInt32LE(data.length, 1);
  return Buffer.concat([header, data]);
}

// ===== Build META section =====
function buildMeta() {
  const w = new BinaryWriter();
  // key-value pairs: language, build_system, platform, dependencies
  const meta = [
    ['language', 'C++17'],
    ['build_system', 'CMake 3.10+'],
    ['platform', 'Windows 11'],
    ['dependencies', 'SDL3, stb_image'],
    ['description', 'Pure CPU software renderer with SDL3 window management and SIMD accelerated vector operations'],
  ];
  w.writeUint16(meta.length);
  for (const [k, v] of meta) {
    w.writeString(k);
    w.writeString(v);
  }
  return encodeSection(SECTION.META, w);
}

// ===== Build FILE_TREE section =====
function buildFileTree() {
  const w = new BinaryWriter();
  const files = [
    ['src/main.cpp', 'Program entry & main loop', 1, 146, 'entry'],
    ['src/Mesh.h', 'Triangle vertex data (triangle[3])', 1, 12, 'header'],
    ['src/Mesh.cpp', 'Empty, reserved', 0, 0, 'empty'],
    ['src/Shader.h', 'Software rasterization shader', 1, 58, 'header'],
    ['src/Math.h', 'SIMD vector types (Vec4f/Vec4d) + Math utility class', 1, 212, 'header'],
    ['include/stb_image.h', 'Image loading library', 0, 0, 'third_party'],
    ['include/SDL3/', 'SDL3 headers (~90 files)', 0, 0, 'third_party'],
    ['lib/SDL3.lib', 'SDL3 static library', 0, 0, 'lib'],
    ['lib/SDL3_test.lib', 'SDL3 test library', 0, 0, 'lib'],
    ['lib/SDL3.pdb', 'SDL3 debug symbols', 0, 0, 'lib'],
    ['resources/image/test.png', 'Test image', 0, 0, 'resource'],
    ['memory/renderer-implementation.md', 'Dev documentation', 1, 361, 'doc'],
    ['CMakeLists.txt', 'Build configuration', 1, 29, 'config'],
  ];
  w.writeUint16(files.length);
  for (const [path, desc, lineStart, lineEnd, kind] of files) {
    w.writeString(path);
    w.writeString(desc);
    w.writeLineRange(lineStart, lineEnd);
    w.writeString(kind);
  }
  return encodeSection(SECTION.FILE_TREE, w);
}

// ===== Build MODULE section =====
function buildModule() {
  const w = new BinaryWriter();

  const modules = [
    {
      name: 'main.cpp',
      file: 'src/main.cpp',
      description: 'Program entry & main loop',
      callChain: 'main() -> Shader::Render() -> Shader::DoBarycentric() -> Math::SignedTriangleArea()',
      notes: 'Lines 113-136 have OpenMP parallel rendering (commented out)',
      regions: [
        [1, 11, 'Includes & macros (STB_IMAGE_IMPLEMENTATION, IMAGE_DIRECTORY)'],
        [14, 16, 'Global constants (SCREEN_WIDTH=800, SCREEN_HEIGHT=600, TITLE="REnderer")'],
        [18, 37, 'SDL init & window creation (SDL_Init, SDL_CreateWindow)'],
        [39, 60, 'stb_image load (force RGBA 4 channels)'],
        [63, 72, 'Get window Surface (SDL_GetWindowSurface)'],
        [74, 74, 'Create Shader instance'],
        [77, 141, 'Main event loop (SDL_PollEvent -> render -> SDL_UpdateWindowSurface)'],
        [107, 137, 'Render logic: lock Surface -> Shader.Render() -> unlock Surface'],
        [143, 146, 'Cleanup (stbi_image_free, SDL_DestroyWindow, SDL_Quit)'],
      ],
    },
    {
      name: 'Mesh.h',
      file: 'src/Mesh.h',
      description: 'Triangle vertex data',
      callChain: '',
      notes: 'Global triangle[3] array with hardcoded test vertices',
      regions: [
        [6, 10, 'Vec4 triangle[3]: hardcoded test triangle vertices'],
      ],
    },
    {
      name: 'Math.h',
      file: 'src/Math.h',
      description: 'SIMD vector math types and Math utility class',
      callChain: 'Math::SignedTriangleArea()',
      notes: 'Vec4 is default alias for Vec4f; Vec4hp for Vec4d',
      regions: [],
      subModules: [
        {
          name: 'Vec4f',
          lineRange: [11, 105],
          description: '4xfloat, SSE __m128',
          regions: [
            [13, 18, 'union { __m128 simd; float data[4]; struct { float x,y,z,w; }; }'],
            [20, 23, 'Constructors: default zero, __m128, (x,y,z,w), scalar broadcast'],
            [28, 46, 'Arithmetic (SIMD): operator+/-/*/divide'],
            [48, 51, 'Scalar ops: Vec4f +/-/*/divide float'],
            [53, 56, 'Friend scalar ops: float +/-/*/ Vec4f'],
            [58, 61, 'Compound assignment: +=/-=*/divide='],
            [63, 66, 'Negate: operator-()'],
            [68, 74, 'Dot product: _mm_mul_ps -> _mm_movehl_ps -> _mm_add_ss -> _mm_cvtss_f32'],
            [76, 84, '3D cross product: (ay*bz-az*by, az*bx-ax*bz, ax*by-ay*bx, 0)'],
            [86, 89, '2D cross product: x*rhs.y - y*rhs.x'],
            [91, 91, 'Length squared: dot(*this)'],
            [93, 93, 'Length: sqrt(lengthSq())'],
            [95, 99, 'Normalize: *this / len (zero protection)'],
            [101, 104, 'operator<< print debug'],
          ],
        },
        {
          name: 'Vec4d',
          lineRange: [111, 191],
          description: '4xdouble, AVX __m256d',
          regions: [
            [111, 191, 'Symmetric to Vec4f; uses __m256d/_mm256_*_pd; dot uses _mm_hadd_pd; NO cross product'],
          ],
        },
        {
          name: 'Aliases',
          lineRange: [197, 198],
          description: 'using Vec4=Vec4f; using Vec4hp=Vec4d',
          regions: [],
        },
        {
          name: 'Math',
          lineRange: [203, 210],
          description: 'Static math utility class',
          regions: [
            [206, 208, 'SignedTriangleArea(Vec4 a, Vec4 b, Vec4 c): signed triangle area for barycentric coords'],
          ],
        },
      ],
    },
    {
      name: 'Shader.h',
      file: 'src/Shader.h',
      description: 'Software rasterization shader',
      callChain: 'Render() -> DoBarycentric() -> Math::SignedTriangleArea()',
      notes: 'Render() takes Vec4* mesh (array pointer), passes global triangle',
      regions: [
        [10, 12, 'class Shader: default constructor/destructor'],
        [14, 29, 'Render(): iterate all pixels, call DoBarycentric, write white pixel'],
        [32, 47, 'DoBarycentric(): barycentric coords - call Math::SignedTriangleArea(), compute u/v/w, check u>=0 && v>=0 && w>=0'],
        [49, 56, 'VertextShader(): (disabled) multiply normalized coords by screen size'],
      ],
    },
    {
      name: 'CMakeLists.txt',
      file: 'CMakeLists.txt',
      description: 'Build configuration',
      callChain: '',
      notes: '',
      regions: [
        [4, 5, 'C++17 standard'],
        [8, 8, 'OpenMP (commented out)'],
        [10, 10, 'include path: ${CMAKE_SOURCE_DIR}/include'],
        [12, 12, 'lib path: ${CMAKE_SOURCE_DIR}/lib'],
        [15, 15, 'Source files: src/*.h src/*.cpp'],
        [18, 18, 'Link: SDL3'],
        [26, 29, 'Post-build: copy resources to output directory'],
      ],
    },
  ];

  w.writeUint16(modules.length);
  for (const mod of modules) {
    w.writeString(mod.name);
    w.writeString(mod.file);
    w.writeString(mod.description);
    w.writeString(mod.callChain);
    w.writeString(mod.notes);
    w.writeUint16(mod.regions.length);
    for (const [start, end, desc] of mod.regions) {
      w.writeLineRange(start, end);
      w.writeString(desc);
    }
    const subCount = mod.subModules ? mod.subModules.length : 0;
    w.writeUint16(subCount);
    if (mod.subModules) {
      for (const sub of mod.subModules) {
        w.writeString(sub.name);
        w.writeLineRange(sub.lineRange[0], sub.lineRange[1]);
        w.writeString(sub.description);
        w.writeUint16(sub.regions.length);
        for (const [start, end, desc] of sub.regions) {
          w.writeLineRange(start, end);
          w.writeString(desc);
        }
      }
    }
  }
  return encodeSection(SECTION.MODULE, w);
}

// ===== Build DEP_GRAPH section =====
function buildDepGraph() {
  const w = new BinaryWriter();
  // edges: [from_file, to_dependency, dep_type]
  const edges = [
    ['src/main.cpp', 'SDL3/SDL.h', 'include'],
    ['src/main.cpp', 'SDL3/SDL_main.h', 'include'],
    ['src/main.cpp', 'stb_image.h', 'include'],
    ['src/main.cpp', 'src/Shader.h', 'include'],
    ['src/Shader.h', 'SDL3/SDL_stdinc.h', 'include'],
    ['src/Shader.h', 'SDL3/SDL_pixels.h', 'include'],
    ['src/Shader.h', 'src/Math.h', 'include'],
    ['src/Shader.h', 'src/Mesh.h', 'include'],
    ['src/Mesh.h', 'src/Math.h', 'include'],
    ['src/Math.h', '<immintrin.h>', 'include'],
    ['src/Math.h', '<cmath>', 'include'],
    ['src/Math.h', '<ostream>', 'include'],
    ['src/main.cpp', 'SDL3.lib', 'link'],
  ];
  w.writeUint16(edges.length);
  for (const [from, to, type] of edges) {
    w.writeString(from);
    w.writeString(to);
    w.writeString(type);
  }
  return encodeSection(SECTION.DEP_GRAPH, w);
}

// ===== Build DATA_FLOW section =====
function buildDataFlow() {
  const w = new BinaryWriter();
  const flows = [
    { step: 1, description: 'SDL_CreateWindow -> SDL_GetWindowSurface -> get pixel buffer (Uint32*)' },
    { step: 2, description: 'stbi_load -> load RGBA pixel data (unsigned char*)' },
    { step: 3, description: 'Main loop: SDL_PollEvent -> SDL_LockSurface -> Shader.Render(pixelData, triangle, w, h, fmt) -> SDL_UnlockSurface -> SDL_UpdateWindowSurface' },
    { step: 4, description: 'Shader.Render: iterate (x,y) -> DoBarycentric(normalized coords) -> Math::SignedTriangleArea() -> write pixel color' },
  ];
  w.writeUint16(flows.length);
  for (const f of flows) {
    w.writeUint8(f.step);
    w.writeString(f.description);
  }
  return encodeSection(SECTION.DATA_FLOW, w);
}

// ===== Build STATUS section =====
function buildStatus() {
  const w = new BinaryWriter();
  const items = [
    ['Window creation & event loop', 'ok', 'Working'],
    ['SIMD Vec4f/Vec4d', 'ok', 'Complete (moved to Math.h)'],
    ['Math class (utility)', 'ok', 'Implemented (SignedTriangleArea)'],
    ['Barycentric rasterization', 'ok', 'Basically working'],
    ['Image loading & display', 'warn', 'Loaded but not used in Render'],
    ['OpenMP parallel', 'err', 'Commented out'],
    ['Mesh.cpp', 'err', 'Empty file'],
    ['VertextShader', 'err', 'Call commented out'],
  ];
  w.writeUint16(items.length);
  for (const [name, status, desc] of items) {
    w.writeString(name);
    // status: ok=0x01, warn=0x02, err=0x03
    const statusByte = status === 'ok' ? 0x01 : status === 'warn' ? 0x02 : 0x03;
    w.writeUint8(statusByte);
    w.writeString(desc);
  }
  return encodeSection(SECTION.STATUS, w);
}

// ===== Build CONVENTION section =====
function buildConvention() {
  const w = new BinaryWriter();
  const convs = [
    ['header_guard', 'RENDERER_*_INCLUDE'],
    ['simd_first', 'Vector ops all use intrinsics'],
    ['default_type', 'Vec4 = Vec4f (single precision)'],
    ['global_data', 'triangle[3] defined in Mesh.h'],
    ['math_functions', 'Math functions as static methods in Math class'],
    ['comments', 'Chinese'],
    ['build_command', 'cmake -B build; cmake --build build'],
  ];
  w.writeUint16(convs.length);
  for (const [k, v] of convs) {
    w.writeString(k);
    w.writeString(v);
  }
  return encodeSection(SECTION.CONVENTION, w);
}

// ===== Assemble final binary =====
const sections = [
  buildMeta(),
  buildFileTree(),
  buildModule(),
  buildDepGraph(),
  buildDataFlow(),
  buildStatus(),
  buildConvention(),
];

// Header: RIDX + version(uint16) + section_count(uint32)
const headerBuf = Buffer.alloc(10);
headerBuf.write('RIDX', 0);
headerBuf.writeUInt16LE(1, 4); // version 1
headerBuf.writeUInt32LE(sections.length, 6);

const finalBuf = Buffer.concat([headerBuf, ...sections]);

const outPath = path.join(__dirname, 'codeindex.bin');
fs.writeFileSync(outPath, finalBuf);
console.log(`Written ${finalBuf.length} bytes to ${outPath}`);

// Also write a format spec file for reference
const spec = `# RIDX Binary Format Specification

## Header (10 bytes)
| Offset | Size | Type     | Description       |
|--------|------|----------|-------------------|
| 0      | 4    | char[4]  | Magic "RIDX"     |
| 4      | 2    | uint16LE | Version (1)       |
| 6      | 4    | uint32LE | Section count     |

## Section Layout
| Offset | Size     | Type     | Description     |
|--------|----------|----------|-----------------|
| 0      | 1        | uint8    | Section type    |
| 1      | 4        | uint32LE | Data length    |
| 5      | variable | bytes    | Section data    |

## Section Types
| Value | Name        | Description                |
|-------|-------------|----------------------------|
| 0x01  | META        | Project metadata (kvpairs) |
| 0x02  | FILE_TREE   | File tree entries          |
| 0x03  | MODULE      | Module details             |
| 0x04  | DEP_GRAPH   | Dependency edges            |
| 0x05  | DATA_FLOW   | Data flow steps             |
| 0x06  | STATUS      | Project status              |
| 0x07  | CONVENTION  | Coding conventions           |

## Primitive Types
- **String**: uint16LE length + UTF-8 bytes
- **LineRange**: uint32LE start + uint32LE end
- **Status byte**: 0x01=ok, 0x02=warn, 0x03=error

## Section Data Formats

### META (0x01)
uint16 count; for each: String key + String value

### FILE_TREE (0x02)
uint16 count; for each: String path + String desc + LineRange + String kind
  kind: entry|header|empty|third_party|lib|resource|doc|config

### MODULE (0x03)
uint16 count; for each:
  String name + String file + String description + String callChain + String notes
  uint16 regionCount; for each: LineRange + String desc
  uint16 subModuleCount; for each:
    String name + LineRange + String description
    uint16 regionCount; for each: LineRange + String desc

### DEP_GRAPH (0x04)
uint16 count; for each: String from + String to + String type
  type: include|link

### DATA_FLOW (0x05)
uint16 count; for each: uint8 step + String description

### STATUS (0x06)
uint16 count; for each: String name + uint8 status + String desc

### CONVENTION (0x07)
uint16 count; for each: String key + String value
`;

fs.writeFileSync(path.join(__dirname, 'codeindex_format.md'), spec);
console.log('Format spec written.');