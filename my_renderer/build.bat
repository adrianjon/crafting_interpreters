@REM em++ main2.cpp ^
@REM     -std=c++17 -Wall ^
@REM     --use-port=emdawnwebgpu -sASYNCIFY --shell-file template.html ^
@REM     -sEXPORTED_RUNTIME_METHODS="['ccall']" ^
@REM     -sEXPORTED_FUNCTIONS="['_main','_set_canvas_size']" ^
@REM     -sALLOW_MEMORY_GROWTH=1 ^
@REM     -sMAXIMUM_MEMORY=512MB ^
@REM     -o index.html 

@REM regular build
@REM emcc demo/main.c code/src/gpu.c code/src/renderer.c code/src/utils.c code/src/shader_manager.c code/src/pipeline_manager.c code/src/buffer_manager.c code/src/bind_groups.c code/src/asset_manager.c ^
@REM     -std=c11 -Wall -Wextra ^
@REM     --use-port=emdawnwebgpu -sASYNCIFY --shell-file demo/template.html ^
@REM     -sEXPORTED_RUNTIME_METHODS="['ccall']" ^
@REM     -sEXPORTED_FUNCTIONS="['_main', '_set_canvas_size']" ^
@REM     -sALLOW_MEMORY_GROWTH=1 ^
@REM     -sMAXIMUM_MEMORY=512MB ^
@REM     -Icode/include ^
@REM     --preload-file demo/dragon.obj ^
@REM     --preload-file demo/shader.wgsl ^
@REM     -o demo/index.html

@REM software rasterizer build
@REM emcc demo2/main.c ^
@REM     -std=gnu11 -g -O3 -Wall -Wextra ^
@REM     -sASYNCIFY --shell-file demo2/template.html ^
@REM     -sEXPORTED_RUNTIME_METHODS="['ccall','cwrap']" ^
@REM     -sEXPORTED_FUNCTIONS="['_main' ]" ^
@REM     -sALLOW_MEMORY_GROWTH=1 ^
@REM     -sMAXIMUM_MEMORY=512MB ^
@REM     -Icode/include ^
@REM     --preload-file demo2/dragon.obj ^
@REM     --preload-file demo2/cube.obj ^
@REM     -o demo2/index.html

@REM emcc demo3/main.c code/src/gpu.c code/src/utils.c ^
@REM     -std=gnu11 -g -O3 -Wall -Wextra ^
@REM     --use-port=emdawnwebgpu -sASYNCIFY --shell-file demo3/template.html ^
@REM     -sEXPORTED_RUNTIME_METHODS="['ccall','cwrap']" ^
@REM     -sEXPORTED_FUNCTIONS="['_main','_set_parameter']" ^
@REM     -sALLOW_MEMORY_GROWTH=1 ^
@REM     -sMAXIMUM_MEMORY=512MB ^
@REM     -Icode/include ^
@REM     --preload-file demo3/dragon.obj ^
@REM     --preload-file demo3/shader.wgsl ^
@REM     --preload-file demo3/logo.obj ^
@REM     --preload-file demo3/cube.obj ^
@REM     --preload-file demo3/floor.obj ^
@REM     --preload-file demo3/xyz_axis.obj ^
@REM     --preload-file demo3/fourareen/fourareen.obj ^
@REM     --preload-file demo3/fourareen/fourareen2K_albedo.jpg ^
@REM     --preload-file demo3/test.obj ^
@REM     --preload-file demo3/43-obj/obj/Wolf_obj.obj ^
@REM     --preload-file demo3/43-obj/obj/Wolf_One_obj.obj ^
@REM     --preload-file demo3/43-obj/obj/textures/Wolf_Body.jpg ^
@REM     -o demo3/index.html

emcc demo4/main.c code/src/gpu.c code/src/utils.c ^
    -std=gnu11 -g -O3 -Wall -Wextra ^
    --use-port=emdawnwebgpu -sASYNCIFY --shell-file demo4/template.html ^
    -sEXPORTED_RUNTIME_METHODS="['ccall','cwrap']" ^
    -sEXPORTED_FUNCTIONS="['_main','_set_parameter']" ^
    -sALLOW_MEMORY_GROWTH=1 ^
    -sMAXIMUM_MEMORY=512MB ^
    -Icode/include ^
    --preload-file demo4/dragon.obj ^
    -o demo4/index.html