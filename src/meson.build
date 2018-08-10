api_version = '0.1'

fluid_sources = [
  'fld-window.c'
]

fluid_headers = [
  'fluid.h',
]

version_split = meson.project_version().split('.')
MAJOR_VERSION = version_split[0]
MINOR_VERSION = version_split[1]
MICRO_VERSION = version_split[2]

version_conf = configuration_data()
version_conf.set('VERSION', meson.project_version())
version_conf.set('MAJOR_VERSION', MAJOR_VERSION)
version_conf.set('MINOR_VERSION', MINOR_VERSION)
version_conf.set('MICRO_VERSION', MICRO_VERSION)

configure_file(
  input: 'fluid-version.h.in',
  output: 'fluid-version.h',
  configuration: version_conf,
  install: true,
  install_dir: join_paths(get_option('includedir'), 'fluid')
)

fluid_deps = [
  dependency('gio-2.0', version: '>= 2.50'),
  dependency('gtk+-3.0'),
]

fluid_lib = shared_library('fluid-' + api_version,
  fluid_sources,
  dependencies: fluid_deps,
  install: true,
)

install_headers(fluid_headers, subdir: 'fluid')

pkg = import('pkgconfig')

pkg.generate(
  description: 'Tools for adaptive applications',
    libraries: fluid_lib,
         name: 'fluid',
     filebase: 'fluid-' + api_version,
      version: meson.project_version(),
      subdirs: 'fluid',
     requires: 'gtk+-3.0',
  install_dir: join_paths(get_option('libdir'), 'pkgconfig')
)
