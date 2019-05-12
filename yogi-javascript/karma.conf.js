var webpackConfig = require('./webpack.config.js');
webpackConfig.devtool = 'inline-source-map';

module.exports = function(config) {
  config.set({
    basePath: '',
    frameworks: ['jasmine', 'detectBrowsers'],
    files: webpackConfig.entry.yogi.concat(['test/*.spec.js']),
    exclude: [],
    preprocessors: {
      'src/*.js': ['webpack', 'sourcemap'],
      'test/*.js': ['webpack', 'sourcemap']
    },
    webpack: webpackConfig,
    reporters: ['mocha'],
    port: 9876,
    colors: true,
    logLevel: config.LOG_INFO,
    autoWatch: true,
    singleRun: false,
    concurrency: Infinity,
    detectBrowsers: {
      usePhantomJS: false,
      preferHeadless: true,
      postDetection: function(availableBrowsers) {
        return availableBrowsers
          .filter(function(browser) {
            return browser != 'IE';
          })
          .map(function(browser) {
            return browser == 'Safari' ? 'SafariPrivate' : browser;
          });
      }
    },
    plugins: [
      'karma-webpack',
      'karma-sourcemap-loader',
      'karma-jasmine',
      'karma-mocha-reporter',
      'sourcemap',
      'karma-chrome-launcher',
      'karma-firefox-launcher',
      'karma-opera-launcher',
      'karma-safari-private-launcher',
      'karma-safaritechpreview-launcher',
      'karma-edge-launcher',
      'karma-detect-browsers'
    ]
  });
};
