const path = require('path');
const glob = require('glob');

const namespaceFile = './src/namespace.js';

module.exports = {
  mode: 'production',
  entry: {
    // namespaceFile goes first in list
    yogi: [namespaceFile].concat(
      glob.sync('./src/*.js').filter(x => x !== namespaceFile)
    )
  },
  output: {
    filename: '[name].js',
    path: path.resolve(__dirname, 'dist')
  },
  devtool: 'source-map'
};
