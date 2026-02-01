#include <stdio.h>
#include <string.h>

#include <emscripten.h>

#define TINYOBJ_LOADER_C_IMPLEMENTATION
#include "tinyobj_loader_c.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

struct VertexAttributes{
    float position[3];
    float texcoord[2];
    float normal[3];
};

typedef struct {
    int v_idx;
    int vt_idx;
    int vn_idx;
} VertexKey;

static size_t hash_vertex(VertexKey const * key) {
    size_t h = 14695981039346656037ULL; // FNV offset
    h ^= (size_t)key->v_idx;  h *= 1099511628211ULL;
    h ^= (size_t)key->vn_idx; h *= 1099511628211ULL;
    h ^= (size_t)key->vt_idx; h *= 1099511628211ULL;
    return h;
}
typedef struct VertexEntry {
    VertexKey key;
    uint32_t index;
    struct VertexEntry * next;
}  VertexEntry;
// #define HASH_SIZE 131071 // a large prime number
#define HASH_SIZE 59 // a prime number
typedef struct {
    VertexEntry * table[HASH_SIZE];
} VertexHash;
static void vertex_hash_init(VertexHash * hash) {
    memset(hash->table, 0, sizeof(hash->table));
}
static uint32_t vertex_hash_get_or_add(VertexHash * hash, VertexKey key, struct VertexAttributes * vertex_array, uint32_t * vertex_count) {
    size_t bucket = hash_vertex(&key) % HASH_SIZE;
    //printf("Hashed key (v_idx=%d, vt_idx=%d, vn_idx=%d) to bucket %zu\n", key.v_idx, key.vt_idx, key.vn_idx, bucket);
    VertexEntry * entry = hash->table[bucket];

    // search for existing entry
    while (entry) {
        if (entry->key.v_idx == key.v_idx &&
            entry->key.vn_idx == key.vn_idx &&
            entry->key.vt_idx == key.vt_idx) {
            return entry->index;
        }
        entry = entry->next;
    }
    // not found, add new entry
    uint32_t new_index = (uint32_t)(*vertex_count);

    VertexEntry * new_entry = (VertexEntry *)malloc(sizeof(VertexEntry));
    new_entry->key = key;
    new_entry->index = new_index;
    new_entry->next = hash->table[bucket];
    hash->table[bucket] = new_entry;

    (*vertex_count)++;

    return new_index;
}
static void vertex_hash_free(VertexHash * hash) {
    for (size_t i = 0; i < HASH_SIZE; i++) {
        VertexEntry * entry = hash->table[i];
        while (entry) {
            VertexEntry * next = entry->next;
            free(entry);
            entry = next;
        }
        hash->table[i] = NULL;
    }
}

struct MeshRange{
    uint32_t index_offset;
    uint32_t index_count;
    uint32_t vertex_start;
};
struct MeshBatch{
    struct VertexAttributes * vertex_data;
    uint32_t * index_data;

    struct MeshRange * meshes;

    uint32_t vertex_count;
    uint32_t index_count;
    uint32_t mesh_count;
};
struct Mesh {
    struct VertexAttributes * vertices;
    uint32_t vertex_count;
    uint32_t * indices;
    uint32_t index_count;
};



// File reader callback for tinyobj with Emscripten
void emscripten_file_reader(void *ctx, const char *filename, int is_mtl, const char *obj_filename, char **buf, size_t *len) {
    (void)ctx; // unused
    (void)is_mtl; // unused
    (void)obj_filename; // unused
    FILE *file = fopen(filename, "rb");
    if (!file) {
        printf("Failed to open file: %s\n", filename);
        *buf = NULL;
        *len = 0;
        return;
    }
    // Get file size
    fseek(file, 0, SEEK_END);
    *len = (size_t)ftell(file);
    fseek(file, 0, SEEK_SET);
    // Allocate buffer and read file
    printf("Allocating %zu bytes for file: %s\n", *len, filename);

    *buf = (char*)malloc(*len + 1);
    if (*buf) {
        size_t read_size = fread(*buf, 1, *len, file);
        (*buf)[read_size] = '\0'; // Null terminate
        *len = read_size;
    } else {
        *len = 0;
    }
    fclose(file);
}

struct Mesh loadGeometryFromObj(char const * filename) {
    struct Mesh mesh = {0};
    VertexHash hash = {0};
    vertex_hash_init(&hash);

    tinyobj_attrib_t attrib;
    tinyobj_shape_t * shapes = NULL;
    size_t num_shapes = 0;
    tinyobj_material_t * materials = NULL;
    size_t num_materials = 0;
    int ret = tinyobj_parse_obj(&attrib, &shapes, &num_shapes, &materials, &num_materials, filename, emscripten_file_reader, NULL, TINYOBJ_FLAG_TRIANGULATE);
    if (ret != TINYOBJ_SUCCESS) {
        printf("Failed to load OBJ file: %s\n", filename);
        return mesh;
    }

    // print info
    printf("Loaded OBJ file: %s\n", filename);
    printf("  Vertices: %u\n", attrib.num_vertices);
    printf("  Normals: %u\n", attrib.num_normals);
    printf("  Texcoords: %u\n", attrib.num_texcoords);
    printf("  Faces: %u\n", attrib.num_faces / 3); // attrib.num_faces is actually number of indices where each face has 3 indices after triangulation

    // assume one shape
    if (num_shapes != 1) {
        printf("Warning: Expected one shape in OBJ file, found %zu\n", num_shapes);
    }

    uint32_t mesh_vertex_capacity = 64;
    mesh.vertices = (struct VertexAttributes*)malloc(sizeof(struct VertexAttributes) * mesh_vertex_capacity);
    
    mesh.vertex_count = 0;

