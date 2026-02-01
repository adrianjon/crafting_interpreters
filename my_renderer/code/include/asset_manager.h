#ifndef MY_RENDERER_ASSET_MANAGER_H
#define MY_RENDERER_ASSET_MANAGER_H

typedef struct asset_manager * asset_manager_t;
typedef struct asset asset_t;
struct asset_manager {
    void (*load_mesh)(char const * filename);
    void (*destroy)(void);
    void const * (*get_asset_data)(char const * asset_name);
    void (*append_asset)(struct asset * asset);
    void (*free_assets)(void);
};

asset_manager_t new_asset_manager(void);

#endif // MY_RENDERER_ASSET_MANAGER_H