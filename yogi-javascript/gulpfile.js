var gulp       = require('gulp');
var concat     = require('gulp-concat');
var sourcemaps = require('gulp-sourcemaps');
var babel      = require('gulp-babel');
var uglify     = require('gulp-uglify');
var filter     = require('gulp-filter');
var pump       = require('pump');

gulp.task('make-hub-bundle', (cb) => {
    pump([
        gulp.src(['src/*.js', '!src/*.spec.js']),
        sourcemaps.init(),
        concat('yogi-hub.min.js'),
        babel({presets: ['es2015']}),
        uglify({mangle: false}),
        sourcemaps.write('.'),
        gulp.dest('build')
        ], cb);
});

gulp.task('make-all-bundle', (cb) => {
    const srcFilter = filter('src/*.js', {restore: true});

    pump([
        gulp.src([
            'node_modules/long/dist/long.js',
            'node_modules/bytebuffer/dist/bytebuffer.js',
            'node_modules/protobufjs/dist/protobuf.js',
            'src/*.js', '!src/*.spec.js'
        ]),
        sourcemaps.init(),
        srcFilter,
        babel({presets: ['es2015']}),
        srcFilter.restore,
        concat('yogi-hub-all.min.js'),
        uglify({mangle: false}),
        sourcemaps.write('.'),
        gulp.dest('build')
        ], cb);
});

gulp.task('default', [
   'make-hub-bundle',
   'make-all-bundle'
]);
