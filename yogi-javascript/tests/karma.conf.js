module.exports = function(config) {
  config.set({
    basePath: '',
    frameworks: ['jasmine'],
    exclude: [],
    preprocessors: {},
    reporters: ['progress'],
    port: 9876,
    colors: true,
    logLevel: config.LOG_INFO,
    autoWatch: false,
    // browsers: ['Chrome', 'Firefox'],
    browsers: ['Chrome'],
    // browsers: ['Firefox'],
    singleRun: true,
    concurrency: Infinity,
    failOnEmptyTestSuite: true,
    files: [
      { pattern: 'http://127.0.0.1:1234/lib/yogi-hub-all.min.js', included: false, served: false, nocache: true },
      '../src/*.spec.js'
    ]
  })
}
