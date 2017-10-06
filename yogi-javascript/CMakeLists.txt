cmake_minimum_required (VERSION 2.8)
project (YogiJavascript NONE)

#===== download node modules =====
set (npm_ok_file
    node_modules_installed_successfully
)

add_custom_command (
    OUTPUT ${npm_ok_file}
    COMMAND npm install
    COMMAND echo > ${PROJECT_BINARY_DIR}/${npm_ok_file}
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
)

#===== build Javascript bundles =====
file (GLOB_RECURSE src_files src/*.js)
file (GLOB_RECURSE spec_files src/*.spec.js)
list (REMOVE_ITEM src_files ${spec_files})

set (bundle_files
    yogi-hub-all.min.js
    yogi-hub-all.min.js.map
    yogi-hub.min.js
    yogi-hub.min.js.map
)

add_custom_command (
    OUTPUT ${bundle_files}
    COMMAND node node_modules/gulp/bin/gulp.js
    DEPENDS gulpfile.js ${src_files} ${npm_ok_file}
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
)

add_custom_target (bundles
    ALL
    DEPENDS ${bundle_files}
)

#===== install bundles =====
set (yogi_hub_base /usr/share/yogi-hub/www)

foreach (file ${bundle_files})
    install (
        FILES ${PROJECT_BINARY_DIR}/${file}
        DESTINATION ${yogi_hub_base}/lib/
    )
endforeach ()

install (
    FILES index.html
    DESTINATION ${yogi_hub_base}
)