    mesh.indices = (uint32_t*)malloc(sizeof(uint32_t) * attrib.num_faces);
    mesh.index_count = attrib.num_faces;

    // copy data to vertex buffer
    for (size_t i = 0; i < attrib.num_faces; i++) {
        tinyobj_vertex_index_t idx = attrib.faces[i];
        struct VertexAttributes vertex = {0};
        if (idx.v_idx != (int)TINYOBJ_INVALID_INDEX) {
            vertex.position[0] = attrib.vertices[idx.v_idx * 3 + 0];
            vertex.position[1] = attrib.vertices[idx.v_idx * 3 + 1];
            vertex.position[2] = attrib.vertices[idx.v_idx * 3 + 2];
        }
        if (idx.vt_idx != (int)TINYOBJ_INVALID_INDEX) {
            vertex.texcoord[0] = attrib.texcoords[idx.vt_idx * 2 + 0];
            vertex.texcoord[1] = attrib.texcoords[idx.vt_idx * 2 + 1];
        }
        if (idx.vn_idx != (int)TINYOBJ_INVALID_INDEX) {
            vertex.normal[0] = attrib.normals[idx.vn_idx * 3 + 0];
            vertex.normal[1] = attrib.normals[idx.vn_idx * 3 + 1];
            vertex.normal[2] = attrib.normals[idx.vn_idx * 3 + 2];
        }
        VertexKey key = {idx.v_idx, idx.vt_idx, idx.vn_idx};
        uint32_t index = vertex_hash_get_or_add(&hash, key, mesh.vertices, &mesh.vertex_count);
        if (index == mesh.vertex_count - 1) {
            // new vertex added
            if (mesh.vertex_count >= mesh_vertex_capacity) {
                mesh_vertex_capacity *= 2;
                mesh.vertices = (struct VertexAttributes*)realloc(mesh.vertices, sizeof(struct VertexAttributes) * mesh_vertex_capacity);
            }
            if (index >= mesh_vertex_capacity) {
                printf("Error: Vertex index %u exceeds capacity %u\n", index, mesh_vertex_capacity);
            }
            mesh.vertices[index] = vertex;
        }
        mesh.indices[i] = (uint32_t)index;
    }
    vertex_hash_free(&hash);



    tinyobj_attrib_free(&attrib);
    tinyobj_shapes_free(shapes, num_shapes);
    tinyobj_materials_free(materials, num_materials);
    return mesh;
}
void add_mesh_to_batch(struct MeshBatch * batch, struct Mesh * mesh) {
    
    batch->vertex_data = realloc(batch->vertex_data, sizeof(struct VertexAttributes) * (batch->vertex_count + mesh->vertex_count));
    if (!batch->vertex_data) {
        printf("Error: Failed to allocate memory for vertex data in batch\n");
        return;
    }
    memcpy(&batch->vertex_data[batch->vertex_count], mesh->vertices, sizeof(struct VertexAttributes) * mesh->vertex_count);
    batch->index_data = realloc(batch->index_data, sizeof(uint32_t) * (batch->index_count + mesh->index_count));
    if (!batch->index_data) {
        printf("Error: Failed to allocate memory for index data in batch\n");
        return;
    }
    for (uint32_t i = 0; i < mesh->index_count; i++) {
        batch->index_data[batch->index_count + i] = mesh->indices[i] + batch->vertex_count;
    }
    
    struct MeshRange range = {
        .index_offset = batch->index_count,
        .index_count = mesh->index_count,
        .vertex_start = batch->vertex_count
    };

    batch->meshes = realloc(batch->meshes, sizeof(struct MeshRange) * (batch->mesh_count + 1));
    if (!batch->meshes) {
        printf("Error: Failed to allocate memory for mesh ranges in batch\n");
        return;
    }
    batch->meshes[batch->mesh_count] = range;
    
    batch->vertex_count += mesh->vertex_count;
    batch->index_count += mesh->index_count;
    batch->mesh_count += 1;

    printf("Added mesh to batch: vertices=%u, indices=%u\n", mesh->vertex_count, mesh->index_count);
}
int main(void) {

    struct MeshBatch batch = {0};
    // struct Mesh mesh = loadGeometryFromObj("demo3/dragon.obj");
    // printf("Mesh loaded:\n");
    // printf("  Vertex count: %u\n", mesh.vertex_count);
    // printf("  Index count: %u\n", mesh.index_count);
    // add_mesh_to_batch(&batch, &mesh);

    // struct Mesh mesh2 = loadGeometryFromObj("demo3/cube.obj");
    // printf("Mesh2 loaded:\n");
    // printf("  Vertex count: %u\n", mesh2.vertex_count);
    // printf("  Index count: %u\n", mesh2.index_count);
    // add_mesh_to_batch(&batch, &mesh2);

    struct Mesh mesh3 = loadGeometryFromObj("demo3/fourareen/fourareen.obj");
    printf("Mesh3 loaded:\n");
    printf("  Vertex count: %u\n", mesh3.vertex_count);
    printf("  Index count: %u\n", mesh3.index_count);
    add_mesh_to_batch(&batch, &mesh3);

    // for (uint32_t i = 0; i < mesh.index_count; i+=3) {
    //     printf("Face %u:\n", i / 3);
    //     printf("  Indices: %u, %u, %u\n", mesh.indices[i + 0], mesh.indices[i + 1], mesh.indices[i + 2]);
    // }

    printf("Batch info:\n");
    printf("  Total vertices: %u\n", batch.vertex_count);
    printf("  Total indices: %u\n", batch.index_count);
    printf("  Mesh count: %u\n", batch.mesh_count);
    
    return 0;
}

EMSCRIPTEN_KEEPALIVE
void set_parameter(const char* param_name, float value) {
    (void)param_name;
    (void)value;
}