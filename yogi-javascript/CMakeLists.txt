cmake_minimum_required(VERSION 2.8)
project(YogiJavascript NONE)

cmake_policy(SET CMP0022 OLD)
if (${CMAKE_MAJOR_VERSION} GREATER 2)
    cmake_policy(SET CMP0038 NEW)
endif (${CMAKE_MAJOR_VERSION} GREATER 2)

#===== download node modules =====
set(npm_ok_file
    node_modules_installed_successfully
)

add_custom_command(
    OUTPUT ${npm_ok_file}
    COMMAND npm install
    COMMAND echo > build/${npm_ok_file}
    WORKING_DIRECTORY ".."
)

#===== build Javascript bundles =====
file(GLOB_RECURSE src_files src/*.js)
file(GLOB_RECURSE spec_files src/*.spec.js)
list(REMOVE_ITEM src_files ${spec_files})

set(bundle_files
    yogi-hub-all.min.js
    yogi-hub-all.min.js.map
    yogi-hub.min.js
    yogi-hub.min.js.map
)

add_custom_command(
    OUTPUT ${bundle_files}
    COMMAND node node_modules/gulp/bin/gulp.js
    DEPENDS gulpfile.js ${src_files} ${npm_ok_file}
    WORKING_DIRECTORY ".."
)

add_custom_target(bundles
    ALL
    DEPENDS ${bundle_files}
)

#===== install bundles =====
set(yogi_hub_base /usr/share/yogi-hub/www)
file(GLOB files build/*.min.js build/*.min.js.map)

install(
    FILES ${files}
    DESTINATION ${yogi_hub_base}/lib
)

install(
    FILES index.html
    DESTINATION ${yogi_hub_base}
)
