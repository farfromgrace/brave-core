const config = require('../lib/config')
const util = require('../lib/util')

const gnCheck = (buildConfig = config.defaultBuildConfig, options) => {
  config.buildConfig = buildConfig
  config.update(options)
  util.run('gn', ['check', config.outputDir, '//osiris/*'], config.defaultOptions)
  util.run('python', ['buildtools/checkdeps/checkdeps.py', 'osiris', '--extra-repos=brave', '--no-resolve-dotdot'], config.defaultOptions)
}

module.exports = gnCheck
