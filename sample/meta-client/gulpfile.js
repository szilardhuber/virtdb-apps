var gulp = require('gulp');
var coffee = require('gulp-coffee');
var spawn = require('child_process').spawn;
var sourcemaps = require('gulp-sourcemaps');
var node;

/**
 * $ gulp server
 * description: launch the server. If there's a server already running, kill it.
 */
gulp.task('server', ['coffee'], function() {
  if (node) node.kill()
  node = spawn('node', ['out/metaDataClient.js'], {stdio: 'inherit'})
  node.on('close', function (code) {
    if (code === 8) {
      console.log('Error detected, waiting for changes...');
      gulp.start('server');
    }
    else if (code === 0)
    {
       process.exit(0);
    }
  });
  node.on('error', function () {
    console.log('Error detected, waiting for changes...');
    gulp.start('server');
  });
})

gulp.task('coffee', function() {
    gulp.src('*.coffee')
        .pipe(sourcemaps.init())
        .pipe(coffee({bare: true}))
        .pipe(sourcemaps.write('.'))
        .pipe(gulp.dest('./out'))
});

gulp.task('watch', function()
{
    gulp.watch(['./*.coffee'], ['coffee']);
    gulp.watch(['out/metaDataClient.js'], function() {
        gulp.run('server')
    })
});

gulp.task('default', ['server', 'watch']);
