const path = require('path');
const glob = require('glob');
const UglifyJsPlugin = require('uglifyjs-webpack-plugin');

const namespaceFile = './src/namespace.js';

module.exports = {
  entry: {
    yogi: [namespaceFile].concat(glob.sync('./src/*.js').filter(x => x !== namespaceFile))
  },
  output: {
    filename: '[name].js'
  },
  module: {
    rules: []
  },
  plugins: [
    new UglifyJsPlugin({
      sourceMap: true
    })
  ],
  devtool: 'source-map'
};
